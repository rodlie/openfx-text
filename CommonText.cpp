/*
####################################################################
#
# Copyright (C) 2019 Ole-André Rodlie <ole.andre.rodlie@gmail.com>
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
####################################################################
*/

#include "CommonText.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <sys/stat.h>
#include <sys/types.h>

bool CommonText::fileExists(const std::string &str)
{
    struct stat st;
    return (stat(str.c_str(), &st) == 0);
}

const std::string CommonText::extract(const std::string &str,
                                      const std::string &start,
                                      const std::string &end)
{
    std::size_t startIndex = str.find(start);
    if (startIndex == std::string::npos) { return std::string(); }
    startIndex += start.length();
    std::string::size_type endIndex = str.find(end, startIndex);
    return str.substr(startIndex, endIndex - startIndex);
}

bool CommonText::compare(const std::string &l,
                         const std::string &r)
{
    return (l==r);
}

bool CommonText::replace(std::string &str,
                         const std::string &from,
                         const std::string &to)
{
    size_t start_pos = str.find(from);
    if (start_pos == std::string::npos) { return false; }
    str.replace(start_pos, from.length(), to);
    return true;
}

const std::string CommonText::trimmed(const std::string &str,
                                      bool whitespace,
                                      bool singlequote,
                                      bool doublequote)
{
    std::string result = str;
    if (whitespace) {
        result.erase(std::remove(result.begin(),
                                 result.end(),
                                 ' '),
                                 result.end());
    }
    if (singlequote) {
        result.erase(std::remove(result.begin(),
                                 result.end(),
                                 '\''),
                                 result.end());
    }
    if (doublequote) {
        result.erase(std::remove(result.begin(),
                                 result.end(),
                                 '"'),
                                 result.end());
    }
    return result;
}

bool CommonText::contains(const std::string &str,
                          const std::string &what)
{
    return str.find(what) != std::string::npos;
}

bool CommonText::startsWith(const std::string &str,
                            const std::string &what)
{
    return str.rfind(what, 0) == 0;
}

double CommonText::strTimeToSecs(const std::string &str)
{
    size_t start = 0U;
    std::string delim = ":";
    size_t end = str.find(delim);
    std::vector<std::string> times;
    while (end != std::string::npos) {
        times.push_back(str.substr(start, end - start));
        start = end + delim.length();
        end = str.find(delim, start);
    }
    times.push_back(str.substr(start, end));
    if (times.size()==3) {
        double h = atof(times.at(0).c_str());
        double m = atof(times.at(1).c_str());
        std::string t = times.at(2);
        replace(t, ",", ".");
        double s = atof(t.c_str());
        double result = 0.0;
        result += h*3600;
        result += m*60;
        result += s;
        return result;
    }
    return -1;
}

bool CommonText::isHtml(const std::string &str)
{
    return CommonText::contains(str, "<html") ||
           CommonText::contains(str, "<head") ||
           CommonText::contains(str, "<body") ||
           CommonText::contains(str, "<font") ||
           CommonText::contains(str, "<p");
}

bool CommonText::isRichText(const std::string &str,
                            bool strictMode)
{
    bool result = CommonText::contains(str, "<body");
    if (strictMode) {
        return result && contains(str, "<meta name=\"qrichtext");
    }
    return result;
}

bool CommonText::isNatronLegacyRichText(const std::string &str)
{
    return CommonText::startsWith(str, "<font");
}

bool CommonText::isMarkup(const std::string &str)
{
    return !CommonText::isHtml(str) &&
           !CommonText::isRichText(str, true) &&
           CommonText::contains(str, "<span") &&
           CommonText::contains(str, "</span>");
}

