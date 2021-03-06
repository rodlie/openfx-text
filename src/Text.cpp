/*
########################################################################
#
# Copyright (C) 2019, 2020 Ole-André Rodlie <ole.andre.rodlie@gmail.com>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
########################################################################
*/

#include "Text.h"
#include <iostream>

TextOFXPlugin::TextOFXPlugin(OfxImageEffectHandle handle)
    : ImageEffect(handle)
    , _dstClip(nullptr)
    , _text(nullptr)
    , _fontSize(nullptr)
    , _textColor(nullptr)
    , _bgColor(nullptr)
    , _font(nullptr)
    , _justify(nullptr)
    , _wrap(nullptr)
    , _align(nullptr)
    , _valign(nullptr)
    , _style(nullptr)
    , _stretch(nullptr)
    , _weight(nullptr)
    , _strokeColor(nullptr)
    , _strokeWidth(nullptr)
    , _hintStyle(nullptr)
    , _hintMetrics(nullptr)
    , _letterSpace(nullptr)
    , _canvas(nullptr)
    , _fontName(nullptr)
    , _fcConfig(nullptr)
    , _markup(nullptr)
    , _range(nullptr)
    , _auto(nullptr)
    , _arcRadius(nullptr)
    , _arcAngle(nullptr)
{
    _dstClip = fetchClip(kOfxImageEffectOutputClipName);
    assert(_dstClip);
    if ( !CommonOFX::isNuke(ofxHostName) ) { // Nuke is always RGBA (check will fail)
        assert(_dstClip->getPixelComponents() == ePixelComponentRGBA);
    }

    _text = fetchStringParam(kParamText);
    _fontSize = fetchIntParam(kParamFontSize);
    _fontName = fetchChoiceParam(kParamFontName);
    _textColor = fetchRGBAParam(kParamTextColor);
    _bgColor = fetchRGBAParam(kParamBGColor);
    _font = fetchStringParam(kParamFont);
    _justify = fetchBooleanParam(kParamJustify);
    _wrap = fetchChoiceParam(kParamWrap);
    _align = fetchChoiceParam(kParamAlign);
    _valign = fetchChoiceParam(kParamVAlign);
    _style = fetchChoiceParam(kParamStyle);
    _stretch = fetchChoiceParam(kParamStretch);
    _weight = fetchChoiceParam(kParamWeight);
    _strokeColor = fetchRGBAParam(kParamStrokeColor);
    _strokeWidth = fetchDoubleParam(kParamStrokeWidth);
    _hintStyle = fetchChoiceParam(kParamHintStyle);
    _hintMetrics = fetchChoiceParam(kParamHintMetrics);
    _letterSpace = fetchIntParam(kParamLetterSpace);
    _canvas = fetchInt2DParam(kParamCanvas);
    _markup = fetchBooleanParam(kParamMarkup);
    _auto = fetchBooleanParam(kParamAutoSize);
    _arcRadius = fetchDoubleParam(kParamArcRadius);
    _arcAngle = fetchDoubleParam(kParamArcAngle);

    if (getContext() == eContextGeneral) {
        _range   = fetchInt2DParam(kParamGeneratorRange);
    }

    assert(_text && _fontSize && _fontName && _textColor && _bgColor && _font && _wrap
           && _justify && _align && _valign && _style && _stretch && _weight && _strokeColor
           && _strokeWidth && _hintStyle && _hintMetrics  && _letterSpace && _canvas && _markup
           && _auto && _arcRadius && _arcAngle);

    // setup fontconfig
    std::string fontConf = ofxPath;
    fontConf.append("/Contents/Resources/fonts");
    if ( CommonText::fileExists(fontConf + "/fonts.conf") ) {
#ifdef _WIN32
        _putenv_s( "FONTCONFIG_PATH", fontConf.c_str() );
#else
        setenv("FONTCONFIG_PATH", fontConf.c_str(), 1);
#endif
    }
    _fcConfig = FcInitLoadConfigAndFonts();
}

