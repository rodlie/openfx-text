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

#include <string>
#include <vector>
#include <list>

#include <pango/pangocairo.h>
#include <pango/pangofc-fontmap.h>
#include <fontconfig/fontconfig.h>

class CommonText
{
public:
    enum CommonTextAlignment
    {
        CommonTextAlignLeft,
        CommonTextAlignRight,
        CommonTextAlignCenter
    };
    enum CommonTextVAligmnet
    {
        CommonTextAlignTop,
        CommonTextAlignMiddle,
        CommonTextAlignBottom
    };
    enum CommonTextWrap
    {
        CommonTextWrapNone,
        CommonTextWrapWord,
        CommonTextWrapChar,
        CommonTextWrapWordChar
    };
    enum CommonTextHintStyle
    {
        CommonTextHintDefault,
        CommonTextHintNone,
        CommonTextHintSlight,
        CommonTextHintMedium,
        CommonTextHintFull
    };
    enum CommonTextHintMetrics
    {
        CommonTextHintMetricsDefault,
        CommonTextHintMetricsOn,
        CommonTextHintMetricsOff
    };
    enum CommonTextFontAntialias
    {
        CommonTextFontAntialiasDefault,
        CommonTextFontAntialiasNone,
        CommonTextFontAntialiasGray,
        CommonTextFontAntialiasSubpixel
    };
    enum CommonTextFontSubpixel
    {
        CommonTextFontSubpixelDefault,
        CommonTextFontSubpixelRGB,
        CommonTextFontSubpixelBGR,
        CommonTextFontSubpixelVRGB,
        CommonTextFontSubpixelVBGR
    };
    enum CommonTextFontStretch
    {
        CommonTextFontStretchUltraCondensed,
        CommonTextFontStretchExtraCondensed,
        CommonTextFontStretchCondensed,
        CommonTextFontStretchSemiCondensed,
        CommonTextFontStretchNormal,
        CommonTextFontStretchSemiExpanded,
        CommonTextFontStretchExpanded,
        CommonTextFontStretchExtraExpanded,
        CommonTextFontStretchUltraExpanded
    };
    enum CommonTextFontWeight
    {
        CommonTextFontWeightThin,
        CommonTextFontWeightUltraLight,
        CommonTextFontWeightLight,
        CommonTextFontWeightSemiLight,
        CommonTextFontWeightBook,
        CommonTextFontWeightNormal,
        CommonTextFontWeightMedium,
        CommonTextFontWeightSemiBold,
        CommonTextFontWeightBold,
        CommonTextFontWeightUltraBold,
        CommonTextFontWeightHeavy,
        CommonTextFontWeightUltraHeavy
    };
    struct CommonTextRenderResult
    {
        bool success;
        unsigned char* buffer;
        int sW; // cairo surface width
        int sH; // cairo surface height
        int pW; // pango layout width
        int pH; // pango layout height
    };
    struct CommonTextSubtitle
    {
        double start;
        double end;
        std::string str;
    };
    struct CommonTextColor
    {
        double r;
        double g;
        double b;
        double a;
    };
    struct CommonTextStyle
    {
        int wrap;
        int align;
        int valign;
        bool justify;
        int stretch;
        int weight;
        int hint;
        int metrics;
        int aa;
        int subpixel;
        int letterSpace;
        double strokeWidth;
        CommonTextColor textColor;
        CommonTextColor strokeColor;
        CommonTextColor backgroundColor;
    };

    /** @brief file exists? */
    static bool fileExists(const std::string &str);

    /** @brief extract string from start to end */
    static const std::string extract(const std::string &str,
                                     const std::string &start,
                                     const std::string &end);

    /** @brief compare strings */
    static bool compare(const std::string & l,
                        const std::string & r);

    /** @brief replace from to in string */
    static bool replace(std::string& str,
                        const std::string& from,
                        const std::string& to);

    /** @brief trim string */
    static const std::string trimmed(const std::string &str,
                                     bool whitespace = true,
                                     bool singlequote = true,
                                     bool doublequote = true);