// WIP
const std::string CommonText::convertHtmlToMarkup(const std::string &str,
                                                  double renderScale)
{
    std::string result = str;

    if (CommonText::isNatronLegacyRichText(str)) {
         std::cout << "\n\n==========> THIS VERSION OF NATRON HAS LIMITED SUPPORT FOR RICH TEXT !!!" << std::endl;
    }

    std::cout << "\n\n==========> CONVERT HTML TO PANGO\n\nHTML SOURCE:\n\n" << result << std::endl;

    // mark all tags we want to keep
    std::vector<std::string> tags;
    tags.push_back("font");
    tags.push_back("p");
    tags.push_back("h1");
    tags.push_back("h2");
    tags.push_back("h3");
    tags.push_back("h4");
    tags.push_back("body");
    tags.push_back("span"); // yes, we also need to include span's

    for (size_t i=0; i < tags.size(); ++i) {
        std::string startTag = "<" + tags[i];
        if (CommonText::contains(result, startTag)) {
            if (startTag == "<body") { // body is used for START and END line
                result.replace(result.find(startTag), startTag.length(), "<!_BODY");
                std::string endTag = "/" + tags[i];
                result.replace(result.find(endTag), endTag.length(), "/!_BODY");
                continue;
            }
            while (result.find(startTag) != std::string::npos) {
                result.replace(result.find(startTag), startTag.length(), "<!_SPAN");
            }
        }
        std::string endTag = "/" + tags[i];
        if (CommonText::contains(result, endTag)) {
            while (result.find(endTag) != std::string::npos) {
                result.replace(result.find(endTag), endTag.length(), "/!_SPAN");
            }
        }
    }

    // std::cout << "\n\nHTML FILTER:\n\n" << result << std::endl;

    // go through each tag
    bool bodyExists = CommonText::contains(result, "<!_BODY");
    bool bodyStart = false;
    bool bodyEnd = false;
    std::stringstream ss(result);
    std::string line, markup;
    while (std::getline(ss, line, '<')) {
        //// std::cout << "LINE: " << line << std::endl;
        // body stuff
        if (bodyExists) {
            if (CommonText::startsWith(line, "!_BODY")) { bodyStart = true; }
            if (CommonText::startsWith(line, "/!_BODY")) {
                bodyEnd = true;
                std::string tag = "/!_BODY";
                line.replace(line.find(tag), tag.length(), "</span");
                markup += line;
                continue;
            }
            if (!bodyStart || bodyEnd) { continue; } // ignore everything before and after body
        }
        // get options from tag
        if (CommonText::startsWith(line, "!_")) {

            // std::cout << "\n\nFOUND TAG: " << line << std::endl;

            // get css options
            std::string getFontSizeCSS = CommonText::trimmed(CommonText::extract(line, "font-size:", ";"));
            std::string getFontFamilyCSS = CommonText::trimmed(CommonText::extract(line, "font-family:", ";"), false /* whitespace */);
            std::string getFontWeightCSS = CommonText::trimmed(CommonText::extract(line, "font-weight:", ";"), false /* whitespace */);
            std::string getFontStyleCSS = CommonText::trimmed(CommonText::extract(line, "font-style:", ";"), false /* whitespace */);
            std::string getFontColorCSS = CommonText::trimmed(CommonText::extract(line, "color:", ";"));

            // std::cout << "FONT CSS :size: " << getFontSizeCSS << " :family: " << getFontFamilyCSS << " :weight: " << getFontWeightCSS << " :style: " << getFontStyleCSS << " :color: " << getFontColorCSS << std::endl;

            // get html options
            std::string getFontSizeHTML = CommonText::trimmed(CommonText::extract(line, "size=\"", "\""));
            std::string getFontFamilyHTML = CommonText::trimmed(CommonText::extract(line, "face=\"", "\""), false /* whitespace */);
            std::string getFontColorHTML = CommonText::trimmed(CommonText::extract(line, "color=\"", "\""));

            // std::cout << "FONT HTML :size: " << getFontSizeHTML << " :family: " << getFontFamilyHTML << " :color: " << getFontColorHTML << std::endl;

            // check options
            bool hasStyle = CommonText::contains(line, "style=\"");
            bool hasFontFamily = !getFontFamilyCSS.empty() || !getFontFamilyHTML.empty();
            bool hasFontSize = !getFontSizeCSS.empty() || !getFontSizeHTML.empty();
            bool hasFontColor = !getFontSizeCSS.empty() || !getFontSizeHTML.empty();

            std::string fontFamily = !getFontFamilyCSS.empty()?getFontFamilyCSS:getFontFamilyHTML;
            std::string fontSize = !getFontSizeCSS.empty()?getFontSizeCSS:getFontSizeHTML;
            std::string fontColor = !getFontColorCSS.empty()?getFontColorCSS:getFontColorHTML;

            // remove 'pt' in font size
            if (CommonText::contains(fontSize, "pt")) {
                fontSize.replace(fontSize.find("pt"), sizeof("pt"), "");
            }

            // add render scale
            if (!fontSize.empty() && renderScale>0) {
                int fs = std::stoi(fontSize);
                if (fs>0) {
                    fs = fs * renderScale + 0.5;
                    fontSize = std::to_string(fs);
                }
            }

            // remove style opt if found
            if (hasStyle) {
                std::string opt = CommonText::extract(line, "style=\"", "\"");
                if (!opt.empty()) {
                    //// std::cout << "STYLE? " << opt << std::endl;
                    std::string style = "style=\"" + opt + "\"";
                    line.replace(line.find(style), style.length() /*+ 2*/, "");
                }
            }
            // remove size opt if found
            if (!getFontSizeHTML.empty()) {
                std::string opt = "size=\"" + getFontSizeHTML;
                line.replace(line.find(opt), opt.length() + 1, "");
            }
            // remove face opt if found
            if (!getFontSizeHTML.empty()) {
                std::string opt = "face=\"" + getFontFamilyHTML;
                line.replace(line.find(opt), opt.length() + 1, "");
            }

            std::cout << "FONT FAMILY? " << hasFontFamily << ":" << fontFamily << " SIZE? " << hasFontSize << ":" << fontSize << std::endl;

            // add font family (and size if exists)
            if (hasFontFamily && !fontFamily.empty()) {
                std::string span = CommonText::startsWith(line, "!_BODY")?"!_BODY":"!_SPAN";
                std::string fontDesc = span + " font_desc=\"" + fontFamily;
                if (hasFontSize) { fontDesc += " " + fontSize; }
                fontDesc += "\" ";
                line.replace(line.find(span), span.length(), fontDesc);
            }

            // add font size (if we don't have family)
            if (!hasFontFamily && hasFontSize) {

            }

            // std::cout << "FONT COLOR? " << hasFontColor << " " << fontColor << std::endl;

            // add font color
            if (hasFontColor && !fontColor.empty() && getFontFamilyHTML.empty()) {
                std::string span = CommonText::startsWith(line, "!_BODY")?"!_BODY":"!_SPAN";
                std::string opt = span + " color=\"" + fontColor + "\" ";
                line.replace(line.find(span), span.length(), opt);
            }
        }
        // fix tag TODO!
        if (CommonText::startsWith(line, "/!_BODY")) {
            std::string tag = "/!_BODY";
            line.replace(line.find(tag), tag.length(), "</span");
        }
        if (CommonText::startsWith(line, "/!_SPAN")) {
            std::string tag = "/!_SPAN";
            line.replace(line.find(tag), tag.length(), "</span");
        }
        if (CommonText::startsWith(line, "!_BODY")) {
            std::string tag = "!_BODY";
            line.replace(line.find(tag), tag.length(), "<span");
        }
        if (CommonText::startsWith(line, "!_SPAN")) {
            std::string tag = "!_SPAN";
            line.replace(line.find(tag), tag.length(), "<span");
        }

        // add to markup
        markup += line;
    }
    if (!markup.empty()) { result = markup; } // add markup
     std::cout << "\n\nPANGO MARKUP RESULT:\n\n" << result << std::endl; // DEBUG
    return result;
}

