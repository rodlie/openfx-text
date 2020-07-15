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

#include "ofxsImageEffect.h"
#include "RichText.h"
#include <iostream>

#define kSupportsTiles 0
#define kSupportsMultiResolution 0
#define kSupportsRenderScale 1
#define kRenderThreadSafety eRenderFullySafe

#define kParamText "text"
#define kParamTextLabel "Text"
#define kParamTextHint "The text that will be drawn."

#define kParamFontSize "size"
#define kParamFontSizeLabel "Text size"
#define kParamFontSizeHint "The height of the characters to render in pixels."
#define kParamFontSizeDefault 64

#define kParamFontName "name"
#define kParamFontNameLabel "Select Font"
#define kParamFontNameHint "The name of the font to be used."
#define kParamFontNameDefault "Sans"

#define kParamFont "font"
#define kParamFontLabel "Font"
#define kParamFontHint "Selected font."

#define kParamStyle "style"
#define kParamStyleLabel "Style"
#define kParamStyleHint "Font style."
#define kParamStyleDefault 0

#define kParamTextColor "color"
#define kParamTextColorLabel "Text color"
#define kParamTextColorHint "The fill color of the text to render."

#define kParamBGColor "backgroundColor"
#define kParamBGColorLabel "Background Color"
#define kParamBGColorHint "The fill color of the background."

#define kParamJustify "justify"
#define kParamJustifyLabel "Justify"
#define kParamJustifyHint "Text justify."
#define kParamJustifyDefault false

#define kParamWrap "wrap"
#define kParamWrapLabel "Wrap"
#define kParamWrapHint "Word wrap."
#define kParamWrapDefault RichText::RichTextWrapWord

#define kParamAlign "align"
#define kParamAlignLabel "Horizontal align"
#define kParamAlignHint "Horizontal text align."
#define kParamAlignDefault RichText::RichTextAlignLeft

#define kParamVAlign "valign"
#define kParamVAlignLabel "Vertical align"
#define kParamVAlignHint "Vertical text align."
#define kParamVAlignDefault RichText::RichTextAlignTop

#define kParamStretch "stretch"
#define kParamStretchLabel "Stretch"
#define kParamStretchHint "Width of the font relative to other designs within a family."
#define kParamStretchDefault RichText::RichTextFontStretchNormal

#define kParamWeight "weight"
#define kParamWeightLabel "Weight"
#define kParamWeightHint "The weight field specifies how bold or light the font should be."
#define kParamWeightDefault RichText::RichTextFontWeightNormal

#define kParamStrokeColor "strokeColor"
#define kParamStrokeColorLabel "Stroke color"
#define kParamStrokeColorHint "The fill color of the stroke to render."

#define kParamStrokeWidth "strokeSize"
#define kParamStrokeWidthLabel "Stroke size"
#define kParamStrokeWidthHint "Stroke size."
#define kParamStrokeWidthDefault 0.0

#define kParamHintStyle "hintStyle"
#define kParamHintStyleLabel "Hint style"
#define kParamHintStyleHint "This controls whether to fit font outlines to the pixel grid, and if so, whether to optimize for fidelity or contrast."
#define kParamHintStyleDefault 0

#define kParamHintMetrics "hintMetrics"
#define kParamHintMetricsLabel "Hint metrics"
#define kParamHintMetricsHint "This controls whether metrics are quantized to integer values in device units."
#define kParamHintMetricsDefault RichText::RichTextHintDefault

#define kParamLetterSpace "letterSpace"
#define kParamLetterSpaceLabel "Letter spacing"
#define kParamLetterSpaceHint "Spacing between letters."
#define kParamLetterSpaceDefault 0

using namespace OFX;

// workaround for stupid Fusion!
static std::vector<std::string> _fonts;
//

