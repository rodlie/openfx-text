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
#include "TextHosts.h"
#include "TextParams.h"
#include <iostream>

using namespace OFX;

// workaround for stupid Fusion!
static std::vector<std::string> _fonts;
//

class TextOFXPlugin : public ImageEffect
{
public:
    TextOFXPlugin(OfxImageEffectHandle handle);
    virtual void render(const RenderArguments &args) override final;
    virtual void changedParam(const InstanceChangedArgs &args,
                              const std::string &paramName) override final;

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
    ChoiceParam *_fontName;
    FcConfig* _fcConfig;
};

mDeclarePluginFactory(TextOFXPluginFactory, {}, {});

static TextOFXPluginFactory p(kPluginIdentifier,
                              kPluginVersionMajor,
                              kPluginVersionMinor);
mRegisterPluginFactoryInstance(p)

#endif // TEXT_H