void CommonText::setLayoutAlign(PangoLayout *layout,
                                int align)
{
    if (!layout) { return; }
    switch(align) {
    case CommonTextAlignRight:
        pango_layout_set_alignment(layout, PANGO_ALIGN_RIGHT);
        break;
    case CommonTextAlignCenter:
        pango_layout_set_alignment(layout, PANGO_ALIGN_CENTER);
        break;
    default:
        pango_layout_set_alignment(layout, PANGO_ALIGN_LEFT);
        break;
    }
}

void CommonText::setLayoutWrap(PangoLayout *layout,
                               int wrap)
{
    if (!layout) { return; }
    switch(wrap) {
    case CommonTextWrapChar:
        pango_layout_set_wrap(layout, PANGO_WRAP_CHAR);
        break;
    case CommonTextWrapWordChar:
        pango_layout_set_wrap(layout, PANGO_WRAP_WORD_CHAR);
        break;
    default:
        pango_layout_set_wrap(layout, PANGO_WRAP_WORD);
        break;
    }
}

void CommonText::setLayoutJustify(PangoLayout *layout,
                                  bool justify)
{
    if (!layout) { return; }
    pango_layout_set_justify(layout, justify);
}

void CommonText::setLayoutMarkup(PangoLayout *layout,
                                 const std::string &str,
                                 double renderScale)
{
    if (!layout || str.empty()) { return; }
    std::string markup = str;
    if (CommonText::isHtml(markup)) {
        markup = CommonText::convertHtmlToMarkup(markup, renderScale);
    }
    pango_layout_set_markup(layout,
                            markup.c_str(),
                            -1);
}