class TextOFXPlugin : public OFX::ImageEffect
{
public:
    TextOFXPlugin(OfxImageEffectHandle handle);
    virtual void render(const RenderArguments &args) override final;
    virtual void changedParam(const InstanceChangedArgs &args,
                              const std::string &paramName) override final;

private:
    OFX::Clip *_dstClip;
    OFX::StringParam *_text;
    OFX::IntParam *_fontSize;
    OFX::RGBAParam *_textColor;
    OFX::RGBAParam *_bgColor;
    OFX::StringParam *_font;
    OFX::BooleanParam *_justify;
    OFX::ChoiceParam *_wrap;
    OFX::ChoiceParam *_align;
    OFX::ChoiceParam *_valign;
    OFX::ChoiceParam *_style;
    OFX::ChoiceParam *_stretch;
    OFX::ChoiceParam *_weight;
    OFX::RGBAParam *_strokeColor;
    OFX::DoubleParam *_strokeWidth;
    OFX::ChoiceParam *_hintStyle;
    OFX::ChoiceParam *_hintMetrics;
    OFX::IntParam *_letterSpace;
    OFX::ChoiceParam *_fontName;
    FcConfig* _fcConfig;
};

TextOFXPlugin::TextOFXPlugin(OfxImageEffectHandle handle)
: OFX::ImageEffect(handle)
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
, _fontName(nullptr)
, _fcConfig(nullptr)
{
    _dstClip = fetchClip(kOfxImageEffectOutputClipName);
    assert(_dstClip && _dstClip->getPixelComponents() == OFX::ePixelComponentRGBA);

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

    assert(_text && _fontSize && _fontName && _textColor && _bgColor && _font && _wrap
           && _justify && _align && _valign && _style && _stretch && _weight && _strokeColor
           && _strokeWidth && _hintStyle && _hintMetrics  && _letterSpace);

    _fcConfig = FcInitLoadConfigAndFonts();
}

void TextOFXPlugin::render(const OFX::RenderArguments &args)
{
    // renderscale
    if (!kSupportsRenderScale && (args.renderScale.x != 1. || args.renderScale.y != 1.)) {
        OFX::throwSuiteStatusException(kOfxStatFailed);
        return;
    }

    // dstclip
    if (!_dstClip) {
        OFX::throwSuiteStatusException(kOfxStatFailed);
        return;
    }
    assert(_dstClip);

    // get dstclip
    OFX::auto_ptr<OFX::Image> dstImg(_dstClip->fetchImage(args.time));
    if (!dstImg.get()) {
        OFX::throwSuiteStatusException(kOfxStatFailed);
        return;
    }

    // renderscale
    checkBadRenderScaleOrField(dstImg, args);

    // get bitdepth
    OFX::BitDepthEnum dstBitDepth = dstImg->getPixelDepth();
    if (dstBitDepth != OFX::eBitDepthFloat) {
        OFX::throwSuiteStatusException(kOfxStatErrFormat);
        return;
    }

    // get channels
    OFX::PixelComponentEnum dstComponents  = dstImg->getPixelComponents();
    if (dstComponents != OFX::ePixelComponentRGBA) {
        OFX::throwSuiteStatusException(kOfxStatErrFormat);
        return;
    }

    // are we in the image bounds
    OfxRectI dstBounds = dstImg->getBounds();
    OfxRectI dstRod = dstImg->getRegionOfDefinition();
    if(args.renderWindow.x1 < dstBounds.x1 || args.renderWindow.x1 >= dstBounds.x2 || args.renderWindow.y1 < dstBounds.y1 || args.renderWindow.y1 >= dstBounds.y2 ||
       args.renderWindow.x2 <= dstBounds.x1 || args.renderWindow.x2 > dstBounds.x2 || args.renderWindow.y2 <= dstBounds.y1 || args.renderWindow.y2 > dstBounds.y2) {
        OFX::throwSuiteStatusException(kOfxStatErrValue);
        return;
    }

    // get params
    int style = 0;
    int fontSize = 0;
    std::string text;
    std::string font;
    RichText::RichTextStyle textStyle;
    textStyle.aa = RichText::RichTextFontAntialiasGray;
    textStyle.subpixel = RichText::RichTextFontSubpixelDefault;
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

    // workaround for stupid Fusion!
    /*int fontID = 0;
    _fontName->getValueAtTime(args.time, fontID);
    if (_fonts.size()>=fontID) {
        font = _fonts.at(fontID);
    }*/
    //

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

    int width = dstRod.x2-dstRod.x1;
    int height = dstRod.y2-dstRod.y1;

    // render image
    RichText::RichTextRenderResult result = RichText::renderText(width,
                                                                 height,
                                                                 _fcConfig,
                                                                 text,
                                                                 pangoFont.str(),
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
                                                                 true /* flip */);
    if (!result.success || (result.sW != width || result.sH != height)) {
        setPersistentMessage(Message::eMessageError, "", "Text Renderer failed");
        throwSuiteStatusException(kOfxStatErrFormat);
        return;
    }

    // write output
    float* pixelData = (float*)dstImg->getPixelData();
    int offset = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            pixelData[offset + 0] = result.buffer[offset + 0] * (1.f / 255);
            pixelData[offset + 1] = result.buffer[offset + 1] * (1.f / 255);
            pixelData[offset + 2] = result.buffer[offset + 2] * (1.f / 255);
            pixelData[offset + 3] = result.buffer[offset + 3] * (1.f / 255);
            offset += 4;
        }
    }
    delete [] result.buffer;
    result.buffer = nullptr;
    pixelData = nullptr;
}

