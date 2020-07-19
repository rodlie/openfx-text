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

cmake_minimum_required(VERSION 3.1)
project(Text VERSION 1.0.0 LANGUAGES CXX)

option(BUNDLE_FONTS_CONF "Bundle fonts.conf" OFF)
option(ENABLE_RICH_TEXT "Enable Rich Text Editor" OFF)
option(LOG_EVENTS "Log events" OFF)

add_definitions(-DkPluginVersionMajor=${PROJECT_VERSION_MAJOR} -DkPluginVersionMinor=${PROJECT_VERSION_MINOR})
if(${LOG_EVENTS})
    add_definitions(-DLOG_EVENTS)
endif()

# Require OpenGL
if(POLICY CMP0072)
    # prefer GLVND ABI. We use the compatibility via OPENGL_LIBRARIES variable
    # see https://cmake.org/cmake/help/v3.11/policy/CMP0072.html
    # see https://cmake.org/cmake/help/v3.11/module/FindOpenGL.html#linux-specific
    cmake_policy(SET CMP0072 NEW)
endif()
include(FindOpenGL REQUIRED)

# Require fontconfig and pangocairo
find_package(PkgConfig REQUIRED)
pkg_search_module(FONTCONFIG REQUIRED fontconfig)
pkg_search_module(PANGO REQUIRED pangocairo)

# find git (if needed later)
find_package(Git QUIET)

# find qt (for rich text editor)
if(${ENABLE_RICH_TEXT})
    set(CMAKE_AUTOMOC ON)
    find_package(Qt5 REQUIRED Core Gui Widgets)
endif()

set (CMAKE_CXX_STANDARD 11)
set(OPENFX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/OpenFX")
set(OFX_SUPPORT_HEADER_DIR "${OPENFX_PATH}/Support/include")
set(OFX_SUPPORT_LIBRARY_DIR "${OPENFX_PATH}/Support/Library")

if(NOT EXISTS ${OPENFX_PATH}/include)
    if(${GIT_FOUND})
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init
                                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                                RESULT_VARIABLE GIT_SUBMOD_RESULT)
    else()
        message(FATAL_ERROR "\nOpenFX is missing!\n")
    endif()
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${OPENFX_PATH}/include)
include_directories(${OFX_SUPPORT_HEADER_DIR})
include_directories(${OPENFX_PATH}/Support/Plugins/include)

add_compile_options(-Wall -Wextra)
if(CMAKE_BUILD_TYPE MATCHES "^(debug|Debug|DEBUG)$")
        add_definitions(-DDEBUG)
else()
        add_definitions(-DNDEBUG)
endif()

FILE(GLOB SUPPORT_SOURCES
    "${OFX_SUPPORT_HEADER_DIR}/*.h"
    "${OFX_SUPPORT_LIBRARY_DIR}/ofxsCore.cpp"
    "${OFX_SUPPORT_LIBRARY_DIR}/ofxsImageEffect.cpp"
    "${OFX_SUPPORT_LIBRARY_DIR}/ofxsLog.cpp"
    "${OFX_SUPPORT_LIBRARY_DIR}/ofxsParams.cpp"
    "${OFX_SUPPORT_LIBRARY_DIR}/ofxsProperty.cpp"
    "${OFX_SUPPORT_LIBRARY_DIR}/ofxsPropertyValidation.cpp"
    "${OFX_SUPPORT_LIBRARY_DIR}/*.h"
)

file(GLOB SOURCES
    "src/Text.cpp"
    "src/Text.h"
    "src/TextFX.h"
    "src/CommonText.cpp"
    "src/CommonText.h"
    "src/CommonOFX.h"
)
file(GLOB RESOURCES "share/net.sf.openfx.Text.png")

if (APPLE)
    set(FC_FONTS_DIR "<dir>/System/Library/Fonts</dir><dir>/Library/Fonts</dir><dir>~/Library/Fonts</dir>")
    set(FC_CACHE_DIR "<cachedir>~/.cache/fontconfig</cachedir>")
elseif(WIN32)
    set(FC_FONTS_DIR "<dir>WINDOWSFONTDIR</dir>")
    set(FC_CACHE_DIR "<cachedir>LOCAL_APPDATA_FONTCONFIG_CACHE</cachedir>")
else()
    set(FC_FONTS_DIR "<dir>/usr/share/fonts/OTF</dir><dir>/usr/share/fonts/TTF</dir><dir>/usr/share/fonts/Type1</dir><dir>/usr/share/fonts/Speedo</dir><dir>/usr/share/fonts/cyrillic</dir><dir>/usr/share/fonts/misc</dir><dir prefix=\"xdg\">fonts</dir>")
    set(FC_CACHE_DIR "<cachedir prefix=\"xdg\">fontconfig</cachedir><cachedir>~/.fontconfig</cachedir>")