void CommonText::setLayoutWidth(PangoLayout *layout,
                                int width)
{
    if (!layout || width<=0) { return; }
    pango_layout_set_width(layout, width * PANGO_SCALE);
}

void CommonText::setFontHintStyleOption(cairo_font_options_t *options,
                                        int hint)
{
    if (!options) { return; }
    switch(hint) {
    case CommonTextHintNone:
        cairo_font_options_set_hint_style(options, CAIRO_HINT_STYLE_NONE);
        break;
    case CommonTextHintSlight:
        cairo_font_options_set_hint_style(options, CAIRO_HINT_STYLE_SLIGHT);
        break;
    case CommonTextHintMedium:
        cairo_font_options_set_hint_style(options, CAIRO_HINT_STYLE_MEDIUM);
        break;
    case CommonTextHintFull:
        cairo_font_options_set_hint_style(options, CAIRO_HINT_STYLE_FULL);
        break;
    default:
        cairo_font_options_set_hint_style(options, CAIRO_HINT_STYLE_DEFAULT);
        break;
    }
}

void CommonText::setFontHintMetricsOption(cairo_font_options_t *options,
                                          int metric)
{
    if (!options) { return; }
    switch(metric) {
    case CommonTextHintMetricsOff:
        cairo_font_options_set_hint_metrics(options, CAIRO_HINT_METRICS_OFF);
        break;
    case CommonTextHintMetricsOn:
        cairo_font_options_set_hint_metrics(options, CAIRO_HINT_METRICS_ON);
        break;
    default:
        cairo_font_options_set_hint_metrics(options, CAIRO_HINT_METRICS_DEFAULT);
        break;
    }
}

void CommonText::setFontAntialiasOption(cairo_font_options_t *options,
                                        int antialias)
{
    if (!options) { return; }
    switch(antialias) {
    case CommonTextFontAntialiasNone:
        cairo_font_options_set_antialias(options, CAIRO_ANTIALIAS_NONE);
        break;
    case CommonTextFontAntialiasGray:
        cairo_font_options_set_antialias(options, CAIRO_ANTIALIAS_GRAY);
        break;
    case CommonTextFontAntialiasSubpixel:
        cairo_font_options_set_antialias(options, CAIRO_ANTIALIAS_SUBPIXEL);
        break;
    default:
        cairo_font_options_set_antialias(options, CAIRO_ANTIALIAS_DEFAULT);
        break;
    }
}

void CommonText::setFontSubpixelOption(cairo_font_options_t *options,
                                       int subpixel)
{
    if (!options) { return; }
    switch(subpixel) {
    case CommonTextFontSubpixelRGB:
        cairo_font_options_set_subpixel_order(options, CAIRO_SUBPIXEL_ORDER_RGB);
        break;
    case CommonTextFontSubpixelBGR:
        cairo_font_options_set_subpixel_order(options, CAIRO_SUBPIXEL_ORDER_BGR);
        break;
    case CommonTextFontSubpixelVRGB:
        cairo_font_options_set_subpixel_order(options, CAIRO_SUBPIXEL_ORDER_VRGB);
        break;
    case CommonTextFontSubpixelVBGR:
        cairo_font_options_set_subpixel_order(options, CAIRO_SUBPIXEL_ORDER_VBGR);
        break;
    default:
        cairo_font_options_set_subpixel_order(options, CAIRO_SUBPIXEL_ORDER_DEFAULT);
        break;
    }
}