void TextOFXPlugin::render(const RenderArguments &args)
{
    // renderscale
    if ( !kSupportsRenderScale && (args.renderScale.x != 1. || args.renderScale.y != 1.) ) {
        throwSuiteStatusException(kOfxStatFailed);
        return;
    }

    // dstclip
    if (!_dstClip) {
        throwSuiteStatusException(kOfxStatFailed);
        return;
    }

    // get dstclip
    OFX::auto_ptr<Image> dstImg( _dstClip->fetchImage(args.time) );
    if ( !dstImg.get() ) {
        throwSuiteStatusException(kOfxStatFailed);
        return;
    }

    // check renderscale or field
    checkBadRenderScaleOrField(dstImg, args);

    // check bitdepth and channels
    BitDepthEnum dstBitDepth = dstImg->getPixelDepth();
    PixelComponentEnum dstComponents  = dstImg->getPixelComponents();
    if ( !CommonOFX::isNuke(ofxHostName) ) { // Nuke is always 32-BIT FLOAT and RGBA (check will fail)
        if ( (dstBitDepth != eBitDepthFloat && dstBitDepth != eBitDepthUByte) ||
             (dstComponents != ePixelComponentRGBA) ) {
            throwSuiteStatusException(kOfxStatErrFormat);
            return;
        }
    }

    // are we in the image bounds
    OfxRectI dstBounds = dstImg->getBounds();
    OfxRectI dstRod = dstImg->getRegionOfDefinition();
    if (args.renderWindow.x1 < dstBounds.x1 ||
        args.renderWindow.x1 >= dstBounds.x2 ||
        args.renderWindow.y1 < dstBounds.y1 ||
        args.renderWindow.y1 >= dstBounds.y2 ||
        args.renderWindow.x2 <= dstBounds.x1 ||
        args.renderWindow.x2 > dstBounds.x2 ||
        args.renderWindow.y2 <= dstBounds.y1 ||
        args.renderWindow.y2 > dstBounds.y2)
    {
        throwSuiteStatusException(kOfxStatErrValue);
        return;
    }

    // image size
    int width = dstRod.x2-dstRod.x1;
    int height = dstRod.y2-dstRod.y1;

    // render image
    CommonText::CommonTextRenderResult result = renderText(_fcConfig, args, width, height);
    if ( !result.success || (result.sW != width || result.sH != height) ) {
        setPersistentMessage(Message::eMessageError, "", "Text Renderer failed");
        throwSuiteStatusException(kOfxStatErrFormat);
        return;
    }

    // write output
    if (dstBitDepth == eBitDepthUByte) { // 8-bit int
        unsigned char* pixelData = (unsigned char*)dstImg->getPixelData();
        CommonOFX::writeOutput(width, height, result.buffer, pixelData);
        pixelData = nullptr;
    } else if (dstBitDepth == eBitDepthFloat || CommonOFX::isNuke(ofxHostName) ) { // 32-bit float
        // Note that we do a check for Nuke, since dstBithDepth from Nuke is wrong
        float* pixelData = (float*)dstImg->getPixelData();
        CommonOFX::writeOutput(width, height, result.buffer, pixelData);
        pixelData = nullptr;
    }

    // clear
    delete [] result.buffer;
    result.buffer = nullptr;
}

void TextOFXPlugin::changedParam(const InstanceChangedArgs &args,
                                 const std::string &paramName)
{
    clearPersistentMessage();

    if ( !kSupportsRenderScale && (args.renderScale.x != 1. || args.renderScale.y != 1.) ) {
        throwSuiteStatusException(kOfxStatFailed);
        return;
    }

    if (paramName == kParamFontName) {
        /*int selected;
        std::string label;
        _fontName->getValue(selected);
        std::cout << "LABEL " << label << " " << selected << std::endl;
        _fontName->getOption(selected, label);
        if (!label.empty()) {
            _font->setValue(label);
        }*/
        // workaround for fusion
        int selected = -1;
        _fontName->getValue(selected);
        if ( selected <= static_cast<int>( _fonts.size() ) ) {
            _font->setValue( _fonts.at(selected) );
        }
    }
}

bool TextOFXPlugin::getRegionOfDefinition(const RegionOfDefinitionArguments &args,
                                          OfxRectD &rod)
{
    if (!kSupportsRenderScale && (args.renderScale.x != 1. || args.renderScale.y != 1.)) {
        OFX::throwSuiteStatusException(kOfxStatFailed);
        return false;
    }

    bool autoSize = false;
    int width, height;
    _canvas->getValue(width, height);
    _auto->getValue(autoSize);
    /*if ( !CommonOFX::hostSupportsRoD(ofxHostName) ) { // host does not support custom size
        width = 0;
        height = 0;
    }*/

    if ( (width == 0 || height == 0) /*&&
         CommonOFX::hostSupportsRoD(ofxHostName)*/  && autoSize)
    { // no custom size, get RoD from layout
        width = rod.x2-rod.x1;
        height = rod.y2-rod.y1;
        CommonText::CommonTextRenderResult result = renderTextRoD(_fcConfig,
                                                                  args,
                                                                  width,
                                                                  height);
        if (result.success) {
            width = result.pW;
            height = result.pH;
        }
    }

    if (width > 0 && height > 0) {
        rod.x1 = rod.y1 = 0;
        rod.x2 = width;
        rod.y2 = height;
    }
    else { return false; }
    return true;
}