void TextOFXPlugin::changedParam(const OFX::InstanceChangedArgs &args,
                                 const std::string &paramName)
{
    if (!kSupportsRenderScale && (args.renderScale.x != 1. || args.renderScale.y != 1.)) {
        OFX::throwSuiteStatusException(kOfxStatFailed);
        return;
    }

    clearPersistentMessage();

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
        if (selected<=_fonts.size()) {
            _font->setValue(_fonts.at(selected));
        }
    }
}

mDeclarePluginFactory(TextOFXPluginFactory, {}, {});

void TextOFXPluginFactory::describe(OFX::ImageEffectDescriptor &desc)
{
    // basic labels
    desc.setLabel(kPluginName);
    desc.setPluginGrouping(kPluginGrouping);
    desc.setPluginDescription(kPluginDescription);

    // add the supported contexts
    desc.addSupportedContext(eContextGenerator);

    // add supported pixel depths
    desc.addSupportedBitDepth(eBitDepthFloat);

    // add other
    desc.setSupportsTiles(kSupportsTiles);
    desc.setSupportsMultiResolution(kSupportsMultiResolution);
    desc.setRenderThreadSafety(kRenderThreadSafety);
}

void TextOFXPluginFactory::describeInContext(OFX::ImageEffectDescriptor &desc,
                                             ContextEnum /*context*/)
{
    // there has to be an input clip, even for generators
    ClipDescriptor* srcClip = desc.defineClip(kOfxImageEffectSimpleSourceClipName);
    srcClip->setOptional(true);

    // create the mandated output clip
    ClipDescriptor *dstClip = desc.defineClip(kOfxImageEffectOutputClipName);
    dstClip->addSupportedComponent(ePixelComponentRGBA);
    dstClip->setSupportsTiles(kSupportsTiles);

    // make some pages
    PageParamDescriptor *page = desc.definePageParam("Controls");
    {
        ChoiceParamDescriptor *param = desc.defineChoiceParam(kParamFontName);
        param->setLabel(kParamFontNameLabel);
        param->setHint(kParamFontNameHint);

        std::vector<std::string> fonts = RichText::getFontFamilyList(nullptr);
        // workaround for stupid Fusion!
        _fonts = fonts;
        //
        for(int i=0;i<fonts.size();++i) { param->appendOption(fonts.at(i)); }
        param->setAnimates(false);
        if (fonts.empty()) {
            param->appendOption("N/A");
        }
        if (page) {
            page->addChild(*param);
        }
    }
    {
        StringParamDescriptor* param = desc.defineStringParam(kParamFont);
        param->setLabel(kParamFontLabel);
        param->setHint(kParamFontHint);
        param->setStringType(eStringTypeSingleLine);
        param->setAnimates(false);

        if (page) {
            page->addChild(*param);
        }
    }
    {
        IntParamDescriptor* param = desc.defineIntParam(kParamFontSize);
        param->setLabel(kParamFontSizeLabel);
        param->setHint(kParamFontSizeHint);
        param->setRange(1, 10000);
        param->setDisplayRange(1, 500);
        param->setDefault(kParamFontSizeDefault);
        param->setAnimates(true);
        if (page) {
            page->addChild(*param);
        }
    }
    {
        DoubleParamDescriptor* param = desc.defineDoubleParam(kParamStrokeWidth);
        param->setLabel(kParamStrokeWidthLabel);
        param->setHint(kParamStrokeWidthHint);
        param->setRange(0, 500);
        param->setDisplayRange(0, 100);
        param->setDefault(kParamStrokeWidthDefault);
        param->setAnimates(true);
        if (page) {
            page->addChild(*param);
        }
    }
    {
        IntParamDescriptor* param = desc.defineIntParam(kParamLetterSpace);
        param->setLabel(kParamLetterSpaceLabel);
        param->setHint(kParamLetterSpaceHint);
        param->setRange(0, 10000);
        param->setDisplayRange(0, 500);
        param->setDefault(kParamLetterSpaceDefault);
        param->setAnimates(false);
        if (page) {
            page->addChild(*param);
        }
    }
    {
        StringParamDescriptor* param = desc.defineStringParam(kParamText);
        param->setLabel(kParamTextLabel);
        param->setHint(kParamTextHint);
        param->setStringType(eStringTypeMultiLine);
        param->setAnimates(true);
        param->setDefault("Enter text");
        if (page) {
            page->addChild(*param);
        }
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
        if (page) {
            page->addChild(*param);
        }
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
        if (page) {
            page->addChild(*param);
        }
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
        if (page) {
            page->addChild(*param);
        }
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
        if (page) {
            page->addChild(*param);
        }
    }
    {
        BooleanParamDescriptor *param = desc.defineBooleanParam(kParamJustify);
        param->setLabel(kParamJustifyLabel);
        param->setHint(kParamJustifyHint);
        param->setDefault(kParamJustifyDefault);
        param->setAnimates(false);
        if (page) {
            page->addChild(*param);
        }
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
        if (page) {
            page->addChild(*param);
        }
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
        if (page) {
            page->addChild(*param);
        }
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
        if (page) {
            page->addChild(*param);
        }
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
        if (page) {
            page->addChild(*param);
        }
    }
    {
        RGBAParamDescriptor* param = desc.defineRGBAParam(kParamTextColor);
        param->setLabel(kParamTextColorLabel);
        param->setHint(kParamTextColorHint);
        param->setDefault(1., 1., 1., 1.);
        param->setAnimates(true);
        if (page) {
            page->addChild(*param);
        }
    }
    {
        RGBAParamDescriptor* param = desc.defineRGBAParam(kParamStrokeColor);
        param->setLabel(kParamStrokeColorLabel);
        param->setHint(kParamStrokeColorHint);
        param->setDefault(1., 0., 0., 1.);
        param->setAnimates(true);
        if (page) {
            page->addChild(*param);
        }
    }
    {
        RGBAParamDescriptor* param = desc.defineRGBAParam(kParamBGColor);
        param->setLabel(kParamBGColorLabel);
        param->setHint(kParamBGColorHint);
        param->setDefault(0., 0., 0., 0.);
        param->setAnimates(true);
        if (page) {
            page->addChild(*param);
        }
    }
}

ImageEffect* TextOFXPluginFactory::createInstance(OfxImageEffectHandle handle,
                                                  ContextEnum /*context*/)
{
    return new TextOFXPlugin(handle);
}

static TextOFXPluginFactory p(kPluginIdentifier,
                              kPluginVersionMajor,
                              kPluginVersionMinor);
mRegisterPluginFactoryInstance(p)