void CommonText::setFontStretchDescription(PangoFontDescription *description,
                                           int stretch)
{
    if (!description) { return; }
    switch(stretch) {
    case CommonTextFontStretchUltraCondensed:
        pango_font_description_set_stretch(description, PANGO_STRETCH_ULTRA_CONDENSED);
        break;
    case CommonTextFontStretchExtraCondensed:
        pango_font_description_set_stretch(description, PANGO_STRETCH_EXTRA_CONDENSED);
        break;
    case CommonTextFontStretchCondensed:
        pango_font_description_set_stretch(description, PANGO_STRETCH_CONDENSED);
        break;
    case CommonTextFontStretchSemiCondensed:
        pango_font_description_set_stretch(description, PANGO_STRETCH_SEMI_CONDENSED);
        break;
    case CommonTextFontStretchSemiExpanded:
        pango_font_description_set_stretch(description, PANGO_STRETCH_SEMI_EXPANDED);
        break;
    case CommonTextFontStretchExpanded:
        pango_font_description_set_stretch(description, PANGO_STRETCH_EXPANDED);
        break;
    case CommonTextFontStretchExtraExpanded:
        pango_font_description_set_stretch(description, PANGO_STRETCH_EXTRA_EXPANDED);
        break;
    case CommonTextFontStretchUltraExpanded:
        pango_font_description_set_stretch(description, PANGO_STRETCH_ULTRA_EXPANDED);
        break;
    default:
        pango_font_description_set_stretch(description, PANGO_STRETCH_NORMAL);
        break;
    }
}

void CommonText::setFontWeightDescription(PangoFontDescription *description,
                                          int weight)
{
    if (!description) { return; }
    switch(weight) {
    case CommonTextFontWeightThin:
        pango_font_description_set_weight(description, PANGO_WEIGHT_THIN);
        break;
    case CommonTextFontWeightUltraLight:
        pango_font_description_set_weight(description, PANGO_WEIGHT_ULTRALIGHT);
        break;
    case CommonTextFontWeightLight:
        pango_font_description_set_weight(description, PANGO_WEIGHT_LIGHT);
        break;
    case CommonTextFontWeightSemiLight:
        pango_font_description_set_weight(description, PANGO_WEIGHT_SEMILIGHT);
        break;
    case CommonTextFontWeightBook:
        pango_font_description_set_weight(description, PANGO_WEIGHT_BOOK);
        break;
    case CommonTextFontWeightMedium:
        pango_font_description_set_weight(description, PANGO_WEIGHT_MEDIUM);
        break;
    case CommonTextFontWeightSemiBold:
        pango_font_description_set_weight(description, PANGO_WEIGHT_SEMIBOLD);
        break;
    case CommonTextFontWeightBold:
        pango_font_description_set_weight(description, PANGO_WEIGHT_BOLD);
        break;
    case CommonTextFontWeightUltraBold:
        pango_font_description_set_weight(description, PANGO_WEIGHT_ULTRABOLD);
        break;
    case CommonTextFontWeightHeavy:
        pango_font_description_set_weight(description, PANGO_WEIGHT_HEAVY);
        break;
    case CommonTextFontWeightUltraHeavy:
        pango_font_description_set_weight(description, PANGO_WEIGHT_ULTRAHEAVY);
        break;
    default:
        pango_font_description_set_weight(description, PANGO_WEIGHT_NORMAL);
        break;
    }
}

void CommonText::setupFontmap(FcConfig *fc,
                              PangoFontMap *map)
{
    if (!fc) { return; }
    if (!map) { map = pango_cairo_font_map_get_default(); }
    if (pango_cairo_font_map_get_font_type((PangoCairoFontMap*)(map)) != CAIRO_FONT_TYPE_FT) {
        map = pango_cairo_font_map_new_for_font_type(CAIRO_FONT_TYPE_FT);
    }
    pango_fc_font_map_set_config((PangoFcFontMap*)map, fc);
    pango_cairo_font_map_set_default((PangoCairoFontMap*)(map));
}

