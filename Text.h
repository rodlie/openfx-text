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

#ifndef TEXT_H
#define TEXT_H

#include "ofxsImageEffect.h"
#include "CommonText.h"
#include "CommonOFX.h"

#define kPluginName "TextOFX"
#define kPluginNameNatron "TextFX"
#define kPluginGrouping "Draw"
#define kPluginIdentifier "net.sf.openfx.Text"
#define kPluginDescription "Advanced internationalized OpenFX text generator. See https://github.com/rodlie/openfx-text for more information."
#define kSupportsRenderScale 1

#define kParamCanvas "canvas"
#define kParamCanvasLabel "Custom size"
#define kParamCanvasHint "Set custom canvas size, default (0) is project format."
#define kParamCanvasDefault 0

#define kParamMarkup "markup"
#define kParamMarkupLabel "Markup"
#define kParamMarkupHint "Pango Text Attribute Markup Language, https://developer.gnome.org/pango/stable/PangoMarkupFormat.html."
#define kParamMarkupDefault false

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
#define kParamWrapDefault CommonText::CommonTextWrapWord

#define kParamAlign "align"
#define kParamAlignLabel "Horizontal align"
#define kParamAlignHint "Horizontal text align."
#define kParamAlignDefault CommonText::CommonTextAlignLeft

#define kParamVAlign "valign"
#define kParamVAlignLabel "Vertical align"
#define kParamVAlignHint "Vertical text align."
#define kParamVAlignDefault CommonText::CommonTextAlignTop

#define kParamStretch "stretch"
#define kParamStretchLabel "Stretch"
#define kParamStretchHint "Width of the font relative to other designs within a family."
#define kParamStretchDefault CommonText::CommonTextFontStretchNormal

#define kParamWeight "weight"
#define kParamWeightLabel "Weight"
#define kParamWeightHint "The weight field specifies how bold or light the font should be."
#define kParamWeightDefault CommonText::CommonTextFontWeightNormal

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
#define kParamHintMetricsDefault CommonText::CommonTextHintDefault

#define kParamLetterSpace "letterSpace"
#define kParamLetterSpaceLabel "Letter spacing"
#define kParamLetterSpaceHint "Spacing between letters. Will not work if markup is enabled."
#define kParamLetterSpaceDefault 0

#define kParamGeneratorRange "frameRange"
#define kParamGeneratorRangeLabel "Frame Range"
#define kParamGeneratorRangeHint "Time domain."

using namespace OFX;

// workaround for stupid Fusion!
static std::vector<std::string> _fonts;
//
static std::string ofxPath;
static bool gHostIsNatron = false;
static std::string ofxHostName;

class TextOFXPlugin : public ImageEffect
{
public:
    TextOFXPlugin(OfxImageEffectHandle handle);
    virtual void render(const RenderArguments &args) override final;
    virtual void changedParam(const InstanceChangedArgs &args,
                              const std::string &paramName) override final;
    virtual bool getRegionOfDefinition(const RegionOfDefinitionArguments &args,
                                       OfxRectD &rod) override final;
    CommonText::CommonTextRenderResult renderText(FcConfig *fc,
                                                  const RenderArguments &args,
                                                  int width,
                                                  int height,
                                                  bool getRoD = false);
    CommonText::CommonTextRenderResult renderTextRoD(FcConfig *fc,
                                                     const RegionOfDefinitionArguments &args,
                                                     int width,
                                                     int height);

private:
    Clip *_dstClip;
    StringParam *_text;
    IntParam *_fontSize;
    RGBAParam *_textColor;
    RGBAParam *_bgColor;
    StringParam *_font;
    BooleanParam *_justify;
    ChoiceParam *_wrap;
    ChoiceParam *_align;
    ChoiceParam *_valign;
    ChoiceParam *_style;
    ChoiceParam *_stretch;
    ChoiceParam *_weight;
    RGBAParam *_strokeColor;
    DoubleParam *_strokeWidth;
    ChoiceParam *_hintStyle;
    ChoiceParam *_hintMetrics;
    IntParam *_letterSpace;
    Int2DParam *_canvas;
    ChoiceParam *_fontName;
    FcConfig* _fcConfig;
    BooleanParam *_markup;
    Int2DParam  *_range;
};

mDeclarePluginFactory(TextOFXPluginFactory, {}, {});

static TextOFXPluginFactory p(kPluginIdentifier,
                              kPluginVersionMajor,
                              kPluginVersionMinor);
mRegisterPluginFactoryInstance(p)

#endif // TEXT_H