    /** @brief does string contain what? */
    static bool contains(const std::string &str,
                         const std::string &what);

    /** @brief does string start with what? */
    static bool startsWith(const std::string &str,
                           const std::string &what);

    /** @brief convert string (HH:MM:SS,MS) to double */
    static double strTimeToSecs(const std::string &str);

    /** @brief is string html? */
    static bool isHtml(const std::string &str);

    /** @brief is string "rich text" html? */
    static bool isRichText(const std::string &str,
                           bool strictMode = true);

    /** @brief is string legacy Natron "rich text" html? */
    static bool isNatronLegacyRichText(const std::string &str);

    /** @brief is string valid Pango markup? */
    static bool isMarkup(const std::string &str);

    /** @brief convert html to pango markup */
    static const std::string convertHtmlToMarkup(const std::string &str,
                                                 double renderScale = 0.0);

    /** @brief set pango layout text align */
    static void setLayoutAlign(PangoLayout *layout,
                               int align = CommonTextAlignLeft);

    /** @brief set pango layout word wrap */
    static void setLayoutWrap(PangoLayout *layout,
                              int wrap = CommonTextWrapWord);

    /** @brief set pango layout text justify */
    static void setLayoutJustify(PangoLayout *layout,
                                 bool justify = true);

    /** @brief set pango layout markup */
    static void setLayoutMarkup(PangoLayout *layout,
                                const std::string &str,
                                double renderScale = 0.0);

    /** @brief set pango layout width */
    static void setLayoutWidth(PangoLayout *layout,
                               int width);

    /** @brief set font hint style */
    static void setFontHintStyleOption(cairo_font_options_t *options,
                                       int hint = CommonTextHintDefault);

    /** @brief set font hint metrics */
    static void setFontHintMetricsOption(cairo_font_options_t *options,
                                         int metric = CommonTextHintMetricsDefault);

    /** @brief set font antialias */
    static void setFontAntialiasOption(cairo_font_options_t *options,
                                       int antialias = CommonTextFontAntialiasDefault);

    /** @brief set font subpixel order */
    static void setFontSubpixelOption(cairo_font_options_t *options,
                                      int subpixel = CommonTextFontSubpixelDefault);

    /** @brief set font stretch */
    static void setFontStretchDescription(PangoFontDescription *description,
                                          int stretch = CommonTextFontStretchNormal);

    /** @brief set font weight */
    static void setFontWeightDescription(PangoFontDescription *description,
                                         int weight = CommonTextFontWeightNormal);

    /** @brief setup pango fontmap (freetype+fontconfig) */
    static void setupFontmap(FcConfig *fc,
                             PangoFontMap *map);

    /** @brief render rich text to buffer */
    static CommonTextRenderResult renderRichText(int width,
                                                 int height,
                                                 FcConfig *fc,
                                                 const std::string &html,
                                                 int wrap,
                                                 int align,
                                                 int justify,
                                                 double rX,
                                                 double rY,
                                                 bool flip = false,
                                                 bool noBuffer = false);

    /** @brief render text to buffer */
    static CommonTextRenderResult renderText(int width,
                                             int height,
                                             FcConfig *fc,
                                             const std::string &txt,
                                             const std::string &font,
                                             CommonTextStyle style,
                                             double x,
                                             double y,
                                             double scX,
                                             double scY,
                                             double skX,
                                             double skY,
                                             double rX,
                                             double rY,
                                             double rotate,
                                             bool flip = false,
                                             bool noBuffer = false);

    /** @brief parse SRT subtitle file */
    static std::vector<CommonTextSubtitle> parseSRT(const std::string &filename);

    /** @brief generate font family list */
    static std::vector<std::string> getFontFamilyList(FcConfig *fc,
                                                      const std::string &extra = std::string(),
                                                      bool extraisDir = false);

    /** @brief read text from file */
    static std::string readTextFile(const std::string &txt);
};