CommonText::CommonTextRenderResult CommonText::renderRichText(int width,
                                                              int height,
                                                              FcConfig *fc,
                                                              const std::string &html,
                                                              int wrap,
                                                              int align,
                                                              int justify,
                                                              double rX,
                                                              double rY,
                                                              bool flip,
                                                              bool noBuffer)
{
    std::cout << "RICHT TEXT WIP RENDER " << width << " " << height << " " << rX << " " << rY <<std::endl;

    CommonTextRenderResult result;
    result.success = false;

    if (!fc) {
        // std::cout << "NO FONTCONFIG" << std::endl;
        return result;
    }

    // setup font map
    PangoFontMap *map = pango_cairo_font_map_get_default();
    CommonText::setupFontmap(fc, map);

    // setup surface and layout
    cairo_t *cr;
    cairo_status_t status;
    cairo_surface_t *surface;
    PangoLayout *layout;
    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cr = cairo_create(surface);
    layout = pango_cairo_create_layout(cr);

    // flip
    if (flip) {
        cairo_scale(cr, 1.0f, -1.0f);
        cairo_translate(cr, 0.0f, -height);
    }

    // render layout
    CommonText::setLayoutMarkup(layout, html, rX);
    if (width>0) {
        CommonText::setLayoutWidth(layout, width);
    }
    CommonText::setLayoutWrap(layout, wrap);
    CommonText::setLayoutAlign(layout, align);
    CommonText::setLayoutJustify(layout, justify);

    // update layout
    pango_cairo_update_layout(cr, layout);
    pango_cairo_show_layout(cr, layout);

    // add pango layout width/height
    result.pW = -1;
    result.pH = -1;
    pango_layout_get_pixel_size(layout, &result.pW, &result.pH);
    std::cout << "PANGO SIZE " << result.pW << " " << result.pH << std::endl;

    // success?
    status = cairo_status(cr);
    if (!status) { result.success = true; }

    // flush
    cairo_surface_flush(surface);

    // add cairo surface width/height
    result.sW = -1;
    result.sH = -1;
    result.sW = cairo_image_surface_get_width(surface);
    result.sH = cairo_image_surface_get_height(surface);
    std::cout << "SURFACE SIZE " << result.sW << " " << result.sH << std::endl;

    if (result.sW != width || result.sH != height) { // size differ!
        noBuffer = true; // skip buffer
    }

    // get buffer
    if (result.success && !noBuffer) {
        unsigned char* buffer = cairo_image_surface_get_data(surface);
        result.buffer = new unsigned char[width * height * 4];
        int offset = 0;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                result.buffer[offset + 0] = buffer[offset + 2];
                result.buffer[offset + 1] = buffer[offset + 1];
                result.buffer[offset + 2] = buffer[offset + 0];
                result.buffer[offset + 3] = buffer[offset + 3];
                offset += 4;
            }
        }
        buffer = nullptr;
    }

    g_object_unref(layout);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    return  result;
}