bool TextOFXPlugin::getTimeDomain(OfxRangeD &range)
{
#ifdef LOG_EVENTS
    CommonOFX::logToFile("get time domain");
#endif

    if (getContext() == eContextGeneral) {
        assert(_range);
        // how many frames on the input clip
        //OfxRangeD srcRange = _srcClip->getFrameRange();

        int min, max;
        _range->getValue(min, max);
        range.min = min;
        range.max = max;

        return true;
    }
    return false;
}

CommonText::CommonTextRenderResult TextOFXPlugin::renderText(FcConfig *fc,
                                                             const RenderArguments &args,
                                                             int width,
                                                             int height,
                                                             bool getRoD)
{
    CommonText::CommonTextRenderResult result;
    if (!fc) { return result; }

    // get params
    int style = 0;
    int fontSize = 0;
    std::string text;
    std::string font;
    CommonText::CommonTextStyle textStyle;
    textStyle.aa = CommonText::CommonTextFontAntialiasGray;
    textStyle.subpixel = CommonText::CommonTextFontSubpixelDefault;
    _arcRadius->getValueAtTime(args.time, textStyle.arcRadius);
    _arcAngle->getValueAtTime(args.time, textStyle.arcAngle);
    _auto->getValueAtTime(args.time, textStyle.autoSize);
    _markup->getValueAtTime(args.time, textStyle.markup);
    _justify->getValueAtTime(args.time, textStyle.justify);
    _hintMetrics->getValueAtTime(args.time, textStyle.metrics);
    _hintStyle->getValueAtTime(args.time, textStyle.hint);
    _weight->getValueAtTime(args.time, textStyle.weight);
    _stretch->getValueAtTime(args.time, textStyle.stretch);
    _align->getValueAtTime(args.time, textStyle.align);
    _wrap->getValueAtTime(args.time, textStyle.wrap);
    _text->getValueAtTime(args.time, text);
    _font->getValueAtTime(args.time, font);
    _fontSize->getValueAtTime(args.time, fontSize);
    _style->getValueAtTime(args.time, style);
    _strokeWidth->getValueAtTime(args.time, textStyle.strokeWidth);
    _valign->getValueAtTime(args.time, textStyle.valign);
    _letterSpace->getValueAtTime(args.time, textStyle.letterSpace);
    _textColor->getValueAtTime(args.time,
                               textStyle.textColor.r,
                               textStyle.textColor.g,
                               textStyle.textColor.b,
                               textStyle.textColor.a);
    _bgColor->getValueAtTime(args.time,
                             textStyle.backgroundColor.r,
                             textStyle.backgroundColor.g,
                             textStyle.backgroundColor.b,
                             textStyle.backgroundColor.a);
    _strokeColor->getValueAtTime(args.time,
                                 textStyle.strokeColor.r,
                                 textStyle.strokeColor.g,
                                 textStyle.strokeColor.b,
                                 textStyle.strokeColor.a);

    if ( font.empty() ) { font = kParamFontNameDefault; }
    std::ostringstream pangoFont;
    pangoFont << font;
    switch(style) {
    case 0:
        pangoFont << " " << "normal";
        break;
    case 1:
        pangoFont << " " << "bold";
        break;
    case 2:
        pangoFont << " " << "italic";
        break;
    }
    pangoFont << " " << std::floor(fontSize * args.renderScale.x + 0.5);
    textStyle.fontFamily = pangoFont.str();
    result = CommonText::renderText(width,
                                    height,
                                    fc,
                                    text,
                                    textStyle,
                                    0,
                                    0,
                                    1.0,
                                    1.0,
                                    0,
                                    0,
                                    args.renderScale.x,
                                    args.renderScale.y,
                                    0,
                                    true /* flip */,
                                    getRoD);
    if (!result.error.empty()) {
        setPersistentMessage(Message::eMessageError, "", result.error);
    }
    if (!result.warning.empty()) {
        setPersistentMessage(Message::eMessageWarning, "", result.warning);
    }
    return  result;
}