endif()
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/share/fonts.conf.in
    ${CMAKE_CURRENT_BINARY_DIR}/fonts.conf
    @ONLY
)

if(${ENABLE_RICH_TEXT})
    add_executable(RichText src/RichTextEditor.cpp src/RichTextWidget.cpp src/RichTextWidget.h)
endif()
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${SUPPORT_SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".ofx")

if(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-fvisibility=hidden -exported_symbols_list,${OFX_SUPPORT_HEADER_DIR}/osxSymbols")
    set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "@loader_path/../Frameworks;@loader_path/../Libraries")
elseif(MINGW)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-shared -fvisibility=hidden -Xlinker --version-script=${OFX_SUPPORT_HEADER_DIR}/linuxSymbols")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD" OR ${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-Wl,-fvisibility=hidden,--version-script=${OFX_SUPPORT_HEADER_DIR}/linuxSymbols")
    set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "$ORIGIN/../../Libraries")
endif()

target_compile_definitions(
    ${PROJECT_NAME}
    PRIVATE
    OFX_EXTENSIONS_VEGAS
    OFX_EXTENSIONS_NUKE
    OFX_EXTENSIONS_NATRON
    OFX_EXTENSIONS_TUTTLE
    OFX_SUPPORTS_OPENGLRENDER
    NOMINMAX
)

include_directories(
    ${FONTCONFIG_INCLUDE_DIRS}
    ${PANGO_INCLUDE_DIRS}
)
target_link_libraries(
    ${PROJECT_NAME}
    ${OPENGL_LIBRARIES}
    ${FONTCONFIG_LDFLAGS}
    ${PANGO_LDFLAGS}
)
if(${ENABLE_RICH_TEXT})
    target_link_libraries(RichText Qt5::Core Qt5::Gui Qt5::Widgets)
endif()

# Find and set the arch name.
# http://openeffects.org/documentation/reference/ch02s02.html
SET(OFX_ARCH UNKNOWN)
# Win
if(MINGW)
    set(OFX_ARCH "Win32")
    if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
        set(OFX_ARCH "Win64")
    endif()
# Visual Studio
elseif(MSVC)
    set(OFX_ARCH "Win32")
    if((${CMAKE_SYSTEM_PROCESSOR} STREQUAL "IA64") OR (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "AMD64"))
        set(OFX_ARCH "Win64")
    endif()
# FreeBSD
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")
    set(OFX_ARCH "FreeBSD-x86")
    if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
        set(OFX_ARCH "FreeBSD-x86-64")
    endif()
    if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
        if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
            set(OFX_ARCH FreeBSD-arm-64)
        else()
            set(OFX_ARCH FreeBSD-arm)
        endif()
    endif()
# MacOS
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    set(OFX_ARCH "MacOS")
# Linux
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(OFX_ARCH "Linux-x86")
    if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
        set(OFX_ARCH "Linux-x86-64")
    endif()
    if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
        if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
            set(OFX_ARCH Linux-arm-64)
        else()
            set(OFX_ARCH Linux-arm)
        endif()
    endif()
endif()

set(OFX_ARCH_NAME ${OFX_ARCH} CACHE STRING "OpenFX target OS and architecture")
if("${OFX_ARCH_NAME}" EQUAL "UNKNOWN")
    message(SEND_ERROR "OFX_ARCH_NAME was not detected. Install targets will be unavailable. See http://openeffects.org/documentation/reference/ch02s02.html to pick an arch name.")
else()
    install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION ${PROJECT_NAME}.ofx.bundle/Contents/${OFX_ARCH_NAME}
        LIBRARY DESTINATION ${PROJECT_NAME}.ofx.bundle/Contents/${OFX_ARCH_NAME}
    )
    install(FILES ${RESOURCES} LICENSE README.md
        DESTINATION ${PROJECT_NAME}.ofx.bundle/Contents/Resources
    )
    install(FILES share/Info.plist
        DESTINATION ${PROJECT_NAME}.ofx.bundle/Contents
    )
    if(${BUNDLE_FONTS_CONF})
        install(
            FILES ${CMAKE_CURRENT_BINARY_DIR}/fonts.conf
            DESTINATION ${PROJECT_NAME}.ofx.bundle/Contents/Resources/fonts
        )
    endif()
endif()