CommonText::CommonTextRenderResult CommonText::renderText(int width,
                                                          int height,
                                                          FcConfig *fc,
                                                          const std::string &txt,
                                                          const std::string &font,
                                                          CommonText::CommonTextStyle style,
                                                          double x,
                                                          double y,
                                                          double scX,
                                                          double scY,
                                                          double skX,
                                                          double skY,
                                                          double rX,
                                                          double rY,
                                                          double rotate,
                                                          bool flip,
                                                          bool getRoD,
                                                          bool noBuffer)
{
    CommonTextRenderResult result;
    result.success = false;

    if (!fc) {
        std::cout << "NO FONTCONFIG!!!" << std::endl;
        return result;
    }

    // setup font map
    PangoFontMap *map = pango_cairo_font_map_get_default();
    CommonText::setupFontmap(fc, map);

    // setup surface and layout
    cairo_t *cr;
    cairo_status_t status;
    cairo_surface_t *surface;
    PangoLayout *layout;
    cairo_font_options_t* options;
    PangoAttrList *alist;
    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cr = cairo_create(surface);
    layout = pango_cairo_create_layout(cr);
    options = cairo_font_options_create();
    alist = pango_attr_list_new();

    // flip
    if (flip && !getRoD) {
        cairo_scale(cr, 1.0f, -1.0f);
        cairo_translate(cr, 0.0f, -height);
    }

    // set background
    if (!getRoD) {
        cairo_rectangle(cr, 0, 0, width, height);
        cairo_set_source_rgba(cr,
                              style.backgroundColor.r,
                              style.backgroundColor.g,
                              style.backgroundColor.b,
                              style.backgroundColor.a);
        cairo_fill(cr);
    }

    // set font options
    CommonText::setFontHintStyleOption(options, style.hint);
    CommonText::setFontHintMetricsOption(options, style.metrics);
    CommonText::setFontAntialiasOption(options, style.aa);
    CommonText::setFontSubpixelOption(options, style.subpixel);

    pango_cairo_context_set_font_options(pango_layout_get_context(layout), options);

    // set text
    if (style.markup) {
        GError *error = nullptr;
        if ( pango_parse_markup(txt.c_str(), -1, 0, nullptr, nullptr, nullptr, &error) ) {
            pango_layout_set_markup(layout, txt.c_str(), -1);
        } else {
            result.error = error->message;
            style.markup = false; // fallback to plain text
        }
        if (error) { g_clear_error(&error); }
    }
    if (!style.markup) {
        pango_layout_set_text(layout, txt.c_str(), -1);
    }

    // set font desc
    PangoFontDescription *desc;
    desc = pango_font_description_from_string(font.c_str());
    CommonText::setFontWeightDescription(desc, style.weight);
    CommonText::setFontStretchDescription(desc, style.stretch);
    pango_layout_set_font_description(layout, desc);
    pango_font_description_free(desc);

    // set layout
    if (!getRoD) {
        //CommonText::setLayoutWidth(layout, width);
        CommonText::setLayoutWrap(layout, style.wrap);
        CommonText::setLayoutAlign(layout, style.align);
        if (style.valign != 0) {
            int text_width, text_height;
            pango_layout_get_pixel_size(layout, &text_width, &text_height);
            switch (style.valign) {
            case 1:
                cairo_move_to(cr, 0, (height-text_height)/2);
                break;
            case 2:
                cairo_move_to(cr, 0, height-text_height);
                break;
            }
        }
        CommonText::setLayoutJustify(layout, style.justify);
    }

    if (style.letterSpace != 0) {
            pango_attr_list_insert(alist,
                                   pango_attr_letter_spacing_new(std::floor((style.letterSpace*PANGO_SCALE) * rX + 0.5)));
    }
    if (!style.markup) {
        pango_layout_set_attributes(layout,alist);
    }

    // set stroke
    if (style.strokeWidth>0) {
        cairo_new_path(cr);
        pango_cairo_layout_path(cr, layout);
        cairo_set_line_width(cr, std::floor(style.strokeWidth * rX + 0.5));
        cairo_set_source_rgba(cr,
                              style.strokeColor.r,
                              style.strokeColor.g,
                              style.strokeColor.b,
                              style.strokeColor.a);
        cairo_stroke_preserve(cr);
    }

    // set color
    if (!getRoD) {
        cairo_set_source_rgba(cr,
                              style.textColor.r,
                              style.textColor.g,
                              style.textColor.b,
                              style.textColor.a);
        cairo_fill(cr);
    }

    // update layout
    pango_cairo_update_layout(cr, layout);
    pango_cairo_show_layout(cr, layout);

    // add pango layout width/height
    result.pW = -1;
    result.pH = -1;
    pango_layout_get_pixel_size(layout, &result.pW, &result.pH);

    // success?
    status = cairo_status(cr);
    if (!status) { result.success = true; }

    // flush
    cairo_surface_flush(surface);

    // add cairo surface width/height
    result.sW = -1;
    result.sH = -1;
    result.sW = cairo_image_surface_get_width(surface);
    result.sH = cairo_image_surface_get_height(surface);

    if (result.sW != width || result.sH != height || getRoD) { // size differ! or we only want RoD
        noBuffer = true; // skip buffer
    }

    // get buffer
    if (result.success && !noBuffer) {
        unsigned char* buffer = cairo_image_surface_get_data(surface);
        result.buffer = new unsigned char[width * height * 4];
        int offset = 0;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                result.buffer[offset + 0] = buffer[offset + 2];
                result.buffer[offset + 1] = buffer[offset + 1];
                result.buffer[offset + 2] = buffer[offset + 0];
                result.buffer[offset + 3] = buffer[offset + 3];
                offset += 4;
            }
        }
        buffer = nullptr;
    }

    pango_attr_list_unref(alist);
    cairo_font_options_destroy(options);
    g_object_unref(layout);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    return result;
}