CommonText::CommonTextRenderResult TextOFXPlugin::renderTextRoD(FcConfig *fc,
                                                                const RegionOfDefinitionArguments &args,
                                                                int width,
                                                                int height)
{
    RenderArguments newargs;
    newargs.renderScale = args.renderScale;
    newargs.renderScale.x = 1.;
    newargs.renderScale.y = 1.;
    newargs.time = args.time;
    return renderText(fc, newargs, width, height, true);
}

void TextOFXPluginFactory::describe(ImageEffectDescriptor &desc)
{
    ofxHostName = getImageEffectHostDescription()->hostName;
    gHostIsNatron = (getImageEffectHostDescription()->isNatron);

    if (gHostIsNatron) { desc.setLabel(kPluginNameNatron); }
    else { desc.setLabel(kPluginName); }
    if ( CommonOFX::isBMD(ofxHostName) ) { desc.setPluginGrouping("Generator"); } // Creator on v9
    else { desc.setPluginGrouping(kPluginGrouping); }
    desc.setPluginDescription(kPluginDescription);
    desc.addSupportedContext(eContextGeneral);
    desc.addSupportedContext(eContextGenerator);
    desc.addSupportedBitDepth(eBitDepthUByte);
    desc.addSupportedBitDepth(eBitDepthFloat);
    desc.setSupportsTiles(0);
    desc.setSupportsMultiResolution(0);
    desc.setRenderThreadSafety(eRenderFullySafe);
}