std::vector<CommonText::CommonTextSubtitle> CommonText::parseSRT(const std::string &filename)
{
    std::string delimiter = " --> ";
    std::vector<CommonTextSubtitle> sdata;
    std::ifstream file;
    file.open(filename.c_str());
    if (file.is_open()) {
        bool found = false;
        int i = 0;
        CommonTextSubtitle tdata;
        while (!file.eof()) {
            std::string line;
            std::getline(file, line);
            if (contains(line, delimiter)) {
                std::string startFrameString;
                std::string endFrameString = line;
                size_t pos = 0;
                while ((pos = endFrameString.find(delimiter)) != std::string::npos) {
                    startFrameString = endFrameString.substr(0, pos);
                    endFrameString.erase(0, pos + delimiter.length());
                }
                double startSec = strTimeToSecs(startFrameString);
                double endSec = strTimeToSecs(endFrameString);
                if (startSec>=0.0 && endSec>=0.0) {
                    found = true;
                    if (tdata.start>=0.0 &&
                        tdata.end>=0.0 &&
                        !tdata.str.empty())
                    {
                        sdata.push_back(tdata);
                    }
                    tdata.start = startSec;
                    tdata.end = endSec;
                    tdata.str.clear();
                    continue;
                }
            }
            if (line.empty()) {
                found = false;
                continue;
            } else {
                if (atoi(line.c_str())>0 || !found) { continue; }
                if (tdata.str.empty()) { tdata.str = line; }
                else { tdata.str += "\n" + line; }
            }
            i++;
        }
        if (tdata.start>=0.0 &&
            tdata.end>=0.0 &&
            !tdata.str.empty())
        {
            sdata.push_back(tdata);
        }
    }
    file.close();
    return sdata;
}

std::vector<std::string> CommonText::getFontFamilyList(FcConfig *fc,
                                                       const std::string &extra,
                                                       bool extraisDir)
{
    bool noFC = false;
    if (!fc) { noFC = true; fc = FcInitLoadConfigAndFonts(); } // init fc
    if (!extra.empty()) { // add extra font/dir
        const FcChar8 * custom = (const FcChar8 *)extra.c_str();
        if (!extraisDir) {
            FcConfigAppFontAddFile(fc, custom); // add file
        } else {
            FcConfigAppFontAddDir(fc, custom); // add dir
        }
    }

    // get font families from fc
    FcPattern *p = FcPatternCreate();
    FcObjectSet *os = FcObjectSetBuild(FC_FAMILY, nullptr);
    FcFontSet *fs = FcFontList(fc, p, os);
    std::list<std::string> fonts;
    for (int i=0; fs && i < fs->nfont; i++) {
        FcPattern *font = fs->fonts[i];
        FcChar8 *s;
        s = FcPatternFormat(font, (const FcChar8 *)"%{family[0]}");
        std::string fontName(reinterpret_cast<char*>(s));
        fonts.push_back(fontName);
        if (font) { FcPatternDestroy(font); }
    }
    FcObjectSetDestroy(os);
    FcPatternDestroy(p);
    if (noFC) { FcConfigDestroy(fc); }

    // sort and return
    fonts.sort();
    fonts.erase(std::unique(fonts.begin(), fonts.end(), CommonText::compare), fonts.end());

    std::vector<std::string> result(fonts.begin(), fonts.end());
    return result;
}

std::string CommonText::readTextFile(const std::string &txt)
{
    std::string result;
    if (txt.empty()) { return result; }
    std::ifstream f;
    f.open(txt.c_str());
    std::ostringstream s;
    s << f.rdbuf();
    f.close();
    if (!s.str().empty()) { result = s.str(); }
    return result;
}