void TextOFXPluginFactory::describeInContext(ImageEffectDescriptor &desc,
                                             ContextEnum context)
{
    ofxPath = desc.getPropertySet().propGetString(kOfxPluginPropFilePath, false);
    ofxHostName = getImageEffectHostDescription()->hostName;
    gHostIsNatron = (getImageEffectHostDescription()->isNatron);

    // there has to be an input clip, even for generators
    ClipDescriptor* srcClip = desc.defineClip(kOfxImageEffectSimpleSourceClipName);
    srcClip->setOptional(true);

    // create the mandated output clip
    ClipDescriptor *dstClip = desc.defineClip(kOfxImageEffectOutputClipName);
    dstClip->addSupportedComponent(ePixelComponentRGBA);
    dstClip->setSupportsTiles(0);

    // make some pages
    PageParamDescriptor *page = desc.definePageParam("Controls");
    {
        Int2DParamDescriptor* param = desc.defineInt2DParam(kParamCanvas);
        param->setLabel(kParamCanvasLabel);
        param->setHint(kParamCanvasHint);
        param->setRange(0, 0, 10000, 10000);
        param->setDisplayRange(0, 0, 4000, 4000);
        param->setDefault(kParamCanvasDefault, kParamCanvasDefault);
        param->setAnimates(false);
        //param->setIsSecret(!CommonOFX::hostSupportsRoD(ofxHostName));
        if (page) { page->addChild(*param); }
    }
    {
        BooleanParamDescriptor *param = desc.defineBooleanParam(kParamAutoSize);
        param->setLabel(kParamAutoSizeLabel);
        param->setHint(kParamAutoSizeHint);
        param->setAnimates(false);
        param->setDefault(kParamAutoSizeDefault);
        if (page) { page->addChild(*param); }
    }
    {
        BooleanParamDescriptor *param = desc.defineBooleanParam(kParamMarkup);
        param->setLabel(kParamMarkupLabel);
        param->setHint(kParamMarkupHint);
        param->setDefault(kParamMarkupDefault);
        param->setAnimates(false);
        if (page) { page->addChild(*param); }
    }
    {
        StringParamDescriptor* param = desc.defineStringParam(kParamText);
        param->setLabel(kParamTextLabel);
        param->setHint(kParamTextHint);
        param->setStringType(eStringTypeMultiLine);
        param->setAnimates(true);
        param->setDefault("Enter text");
        if (page) { page->addChild(*param); }
    }
    {
        ChoiceParamDescriptor *param = desc.defineChoiceParam(kParamFontName);
        param->setLabel(kParamFontNameLabel);
        param->setHint(kParamFontNameHint);

        std::string fontConf = ofxPath;
        fontConf.append("/Contents/Resources/fonts");
        std::vector<std::string> fonts = CommonText::getFontFamilyList(nullptr, "", false, fontConf);
        // workaround for stupid Fusion!
        _fonts = fonts;
        //
        for(unsigned int i = 0; i < _fonts.size(); ++i) { param->appendOption( fonts.at(i) ); }
        param->setAnimates(false);
        if ( fonts.empty() ) { param->appendOption("N/A"); }
        if (page) { page->addChild(*param); }
    }
    {
        StringParamDescriptor* param = desc.defineStringParam(kParamFont);
        param->setLabel(kParamFontLabel);
        param->setHint(kParamFontHint);
        param->setStringType(eStringTypeSingleLine);
        param->setAnimates(false);
        if (page) { page->addChild(*param); }
    }
    {
        IntParamDescriptor* param = desc.defineIntParam(kParamFontSize);
        param->setLabel(kParamFontSizeLabel);
        param->setHint(kParamFontSizeHint);
        param->setRange(1, 10000);
        param->setDisplayRange(1, 500);
        param->setDefault(kParamFontSizeDefault);
        param->setAnimates(true);
        if (page) { page->addChild(*param); }
    }
    {
        DoubleParamDescriptor* param = desc.defineDoubleParam(kParamStrokeWidth);
        param->setLabel(kParamStrokeWidthLabel);
        param->setHint(kParamStrokeWidthHint);
        param->setRange(0, 500);
        param->setDisplayRange(0, 100);
        param->setDefault(kParamStrokeWidthDefault);
        param->setAnimates(true);
        if (page) { page->addChild(*param); }
    }
    {
        IntParamDescriptor* param = desc.defineIntParam(kParamLetterSpace);
        param->setLabel(kParamLetterSpaceLabel);
        param->setHint(kParamLetterSpaceHint);
        param->setRange(0, 10000);
        param->setDisplayRange(0, 500);
        param->setDefault(kParamLetterSpaceDefault);
        param->setAnimates(false);
        if (page) { page->addChild(*param); }
    }
    {
        DoubleParamDescriptor* param = desc.defineDoubleParam(kParamArcRadius);
        param->setLabel(kParamArcRadiusLabel);
        param->setHint(kParamArcRadiusHint);
        param->setRange(0, 10000);
        param->setDisplayRange(0, 1000);
        param->setDefault(kParamArcRadiusDefault);
        param->setAnimates(true);
        if (page) { page->addChild(*param); }
    }
    {
        DoubleParamDescriptor* param = desc.defineDoubleParam(kParamArcAngle);
        param->setLabel(kParamArcAngleLabel);
        param->setHint(kParamArcAngleHint);
        param->setRange(0, 360);
        param->setDisplayRange(0, 360);
        param->setDefault(kParamArcAngleDefault);
        param->setAnimates(true);
        if (page) { page->addChild(*param); }
    }
    {
        ChoiceParamDescriptor *param = desc.defineChoiceParam(kParamStyle);
        param->setLabel(kParamStyleLabel);
        param->setHint(kParamStyleHint);
        param->appendOption("Normal");
        param->appendOption("Bold");
        param->appendOption("Italic");
        param->setDefault(kParamStyleDefault);
        param->setAnimates(false);
        if (page) { page->addChild(*param); }
    }
    {
        ChoiceParamDescriptor *param = desc.defineChoiceParam(kParamWrap);
        param->setLabel(kParamWrapLabel);
        param->setHint(kParamWrapHint);
        param->appendOption("None");
        param->appendOption("Word");
        param->appendOption("Char");
        param->appendOption("Word-Char");
        param->setDefault(kParamWrapDefault);
        param->setAnimates(false);
        if (page) { page->addChild(*param); }
    }
    {
        ChoiceParamDescriptor *param = desc.defineChoiceParam(kParamAlign);
        param->setLabel(kParamAlignLabel);
        param->setHint(kParamAlignHint);
        param->appendOption("Left");
        param->appendOption("Right");
        param->appendOption("Center");
        param->setDefault(kParamAlignDefault);
        param->setAnimates(false);
        if (page) { page->addChild(*param); }
    }
    {
        ChoiceParamDescriptor *param = desc.defineChoiceParam(kParamVAlign);
        param->setLabel(kParamVAlignLabel);
        param->setHint(kParamVAlignHint);
        param->appendOption("Top");
        param->appendOption("Center");
        param->appendOption("Bottom");
        param->setDefault(kParamVAlignDefault);
        param->setAnimates(false);
        if (page) { page->addChild(*param); }
    }
    {
        BooleanParamDescriptor *param = desc.defineBooleanParam(kParamJustify);
        param->setLabel(kParamJustifyLabel);
        param->setHint(kParamJustifyHint);
        param->setDefault(kParamJustifyDefault);
        param->setAnimates(false);
        if (page) { page->addChild(*param); }
    }
    {
        ChoiceParamDescriptor *param = desc.defineChoiceParam(kParamHintStyle);
        param->setLabel(kParamHintStyleLabel);
        param->setHint(kParamHintStyleHint);
        param->appendOption("Default");
        param->appendOption("None");
        param->appendOption("Slight");
        param->appendOption("Medium");
        param->appendOption("Full");
        param->setDefault(kParamHintStyleDefault);
        param->setAnimates(false);
        if (page) { page->addChild(*param); }
    }
    {
        ChoiceParamDescriptor *param = desc.defineChoiceParam(kParamHintMetrics);
        param->setLabel(kParamHintMetricsLabel);
        param->setHint(kParamHintMetricsHint);
        param->appendOption("Default");
        param->appendOption("Off");
        param->appendOption("On");
        param->setDefault(kParamHintMetricsDefault);
        param->setAnimates(false);
        if (page) { page->addChild(*param); }
    }
    {
        ChoiceParamDescriptor *param = desc.defineChoiceParam(kParamWeight);
        param->setLabel(kParamWeightLabel);
        param->setHint(kParamWeightHint);
        param->appendOption("Thin");
        param->appendOption("Ultra light");
        param->appendOption("Light");
        param->appendOption("Semi light");
        param->appendOption("Book");
        param->appendOption("Normal");
        param->appendOption("Medium");
        param->appendOption("Semi bold");
        param->appendOption("Bold");
        param->appendOption("Ultra bold");
        param->appendOption("Heavy");
        param->appendOption("Ultra heavy");
        param->setDefault(kParamWeightDefault);
        param->setAnimates(false);
        if (page) { page->addChild(*param); }
    }
    {
        ChoiceParamDescriptor *param = desc.defineChoiceParam(kParamStretch);
        param->setLabel(kParamStretchLabel);
        param->setHint(kParamStretchHint);
        param->appendOption("Ultra condensed");
        param->appendOption("Extra condensed");
        param->appendOption("Condensed");
        param->appendOption("Semi condensed");
        param->appendOption("Normal");
        param->appendOption("Semi expanded");
        param->appendOption("Expanded");
        param->appendOption("Extra expanded");
        param->appendOption("Ultra expanded");
        param->setDefault(kParamStretchDefault);
        param->setAnimates(false);
        if (page) { page->addChild(*param); }
    }
    {
        RGBAParamDescriptor* param = desc.defineRGBAParam(kParamTextColor);
        param->setLabel(kParamTextColorLabel);
        param->setHint(kParamTextColorHint);
        param->setDefault(1., 1., 1., 1.);
        param->setAnimates(true);
        if (page) { page->addChild(*param); }
    }
    {
        RGBAParamDescriptor* param = desc.defineRGBAParam(kParamStrokeColor);
        param->setLabel(kParamStrokeColorLabel);
        param->setHint(kParamStrokeColorHint);
        param->setDefault(1., 0., 0., 1.);
        param->setAnimates(true);
        if (page) { page->addChild(*param); }
    }
    {
        RGBAParamDescriptor* param = desc.defineRGBAParam(kParamBGColor);
        param->setLabel(kParamBGColorLabel);
        param->setHint(kParamBGColorHint);
        param->setDefault(0., 0., 0., 0.);
        param->setAnimates(true);
        if (page) { page->addChild(*param); }
    }
    // range
    if (context == eContextGeneral) {
        Int2DParamDescriptor *param = desc.defineInt2DParam(kParamGeneratorRange);
        param->setLabel(kParamGeneratorRangeLabel);
        param->setHint(kParamGeneratorRangeHint);
        param->setDefault(1, 1);
        param->setDimensionLabels("min", "max");
        //param->setIsSecret(!gHostIsNatron);
        param->setAnimates(false); // can not animate, because it defines the time domain
        if (page) { page->addChild(*param); }
    }
}

ImageEffect *TextOFXPluginFactory::createInstance(OfxImageEffectHandle handle,
                                                  ContextEnum /* context */)
{
    return new TextOFXPlugin(handle);
}
