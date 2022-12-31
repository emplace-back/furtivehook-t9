// dear imgui, v1.76
// (drawing and font code)

/*

Index of this file:

// [SECTION] STB libraries implementation
// [SECTION] Style functions
// [SECTION] ImDrawList
// [SECTION] ImDrawListSplitter
// [SECTION] ImDrawData
// [SECTION] Helpers ShadeVertsXXX functions
// [SECTION] ImFontConfig
// [SECTION] ImFontAtlas
// [SECTION] ImFontAtlas glyph ranges helpers
// [SECTION] ImFontGlyphRangesBuilder
// [SECTION] ImFont
// [SECTION] ImGui Internal Render Helpers
// [SECTION] Decompression code
// [SECTION] Default font data (ProggyClean.ttf)

*/

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "imgui.h"
#ifndef IMGUI_DISABLE

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui_internal.h"

#include <stdio.h>      // vsnprintf, sscanf, printf
#if !defined(alloca)
#if defined(__GLIBC__) || defined(__sun) || defined(__CYGWIN__) || defined(__APPLE__) || defined(__SWITCH__)
#include <alloca.h>     // alloca (glibc uses <alloca.h>. Note that Cygwin may have _WIN32 defined, so the order matters here)
#elif defined(_WIN32)
#include <malloc.h>     // alloca
#if !defined(alloca)
#define alloca _alloca  // for clang with MS Codegen
#endif
#else
#include <stdlib.h>     // alloca
#endif
#endif

// Visual Studio warnings
#ifdef _MSC_VER
#pragma warning (disable: 4127) // condition expression is constant
#pragma warning (disable: 4505) // unreferenced local function has been removed (stb stuff)
#pragma warning (disable: 4996) // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#endif

// Clang/GCC warnings with -Weverything
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wold-style-cast"         // warning : use of old-style cast                              // yes, they are more terse.
#pragma clang diagnostic ignored "-Wfloat-equal"            // warning : comparing floating point with == or != is unsafe   // storing and comparing against same constants ok.
#pragma clang diagnostic ignored "-Wglobal-constructors"    // warning : declaration requires a global destructor           // similar to above, not sure what the exact difference is.
#pragma clang diagnostic ignored "-Wsign-conversion"        // warning : implicit conversion changes signedness             //
#if __has_warning("-Wzero-as-null-pointer-constant")
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"  // warning : zero as null pointer constant              // some standard header variations use #define NULL 0
#endif
#if __has_warning("-Wcomma")
#pragma clang diagnostic ignored "-Wcomma"                  // warning : possible misuse of comma operator here             //
#endif
#if __has_warning("-Wreserved-id-macro")
#pragma clang diagnostic ignored "-Wreserved-id-macro"      // warning : macro name is a reserved identifier                //
#endif
#if __has_warning("-Wdouble-promotion")
#pragma clang diagnostic ignored "-Wdouble-promotion"       // warning: implicit conversion from 'float' to 'double' when passing argument to function  // using printf() is a misery with this as C++ va_arg ellipsis changes float to double.
#endif
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wpragmas"                  // warning: unknown option after '#pragma GCC diagnostic' kind
#pragma GCC diagnostic ignored "-Wunused-function"          // warning: 'xxxx' defined but not used
#pragma GCC diagnostic ignored "-Wdouble-promotion"         // warning: implicit conversion from 'float' to 'double' when passing argument to function
#pragma GCC diagnostic ignored "-Wconversion"               // warning: conversion to 'xxxx' from 'xxxx' may alter its value
#pragma GCC diagnostic ignored "-Wstack-protector"          // warning: stack protector not protecting local variables: variable length buffer
#pragma GCC diagnostic ignored "-Wclass-memaccess"          // [__GNUC__ >= 8] warning: 'memset/memcpy' clearing/writing an object of type 'xxxx' with no trivial copy-assignment; use assignment or value-initialization instead
#endif

//-------------------------------------------------------------------------
// [SECTION] STB libraries implementation
//-------------------------------------------------------------------------

// Compile time options:
//#define IMGUI_STB_NAMESPACE           ImStb
//#define IMGUI_STB_TRUETYPE_FILENAME   "my_folder/stb_truetype.h"
//#define IMGUI_STB_RECT_PACK_FILENAME  "my_folder/stb_rect_pack.h"
//#define IMGUI_DISABLE_STB_TRUETYPE_IMPLEMENTATION
//#define IMGUI_DISABLE_STB_RECT_PACK_IMPLEMENTATION

#ifdef IMGUI_STB_NAMESPACE
namespace IMGUI_STB_NAMESPACE
{
#endif

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4456)                             // declaration of 'xx' hides previous local declaration
#endif

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wimplicit-fallthrough"
#pragma clang diagnostic ignored "-Wcast-qual"              // warning : cast from 'const xxxx *' to 'xxx *' drops const qualifier //
#endif

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"              // warning: comparison is always true due to limited range of data type [-Wtype-limits]
#pragma GCC diagnostic ignored "-Wcast-qual"                // warning: cast from type 'const xxxx *' to type 'xxxx *' casts away qualifiers
#endif

#ifndef STB_RECT_PACK_IMPLEMENTATION                        // in case the user already have an implementation in the _same_ compilation unit (e.g. unity builds)
#ifndef IMGUI_DISABLE_STB_RECT_PACK_IMPLEMENTATION
#define STBRP_STATIC
#define STBRP_ASSERT(x)     do { IM_ASSERT(x); } while (0)
#define STBRP_SORT          ImQsort
#define STB_RECT_PACK_IMPLEMENTATION
#endif
#ifdef IMGUI_STB_RECT_PACK_FILENAME
#include IMGUI_STB_RECT_PACK_FILENAME
#else
#include "imstb_rectpack.h"
#endif
#endif

#ifndef STB_TRUETYPE_IMPLEMENTATION                         // in case the user already have an implementation in the _same_ compilation unit (e.g. unity builds)
#ifndef IMGUI_DISABLE_STB_TRUETYPE_IMPLEMENTATION
#define STBTT_malloc(x,u)   ((void)(u), IM_ALLOC(x))
#define STBTT_free(x,u)     ((void)(u), IM_FREE(x))
#define STBTT_assert(x)     do { IM_ASSERT(x); } while(0)
#define STBTT_fmod(x,y)     ImFmod(x,y)
#define STBTT_sqrt(x)       ImSqrt(x)
#define STBTT_pow(x,y)      ImPow(x,y)
#define STBTT_fabs(x)       ImFabs(x)
#define STBTT_ifloor(x)     ((int)ImFloorStd(x))
#define STBTT_iceil(x)      ((int)ImCeil(x))
#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#else
#define STBTT_DEF 
#endif
#ifdef IMGUI_STB_TRUETYPE_FILENAME
#include IMGUI_STB_TRUETYPE_FILENAME
#else
#include "imstb_truetype.h"
#endif
#endif

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#if defined(_MSC_VER)
#pragma warning (pop)
#endif

#ifdef IMGUI_STB_NAMESPACE
} // namespace ImStb
using namespace IMGUI_STB_NAMESPACE;
#endif

//-----------------------------------------------------------------------------
// [SECTION] Style functions
//-----------------------------------------------------------------------------

void ImGui::StyleColorsDark(ImGuiStyle* dst)
{
    ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImGuiCol_Text] = ImVec4(0.86f, 0.93f, 0.89f, 0.78f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.86f, 0.93f, 0.89f, 0.78f);
    colors[ImGuiCol_WindowBg] = ImColor(15, 15, 15, 255);
    colors[ImGuiCol_ChildBg] = ImColor(10, 10, 10, 255);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border] = ImColor(15, 15, 15, 255);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.09f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.04f, 0.04f, 0.04f, 0.88f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.60f, 0.78f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.20f, 0.22f, 0.27f, 0.75f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);

    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.09f, 0.15f, 0.16f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.15f, 0.60f, 0.78f, 0.78f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.15f, 0.60f, 0.78f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.24f, 0.40f, 0.95f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.40f, 0.95f, 0.59f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.24f, 0.40f, 0.95f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.24f, 0.40f, 0.95f, 0.59f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.70f, 0.02f, 0.60f, 0.22f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_Tab] = ImLerp(colors[ImGuiCol_Header],
        colors[ImGuiCol_TitleBgActive], 0.80f);
    colors[ImGuiCol_TabHovered] = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_TabActive] = ImLerp(colors[ImGuiCol_HeaderActive],
        colors[ImGuiCol_TitleBgActive], 0.60f);
    colors[ImGuiCol_TabUnfocused] = ImLerp(colors[ImGuiCol_Tab],
        colors[ImGuiCol_TitleBg], 0.80f);
    colors[ImGuiCol_TabUnfocusedActive] = ImLerp(colors[ImGuiCol_TabActive],
        colors[ImGuiCol_TitleBg], 0.40f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

void ImGui::StyleColorsClassic(ImGuiStyle* dst)
{
    ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.70f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.11f, 0.11f, 0.14f, 0.92f);
    colors[ImGuiCol_Border] = ImVec4(0.50f, 0.50f, 0.50f, 0.50f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.43f, 0.43f, 0.43f, 0.39f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.47f, 0.47f, 0.69f, 0.40f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.42f, 0.41f, 0.64f, 0.69f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.27f, 0.27f, 0.54f, 0.83f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.32f, 0.32f, 0.63f, 0.87f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.40f, 0.40f, 0.80f, 0.20f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.40f, 0.40f, 0.55f, 0.80f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.20f, 0.25f, 0.30f, 0.60f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.40f, 0.40f, 0.80f, 0.30f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.80f, 0.40f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.41f, 0.39f, 0.80f, 0.60f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.90f, 0.90f, 0.90f, 0.50f);
    colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 1.00f, 1.00f, 0.30f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.41f, 0.39f, 0.80f, 0.60f);
    colors[ImGuiCol_Button] = ImVec4(0.35f, 0.40f, 0.61f, 0.62f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.40f, 0.48f, 0.71f, 0.79f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.46f, 0.54f, 0.80f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.40f, 0.40f, 0.90f, 0.45f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.45f, 0.45f, 0.90f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.53f, 0.53f, 0.87f, 0.80f);
    colors[ImGuiCol_Separator] = ImVec4(0.50f, 0.50f, 0.50f, 0.60f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.60f, 0.60f, 0.70f, 1.00f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.70f, 0.70f, 0.90f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.16f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.78f, 0.82f, 1.00f, 0.60f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.78f, 0.82f, 1.00f, 0.90f);
    colors[ImGuiCol_Tab] = ImLerp(colors[ImGuiCol_Header], colors[ImGuiCol_TitleBgActive], 0.80f);
    colors[ImGuiCol_TabHovered] = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_TabActive] = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
    colors[ImGuiCol_TabUnfocused] = ImLerp(colors[ImGuiCol_Tab], colors[ImGuiCol_TitleBg], 0.80f);
    colors[ImGuiCol_TabUnfocusedActive] = ImLerp(colors[ImGuiCol_TabActive], colors[ImGuiCol_TitleBg], 0.40f);
    colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.00f, 0.00f, 1.00f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight] = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
}

// Those light colors are better suited with a thicker font than the default one + FrameBorder
void ImGui::StyleColorsLight(ImGuiStyle* dst)
{
    ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.98f);
    colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.30f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.69f, 0.69f, 0.69f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.49f, 0.49f, 0.49f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.46f, 0.54f, 0.80f, 0.60f);
    colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.39f, 0.39f, 0.39f, 0.62f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.14f, 0.44f, 0.80f, 0.78f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.14f, 0.44f, 0.80f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.80f, 0.80f, 0.80f, 0.56f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_Tab] = ImLerp(colors[ImGuiCol_Header], colors[ImGuiCol_TitleBgActive], 0.90f);
    colors[ImGuiCol_TabHovered] = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_TabActive] = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
    colors[ImGuiCol_TabUnfocused] = ImLerp(colors[ImGuiCol_Tab], colors[ImGuiCol_TitleBg], 0.80f);
    colors[ImGuiCol_TabUnfocusedActive] = ImLerp(colors[ImGuiCol_TabActive], colors[ImGuiCol_TitleBg], 0.40f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.45f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_NavHighlight] = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.70f, 0.70f, 0.70f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
}

//-----------------------------------------------------------------------------
// ImDrawList
//-----------------------------------------------------------------------------

ImDrawListSharedData::ImDrawListSharedData()
{
    Font = NULL;
    FontSize = 0.0f;
    CurveTessellationTol = 0.0f;
    CircleSegmentMaxError = 0.0f;
    ClipRectFullscreen = ImVec4(-8192.0f, -8192.0f, +8192.0f, +8192.0f);
    InitialFlags = ImDrawListFlags_None;

    // Lookup tables
    for (int i = 0; i < IM_ARRAYSIZE(ArcFastVtx); i++)
    {
        const float a = ((float)i * 2 * IM_PI) / (float)IM_ARRAYSIZE(ArcFastVtx);
        ArcFastVtx[i] = ImVec2(ImCos(a), ImSin(a));
    }
    memset(CircleSegmentCounts, 0, sizeof(CircleSegmentCounts)); // This will be set by SetCircleSegmentMaxError()
}

void ImDrawListSharedData::SetCircleSegmentMaxError(float max_error)
{
    if (CircleSegmentMaxError == max_error)
        return;
    CircleSegmentMaxError = max_error;
    for (int i = 0; i < IM_ARRAYSIZE(CircleSegmentCounts); i++)
    {
        const float radius = i + 1.0f;
        const int segment_count = IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC(radius, CircleSegmentMaxError);
        CircleSegmentCounts[i] = (ImU8)ImMin(segment_count, 255);
    }
}

void ImDrawList::Clear()
{
    CmdBuffer.resize(0);
    IdxBuffer.resize(0);
    VtxBuffer.resize(0);
    Flags = _Data ? _Data->InitialFlags : ImDrawListFlags_None;
    _VtxCurrentOffset = 0;
    _VtxCurrentIdx = 0;
    _VtxWritePtr = NULL;
    _IdxWritePtr = NULL;
    _ClipRectStack.resize(0);
    _TextureIdStack.resize(0);
    _Path.resize(0);
    _Splitter.Clear();
}

void ImDrawList::ClearFreeMemory()
{
    CmdBuffer.clear();
    IdxBuffer.clear();
    VtxBuffer.clear();
    _VtxCurrentIdx = 0;
    _VtxWritePtr = NULL;
    _IdxWritePtr = NULL;
    _ClipRectStack.clear();
    _TextureIdStack.clear();
    _Path.clear();
    _Splitter.ClearFreeMemory();
}

ImDrawList* ImDrawList::CloneOutput() const
{
    ImDrawList* dst = IM_NEW(ImDrawList(_Data));
    dst->CmdBuffer = CmdBuffer;
    dst->IdxBuffer = IdxBuffer;
    dst->VtxBuffer = VtxBuffer;
    dst->Flags = Flags;
    return dst;
}

// Using macros because C++ is a terrible language, we want guaranteed inline, no code in header, and no overhead in Debug builds
#define GetCurrentClipRect()    (_ClipRectStack.Size ? _ClipRectStack.Data[_ClipRectStack.Size-1]  : _Data->ClipRectFullscreen)
#define GetCurrentTextureId()   (_TextureIdStack.Size ? _TextureIdStack.Data[_TextureIdStack.Size-1] : (ImTextureID)NULL)

void ImDrawList::AddDrawCmd()
{
    ImDrawCmd draw_cmd;
    draw_cmd.ClipRect = GetCurrentClipRect();
    draw_cmd.TextureId = GetCurrentTextureId();
    draw_cmd.VtxOffset = _VtxCurrentOffset;
    draw_cmd.IdxOffset = IdxBuffer.Size;

    IM_ASSERT(draw_cmd.ClipRect.x <= draw_cmd.ClipRect.z && draw_cmd.ClipRect.y <= draw_cmd.ClipRect.w);
    CmdBuffer.push_back(draw_cmd);
}

void ImDrawList::AddCallback(ImDrawCallback callback, void* callback_data)
{
    ImDrawCmd* current_cmd = CmdBuffer.Size ? &CmdBuffer.back() : NULL;
    if (!current_cmd || current_cmd->ElemCount != 0 || current_cmd->UserCallback != NULL)
    {
        AddDrawCmd();
        current_cmd = &CmdBuffer.back();
    }
    current_cmd->UserCallback = callback;
    current_cmd->UserCallbackData = callback_data;

    AddDrawCmd(); // Force a new command after us (see comment below)
}

// Our scheme may appears a bit unusual, basically we want the most-common calls AddLine AddRect etc. to not have to perform any check so we always have a command ready in the stack.
// The cost of figuring out if a new command has to be added or if we can merge is paid in those Update** functions only.
void ImDrawList::UpdateClipRect()
{
    // If current command is used with different settings we need to add a new command
    const ImVec4 curr_clip_rect = GetCurrentClipRect();
    ImDrawCmd* curr_cmd = CmdBuffer.Size > 0 ? &CmdBuffer.Data[CmdBuffer.Size - 1] : NULL;
    if (!curr_cmd || (curr_cmd->ElemCount != 0 && memcmp(&curr_cmd->ClipRect, &curr_clip_rect, sizeof(ImVec4)) != 0) || curr_cmd->UserCallback != NULL)
    {
        AddDrawCmd();
        return;
    }

    // Try to merge with previous command if it matches, else use current command
    ImDrawCmd* prev_cmd = CmdBuffer.Size > 1 ? curr_cmd - 1 : NULL;
    if (curr_cmd->ElemCount == 0 && prev_cmd && memcmp(&prev_cmd->ClipRect, &curr_clip_rect, sizeof(ImVec4)) == 0 && prev_cmd->TextureId == GetCurrentTextureId() && prev_cmd->UserCallback == NULL)
        CmdBuffer.pop_back();
    else
        curr_cmd->ClipRect = curr_clip_rect;
}

void ImDrawList::UpdateTextureID()
{
    // If current command is used with different settings we need to add a new command
    const ImTextureID curr_texture_id = GetCurrentTextureId();
    ImDrawCmd* curr_cmd = CmdBuffer.Size ? &CmdBuffer.back() : NULL;
    if (!curr_cmd || (curr_cmd->ElemCount != 0 && curr_cmd->TextureId != curr_texture_id) || curr_cmd->UserCallback != NULL)
    {
        AddDrawCmd();
        return;
    }

    // Try to merge with previous command if it matches, else use current command
    ImDrawCmd* prev_cmd = CmdBuffer.Size > 1 ? curr_cmd - 1 : NULL;
    if (curr_cmd->ElemCount == 0 && prev_cmd && prev_cmd->TextureId == curr_texture_id && memcmp(&prev_cmd->ClipRect, &GetCurrentClipRect(), sizeof(ImVec4)) == 0 && prev_cmd->UserCallback == NULL)
        CmdBuffer.pop_back();
    else
        curr_cmd->TextureId = curr_texture_id;
}

#undef GetCurrentClipRect
#undef GetCurrentTextureId

// Render-level scissoring. This is passed down to your render function but not used for CPU-side coarse clipping. Prefer using higher-level ImGui::PushClipRect() to affect logic (hit-testing and widget culling)
void ImDrawList::PushClipRect(ImVec2 cr_min, ImVec2 cr_max, bool intersect_with_current_clip_rect)
{
    ImVec4 cr(cr_min.x, cr_min.y, cr_max.x, cr_max.y);
    if (intersect_with_current_clip_rect && _ClipRectStack.Size)
    {
        ImVec4 current = _ClipRectStack.Data[_ClipRectStack.Size - 1];
        if (cr.x < current.x) cr.x = current.x;
        if (cr.y < current.y) cr.y = current.y;
        if (cr.z > current.z) cr.z = current.z;
        if (cr.w > current.w) cr.w = current.w;
    }
    cr.z = ImMax(cr.x, cr.z);
    cr.w = ImMax(cr.y, cr.w);

    _ClipRectStack.push_back(cr);
    UpdateClipRect();
}

void ImDrawList::PushClipRectFullScreen()
{
    PushClipRect(ImVec2(_Data->ClipRectFullscreen.x, _Data->ClipRectFullscreen.y), ImVec2(_Data->ClipRectFullscreen.z, _Data->ClipRectFullscreen.w));
}

void ImDrawList::PopClipRect()
{
    IM_ASSERT(_ClipRectStack.Size > 0);
    _ClipRectStack.pop_back();
    UpdateClipRect();
}

void ImDrawList::PushTextureID(ImTextureID texture_id)
{
    _TextureIdStack.push_back(texture_id);
    UpdateTextureID();
}

void ImDrawList::PopTextureID()
{
    IM_ASSERT(_TextureIdStack.Size > 0);
    _TextureIdStack.pop_back();
    UpdateTextureID();
}

// Reserve space for a number of vertices and indices.
// You must finish filling your reserved data before calling PrimReserve() again, as it may reallocate or
// submit the intermediate results. PrimUnreserve() can be used to release unused allocations.
void ImDrawList::PrimReserve(int idx_count, int vtx_count)
{
    // Large mesh support (when enabled)
    IM_ASSERT_PARANOID(idx_count >= 0 && vtx_count >= 0);
    if (sizeof(ImDrawIdx) == 2 && (_VtxCurrentIdx + vtx_count >= (1 << 16)) && (Flags & ImDrawListFlags_AllowVtxOffset))
    {
        _VtxCurrentOffset = VtxBuffer.Size;
        _VtxCurrentIdx = 0;
        AddDrawCmd();
    }

    ImDrawCmd& draw_cmd = CmdBuffer.Data[CmdBuffer.Size - 1];
    draw_cmd.ElemCount += idx_count;

    int vtx_buffer_old_size = VtxBuffer.Size;
    VtxBuffer.resize(vtx_buffer_old_size + vtx_count);
    _VtxWritePtr = VtxBuffer.Data + vtx_buffer_old_size;

    int idx_buffer_old_size = IdxBuffer.Size;
    IdxBuffer.resize(idx_buffer_old_size + idx_count);
    _IdxWritePtr = IdxBuffer.Data + idx_buffer_old_size;
}

// Release the a number of reserved vertices/indices from the end of the last reservation made with PrimReserve().
void ImDrawList::PrimUnreserve(int idx_count, int vtx_count)
{
    IM_ASSERT_PARANOID(idx_count >= 0 && vtx_count >= 0);

    ImDrawCmd& draw_cmd = CmdBuffer.Data[CmdBuffer.Size - 1];
    draw_cmd.ElemCount -= idx_count;
    VtxBuffer.shrink(VtxBuffer.Size - vtx_count);
    IdxBuffer.shrink(IdxBuffer.Size - idx_count);
}

// Fully unrolled with inline call to keep our debug builds decently fast.
void ImDrawList::PrimRect(const ImVec2& a, const ImVec2& c, ImU32 col)
{
    ImVec2 b(c.x, a.y), d(a.x, c.y), uv(_Data->TexUvWhitePixel);
    ImDrawIdx idx = (ImDrawIdx)_VtxCurrentIdx;
    _IdxWritePtr[0] = idx; _IdxWritePtr[1] = (ImDrawIdx)(idx + 1); _IdxWritePtr[2] = (ImDrawIdx)(idx + 2);
    _IdxWritePtr[3] = idx; _IdxWritePtr[4] = (ImDrawIdx)(idx + 2); _IdxWritePtr[5] = (ImDrawIdx)(idx + 3);
    _VtxWritePtr[0].pos = a; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
    _VtxWritePtr[1].pos = b; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col;
    _VtxWritePtr[2].pos = c; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col;
    _VtxWritePtr[3].pos = d; _VtxWritePtr[3].uv = uv; _VtxWritePtr[3].col = col;
    _VtxWritePtr += 4;
    _VtxCurrentIdx += 4;
    _IdxWritePtr += 6;
}

void ImDrawList::PrimRectUV(const ImVec2& a, const ImVec2& c, const ImVec2& uv_a, const ImVec2& uv_c, ImU32 col)
{
    ImVec2 b(c.x, a.y), d(a.x, c.y), uv_b(uv_c.x, uv_a.y), uv_d(uv_a.x, uv_c.y);
    ImDrawIdx idx = (ImDrawIdx)_VtxCurrentIdx;
    _IdxWritePtr[0] = idx; _IdxWritePtr[1] = (ImDrawIdx)(idx + 1); _IdxWritePtr[2] = (ImDrawIdx)(idx + 2);
    _IdxWritePtr[3] = idx; _IdxWritePtr[4] = (ImDrawIdx)(idx + 2); _IdxWritePtr[5] = (ImDrawIdx)(idx + 3);
    _VtxWritePtr[0].pos = a; _VtxWritePtr[0].uv = uv_a; _VtxWritePtr[0].col = col;
    _VtxWritePtr[1].pos = b; _VtxWritePtr[1].uv = uv_b; _VtxWritePtr[1].col = col;
    _VtxWritePtr[2].pos = c; _VtxWritePtr[2].uv = uv_c; _VtxWritePtr[2].col = col;
    _VtxWritePtr[3].pos = d; _VtxWritePtr[3].uv = uv_d; _VtxWritePtr[3].col = col;
    _VtxWritePtr += 4;
    _VtxCurrentIdx += 4;
    _IdxWritePtr += 6;
}

void ImDrawList::PrimQuadUV(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, const ImVec2& uv_a, const ImVec2& uv_b, const ImVec2& uv_c, const ImVec2& uv_d, ImU32 col)
{
    ImDrawIdx idx = (ImDrawIdx)_VtxCurrentIdx;
    _IdxWritePtr[0] = idx; _IdxWritePtr[1] = (ImDrawIdx)(idx + 1); _IdxWritePtr[2] = (ImDrawIdx)(idx + 2);
    _IdxWritePtr[3] = idx; _IdxWritePtr[4] = (ImDrawIdx)(idx + 2); _IdxWritePtr[5] = (ImDrawIdx)(idx + 3);
    _VtxWritePtr[0].pos = a; _VtxWritePtr[0].uv = uv_a; _VtxWritePtr[0].col = col;
    _VtxWritePtr[1].pos = b; _VtxWritePtr[1].uv = uv_b; _VtxWritePtr[1].col = col;
    _VtxWritePtr[2].pos = c; _VtxWritePtr[2].uv = uv_c; _VtxWritePtr[2].col = col;
    _VtxWritePtr[3].pos = d; _VtxWritePtr[3].uv = uv_d; _VtxWritePtr[3].col = col;
    _VtxWritePtr += 4;
    _VtxCurrentIdx += 4;
    _IdxWritePtr += 6;
}

// On AddPolyline() and AddConvexPolyFilled() we intentionally avoid using ImVec2 and superflous function calls to optimize debug/non-inlined builds.
// Those macros expects l-values.
#define IM_NORMALIZE2F_OVER_ZERO(VX,VY)     do { float d2 = VX*VX + VY*VY; if (d2 > 0.0f) { float inv_len = 1.0f / ImSqrt(d2); VX *= inv_len; VY *= inv_len; } } while (0)
#define IM_FIXNORMAL2F(VX,VY)               do { float d2 = VX*VX + VY*VY; if (d2 < 0.5f) d2 = 0.5f; float inv_lensq = 1.0f / d2; VX *= inv_lensq; VY *= inv_lensq; } while (0)

// TODO: Thickness anti-aliased lines cap are missing their AA fringe.
// We avoid using the ImVec2 math operators here to reduce cost to a minimum for debug/non-inlined builds.
void ImDrawList::AddPolyline(const ImVec2* points, const int points_count, ImU32 col, bool closed, float thickness)
{
    if (points_count < 2)
        return;

    const ImVec2 uv = _Data->TexUvWhitePixel;

    int count = points_count;
    if (!closed)
        count = points_count - 1;

    const bool thick_line = thickness > 1.0f;
    if (Flags & ImDrawListFlags_AntiAliasedLines)
    {
        // Anti-aliased stroke
        const float AA_SIZE = 1.0f;
        const ImU32 col_trans = col & ~IM_COL32_A_MASK;

        const int idx_count = thick_line ? count * 18 : count * 12;
        const int vtx_count = thick_line ? points_count * 4 : points_count * 3;
        PrimReserve(idx_count, vtx_count);

        // Temporary buffer
        ImVec2* temp_normals = (ImVec2*)alloca(points_count * (thick_line ? 5 : 3) * sizeof(ImVec2)); //-V630
        ImVec2* temp_points = temp_normals + points_count;

        for (int i1 = 0; i1 < count; i1++)
        {
            const int i2 = (i1 + 1) == points_count ? 0 : i1 + 1;
            float dx = points[i2].x - points[i1].x;
            float dy = points[i2].y - points[i1].y;
            IM_NORMALIZE2F_OVER_ZERO(dx, dy);
            temp_normals[i1].x = dy;
            temp_normals[i1].y = -dx;
        }
        if (!closed)
            temp_normals[points_count - 1] = temp_normals[points_count - 2];

        if (!thick_line)
        {
            if (!closed)
            {
                temp_points[0] = points[0] + temp_normals[0] * AA_SIZE;
                temp_points[1] = points[0] - temp_normals[0] * AA_SIZE;
                temp_points[(points_count - 1) * 2 + 0] = points[points_count - 1] + temp_normals[points_count - 1] * AA_SIZE;
                temp_points[(points_count - 1) * 2 + 1] = points[points_count - 1] - temp_normals[points_count - 1] * AA_SIZE;
            }

            // FIXME-OPT: Merge the different loops, possibly remove the temporary buffer.
            unsigned int idx1 = _VtxCurrentIdx;
            for (int i1 = 0; i1 < count; i1++)
            {
                const int i2 = (i1 + 1) == points_count ? 0 : i1 + 1;
                unsigned int idx2 = (i1 + 1) == points_count ? _VtxCurrentIdx : idx1 + 3;

                // Average normals
                float dm_x = (temp_normals[i1].x + temp_normals[i2].x) * 0.5f;
                float dm_y = (temp_normals[i1].y + temp_normals[i2].y) * 0.5f;
                IM_FIXNORMAL2F(dm_x, dm_y);
                dm_x *= AA_SIZE;
                dm_y *= AA_SIZE;

                // Add temporary vertexes
                ImVec2* out_vtx = &temp_points[i2 * 2];
                out_vtx[0].x = points[i2].x + dm_x;
                out_vtx[0].y = points[i2].y + dm_y;
                out_vtx[1].x = points[i2].x - dm_x;
                out_vtx[1].y = points[i2].y - dm_y;

                // Add indexes
                _IdxWritePtr[0] = (ImDrawIdx)(idx2 + 0); _IdxWritePtr[1] = (ImDrawIdx)(idx1 + 0); _IdxWritePtr[2] = (ImDrawIdx)(idx1 + 2);
                _IdxWritePtr[3] = (ImDrawIdx)(idx1 + 2); _IdxWritePtr[4] = (ImDrawIdx)(idx2 + 2); _IdxWritePtr[5] = (ImDrawIdx)(idx2 + 0);
                _IdxWritePtr[6] = (ImDrawIdx)(idx2 + 1); _IdxWritePtr[7] = (ImDrawIdx)(idx1 + 1); _IdxWritePtr[8] = (ImDrawIdx)(idx1 + 0);
                _IdxWritePtr[9] = (ImDrawIdx)(idx1 + 0); _IdxWritePtr[10] = (ImDrawIdx)(idx2 + 0); _IdxWritePtr[11] = (ImDrawIdx)(idx2 + 1);
                _IdxWritePtr += 12;

                idx1 = idx2;
            }

            // Add vertexes
            for (int i = 0; i < points_count; i++)
            {
                _VtxWritePtr[0].pos = points[i];          _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
                _VtxWritePtr[1].pos = temp_points[i * 2 + 0]; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col_trans;
                _VtxWritePtr[2].pos = temp_points[i * 2 + 1]; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col_trans;
                _VtxWritePtr += 3;
            }
        }
        else
        {
            const float half_inner_thickness = (thickness - AA_SIZE) * 0.5f;
            if (!closed)
            {
                temp_points[0] = points[0] + temp_normals[0] * (half_inner_thickness + AA_SIZE);
                temp_points[1] = points[0] + temp_normals[0] * (half_inner_thickness);
                temp_points[2] = points[0] - temp_normals[0] * (half_inner_thickness);
                temp_points[3] = points[0] - temp_normals[0] * (half_inner_thickness + AA_SIZE);
                temp_points[(points_count - 1) * 4 + 0] = points[points_count - 1] + temp_normals[points_count - 1] * (half_inner_thickness + AA_SIZE);
                temp_points[(points_count - 1) * 4 + 1] = points[points_count - 1] + temp_normals[points_count - 1] * (half_inner_thickness);
                temp_points[(points_count - 1) * 4 + 2] = points[points_count - 1] - temp_normals[points_count - 1] * (half_inner_thickness);
                temp_points[(points_count - 1) * 4 + 3] = points[points_count - 1] - temp_normals[points_count - 1] * (half_inner_thickness + AA_SIZE);
            }

            // FIXME-OPT: Merge the different loops, possibly remove the temporary buffer.
            unsigned int idx1 = _VtxCurrentIdx;
            for (int i1 = 0; i1 < count; i1++)
            {
                const int i2 = (i1 + 1) == points_count ? 0 : i1 + 1;
                unsigned int idx2 = (i1 + 1) == points_count ? _VtxCurrentIdx : idx1 + 4;

                // Average normals
                float dm_x = (temp_normals[i1].x + temp_normals[i2].x) * 0.5f;
                float dm_y = (temp_normals[i1].y + temp_normals[i2].y) * 0.5f;
                IM_FIXNORMAL2F(dm_x, dm_y);
                float dm_out_x = dm_x * (half_inner_thickness + AA_SIZE);
                float dm_out_y = dm_y * (half_inner_thickness + AA_SIZE);
                float dm_in_x = dm_x * half_inner_thickness;
                float dm_in_y = dm_y * half_inner_thickness;

                // Add temporary vertexes
                ImVec2* out_vtx = &temp_points[i2 * 4];
                out_vtx[0].x = points[i2].x + dm_out_x;
                out_vtx[0].y = points[i2].y + dm_out_y;
                out_vtx[1].x = points[i2].x + dm_in_x;
                out_vtx[1].y = points[i2].y + dm_in_y;
                out_vtx[2].x = points[i2].x - dm_in_x;
                out_vtx[2].y = points[i2].y - dm_in_y;
                out_vtx[3].x = points[i2].x - dm_out_x;
                out_vtx[3].y = points[i2].y - dm_out_y;

                // Add indexes
                _IdxWritePtr[0] = (ImDrawIdx)(idx2 + 1); _IdxWritePtr[1] = (ImDrawIdx)(idx1 + 1); _IdxWritePtr[2] = (ImDrawIdx)(idx1 + 2);
                _IdxWritePtr[3] = (ImDrawIdx)(idx1 + 2); _IdxWritePtr[4] = (ImDrawIdx)(idx2 + 2); _IdxWritePtr[5] = (ImDrawIdx)(idx2 + 1);
                _IdxWritePtr[6] = (ImDrawIdx)(idx2 + 1); _IdxWritePtr[7] = (ImDrawIdx)(idx1 + 1); _IdxWritePtr[8] = (ImDrawIdx)(idx1 + 0);
                _IdxWritePtr[9] = (ImDrawIdx)(idx1 + 0); _IdxWritePtr[10] = (ImDrawIdx)(idx2 + 0); _IdxWritePtr[11] = (ImDrawIdx)(idx2 + 1);
                _IdxWritePtr[12] = (ImDrawIdx)(idx2 + 2); _IdxWritePtr[13] = (ImDrawIdx)(idx1 + 2); _IdxWritePtr[14] = (ImDrawIdx)(idx1 + 3);
                _IdxWritePtr[15] = (ImDrawIdx)(idx1 + 3); _IdxWritePtr[16] = (ImDrawIdx)(idx2 + 3); _IdxWritePtr[17] = (ImDrawIdx)(idx2 + 2);
                _IdxWritePtr += 18;

                idx1 = idx2;
            }

            // Add vertexes
            for (int i = 0; i < points_count; i++)
            {
                _VtxWritePtr[0].pos = temp_points[i * 4 + 0]; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col_trans;
                _VtxWritePtr[1].pos = temp_points[i * 4 + 1]; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col;
                _VtxWritePtr[2].pos = temp_points[i * 4 + 2]; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col;
                _VtxWritePtr[3].pos = temp_points[i * 4 + 3]; _VtxWritePtr[3].uv = uv; _VtxWritePtr[3].col = col_trans;
                _VtxWritePtr += 4;
            }
        }
        _VtxCurrentIdx += (ImDrawIdx)vtx_count;
    }
    else
    {
        // Non Anti-aliased Stroke
        const int idx_count = count * 6;
        const int vtx_count = count * 4;      // FIXME-OPT: Not sharing edges
        PrimReserve(idx_count, vtx_count);

        for (int i1 = 0; i1 < count; i1++)
        {
            const int i2 = (i1 + 1) == points_count ? 0 : i1 + 1;
            const ImVec2& p1 = points[i1];
            const ImVec2& p2 = points[i2];

            float dx = p2.x - p1.x;
            float dy = p2.y - p1.y;
            IM_NORMALIZE2F_OVER_ZERO(dx, dy);
            dx *= (thickness * 0.5f);
            dy *= (thickness * 0.5f);

            _VtxWritePtr[0].pos.x = p1.x + dy; _VtxWritePtr[0].pos.y = p1.y - dx; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
            _VtxWritePtr[1].pos.x = p2.x + dy; _VtxWritePtr[1].pos.y = p2.y - dx; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col;
            _VtxWritePtr[2].pos.x = p2.x - dy; _VtxWritePtr[2].pos.y = p2.y + dx; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col;
            _VtxWritePtr[3].pos.x = p1.x - dy; _VtxWritePtr[3].pos.y = p1.y + dx; _VtxWritePtr[3].uv = uv; _VtxWritePtr[3].col = col;
            _VtxWritePtr += 4;

            _IdxWritePtr[0] = (ImDrawIdx)(_VtxCurrentIdx); _IdxWritePtr[1] = (ImDrawIdx)(_VtxCurrentIdx + 1); _IdxWritePtr[2] = (ImDrawIdx)(_VtxCurrentIdx + 2);
            _IdxWritePtr[3] = (ImDrawIdx)(_VtxCurrentIdx); _IdxWritePtr[4] = (ImDrawIdx)(_VtxCurrentIdx + 2); _IdxWritePtr[5] = (ImDrawIdx)(_VtxCurrentIdx + 3);
            _IdxWritePtr += 6;
            _VtxCurrentIdx += 4;
        }
    }
}

// We intentionally avoid using ImVec2 and its math operators here to reduce cost to a minimum for debug/non-inlined builds.
void ImDrawList::AddConvexPolyFilled(const ImVec2* points, const int points_count, ImU32 col)
{
    if (points_count < 3)
        return;

    const ImVec2 uv = _Data->TexUvWhitePixel;

    if (Flags & ImDrawListFlags_AntiAliasedFill)
    {
        // Anti-aliased Fill
        const float AA_SIZE = 1.0f;
        const ImU32 col_trans = col & ~IM_COL32_A_MASK;
        const int idx_count = (points_count - 2) * 3 + points_count * 6;
        const int vtx_count = (points_count * 2);
        PrimReserve(idx_count, vtx_count);

        // Add indexes for fill
        unsigned int vtx_inner_idx = _VtxCurrentIdx;
        unsigned int vtx_outer_idx = _VtxCurrentIdx + 1;
        for (int i = 2; i < points_count; i++)
        {
            _IdxWritePtr[0] = (ImDrawIdx)(vtx_inner_idx); _IdxWritePtr[1] = (ImDrawIdx)(vtx_inner_idx + ((i - 1) << 1)); _IdxWritePtr[2] = (ImDrawIdx)(vtx_inner_idx + (i << 1));
            _IdxWritePtr += 3;
        }

        // Compute normals
        ImVec2* temp_normals = (ImVec2*)alloca(points_count * sizeof(ImVec2)); //-V630
        for (int i0 = points_count - 1, i1 = 0; i1 < points_count; i0 = i1++)
        {
            const ImVec2& p0 = points[i0];
            const ImVec2& p1 = points[i1];
            float dx = p1.x - p0.x;
            float dy = p1.y - p0.y;
            IM_NORMALIZE2F_OVER_ZERO(dx, dy);
            temp_normals[i0].x = dy;
            temp_normals[i0].y = -dx;
        }

        for (int i0 = points_count - 1, i1 = 0; i1 < points_count; i0 = i1++)
        {
            // Average normals
            const ImVec2& n0 = temp_normals[i0];
            const ImVec2& n1 = temp_normals[i1];
            float dm_x = (n0.x + n1.x) * 0.5f;
            float dm_y = (n0.y + n1.y) * 0.5f;
            IM_FIXNORMAL2F(dm_x, dm_y);
            dm_x *= AA_SIZE * 0.5f;
            dm_y *= AA_SIZE * 0.5f;

            // Add vertices
            _VtxWritePtr[0].pos.x = (points[i1].x - dm_x); _VtxWritePtr[0].pos.y = (points[i1].y - dm_y); _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;        // Inner
            _VtxWritePtr[1].pos.x = (points[i1].x + dm_x); _VtxWritePtr[1].pos.y = (points[i1].y + dm_y); _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col_trans;  // Outer
            _VtxWritePtr += 2;

            // Add indexes for fringes
            _IdxWritePtr[0] = (ImDrawIdx)(vtx_inner_idx + (i1 << 1)); _IdxWritePtr[1] = (ImDrawIdx)(vtx_inner_idx + (i0 << 1)); _IdxWritePtr[2] = (ImDrawIdx)(vtx_outer_idx + (i0 << 1));
            _IdxWritePtr[3] = (ImDrawIdx)(vtx_outer_idx + (i0 << 1)); _IdxWritePtr[4] = (ImDrawIdx)(vtx_outer_idx + (i1 << 1)); _IdxWritePtr[5] = (ImDrawIdx)(vtx_inner_idx + (i1 << 1));
            _IdxWritePtr += 6;
        }
        _VtxCurrentIdx += (ImDrawIdx)vtx_count;
    }
    else
    {
        // Non Anti-aliased Fill
        const int idx_count = (points_count - 2) * 3;
        const int vtx_count = points_count;
        PrimReserve(idx_count, vtx_count);
        for (int i = 0; i < vtx_count; i++)
        {
            _VtxWritePtr[0].pos = points[i]; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
            _VtxWritePtr++;
        }
        for (int i = 2; i < points_count; i++)
        {
            _IdxWritePtr[0] = (ImDrawIdx)(_VtxCurrentIdx); _IdxWritePtr[1] = (ImDrawIdx)(_VtxCurrentIdx + i - 1); _IdxWritePtr[2] = (ImDrawIdx)(_VtxCurrentIdx + i);
            _IdxWritePtr += 3;
        }
        _VtxCurrentIdx += (ImDrawIdx)vtx_count;
    }
}

void ImDrawList::PathArcToFast(const ImVec2& center, float radius, int a_min_of_12, int a_max_of_12)
{
    if (radius == 0.0f || a_min_of_12 > a_max_of_12)
    {
        _Path.push_back(center);
        return;
    }

    // For legacy reason the PathArcToFast() always takes angles where 2*PI is represented by 12,
    // but it is possible to set IM_DRAWLIST_ARCFAST_TESSELATION_MULTIPLIER to a higher value. This should compile to a no-op otherwise.
#if IM_DRAWLIST_ARCFAST_TESSELLATION_MULTIPLIER != 1
    a_min_of_12 *= IM_DRAWLIST_ARCFAST_TESSELLATION_MULTIPLIER;
    a_max_of_12 *= IM_DRAWLIST_ARCFAST_TESSELLATION_MULTIPLIER;
#endif

    _Path.reserve(_Path.Size + (a_max_of_12 - a_min_of_12 + 1));
    for (int a = a_min_of_12; a <= a_max_of_12; a++)
    {
        const ImVec2& c = _Data->ArcFastVtx[a % IM_ARRAYSIZE(_Data->ArcFastVtx)];
        _Path.push_back(ImVec2(center.x + c.x * radius, center.y + c.y * radius));
    }
}

void ImDrawList::PathArcTo(const ImVec2& center, float radius, float a_min, float a_max, int num_segments)
{
    if (radius == 0.0f)
    {
        _Path.push_back(center);
        return;
    }

    // Note that we are adding a point at both a_min and a_max.
    // If you are trying to draw a full closed circle you don't want the overlapping points!
    _Path.reserve(_Path.Size + (num_segments + 1));
    for (int i = 0; i <= num_segments; i++)
    {
        const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
        _Path.push_back(ImVec2(center.x + ImCos(a) * radius, center.y + ImSin(a) * radius));
    }
}

ImVec2 ImBezierCalc(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, float t)
{
    float u = 1.0f - t;
    float w1 = u * u*u;
    float w2 = 3 * u*u*t;
    float w3 = 3 * u*t*t;
    float w4 = t * t*t;
    return ImVec2(w1*p1.x + w2 * p2.x + w3 * p3.x + w4 * p4.x, w1*p1.y + w2 * p2.y + w3 * p3.y + w4 * p4.y);
}

// Closely mimics BezierClosestPointCasteljauStep() in imgui.cpp
static void PathBezierToCasteljau(ImVector<ImVec2>* path, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float tess_tol, int level)
{
    float dx = x4 - x1;
    float dy = y4 - y1;
    float d2 = ((x2 - x4) * dy - (y2 - y4) * dx);
    float d3 = ((x3 - x4) * dy - (y3 - y4) * dx);
    d2 = (d2 >= 0) ? d2 : -d2;
    d3 = (d3 >= 0) ? d3 : -d3;
    if ((d2 + d3) * (d2 + d3) < tess_tol * (dx*dx + dy * dy))
    {
        path->push_back(ImVec2(x4, y4));
    }
    else if (level < 10)
    {
        float x12 = (x1 + x2)*0.5f, y12 = (y1 + y2)*0.5f;
        float x23 = (x2 + x3)*0.5f, y23 = (y2 + y3)*0.5f;
        float x34 = (x3 + x4)*0.5f, y34 = (y3 + y4)*0.5f;
        float x123 = (x12 + x23)*0.5f, y123 = (y12 + y23)*0.5f;
        float x234 = (x23 + x34)*0.5f, y234 = (y23 + y34)*0.5f;
        float x1234 = (x123 + x234)*0.5f, y1234 = (y123 + y234)*0.5f;
        PathBezierToCasteljau(path, x1, y1, x12, y12, x123, y123, x1234, y1234, tess_tol, level + 1);
        PathBezierToCasteljau(path, x1234, y1234, x234, y234, x34, y34, x4, y4, tess_tol, level + 1);
    }
}

void ImDrawList::PathBezierCurveTo(const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, int num_segments)
{
    ImVec2 p1 = _Path.back();
    if (num_segments == 0)
    {
        PathBezierToCasteljau(&_Path, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y, _Data->CurveTessellationTol, 0); // Auto-tessellated
    }
    else
    {
        float t_step = 1.0f / (float)num_segments;
        for (int i_step = 1; i_step <= num_segments; i_step++)
            _Path.push_back(ImBezierCalc(p1, p2, p3, p4, t_step * i_step));
    }
}

void ImDrawList::PathRect(const ImVec2& a, const ImVec2& b, float rounding, ImDrawCornerFlags rounding_corners)
{
    rounding = ImMin(rounding, ImFabs(b.x - a.x) * (((rounding_corners & ImDrawCornerFlags_Top) == ImDrawCornerFlags_Top) || ((rounding_corners & ImDrawCornerFlags_Bot) == ImDrawCornerFlags_Bot) ? 0.5f : 1.0f) - 1.0f);
    rounding = ImMin(rounding, ImFabs(b.y - a.y) * (((rounding_corners & ImDrawCornerFlags_Left) == ImDrawCornerFlags_Left) || ((rounding_corners & ImDrawCornerFlags_Right) == ImDrawCornerFlags_Right) ? 0.5f : 1.0f) - 1.0f);

    if (rounding <= 0.0f || rounding_corners == 0)
    {
        PathLineTo(a);
        PathLineTo(ImVec2(b.x, a.y));
        PathLineTo(b);
        PathLineTo(ImVec2(a.x, b.y));
    }
    else
    {
        const float rounding_tl = (rounding_corners & ImDrawCornerFlags_TopLeft) ? rounding : 0.0f;
        const float rounding_tr = (rounding_corners & ImDrawCornerFlags_TopRight) ? rounding : 0.0f;
        const float rounding_br = (rounding_corners & ImDrawCornerFlags_BotRight) ? rounding : 0.0f;
        const float rounding_bl = (rounding_corners & ImDrawCornerFlags_BotLeft) ? rounding : 0.0f;
        PathArcToFast(ImVec2(a.x + rounding_tl, a.y + rounding_tl), rounding_tl, 6, 9);
        PathArcToFast(ImVec2(b.x - rounding_tr, a.y + rounding_tr), rounding_tr, 9, 12);
        PathArcToFast(ImVec2(b.x - rounding_br, b.y - rounding_br), rounding_br, 0, 3);
        PathArcToFast(ImVec2(a.x + rounding_bl, b.y - rounding_bl), rounding_bl, 3, 6);
    }
}

void ImDrawList::AddLine(const ImVec2& p1, const ImVec2& p2, ImU32 col, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;
    PathLineTo(p1 + ImVec2(0.5f, 0.5f));
    PathLineTo(p2 + ImVec2(0.5f, 0.5f));
    PathStroke(col, false, thickness);
}

// p_min = upper-left, p_max = lower-right
// Note we don't render 1 pixels sized rectangles properly.
void ImDrawList::AddRect(const ImVec2& p_min, const ImVec2& p_max, ImU32 col, float rounding, ImDrawCornerFlags rounding_corners, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;
    if (Flags & ImDrawListFlags_AntiAliasedLines)
        PathRect(p_min + ImVec2(0.50f, 0.50f), p_max - ImVec2(0.50f, 0.50f), rounding, rounding_corners);
    else
        PathRect(p_min + ImVec2(0.50f, 0.50f), p_max - ImVec2(0.49f, 0.49f), rounding, rounding_corners); // Better looking lower-right corner and rounded non-AA shapes.
    PathStroke(col, true, thickness);
}

void ImDrawList::AddRectFilled(const ImVec2& p_min, const ImVec2& p_max, ImU32 col, float rounding, ImDrawCornerFlags rounding_corners)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;
    if (rounding > 0.0f)
    {
        PathRect(p_min, p_max, rounding, rounding_corners);
        PathFillConvex(col);
    }
    else
    {
        PrimReserve(6, 4);
        PrimRect(p_min, p_max, col);
    }
}

// p_min = upper-left, p_max = lower-right
void ImDrawList::AddRectFilledMultiColor(const ImVec2& p_min, const ImVec2& p_max, ImU32 col_upr_left, ImU32 col_upr_right, ImU32 col_bot_right, ImU32 col_bot_left)
{
    if (((col_upr_left | col_upr_right | col_bot_right | col_bot_left) & IM_COL32_A_MASK) == 0)
        return;

    const ImVec2 uv = _Data->TexUvWhitePixel;
    PrimReserve(6, 4);
    PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx + 1)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx + 2));
    PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx + 2)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx + 3));
    PrimWriteVtx(p_min, uv, col_upr_left);
    PrimWriteVtx(ImVec2(p_max.x, p_min.y), uv, col_upr_right);
    PrimWriteVtx(p_max, uv, col_bot_right);
    PrimWriteVtx(ImVec2(p_min.x, p_max.y), uv, col_bot_left);
}

void ImDrawList::AddQuad(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, ImU32 col, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(p1);
    PathLineTo(p2);
    PathLineTo(p3);
    PathLineTo(p4);
    PathStroke(col, true, thickness);
}

void ImDrawList::AddQuadFilled(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, ImU32 col)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(p1);
    PathLineTo(p2);
    PathLineTo(p3);
    PathLineTo(p4);
    PathFillConvex(col);
}

void ImDrawList::AddTriangle(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, ImU32 col, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(p1);
    PathLineTo(p2);
    PathLineTo(p3);
    PathStroke(col, true, thickness);
}

void ImDrawList::AddTriangleFilled(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, ImU32 col)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(p1);
    PathLineTo(p2);
    PathLineTo(p3);
    PathFillConvex(col);
}

void ImDrawList::AddCircle(const ImVec2& center, float radius, ImU32 col, int num_segments, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0 || radius <= 0.0f)
        return;

    // Obtain segment count
    if (num_segments <= 0)
    {
        // Automatic segment count
        const int radius_idx = (int)radius - 1;
        if (radius_idx < IM_ARRAYSIZE(_Data->CircleSegmentCounts))
            num_segments = _Data->CircleSegmentCounts[radius_idx]; // Use cached value
        else
            num_segments = IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC(radius, _Data->CircleSegmentMaxError);
    }
    else
    {
        // Explicit segment count (still clamp to avoid drawing insanely tessellated shapes)
        num_segments = ImClamp(num_segments, 3, IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_MAX);
    }

    // Because we are filling a closed shape we remove 1 from the count of segments/points
    const float a_max = (IM_PI * 2.0f) * ((float)num_segments - 1.0f) / (float)num_segments;
    if (num_segments == 12)
        PathArcToFast(center, radius - 0.5f, 0, 12);
    else
        PathArcTo(center, radius - 0.5f, 0.0f, a_max, num_segments - 1);
    PathStroke(col, true, thickness);
}

void ImDrawList::AddCircleFilled(const ImVec2& center, float radius, ImU32 col, int num_segments)
{
    if ((col & IM_COL32_A_MASK) == 0 || radius <= 0.0f)
        return;

    // Obtain segment count
    if (num_segments <= 0)
    {
        // Automatic segment count
        const int radius_idx = (int)radius - 1;
        if (radius_idx < IM_ARRAYSIZE(_Data->CircleSegmentCounts))
            num_segments = _Data->CircleSegmentCounts[radius_idx]; // Use cached value
        else
            num_segments = IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC(radius, _Data->CircleSegmentMaxError);
    }
    else
    {
        // Explicit segment count (still clamp to avoid drawing insanely tessellated shapes)
        num_segments = ImClamp(num_segments, 3, IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_MAX);
    }

    // Because we are filling a closed shape we remove 1 from the count of segments/points
    const float a_max = (IM_PI * 2.0f) * ((float)num_segments - 1.0f) / (float)num_segments;
    if (num_segments == 12)
        PathArcToFast(center, radius, 0, 12);
    else
        PathArcTo(center, radius, 0.0f, a_max, num_segments - 1);
    PathFillConvex(col);
}

// Guaranteed to honor 'num_segments'
void ImDrawList::AddNgon(const ImVec2& center, float radius, ImU32 col, int num_segments, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0 || num_segments <= 2)
        return;

    // Because we are filling a closed shape we remove 1 from the count of segments/points
    const float a_max = (IM_PI * 2.0f) * ((float)num_segments - 1.0f) / (float)num_segments;
    PathArcTo(center, radius - 0.5f, 0.0f, a_max, num_segments - 1);
    PathStroke(col, true, thickness);
}

// Guaranteed to honor 'num_segments'
void ImDrawList::AddNgonFilled(const ImVec2& center, float radius, ImU32 col, int num_segments)
{
    if ((col & IM_COL32_A_MASK) == 0 || num_segments <= 2)
        return;

    // Because we are filling a closed shape we remove 1 from the count of segments/points
    const float a_max = (IM_PI * 2.0f) * ((float)num_segments - 1.0f) / (float)num_segments;
    PathArcTo(center, radius, 0.0f, a_max, num_segments - 1);
    PathFillConvex(col);
}

// Cubic Bezier takes 4 controls points
void ImDrawList::AddBezierCurve(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, ImU32 col, float thickness, int num_segments)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(p1);
    PathBezierCurveTo(p2, p3, p4, num_segments);
    PathStroke(col, false, thickness);
}

void ImDrawList::AddText(const ImFont* font, float font_size, const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end, float wrap_width, const ImVec4* cpu_fine_clip_rect)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    if (text_end == NULL)
        text_end = text_begin + strlen(text_begin);
    if (text_begin == text_end)
        return;

    // Pull default font/size from the shared ImDrawListSharedData instance
    if (font == NULL)
        font = _Data->Font;
    if (font_size == 0.0f)
        font_size = _Data->FontSize;

    IM_ASSERT(font->ContainerAtlas->TexID == _TextureIdStack.back());  // Use high-level ImGui::PushFont() or low-level ImDrawList::PushTextureId() to change font.

    ImVec4 clip_rect = _ClipRectStack.back();
    if (cpu_fine_clip_rect)
    {
        clip_rect.x = ImMax(clip_rect.x, cpu_fine_clip_rect->x);
        clip_rect.y = ImMax(clip_rect.y, cpu_fine_clip_rect->y);
        clip_rect.z = ImMin(clip_rect.z, cpu_fine_clip_rect->z);
        clip_rect.w = ImMin(clip_rect.w, cpu_fine_clip_rect->w);
    }
    font->RenderText(this, font_size, pos, col, clip_rect, text_begin, text_end, wrap_width, cpu_fine_clip_rect != NULL);
}

void ImDrawList::AddText(const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end)
{
    AddText(NULL, 0.0f, pos, col, text_begin, text_end);
}

void ImDrawList::AddImage(ImTextureID user_texture_id, const ImVec2& p_min, const ImVec2& p_max, const ImVec2& uv_min, const ImVec2& uv_max, ImU32 col)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    const bool push_texture_id = _TextureIdStack.empty() || user_texture_id != _TextureIdStack.back();
    if (push_texture_id)
        PushTextureID(user_texture_id);

    PrimReserve(6, 4);
    PrimRectUV(p_min, p_max, uv_min, uv_max, col);

    if (push_texture_id)
        PopTextureID();
}

void ImDrawList::AddImageQuad(ImTextureID user_texture_id, const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, const ImVec2& uv1, const ImVec2& uv2, const ImVec2& uv3, const ImVec2& uv4, ImU32 col)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    const bool push_texture_id = _TextureIdStack.empty() || user_texture_id != _TextureIdStack.back();
    if (push_texture_id)
        PushTextureID(user_texture_id);

    PrimReserve(6, 4);
    PrimQuadUV(p1, p2, p3, p4, uv1, uv2, uv3, uv4, col);

    if (push_texture_id)
        PopTextureID();
}

void ImDrawList::AddImageRounded(ImTextureID user_texture_id, const ImVec2& p_min, const ImVec2& p_max, const ImVec2& uv_min, const ImVec2& uv_max, ImU32 col, float rounding, ImDrawCornerFlags rounding_corners)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    if (rounding <= 0.0f || (rounding_corners & ImDrawCornerFlags_All) == 0)
    {
        AddImage(user_texture_id, p_min, p_max, uv_min, uv_max, col);
        return;
    }

    const bool push_texture_id = _TextureIdStack.empty() || user_texture_id != _TextureIdStack.back();
    if (push_texture_id)
        PushTextureID(user_texture_id);

    int vert_start_idx = VtxBuffer.Size;
    PathRect(p_min, p_max, rounding, rounding_corners);
    PathFillConvex(col);
    int vert_end_idx = VtxBuffer.Size;
    ImGui::ShadeVertsLinearUV(this, vert_start_idx, vert_end_idx, p_min, p_max, uv_min, uv_max, true);

    if (push_texture_id)
        PopTextureID();
}


//-----------------------------------------------------------------------------
// ImDrawListSplitter
//-----------------------------------------------------------------------------
// FIXME: This may be a little confusing, trying to be a little too low-level/optimal instead of just doing vector swap..
//-----------------------------------------------------------------------------

void ImDrawListSplitter::ClearFreeMemory()
{
    for (int i = 0; i < _Channels.Size; i++)
    {
        if (i == _Current)
            memset(&_Channels[i], 0, sizeof(_Channels[i]));  // Current channel is a copy of CmdBuffer/IdxBuffer, don't destruct again
        _Channels[i]._CmdBuffer.clear();
        _Channels[i]._IdxBuffer.clear();
    }
    _Current = 0;
    _Count = 1;
    _Channels.clear();
}

void ImDrawListSplitter::Split(ImDrawList* draw_list, int channels_count)
{
    IM_ASSERT(_Current == 0 && _Count <= 1 && "Nested channel splitting is not supported. Please use separate instances of ImDrawListSplitter.");
    int old_channels_count = _Channels.Size;
    if (old_channels_count < channels_count)
        _Channels.resize(channels_count);
    _Count = channels_count;

    // Channels[] (24/32 bytes each) hold storage that we'll swap with draw_list->_CmdBuffer/_IdxBuffer
    // The content of Channels[0] at this point doesn't matter. We clear it to make state tidy in a debugger but we don't strictly need to.
    // When we switch to the next channel, we'll copy draw_list->_CmdBuffer/_IdxBuffer into Channels[0] and then Channels[1] into draw_list->CmdBuffer/_IdxBuffer
    memset(&_Channels[0], 0, sizeof(ImDrawChannel));
    for (int i = 1; i < channels_count; i++)
    {
        if (i >= old_channels_count)
        {
            IM_PLACEMENT_NEW(&_Channels[i]) ImDrawChannel();
        }
        else
        {
            _Channels[i]._CmdBuffer.resize(0);
            _Channels[i]._IdxBuffer.resize(0);
        }
        if (_Channels[i]._CmdBuffer.Size == 0)
        {
            ImDrawCmd draw_cmd;
            draw_cmd.ClipRect = draw_list->_ClipRectStack.back();
            draw_cmd.TextureId = draw_list->_TextureIdStack.back();
            _Channels[i]._CmdBuffer.push_back(draw_cmd);
        }
    }
}

static inline bool CanMergeDrawCommands(ImDrawCmd* a, ImDrawCmd* b)
{
    return memcmp(&a->ClipRect, &b->ClipRect, sizeof(a->ClipRect)) == 0 && a->TextureId == b->TextureId && a->VtxOffset == b->VtxOffset && !a->UserCallback && !b->UserCallback;
}

void ImDrawListSplitter::Merge(ImDrawList* draw_list)
{
    // Note that we never use or rely on channels.Size because it is merely a buffer that we never shrink back to 0 to keep all sub-buffers ready for use.
    if (_Count <= 1)
        return;

    SetCurrentChannel(draw_list, 0);
    if (draw_list->CmdBuffer.Size != 0 && draw_list->CmdBuffer.back().ElemCount == 0)
        draw_list->CmdBuffer.pop_back();

    // Calculate our final buffer sizes. Also fix the incorrect IdxOffset values in each command.
    int new_cmd_buffer_count = 0;
    int new_idx_buffer_count = 0;
    ImDrawCmd* last_cmd = (_Count > 0 && draw_list->CmdBuffer.Size > 0) ? &draw_list->CmdBuffer.back() : NULL;
    int idx_offset = last_cmd ? last_cmd->IdxOffset + last_cmd->ElemCount : 0;
    for (int i = 1; i < _Count; i++)
    {
        ImDrawChannel& ch = _Channels[i];
        if (ch._CmdBuffer.Size > 0 && ch._CmdBuffer.back().ElemCount == 0)
            ch._CmdBuffer.pop_back();
        if (ch._CmdBuffer.Size > 0 && last_cmd != NULL && CanMergeDrawCommands(last_cmd, &ch._CmdBuffer[0]))
        {
            // Merge previous channel last draw command with current channel first draw command if matching.
            last_cmd->ElemCount += ch._CmdBuffer[0].ElemCount;
            idx_offset += ch._CmdBuffer[0].ElemCount;
            ch._CmdBuffer.erase(ch._CmdBuffer.Data); // FIXME-OPT: Improve for multiple merges.
        }
        if (ch._CmdBuffer.Size > 0)
            last_cmd = &ch._CmdBuffer.back();
        new_cmd_buffer_count += ch._CmdBuffer.Size;
        new_idx_buffer_count += ch._IdxBuffer.Size;
        for (int cmd_n = 0; cmd_n < ch._CmdBuffer.Size; cmd_n++)
        {
            ch._CmdBuffer.Data[cmd_n].IdxOffset = idx_offset;
            idx_offset += ch._CmdBuffer.Data[cmd_n].ElemCount;
        }
    }
    draw_list->CmdBuffer.resize(draw_list->CmdBuffer.Size + new_cmd_buffer_count);
    draw_list->IdxBuffer.resize(draw_list->IdxBuffer.Size + new_idx_buffer_count);

    // Write commands and indices in order (they are fairly small structures, we don't copy vertices only indices)
    ImDrawCmd* cmd_write = draw_list->CmdBuffer.Data + draw_list->CmdBuffer.Size - new_cmd_buffer_count;
    ImDrawIdx* idx_write = draw_list->IdxBuffer.Data + draw_list->IdxBuffer.Size - new_idx_buffer_count;
    for (int i = 1; i < _Count; i++)
    {
        ImDrawChannel& ch = _Channels[i];
        if (int sz = ch._CmdBuffer.Size) { memcpy(cmd_write, ch._CmdBuffer.Data, sz * sizeof(ImDrawCmd)); cmd_write += sz; }
        if (int sz = ch._IdxBuffer.Size) { memcpy(idx_write, ch._IdxBuffer.Data, sz * sizeof(ImDrawIdx)); idx_write += sz; }
    }
    draw_list->_IdxWritePtr = idx_write;
    draw_list->UpdateClipRect(); // We call this instead of AddDrawCmd(), so that empty channels won't produce an extra draw call.
    draw_list->UpdateTextureID();
    _Count = 1;
}

void ImDrawListSplitter::SetCurrentChannel(ImDrawList* draw_list, int idx)
{
    IM_ASSERT(idx >= 0 && idx < _Count);
    if (_Current == idx)
        return;
    // Overwrite ImVector (12/16 bytes), four times. This is merely a silly optimization instead of doing .swap()
    memcpy(&_Channels.Data[_Current]._CmdBuffer, &draw_list->CmdBuffer, sizeof(draw_list->CmdBuffer));
    memcpy(&_Channels.Data[_Current]._IdxBuffer, &draw_list->IdxBuffer, sizeof(draw_list->IdxBuffer));
    _Current = idx;
    memcpy(&draw_list->CmdBuffer, &_Channels.Data[idx]._CmdBuffer, sizeof(draw_list->CmdBuffer));
    memcpy(&draw_list->IdxBuffer, &_Channels.Data[idx]._IdxBuffer, sizeof(draw_list->IdxBuffer));
    draw_list->_IdxWritePtr = draw_list->IdxBuffer.Data + draw_list->IdxBuffer.Size;
}

//-----------------------------------------------------------------------------
// [SECTION] ImDrawData
//-----------------------------------------------------------------------------

// For backward compatibility: convert all buffers from indexed to de-indexed, in case you cannot render indexed. Note: this is slow and most likely a waste of resources. Always prefer indexed rendering!
void ImDrawData::DeIndexAllBuffers()
{
    ImVector<ImDrawVert> new_vtx_buffer;
    TotalVtxCount = TotalIdxCount = 0;
    for (int i = 0; i < CmdListsCount; i++)
    {
        ImDrawList* cmd_list = CmdLists[i];
        if (cmd_list->IdxBuffer.empty())
            continue;
        new_vtx_buffer.resize(cmd_list->IdxBuffer.Size);
        for (int j = 0; j < cmd_list->IdxBuffer.Size; j++)
            new_vtx_buffer[j] = cmd_list->VtxBuffer[cmd_list->IdxBuffer[j]];
        cmd_list->VtxBuffer.swap(new_vtx_buffer);
        cmd_list->IdxBuffer.resize(0);
        TotalVtxCount += cmd_list->VtxBuffer.Size;
    }
}

// Helper to scale the ClipRect field of each ImDrawCmd.
// Use if your final output buffer is at a different scale than draw_data->DisplaySize,
// or if there is a difference between your window resolution and framebuffer resolution.
void ImDrawData::ScaleClipRects(const ImVec2& fb_scale)
{
    for (int i = 0; i < CmdListsCount; i++)
    {
        ImDrawList* cmd_list = CmdLists[i];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            ImDrawCmd* cmd = &cmd_list->CmdBuffer[cmd_i];
            cmd->ClipRect = ImVec4(cmd->ClipRect.x * fb_scale.x, cmd->ClipRect.y * fb_scale.y, cmd->ClipRect.z * fb_scale.x, cmd->ClipRect.w * fb_scale.y);
        }
    }
}

//-----------------------------------------------------------------------------
// [SECTION] Helpers ShadeVertsXXX functions
//-----------------------------------------------------------------------------

// Generic linear color gradient, write to RGB fields, leave A untouched.
void ImGui::ShadeVertsLinearColorGradientKeepAlpha(ImDrawList* draw_list, int vert_start_idx, int vert_end_idx, ImVec2 gradient_p0, ImVec2 gradient_p1, ImU32 col0, ImU32 col1)
{
    ImVec2 gradient_extent = gradient_p1 - gradient_p0;
    float gradient_inv_length2 = 1.0f / ImLengthSqr(gradient_extent);
    ImDrawVert* vert_start = draw_list->VtxBuffer.Data + vert_start_idx;
    ImDrawVert* vert_end = draw_list->VtxBuffer.Data + vert_end_idx;
    for (ImDrawVert* vert = vert_start; vert < vert_end; vert++)
    {
        float d = ImDot(vert->pos - gradient_p0, gradient_extent);
        float t = ImClamp(d * gradient_inv_length2, 0.0f, 1.0f);
        int r = ImLerp((int)(col0 >> IM_COL32_R_SHIFT) & 0xFF, (int)(col1 >> IM_COL32_R_SHIFT) & 0xFF, t);
        int g = ImLerp((int)(col0 >> IM_COL32_G_SHIFT) & 0xFF, (int)(col1 >> IM_COL32_G_SHIFT) & 0xFF, t);
        int b = ImLerp((int)(col0 >> IM_COL32_B_SHIFT) & 0xFF, (int)(col1 >> IM_COL32_B_SHIFT) & 0xFF, t);
        vert->col = (r << IM_COL32_R_SHIFT) | (g << IM_COL32_G_SHIFT) | (b << IM_COL32_B_SHIFT) | (vert->col & IM_COL32_A_MASK);
    }
}

// Distribute UV over (a, b) rectangle
void ImGui::ShadeVertsLinearUV(ImDrawList* draw_list, int vert_start_idx, int vert_end_idx, const ImVec2& a, const ImVec2& b, const ImVec2& uv_a, const ImVec2& uv_b, bool clamp)
{
    const ImVec2 size = b - a;
    const ImVec2 uv_size = uv_b - uv_a;
    const ImVec2 scale = ImVec2(
        size.x != 0.0f ? (uv_size.x / size.x) : 0.0f,
        size.y != 0.0f ? (uv_size.y / size.y) : 0.0f);

    ImDrawVert* vert_start = draw_list->VtxBuffer.Data + vert_start_idx;
    ImDrawVert* vert_end = draw_list->VtxBuffer.Data + vert_end_idx;
    if (clamp)
    {
        const ImVec2 min = ImMin(uv_a, uv_b);
        const ImVec2 max = ImMax(uv_a, uv_b);
        for (ImDrawVert* vertex = vert_start; vertex < vert_end; ++vertex)
            vertex->uv = ImClamp(uv_a + ImMul(ImVec2(vertex->pos.x, vertex->pos.y) - a, scale), min, max);
    }
    else
    {
        for (ImDrawVert* vertex = vert_start; vertex < vert_end; ++vertex)
            vertex->uv = uv_a + ImMul(ImVec2(vertex->pos.x, vertex->pos.y) - a, scale);
    }
}

//-----------------------------------------------------------------------------
// [SECTION] ImFontConfig
//-----------------------------------------------------------------------------

ImFontConfig::ImFontConfig()
{
    FontData = NULL;
    FontDataSize = 0;
    FontDataOwnedByAtlas = true;
    FontNo = 0;
    SizePixels = 0.0f;
    OversampleH = 3; // FIXME: 2 may be a better default?
    OversampleV = 1;
    PixelSnapH = false;
    GlyphExtraSpacing = ImVec2(0.0f, 0.0f);
    GlyphOffset = ImVec2(0.0f, 0.0f);
    GlyphRanges = NULL;
    GlyphMinAdvanceX = 0.0f;
    GlyphMaxAdvanceX = FLT_MAX;
    MergeMode = false;
    RasterizerFlags = 0x00;
    RasterizerMultiply = 1.0f;
    EllipsisChar = (ImWchar)-1;
    memset(Name, 0, sizeof(Name));
    DstFont = NULL;
}

//-----------------------------------------------------------------------------
// [SECTION] ImFontAtlas
//-----------------------------------------------------------------------------

// A work of art lies ahead! (. = white layer, X = black layer, others are blank)
// The white texels on the top left are the ones we'll use everywhere in Dear ImGui to render filled shapes.
const int FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF = 108;
const int FONT_ATLAS_DEFAULT_TEX_DATA_H = 27;
const unsigned int FONT_ATLAS_DEFAULT_TEX_DATA_ID = 0x80000000;
static const char FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS[FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF * FONT_ATLAS_DEFAULT_TEX_DATA_H + 1] =
{
    "..-         -XXXXXXX-    X    -           X           -XXXXXXX          -          XXXXXXX-     XX          "
    "..-         -X.....X-   X.X   -          X.X          -X.....X          -          X.....X-    X..X         "
    "---         -XXX.XXX-  X...X  -         X...X         -X....X           -           X....X-    X..X         "
    "X           -  X.X  - X.....X -        X.....X        -X...X            -            X...X-    X..X         "
    "XX          -  X.X  -X.......X-       X.......X       -X..X.X           -           X.X..X-    X..X         "
    "X.X         -  X.X  -XXXX.XXXX-       XXXX.XXXX       -X.X X.X          -          X.X X.X-    X..XXX       "
    "X..X        -  X.X  -   X.X   -          X.X          -XX   X.X         -         X.X   XX-    X..X..XXX    "
    "X...X       -  X.X  -   X.X   -    XX    X.X    XX    -      X.X        -        X.X      -    X..X..X..XX  "
    "X....X      -  X.X  -   X.X   -   X.X    X.X    X.X   -       X.X       -       X.X       -    X..X..X..X.X "
    "X.....X     -  X.X  -   X.X   -  X..X    X.X    X..X  -        X.X      -      X.X        -XXX X..X..X..X..X"
    "X......X    -  X.X  -   X.X   - X...XXXXXX.XXXXXX...X -         X.X   XX-XX   X.X         -X..XX........X..X"
    "X.......X   -  X.X  -   X.X   -X.....................X-          X.X X.X-X.X X.X          -X...X...........X"
    "X........X  -  X.X  -   X.X   - X...XXXXXX.XXXXXX...X -           X.X..X-X..X.X           - X..............X"
    "X.........X -XXX.XXX-   X.X   -  X..X    X.X    X..X  -            X...X-X...X            -  X.............X"
    "X..........X-X.....X-   X.X   -   X.X    X.X    X.X   -           X....X-X....X           -  X.............X"
    "X......XXXXX-XXXXXXX-   X.X   -    XX    X.X    XX    -          X.....X-X.....X          -   X............X"
    "X...X..X    ---------   X.X   -          X.X          -          XXXXXXX-XXXXXXX          -   X...........X "
    "X..X X..X   -       -XXXX.XXXX-       XXXX.XXXX       -------------------------------------    X..........X "
    "X.X  X..X   -       -X.......X-       X.......X       -    XX           XX    -           -    X..........X "
    "XX    X..X  -       - X.....X -        X.....X        -   X.X           X.X   -           -     X........X  "
    "      X..X          -  X...X  -         X...X         -  X..X           X..X  -           -     X........X  "
    "       XX           -   X.X   -          X.X          - X...XXXXXXXXXXXXX...X -           -     XXXXXXXXXX  "
    "------------        -    X    -           X           -X.....................X-           ------------------"
    "                    ----------------------------------- X...XXXXXXXXXXXXX...X -                             "
    "                                                      -  X..X           X..X  -                             "
    "                                                      -   X.X           X.X   -                             "
    "                                                      -    XX           XX    -                             "
};

static const ImVec2 FONT_ATLAS_DEFAULT_TEX_CURSOR_DATA[ImGuiMouseCursor_COUNT][3] =
{
    // Pos ........ Size ......... Offset ......
    { ImVec2(0,3), ImVec2(12,19), ImVec2(0, 0) }, // ImGuiMouseCursor_Arrow
    { ImVec2(13,0), ImVec2(7,16), ImVec2(1, 8) }, // ImGuiMouseCursor_TextInput
    { ImVec2(31,0), ImVec2(23,23), ImVec2(11,11) }, // ImGuiMouseCursor_ResizeAll
    { ImVec2(21,0), ImVec2(9,23), ImVec2(4,11) }, // ImGuiMouseCursor_ResizeNS
    { ImVec2(55,18),ImVec2(23, 9), ImVec2(11, 4) }, // ImGuiMouseCursor_ResizeEW
    { ImVec2(73,0), ImVec2(17,17), ImVec2(8, 8) }, // ImGuiMouseCursor_ResizeNESW
    { ImVec2(55,0), ImVec2(17,17), ImVec2(8, 8) }, // ImGuiMouseCursor_ResizeNWSE
    { ImVec2(91,0), ImVec2(17,22), ImVec2(5, 0) }, // ImGuiMouseCursor_Hand
};

ImFontAtlas::ImFontAtlas()
{
    Locked = false;
    Flags = ImFontAtlasFlags_None;
    TexID = (ImTextureID)NULL;
    TexDesiredWidth = 0;
    TexGlyphPadding = 1;

    TexPixelsAlpha8 = NULL;
    TexPixelsRGBA32 = NULL;
    TexWidth = TexHeight = 0;
    TexUvScale = ImVec2(0.0f, 0.0f);
    TexUvWhitePixel = ImVec2(0.0f, 0.0f);
    for (int n = 0; n < IM_ARRAYSIZE(CustomRectIds); n++)
        CustomRectIds[n] = -1;
}

ImFontAtlas::~ImFontAtlas()
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    Clear();
}

void    ImFontAtlas::ClearInputData()
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    for (int i = 0; i < ConfigData.Size; i++)
        if (ConfigData[i].FontData && ConfigData[i].FontDataOwnedByAtlas)
        {
            IM_FREE(ConfigData[i].FontData);
            ConfigData[i].FontData = NULL;
        }

    // When clearing this we lose access to the font name and other information used to build the font.
    for (int i = 0; i < Fonts.Size; i++)
        if (Fonts[i]->ConfigData >= ConfigData.Data && Fonts[i]->ConfigData < ConfigData.Data + ConfigData.Size)
        {
            Fonts[i]->ConfigData = NULL;
            Fonts[i]->ConfigDataCount = 0;
        }
    ConfigData.clear();
    CustomRects.clear();
    for (int n = 0; n < IM_ARRAYSIZE(CustomRectIds); n++)
        CustomRectIds[n] = -1;
}

void    ImFontAtlas::ClearTexData()
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    if (TexPixelsAlpha8)
        IM_FREE(TexPixelsAlpha8);
    if (TexPixelsRGBA32)
        IM_FREE(TexPixelsRGBA32);
    TexPixelsAlpha8 = NULL;
    TexPixelsRGBA32 = NULL;
}

void    ImFontAtlas::ClearFonts()
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    for (int i = 0; i < Fonts.Size; i++)
        IM_DELETE(Fonts[i]);
    Fonts.clear();
}

void    ImFontAtlas::Clear()
{
    ClearInputData();
    ClearTexData();
    ClearFonts();
}

void    ImFontAtlas::GetTexDataAsAlpha8(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel)
{
    // Build atlas on demand
    if (TexPixelsAlpha8 == NULL)
    {
        if (ConfigData.empty())
            AddFontDefault();
        Build();
    }

    *out_pixels = TexPixelsAlpha8;
    if (out_width) *out_width = TexWidth;
    if (out_height) *out_height = TexHeight;
    if (out_bytes_per_pixel) *out_bytes_per_pixel = 1;
}

void    ImFontAtlas::GetTexDataAsRGBA32(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel)
{
    // Convert to RGBA32 format on demand
    // Although it is likely to be the most commonly used format, our font rendering is 1 channel / 8 bpp
    if (!TexPixelsRGBA32)
    {
        unsigned char* pixels = NULL;
        GetTexDataAsAlpha8(&pixels, NULL, NULL);
        if (pixels)
        {
            TexPixelsRGBA32 = (unsigned int*)IM_ALLOC((size_t)TexWidth * (size_t)TexHeight * 4);
            const unsigned char* src = pixels;
            unsigned int* dst = TexPixelsRGBA32;
            for (int n = TexWidth * TexHeight; n > 0; n--)
                *dst++ = IM_COL32(255, 255, 255, (unsigned int)(*src++));
        }
    }

    *out_pixels = (unsigned char*)TexPixelsRGBA32;
    if (out_width) *out_width = TexWidth;
    if (out_height) *out_height = TexHeight;
    if (out_bytes_per_pixel) *out_bytes_per_pixel = 4;
}

ImFont* ImFontAtlas::AddFont(const ImFontConfig* font_cfg)
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    IM_ASSERT(font_cfg->FontData != NULL && font_cfg->FontDataSize > 0);
    IM_ASSERT(font_cfg->SizePixels > 0.0f);

    // Create new font
    if (!font_cfg->MergeMode)
        Fonts.push_back(IM_NEW(ImFont));
    else
        IM_ASSERT(!Fonts.empty() && "Cannot use MergeMode for the first font"); // When using MergeMode make sure that a font has already been added before. You can use ImGui::GetIO().Fonts->AddFontDefault() to add the default imgui font.

    ConfigData.push_back(*font_cfg);
    ImFontConfig& new_font_cfg = ConfigData.back();
    if (new_font_cfg.DstFont == NULL)
        new_font_cfg.DstFont = Fonts.back();
    if (!new_font_cfg.FontDataOwnedByAtlas)
    {
        new_font_cfg.FontData = IM_ALLOC(new_font_cfg.FontDataSize);
        new_font_cfg.FontDataOwnedByAtlas = true;
        memcpy(new_font_cfg.FontData, font_cfg->FontData, (size_t)new_font_cfg.FontDataSize);
    }

    if (new_font_cfg.DstFont->EllipsisChar == (ImWchar)-1)
        new_font_cfg.DstFont->EllipsisChar = font_cfg->EllipsisChar;

    // Invalidate texture
    ClearTexData();
    return new_font_cfg.DstFont;
}

// Default font TTF is compressed with stb_compress then base85 encoded (see misc/fonts/binary_to_compressed_c.cpp for encoder)
static unsigned int stb_decompress_length(const unsigned char *input);
static unsigned int stb_decompress(unsigned char *output, const unsigned char *input, unsigned int length);
static const char*  get_glacial_indifference();
static const char*  get_glacial_indifference_bold();
static unsigned int Decode85Byte(char c) { return c >= '\\' ? c - 36 : c - 35; }
static void         Decode85(const unsigned char* src, unsigned char* dst)
{
    while (*src)
    {
        unsigned int tmp = Decode85Byte(src[0]) + 85 * (Decode85Byte(src[1]) + 85 * (Decode85Byte(src[2]) + 85 * (Decode85Byte(src[3]) + 85 * Decode85Byte(src[4]))));
        dst[0] = ((tmp >> 0) & 0xFF); dst[1] = ((tmp >> 8) & 0xFF); dst[2] = ((tmp >> 16) & 0xFF); dst[3] = ((tmp >> 24) & 0xFF);   // We can't assume little-endianness.
        src += 5;
        dst += 4;
    }
}

std::string ImFontAtlas::GetSecondaryFont()
{
    const char* font = get_glacial_indifference_bold();
    return font;
}

std::string ImFontAtlas::GetPrimaryFont()
{
	const char* font = get_glacial_indifference();
    return font;
}

// Load embedded ProggyClean.ttf at size 13, disable oversampling
ImFont* ImFontAtlas::AddFontDefault(const ImFontConfig* font_cfg_template)
{
    ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();

    if (!font_cfg_template)
    {
        font_cfg.OversampleH = font_cfg.OversampleV = 1;
        font_cfg.PixelSnapH = true;
    }

    font_cfg.SizePixels = 15.0f;

    if (font_cfg.Name[0] == '\0')
        ImFormatString(font_cfg.Name, IM_ARRAYSIZE(font_cfg.Name), "GlacialIndifference-Regular.ttf, %dpx", (int)font_cfg.SizePixels);

    font_cfg.EllipsisChar = static_cast<ImWchar>(0x0085);

    const auto ttf_compressed_base85 = get_glacial_indifference();
    const auto glyph_ranges = font_cfg.GlyphRanges != NULL ? font_cfg.GlyphRanges : GetGlyphRangesDefault();
    const auto font = AddFontFromMemoryCompressedBase85TTF(ttf_compressed_base85, font_cfg.SizePixels, &font_cfg, glyph_ranges);

    return font;
}

ImFont* ImFontAtlas::AddFontFromFileTTF(const char* filename, float size_pixels, const ImFontConfig* font_cfg_template, const ImWchar* glyph_ranges)
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    size_t data_size = 0;
    void* data = ImFileLoadToMemory(filename, "rb", &data_size, 0);
    if (!data)
    {
        IM_ASSERT_USER_ERROR(0, "Could not load font file!");
        return NULL;
    }
    ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
    if (font_cfg.Name[0] == '\0')
    {
        // Store a short copy of filename into into the font name for convenience
        const char* p;
        for (p = filename + strlen(filename); p > filename && p[-1] != '/' && p[-1] != '\\'; p--) {}
        ImFormatString(font_cfg.Name, IM_ARRAYSIZE(font_cfg.Name), "%s, %.0fpx", p, size_pixels);
    }
    return AddFontFromMemoryTTF(data, (int)data_size, size_pixels, &font_cfg, glyph_ranges);
}

// NB: Transfer ownership of 'ttf_data' to ImFontAtlas, unless font_cfg_template->FontDataOwnedByAtlas == false. Owned TTF buffer will be deleted after Build().
ImFont* ImFontAtlas::AddFontFromMemoryTTF(void* ttf_data, int ttf_size, float size_pixels, const ImFontConfig* font_cfg_template, const ImWchar* glyph_ranges)
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
    IM_ASSERT(font_cfg.FontData == NULL);
    font_cfg.FontData = ttf_data;
    font_cfg.FontDataSize = ttf_size;
    font_cfg.SizePixels = size_pixels;
    if (glyph_ranges)
        font_cfg.GlyphRanges = glyph_ranges;
    return AddFont(&font_cfg);
}

ImFont* ImFontAtlas::AddFontFromMemoryCompressedTTF(const void* compressed_ttf_data, int compressed_ttf_size, float size_pixels, const ImFontConfig* font_cfg_template, const ImWchar* glyph_ranges)
{
    const unsigned int buf_decompressed_size = stb_decompress_length((const unsigned char*)compressed_ttf_data);
    unsigned char* buf_decompressed_data = (unsigned char *)IM_ALLOC(buf_decompressed_size);
    stb_decompress(buf_decompressed_data, (const unsigned char*)compressed_ttf_data, (unsigned int)compressed_ttf_size);

    ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
    IM_ASSERT(font_cfg.FontData == NULL);
    font_cfg.FontDataOwnedByAtlas = true;
    return AddFontFromMemoryTTF(buf_decompressed_data, (int)buf_decompressed_size, size_pixels, &font_cfg, glyph_ranges);
}

ImFont* ImFontAtlas::AddFontFromMemoryCompressedBase85TTF(const char* compressed_ttf_data_base85, float size_pixels, const ImFontConfig* font_cfg, const ImWchar* glyph_ranges)
{
    int compressed_ttf_size = (((int)strlen(compressed_ttf_data_base85) + 4) / 5) * 4;
    void* compressed_ttf = IM_ALLOC((size_t)compressed_ttf_size);
    Decode85((const unsigned char*)compressed_ttf_data_base85, (unsigned char*)compressed_ttf);
    ImFont* font = AddFontFromMemoryCompressedTTF(compressed_ttf, compressed_ttf_size, size_pixels, font_cfg, glyph_ranges);
    IM_FREE(compressed_ttf);
    return font;
}

int ImFontAtlas::AddCustomRectRegular(unsigned int id, int width, int height)
{
    // Breaking change on 2019/11/21 (1.74): ImFontAtlas::AddCustomRectRegular() now requires an ID >= 0x110000 (instead of >= 0x10000)
    IM_ASSERT(id >= 0x110000);
    IM_ASSERT(width > 0 && width <= 0xFFFF);
    IM_ASSERT(height > 0 && height <= 0xFFFF);
    ImFontAtlasCustomRect r;
    r.ID = id;
    r.Width = (unsigned short)width;
    r.Height = (unsigned short)height;
    CustomRects.push_back(r);
    return CustomRects.Size - 1; // Return index
}

int ImFontAtlas::AddCustomRectFontGlyph(ImFont* font, ImWchar id, int width, int height, float advance_x, const ImVec2& offset)
{
    IM_ASSERT(font != NULL);
    IM_ASSERT(width > 0 && width <= 0xFFFF);
    IM_ASSERT(height > 0 && height <= 0xFFFF);
    ImFontAtlasCustomRect r;
    r.ID = id;
    r.Width = (unsigned short)width;
    r.Height = (unsigned short)height;
    r.GlyphAdvanceX = advance_x;
    r.GlyphOffset = offset;
    r.Font = font;
    CustomRects.push_back(r);
    return CustomRects.Size - 1; // Return index
}

void ImFontAtlas::CalcCustomRectUV(const ImFontAtlasCustomRect* rect, ImVec2* out_uv_min, ImVec2* out_uv_max) const
{
    IM_ASSERT(TexWidth > 0 && TexHeight > 0);   // Font atlas needs to be built before we can calculate UV coordinates
    IM_ASSERT(rect->IsPacked());                // Make sure the rectangle has been packed
    *out_uv_min = ImVec2((float)rect->X * TexUvScale.x, (float)rect->Y * TexUvScale.y);
    *out_uv_max = ImVec2((float)(rect->X + rect->Width) * TexUvScale.x, (float)(rect->Y + rect->Height) * TexUvScale.y);
}

bool ImFontAtlas::GetMouseCursorTexData(ImGuiMouseCursor cursor_type, ImVec2* out_offset, ImVec2* out_size, ImVec2 out_uv_border[2], ImVec2 out_uv_fill[2])
{
    if (cursor_type <= ImGuiMouseCursor_None || cursor_type >= ImGuiMouseCursor_COUNT)
        return false;
    if (Flags & ImFontAtlasFlags_NoMouseCursors)
        return false;

    IM_ASSERT(CustomRectIds[0] != -1);
    ImFontAtlasCustomRect& r = CustomRects[CustomRectIds[0]];
    IM_ASSERT(r.ID == FONT_ATLAS_DEFAULT_TEX_DATA_ID);
    ImVec2 pos = FONT_ATLAS_DEFAULT_TEX_CURSOR_DATA[cursor_type][0] + ImVec2((float)r.X, (float)r.Y);
    ImVec2 size = FONT_ATLAS_DEFAULT_TEX_CURSOR_DATA[cursor_type][1];
    *out_size = size;
    *out_offset = FONT_ATLAS_DEFAULT_TEX_CURSOR_DATA[cursor_type][2];
    out_uv_border[0] = (pos)* TexUvScale;
    out_uv_border[1] = (pos + size) * TexUvScale;
    pos.x += FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF + 1;
    out_uv_fill[0] = (pos)* TexUvScale;
    out_uv_fill[1] = (pos + size) * TexUvScale;
    return true;
}

bool    ImFontAtlas::Build()
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    return ImFontAtlasBuildWithStbTruetype(this);
}

void    ImFontAtlasBuildMultiplyCalcLookupTable(unsigned char out_table[256], float in_brighten_factor)
{
    for (unsigned int i = 0; i < 256; i++)
    {
        unsigned int value = (unsigned int)(i * in_brighten_factor);
        out_table[i] = value > 255 ? 255 : (value & 0xFF);
    }
}

void    ImFontAtlasBuildMultiplyRectAlpha8(const unsigned char table[256], unsigned char* pixels, int x, int y, int w, int h, int stride)
{
    unsigned char* data = pixels + x + y * stride;
    for (int j = h; j > 0; j--, data += stride)
        for (int i = 0; i < w; i++)
            data[i] = table[data[i]];
}

// Temporary data for one source font (multiple source fonts can be merged into one destination ImFont)
// (C++03 doesn't allow instancing ImVector<> with function-local types so we declare the type here.)
struct ImFontBuildSrcData
{
    stbtt_fontinfo      FontInfo;
    stbtt_pack_range    PackRange;          // Hold the list of codepoints to pack (essentially points to Codepoints.Data)
    stbrp_rect*         Rects;              // Rectangle to pack. We first fill in their size and the packer will give us their position.
    stbtt_packedchar*   PackedChars;        // Output glyphs
    const ImWchar*      SrcRanges;          // Ranges as requested by user (user is allowed to request too much, e.g. 0x0020..0xFFFF)
    int                 DstIndex;           // Index into atlas->Fonts[] and dst_tmp_array[]
    int                 GlyphsHighest;      // Highest requested codepoint
    int                 GlyphsCount;        // Glyph count (excluding missing glyphs and glyphs already set by an earlier source font)
    ImBitVector         GlyphsSet;          // Glyph bit map (random access, 1-bit per codepoint. This will be a maximum of 8KB)
    ImVector<int>       GlyphsList;         // Glyph codepoints list (flattened version of GlyphsMap)
};

// Temporary data for one destination ImFont* (multiple source fonts can be merged into one destination ImFont)
struct ImFontBuildDstData
{
    int                 SrcCount;           // Number of source fonts targeting this destination font.
    int                 GlyphsHighest;
    int                 GlyphsCount;
    ImBitVector         GlyphsSet;          // This is used to resolve collision when multiple sources are merged into a same destination font.
};

static void UnpackBitVectorToFlatIndexList(const ImBitVector* in, ImVector<int>* out)
{
    IM_ASSERT(sizeof(in->Storage.Data[0]) == sizeof(int));
    const ImU32* it_begin = in->Storage.begin();
    const ImU32* it_end = in->Storage.end();
    for (const ImU32* it = it_begin; it < it_end; it++)
        if (ImU32 entries_32 = *it)
            for (ImU32 bit_n = 0; bit_n < 32; bit_n++)
                if (entries_32 & ((ImU32)1 << bit_n))
                    out->push_back((int)(((it - it_begin) << 5) + bit_n));
}

bool    ImFontAtlasBuildWithStbTruetype(ImFontAtlas* atlas)
{
    IM_ASSERT(atlas->ConfigData.Size > 0);

    ImFontAtlasBuildInit(atlas);

    // Clear atlas
    atlas->TexID = (ImTextureID)NULL;
    atlas->TexWidth = atlas->TexHeight = 0;
    atlas->TexUvScale = ImVec2(0.0f, 0.0f);
    atlas->TexUvWhitePixel = ImVec2(0.0f, 0.0f);
    atlas->ClearTexData();

    // Temporary storage for building
    ImVector<ImFontBuildSrcData> src_tmp_array;
    ImVector<ImFontBuildDstData> dst_tmp_array;
    src_tmp_array.resize(atlas->ConfigData.Size);
    dst_tmp_array.resize(atlas->Fonts.Size);
    memset(src_tmp_array.Data, 0, (size_t)src_tmp_array.size_in_bytes());
    memset(dst_tmp_array.Data, 0, (size_t)dst_tmp_array.size_in_bytes());

    // 1. Initialize font loading structure, check font data validity
    for (int src_i = 0; src_i < atlas->ConfigData.Size; src_i++)
    {
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        ImFontConfig& cfg = atlas->ConfigData[src_i];
        IM_ASSERT(cfg.DstFont && (!cfg.DstFont->IsLoaded() || cfg.DstFont->ContainerAtlas == atlas));

        // Find index from cfg.DstFont (we allow the user to set cfg.DstFont. Also it makes casual debugging nicer than when storing indices)
        src_tmp.DstIndex = -1;
        for (int output_i = 0; output_i < atlas->Fonts.Size && src_tmp.DstIndex == -1; output_i++)
            if (cfg.DstFont == atlas->Fonts[output_i])
                src_tmp.DstIndex = output_i;
        IM_ASSERT(src_tmp.DstIndex != -1); // cfg.DstFont not pointing within atlas->Fonts[] array?
        if (src_tmp.DstIndex == -1)
            return false;

        // Initialize helper structure for font loading and verify that the TTF/OTF data is correct
        const int font_offset = stbtt_GetFontOffsetForIndex((unsigned char*)cfg.FontData, cfg.FontNo);
        IM_ASSERT(font_offset >= 0 && "FontData is incorrect, or FontNo cannot be found.");
        if (!stbtt_InitFont(&src_tmp.FontInfo, (unsigned char*)cfg.FontData, font_offset))
            return false;

        // Measure highest codepoints
        ImFontBuildDstData& dst_tmp = dst_tmp_array[src_tmp.DstIndex];
        src_tmp.SrcRanges = cfg.GlyphRanges ? cfg.GlyphRanges : atlas->GetGlyphRangesDefault();
        for (const ImWchar* src_range = src_tmp.SrcRanges; src_range[0] && src_range[1]; src_range += 2)
            src_tmp.GlyphsHighest = ImMax(src_tmp.GlyphsHighest, (int)src_range[1]);
        dst_tmp.SrcCount++;
        dst_tmp.GlyphsHighest = ImMax(dst_tmp.GlyphsHighest, src_tmp.GlyphsHighest);
    }

    // 2. For every requested codepoint, check for their presence in the font data, and handle redundancy or overlaps between source fonts to avoid unused glyphs.
    int total_glyphs_count = 0;
    for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
    {
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        ImFontBuildDstData& dst_tmp = dst_tmp_array[src_tmp.DstIndex];
        src_tmp.GlyphsSet.Create(src_tmp.GlyphsHighest + 1);
        if (dst_tmp.GlyphsSet.Storage.empty())
            dst_tmp.GlyphsSet.Create(dst_tmp.GlyphsHighest + 1);

        for (const ImWchar* src_range = src_tmp.SrcRanges; src_range[0] && src_range[1]; src_range += 2)
            for (unsigned int codepoint = src_range[0]; codepoint <= src_range[1]; codepoint++)
            {
                if (dst_tmp.GlyphsSet.TestBit(codepoint))    // Don't overwrite existing glyphs. We could make this an option for MergeMode (e.g. MergeOverwrite==true)
                    continue;
                if (!stbtt_FindGlyphIndex(&src_tmp.FontInfo, codepoint))    // It is actually in the font?
                    continue;

                // Add to avail set/counters
                src_tmp.GlyphsCount++;
                dst_tmp.GlyphsCount++;
                src_tmp.GlyphsSet.SetBit(codepoint);
                dst_tmp.GlyphsSet.SetBit(codepoint);
                total_glyphs_count++;
            }
    }

    // 3. Unpack our bit map into a flat list (we now have all the Unicode points that we know are requested _and_ available _and_ not overlapping another)
    for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
    {
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        src_tmp.GlyphsList.reserve(src_tmp.GlyphsCount);
        UnpackBitVectorToFlatIndexList(&src_tmp.GlyphsSet, &src_tmp.GlyphsList);
        src_tmp.GlyphsSet.Clear();
        IM_ASSERT(src_tmp.GlyphsList.Size == src_tmp.GlyphsCount);
    }
    for (int dst_i = 0; dst_i < dst_tmp_array.Size; dst_i++)
        dst_tmp_array[dst_i].GlyphsSet.Clear();
    dst_tmp_array.clear();

    // Allocate packing character data and flag packed characters buffer as non-packed (x0=y0=x1=y1=0)
    // (We technically don't need to zero-clear buf_rects, but let's do it for the sake of sanity)
    ImVector<stbrp_rect> buf_rects;
    ImVector<stbtt_packedchar> buf_packedchars;
    buf_rects.resize(total_glyphs_count);
    buf_packedchars.resize(total_glyphs_count);
    memset(buf_rects.Data, 0, (size_t)buf_rects.size_in_bytes());
    memset(buf_packedchars.Data, 0, (size_t)buf_packedchars.size_in_bytes());

    // 4. Gather glyphs sizes so we can pack them in our virtual canvas.
    int total_surface = 0;
    int buf_rects_out_n = 0;
    int buf_packedchars_out_n = 0;
    for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
    {
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        if (src_tmp.GlyphsCount == 0)
            continue;

        src_tmp.Rects = &buf_rects[buf_rects_out_n];
        src_tmp.PackedChars = &buf_packedchars[buf_packedchars_out_n];
        buf_rects_out_n += src_tmp.GlyphsCount;
        buf_packedchars_out_n += src_tmp.GlyphsCount;

        // Convert our ranges in the format stb_truetype wants
        ImFontConfig& cfg = atlas->ConfigData[src_i];
        src_tmp.PackRange.font_size = cfg.SizePixels;
        src_tmp.PackRange.first_unicode_codepoint_in_range = 0;
        src_tmp.PackRange.array_of_unicode_codepoints = src_tmp.GlyphsList.Data;
        src_tmp.PackRange.num_chars = src_tmp.GlyphsList.Size;
        src_tmp.PackRange.chardata_for_range = src_tmp.PackedChars;
        src_tmp.PackRange.h_oversample = (unsigned char)cfg.OversampleH;
        src_tmp.PackRange.v_oversample = (unsigned char)cfg.OversampleV;

        // Gather the sizes of all rectangles we will need to pack (this loop is based on stbtt_PackFontRangesGatherRects)
        const float scale = (cfg.SizePixels > 0) ? stbtt_ScaleForPixelHeight(&src_tmp.FontInfo, cfg.SizePixels) : stbtt_ScaleForMappingEmToPixels(&src_tmp.FontInfo, -cfg.SizePixels);
        const int padding = atlas->TexGlyphPadding;
        for (int glyph_i = 0; glyph_i < src_tmp.GlyphsList.Size; glyph_i++)
        {
            int x0, y0, x1, y1;
            const int glyph_index_in_font = stbtt_FindGlyphIndex(&src_tmp.FontInfo, src_tmp.GlyphsList[glyph_i]);
            IM_ASSERT(glyph_index_in_font != 0);
            stbtt_GetGlyphBitmapBoxSubpixel(&src_tmp.FontInfo, glyph_index_in_font, scale * cfg.OversampleH, scale * cfg.OversampleV, 0, 0, &x0, &y0, &x1, &y1);
            src_tmp.Rects[glyph_i].w = (stbrp_coord)(x1 - x0 + padding + cfg.OversampleH - 1);
            src_tmp.Rects[glyph_i].h = (stbrp_coord)(y1 - y0 + padding + cfg.OversampleV - 1);
            total_surface += src_tmp.Rects[glyph_i].w * src_tmp.Rects[glyph_i].h;
        }
    }

    // We need a width for the skyline algorithm, any width!
    // The exact width doesn't really matter much, but some API/GPU have texture size limitations and increasing width can decrease height.
    // User can override TexDesiredWidth and TexGlyphPadding if they wish, otherwise we use a simple heuristic to select the width based on expected surface.
    const int surface_sqrt = (int)ImSqrt((float)total_surface) + 1;
    atlas->TexHeight = 0;
    if (atlas->TexDesiredWidth > 0)
        atlas->TexWidth = atlas->TexDesiredWidth;
    else
        atlas->TexWidth = (surface_sqrt >= 4096 * 0.7f) ? 4096 : (surface_sqrt >= 2048 * 0.7f) ? 2048 : (surface_sqrt >= 1024 * 0.7f) ? 1024 : 512;

    // 5. Start packing
    // Pack our extra data rectangles first, so it will be on the upper-left corner of our texture (UV will have small values).
    const int TEX_HEIGHT_MAX = 1024 * 32;
    stbtt_pack_context spc = {};
    stbtt_PackBegin(&spc, NULL, atlas->TexWidth, TEX_HEIGHT_MAX, 0, atlas->TexGlyphPadding, NULL);
    ImFontAtlasBuildPackCustomRects(atlas, spc.pack_info);

    // 6. Pack each source font. No rendering yet, we are working with rectangles in an infinitely tall texture at this point.
    for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
    {
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        if (src_tmp.GlyphsCount == 0)
            continue;

        stbrp_pack_rects((stbrp_context*)spc.pack_info, src_tmp.Rects, src_tmp.GlyphsCount);

        // Extend texture height and mark missing glyphs as non-packed so we won't render them.
        // FIXME: We are not handling packing failure here (would happen if we got off TEX_HEIGHT_MAX or if a single if larger than TexWidth?)
        for (int glyph_i = 0; glyph_i < src_tmp.GlyphsCount; glyph_i++)
            if (src_tmp.Rects[glyph_i].was_packed)
                atlas->TexHeight = ImMax(atlas->TexHeight, src_tmp.Rects[glyph_i].y + src_tmp.Rects[glyph_i].h);
    }

    // 7. Allocate texture
    atlas->TexHeight = (atlas->Flags & ImFontAtlasFlags_NoPowerOfTwoHeight) ? (atlas->TexHeight + 1) : ImUpperPowerOfTwo(atlas->TexHeight);
    atlas->TexUvScale = ImVec2(1.0f / atlas->TexWidth, 1.0f / atlas->TexHeight);
    atlas->TexPixelsAlpha8 = (unsigned char*)IM_ALLOC(atlas->TexWidth * atlas->TexHeight);
    memset(atlas->TexPixelsAlpha8, 0, atlas->TexWidth * atlas->TexHeight);
    spc.pixels = atlas->TexPixelsAlpha8;
    spc.height = atlas->TexHeight;

    // 8. Render/rasterize font characters into the texture
    for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
    {
        ImFontConfig& cfg = atlas->ConfigData[src_i];
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        if (src_tmp.GlyphsCount == 0)
            continue;

        stbtt_PackFontRangesRenderIntoRects(&spc, &src_tmp.FontInfo, &src_tmp.PackRange, 1, src_tmp.Rects);

        // Apply multiply operator
        if (cfg.RasterizerMultiply != 1.0f)
        {
            unsigned char multiply_table[256];
            ImFontAtlasBuildMultiplyCalcLookupTable(multiply_table, cfg.RasterizerMultiply);
            stbrp_rect* r = &src_tmp.Rects[0];
            for (int glyph_i = 0; glyph_i < src_tmp.GlyphsCount; glyph_i++, r++)
                if (r->was_packed)
                    ImFontAtlasBuildMultiplyRectAlpha8(multiply_table, atlas->TexPixelsAlpha8, r->x, r->y, r->w, r->h, atlas->TexWidth * 1);
        }
        src_tmp.Rects = NULL;
    }

    // End packing
    stbtt_PackEnd(&spc);
    buf_rects.clear();

    // 9. Setup ImFont and glyphs for runtime
    for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
    {
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        if (src_tmp.GlyphsCount == 0)
            continue;

        ImFontConfig& cfg = atlas->ConfigData[src_i];
        ImFont* dst_font = cfg.DstFont; // We can have multiple input fonts writing into a same destination font (when using MergeMode=true)

        const float font_scale = stbtt_ScaleForPixelHeight(&src_tmp.FontInfo, cfg.SizePixels);
        int unscaled_ascent, unscaled_descent, unscaled_line_gap;
        stbtt_GetFontVMetrics(&src_tmp.FontInfo, &unscaled_ascent, &unscaled_descent, &unscaled_line_gap);

        const float ascent = ImFloor(unscaled_ascent * font_scale + ((unscaled_ascent > 0.0f) ? +1 : -1));
        const float descent = ImFloor(unscaled_descent * font_scale + ((unscaled_descent > 0.0f) ? +1 : -1));
        ImFontAtlasBuildSetupFont(atlas, dst_font, &cfg, ascent, descent);
        const float font_off_x = cfg.GlyphOffset.x;
        const float font_off_y = cfg.GlyphOffset.y + IM_ROUND(dst_font->Ascent);

        for (int glyph_i = 0; glyph_i < src_tmp.GlyphsCount; glyph_i++)
        {
            const int codepoint = src_tmp.GlyphsList[glyph_i];
            const stbtt_packedchar& pc = src_tmp.PackedChars[glyph_i];

            const float char_advance_x_org = pc.xadvance;
            const float char_advance_x_mod = ImClamp(char_advance_x_org, cfg.GlyphMinAdvanceX, cfg.GlyphMaxAdvanceX);
            float char_off_x = font_off_x;
            if (char_advance_x_org != char_advance_x_mod)
                char_off_x += cfg.PixelSnapH ? ImFloor((char_advance_x_mod - char_advance_x_org) * 0.5f) : (char_advance_x_mod - char_advance_x_org) * 0.5f;

            // Register glyph
            stbtt_aligned_quad q;
            float dummy_x = 0.0f, dummy_y = 0.0f;
            stbtt_GetPackedQuad(src_tmp.PackedChars, atlas->TexWidth, atlas->TexHeight, glyph_i, &dummy_x, &dummy_y, &q, 0);
            dst_font->AddGlyph((ImWchar)codepoint, q.x0 + char_off_x, q.y0 + font_off_y, q.x1 + char_off_x, q.y1 + font_off_y, q.s0, q.t0, q.s1, q.t1, char_advance_x_mod);
        }
    }

    // Cleanup temporary (ImVector doesn't honor destructor)
    for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
        src_tmp_array[src_i].~ImFontBuildSrcData();

    ImFontAtlasBuildFinish(atlas);
    return true;
}

// Register default custom rectangles (this is called/shared by both the stb_truetype and the FreeType builder)
void ImFontAtlasBuildInit(ImFontAtlas* atlas)
{
    if (atlas->CustomRectIds[0] >= 0)
        return;
    if (!(atlas->Flags & ImFontAtlasFlags_NoMouseCursors))
        atlas->CustomRectIds[0] = atlas->AddCustomRectRegular(FONT_ATLAS_DEFAULT_TEX_DATA_ID, FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF * 2 + 1, FONT_ATLAS_DEFAULT_TEX_DATA_H);
    else
        atlas->CustomRectIds[0] = atlas->AddCustomRectRegular(FONT_ATLAS_DEFAULT_TEX_DATA_ID, 2, 2);
}

void ImFontAtlasBuildSetupFont(ImFontAtlas* atlas, ImFont* font, ImFontConfig* font_config, float ascent, float descent)
{
    if (!font_config->MergeMode)
    {
        font->ClearOutputData();
        font->FontSize = font_config->SizePixels;
        font->ConfigData = font_config;
        font->ContainerAtlas = atlas;
        font->Ascent = ascent;
        font->Descent = descent;
    }
    font->ConfigDataCount++;
}

void ImFontAtlasBuildPackCustomRects(ImFontAtlas* atlas, void* stbrp_context_opaque)
{
    stbrp_context* pack_context = (stbrp_context*)stbrp_context_opaque;
    IM_ASSERT(pack_context != NULL);

    ImVector<ImFontAtlasCustomRect>& user_rects = atlas->CustomRects;
    IM_ASSERT(user_rects.Size >= 1); // We expect at least the default custom rects to be registered, else something went wrong.

    ImVector<stbrp_rect> pack_rects;
    pack_rects.resize(user_rects.Size);
    memset(pack_rects.Data, 0, (size_t)pack_rects.size_in_bytes());
    for (int i = 0; i < user_rects.Size; i++)
    {
        pack_rects[i].w = user_rects[i].Width;
        pack_rects[i].h = user_rects[i].Height;
    }
    stbrp_pack_rects(pack_context, &pack_rects[0], pack_rects.Size);
    for (int i = 0; i < pack_rects.Size; i++)
        if (pack_rects[i].was_packed)
        {
            user_rects[i].X = pack_rects[i].x;
            user_rects[i].Y = pack_rects[i].y;
            IM_ASSERT(pack_rects[i].w == user_rects[i].Width && pack_rects[i].h == user_rects[i].Height);
            atlas->TexHeight = ImMax(atlas->TexHeight, pack_rects[i].y + pack_rects[i].h);
        }
}

static void ImFontAtlasBuildRenderDefaultTexData(ImFontAtlas* atlas)
{
    IM_ASSERT(atlas->CustomRectIds[0] >= 0);
    IM_ASSERT(atlas->TexPixelsAlpha8 != NULL);
    ImFontAtlasCustomRect& r = atlas->CustomRects[atlas->CustomRectIds[0]];
    IM_ASSERT(r.ID == FONT_ATLAS_DEFAULT_TEX_DATA_ID);
    IM_ASSERT(r.IsPacked());

    const int w = atlas->TexWidth;
    if (!(atlas->Flags & ImFontAtlasFlags_NoMouseCursors))
    {
        // Render/copy pixels
        IM_ASSERT(r.Width == FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF * 2 + 1 && r.Height == FONT_ATLAS_DEFAULT_TEX_DATA_H);
        for (int y = 0, n = 0; y < FONT_ATLAS_DEFAULT_TEX_DATA_H; y++)
            for (int x = 0; x < FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF; x++, n++)
            {
                const int offset0 = (int)(r.X + x) + (int)(r.Y + y) * w;
                const int offset1 = offset0 + FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF + 1;
                atlas->TexPixelsAlpha8[offset0] = FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS[n] == '.' ? 0xFF : 0x00;
                atlas->TexPixelsAlpha8[offset1] = FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS[n] == 'X' ? 0xFF : 0x00;
            }
    }
    else
    {
        IM_ASSERT(r.Width == 2 && r.Height == 2);
        const int offset = (int)(r.X) + (int)(r.Y) * w;
        atlas->TexPixelsAlpha8[offset] = atlas->TexPixelsAlpha8[offset + 1] = atlas->TexPixelsAlpha8[offset + w] = atlas->TexPixelsAlpha8[offset + w + 1] = 0xFF;
    }
    atlas->TexUvWhitePixel = ImVec2((r.X + 0.5f) * atlas->TexUvScale.x, (r.Y + 0.5f) * atlas->TexUvScale.y);
}

void ImFontAtlasBuildFinish(ImFontAtlas* atlas)
{
    // Render into our custom data block
    ImFontAtlasBuildRenderDefaultTexData(atlas);

    // Register custom rectangle glyphs
    for (int i = 0; i < atlas->CustomRects.Size; i++)
    {
        const ImFontAtlasCustomRect& r = atlas->CustomRects[i];
        if (r.Font == NULL || r.ID >= 0x110000)
            continue;

        IM_ASSERT(r.Font->ContainerAtlas == atlas);
        ImVec2 uv0, uv1;
        atlas->CalcCustomRectUV(&r, &uv0, &uv1);
        r.Font->AddGlyph((ImWchar)r.ID, r.GlyphOffset.x, r.GlyphOffset.y, r.GlyphOffset.x + r.Width, r.GlyphOffset.y + r.Height, uv0.x, uv0.y, uv1.x, uv1.y, r.GlyphAdvanceX);
    }

    // Build all fonts lookup tables
    for (int i = 0; i < atlas->Fonts.Size; i++)
        if (atlas->Fonts[i]->DirtyLookupTables)
            atlas->Fonts[i]->BuildLookupTable();

    // Ellipsis character is required for rendering elided text. We prefer using U+2026 (horizontal ellipsis).
    // However some old fonts may contain ellipsis at U+0085. Here we auto-detect most suitable ellipsis character.
    // FIXME: Also note that 0x2026 is currently seldomly included in our font ranges. Because of this we are more likely to use three individual dots.
    for (int i = 0; i < atlas->Fonts.size(); i++)
    {
        ImFont* font = atlas->Fonts[i];
        if (font->EllipsisChar != (ImWchar)-1)
            continue;
        const ImWchar ellipsis_variants[] = { (ImWchar)0x2026, (ImWchar)0x0085 };
        for (int j = 0; j < IM_ARRAYSIZE(ellipsis_variants); j++)
            if (font->FindGlyphNoFallback(ellipsis_variants[j]) != NULL) // Verify glyph exists
            {
                font->EllipsisChar = ellipsis_variants[j];
                break;
            }
    }
}

// Retrieve list of range (2 int per range, values are inclusive)
const ImWchar*   ImFontAtlas::GetGlyphRangesDefault()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0,
    };
    return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesKorean()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x3131, 0x3163, // Korean alphabets
        0xAC00, 0xD79D, // Korean characters
        0,
    };
    return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesChineseFull()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x2000, 0x206F, // General Punctuation
        0x3000, 0x30FF, // CJK Symbols and Punctuations, Hiragana, Katakana
        0x31F0, 0x31FF, // Katakana Phonetic Extensions
        0xFF00, 0xFFEF, // Half-width characters
        0x4e00, 0x9FAF, // CJK Ideograms
        0,
    };
    return &ranges[0];
}

static void UnpackAccumulativeOffsetsIntoRanges(int base_codepoint, const short* accumulative_offsets, int accumulative_offsets_count, ImWchar* out_ranges)
{
    for (int n = 0; n < accumulative_offsets_count; n++, out_ranges += 2)
    {
        out_ranges[0] = out_ranges[1] = (ImWchar)(base_codepoint + accumulative_offsets[n]);
        base_codepoint += accumulative_offsets[n];
    }
    out_ranges[0] = 0;
}

//-------------------------------------------------------------------------
// [SECTION] ImFontAtlas glyph ranges helpers
//-------------------------------------------------------------------------

const ImWchar*  ImFontAtlas::GetGlyphRangesChineseSimplifiedCommon()
{
    // Store 2500 regularly used characters for Simplified Chinese.
    // Sourced from https://zh.wiktionary.org/wiki/%E9%99%84%E5%BD%95:%E7%8E%B0%E4%BB%A3%E6%B1%89%E8%AF%AD%E5%B8%B8%E7%94%A8%E5%AD%97%E8%A1%A8
    // This table covers 97.97% of all characters used during the month in July, 1987.
    // You can use ImFontGlyphRangesBuilder to create your own ranges derived from this, by merging existing ranges or adding new characters.
    // (Stored as accumulative offsets from the initial unicode codepoint 0x4E00. This encoding is designed to helps us compact the source code size.)
    static const short accumulative_offsets_from_0x4E00[] =
    {
        0,1,2,4,1,1,1,1,2,1,3,2,1,2,2,1,1,1,1,1,5,2,1,2,3,3,3,2,2,4,1,1,1,2,1,5,2,3,1,2,1,2,1,1,2,1,1,2,2,1,4,1,1,1,1,5,10,1,2,19,2,1,2,1,2,1,2,1,2,
        1,5,1,6,3,2,1,2,2,1,1,1,4,8,5,1,1,4,1,1,3,1,2,1,5,1,2,1,1,1,10,1,1,5,2,4,6,1,4,2,2,2,12,2,1,1,6,1,1,1,4,1,1,4,6,5,1,4,2,2,4,10,7,1,1,4,2,4,
        2,1,4,3,6,10,12,5,7,2,14,2,9,1,1,6,7,10,4,7,13,1,5,4,8,4,1,1,2,28,5,6,1,1,5,2,5,20,2,2,9,8,11,2,9,17,1,8,6,8,27,4,6,9,20,11,27,6,68,2,2,1,1,
        1,2,1,2,2,7,6,11,3,3,1,1,3,1,2,1,1,1,1,1,3,1,1,8,3,4,1,5,7,2,1,4,4,8,4,2,1,2,1,1,4,5,6,3,6,2,12,3,1,3,9,2,4,3,4,1,5,3,3,1,3,7,1,5,1,1,1,1,2,
        3,4,5,2,3,2,6,1,1,2,1,7,1,7,3,4,5,15,2,2,1,5,3,22,19,2,1,1,1,1,2,5,1,1,1,6,1,1,12,8,2,9,18,22,4,1,1,5,1,16,1,2,7,10,15,1,1,6,2,4,1,2,4,1,6,
        1,1,3,2,4,1,6,4,5,1,2,1,1,2,1,10,3,1,3,2,1,9,3,2,5,7,2,19,4,3,6,1,1,1,1,1,4,3,2,1,1,1,2,5,3,1,1,1,2,2,1,1,2,1,1,2,1,3,1,1,1,3,7,1,4,1,1,2,1,
        1,2,1,2,4,4,3,8,1,1,1,2,1,3,5,1,3,1,3,4,6,2,2,14,4,6,6,11,9,1,15,3,1,28,5,2,5,5,3,1,3,4,5,4,6,14,3,2,3,5,21,2,7,20,10,1,2,19,2,4,28,28,2,3,
        2,1,14,4,1,26,28,42,12,40,3,52,79,5,14,17,3,2,2,11,3,4,6,3,1,8,2,23,4,5,8,10,4,2,7,3,5,1,1,6,3,1,2,2,2,5,28,1,1,7,7,20,5,3,29,3,17,26,1,8,4,
        27,3,6,11,23,5,3,4,6,13,24,16,6,5,10,25,35,7,3,2,3,3,14,3,6,2,6,1,4,2,3,8,2,1,1,3,3,3,4,1,1,13,2,2,4,5,2,1,14,14,1,2,2,1,4,5,2,3,1,14,3,12,
        3,17,2,16,5,1,2,1,8,9,3,19,4,2,2,4,17,25,21,20,28,75,1,10,29,103,4,1,2,1,1,4,2,4,1,2,3,24,2,2,2,1,1,2,1,3,8,1,1,1,2,1,1,3,1,1,1,6,1,5,3,1,1,
        1,3,4,1,1,5,2,1,5,6,13,9,16,1,1,1,1,3,2,3,2,4,5,2,5,2,2,3,7,13,7,2,2,1,1,1,1,2,3,3,2,1,6,4,9,2,1,14,2,14,2,1,18,3,4,14,4,11,41,15,23,15,23,
        176,1,3,4,1,1,1,1,5,3,1,2,3,7,3,1,1,2,1,2,4,4,6,2,4,1,9,7,1,10,5,8,16,29,1,1,2,2,3,1,3,5,2,4,5,4,1,1,2,2,3,3,7,1,6,10,1,17,1,44,4,6,2,1,1,6,
        5,4,2,10,1,6,9,2,8,1,24,1,2,13,7,8,8,2,1,4,1,3,1,3,3,5,2,5,10,9,4,9,12,2,1,6,1,10,1,1,7,7,4,10,8,3,1,13,4,3,1,6,1,3,5,2,1,2,17,16,5,2,16,6,
        1,4,2,1,3,3,6,8,5,11,11,1,3,3,2,4,6,10,9,5,7,4,7,4,7,1,1,4,2,1,3,6,8,7,1,6,11,5,5,3,24,9,4,2,7,13,5,1,8,82,16,61,1,1,1,4,2,2,16,10,3,8,1,1,
        6,4,2,1,3,1,1,1,4,3,8,4,2,2,1,1,1,1,1,6,3,5,1,1,4,6,9,2,1,1,1,2,1,7,2,1,6,1,5,4,4,3,1,8,1,3,3,1,3,2,2,2,2,3,1,6,1,2,1,2,1,3,7,1,8,2,1,2,1,5,
        2,5,3,5,10,1,2,1,1,3,2,5,11,3,9,3,5,1,1,5,9,1,2,1,5,7,9,9,8,1,3,3,3,6,8,2,3,2,1,1,32,6,1,2,15,9,3,7,13,1,3,10,13,2,14,1,13,10,2,1,3,10,4,15,
        2,15,15,10,1,3,9,6,9,32,25,26,47,7,3,2,3,1,6,3,4,3,2,8,5,4,1,9,4,2,2,19,10,6,2,3,8,1,2,2,4,2,1,9,4,4,4,6,4,8,9,2,3,1,1,1,1,3,5,5,1,3,8,4,6,
        2,1,4,12,1,5,3,7,13,2,5,8,1,6,1,2,5,14,6,1,5,2,4,8,15,5,1,23,6,62,2,10,1,1,8,1,2,2,10,4,2,2,9,2,1,1,3,2,3,1,5,3,3,2,1,3,8,1,1,1,11,3,1,1,4,
        3,7,1,14,1,2,3,12,5,2,5,1,6,7,5,7,14,11,1,3,1,8,9,12,2,1,11,8,4,4,2,6,10,9,13,1,1,3,1,5,1,3,2,4,4,1,18,2,3,14,11,4,29,4,2,7,1,3,13,9,2,2,5,
        3,5,20,7,16,8,5,72,34,6,4,22,12,12,28,45,36,9,7,39,9,191,1,1,1,4,11,8,4,9,2,3,22,1,1,1,1,4,17,1,7,7,1,11,31,10,2,4,8,2,3,2,1,4,2,16,4,32,2,
        3,19,13,4,9,1,5,2,14,8,1,1,3,6,19,6,5,1,16,6,2,10,8,5,1,2,3,1,5,5,1,11,6,6,1,3,3,2,6,3,8,1,1,4,10,7,5,7,7,5,8,9,2,1,3,4,1,1,3,1,3,3,2,6,16,
        1,4,6,3,1,10,6,1,3,15,2,9,2,10,25,13,9,16,6,2,2,10,11,4,3,9,1,2,6,6,5,4,30,40,1,10,7,12,14,33,6,3,6,7,3,1,3,1,11,14,4,9,5,12,11,49,18,51,31,
        140,31,2,2,1,5,1,8,1,10,1,4,4,3,24,1,10,1,3,6,6,16,3,4,5,2,1,4,2,57,10,6,22,2,22,3,7,22,6,10,11,36,18,16,33,36,2,5,5,1,1,1,4,10,1,4,13,2,7,
        5,2,9,3,4,1,7,43,3,7,3,9,14,7,9,1,11,1,1,3,7,4,18,13,1,14,1,3,6,10,73,2,2,30,6,1,11,18,19,13,22,3,46,42,37,89,7,3,16,34,2,2,3,9,1,7,1,1,1,2,
        2,4,10,7,3,10,3,9,5,28,9,2,6,13,7,3,1,3,10,2,7,2,11,3,6,21,54,85,2,1,4,2,2,1,39,3,21,2,2,5,1,1,1,4,1,1,3,4,15,1,3,2,4,4,2,3,8,2,20,1,8,7,13,
        4,1,26,6,2,9,34,4,21,52,10,4,4,1,5,12,2,11,1,7,2,30,12,44,2,30,1,1,3,6,16,9,17,39,82,2,2,24,7,1,7,3,16,9,14,44,2,1,2,1,2,3,5,2,4,1,6,7,5,3,
        2,6,1,11,5,11,2,1,18,19,8,1,3,24,29,2,1,3,5,2,2,1,13,6,5,1,46,11,3,5,1,1,5,8,2,10,6,12,6,3,7,11,2,4,16,13,2,5,1,1,2,2,5,2,28,5,2,23,10,8,4,
        4,22,39,95,38,8,14,9,5,1,13,5,4,3,13,12,11,1,9,1,27,37,2,5,4,4,63,211,95,2,2,2,1,3,5,2,1,1,2,2,1,1,1,3,2,4,1,2,1,1,5,2,2,1,1,2,3,1,3,1,1,1,
        3,1,4,2,1,3,6,1,1,3,7,15,5,3,2,5,3,9,11,4,2,22,1,6,3,8,7,1,4,28,4,16,3,3,25,4,4,27,27,1,4,1,2,2,7,1,3,5,2,28,8,2,14,1,8,6,16,25,3,3,3,14,3,
        3,1,1,2,1,4,6,3,8,4,1,1,1,2,3,6,10,6,2,3,18,3,2,5,5,4,3,1,5,2,5,4,23,7,6,12,6,4,17,11,9,5,1,1,10,5,12,1,1,11,26,33,7,3,6,1,17,7,1,5,12,1,11,
        2,4,1,8,14,17,23,1,2,1,7,8,16,11,9,6,5,2,6,4,16,2,8,14,1,11,8,9,1,1,1,9,25,4,11,19,7,2,15,2,12,8,52,7,5,19,2,16,4,36,8,1,16,8,24,26,4,6,2,9,
        5,4,36,3,28,12,25,15,37,27,17,12,59,38,5,32,127,1,2,9,17,14,4,1,2,1,1,8,11,50,4,14,2,19,16,4,17,5,4,5,26,12,45,2,23,45,104,30,12,8,3,10,2,2,
        3,3,1,4,20,7,2,9,6,15,2,20,1,3,16,4,11,15,6,134,2,5,59,1,2,2,2,1,9,17,3,26,137,10,211,59,1,2,4,1,4,1,1,1,2,6,2,3,1,1,2,3,2,3,1,3,4,4,2,3,3,
        1,4,3,1,7,2,2,3,1,2,1,3,3,3,2,2,3,2,1,3,14,6,1,3,2,9,6,15,27,9,34,145,1,1,2,1,1,1,1,2,1,1,1,1,2,2,2,3,1,2,1,1,1,2,3,5,8,3,5,2,4,1,3,2,2,2,12,
        4,1,1,1,10,4,5,1,20,4,16,1,15,9,5,12,2,9,2,5,4,2,26,19,7,1,26,4,30,12,15,42,1,6,8,172,1,1,4,2,1,1,11,2,2,4,2,1,2,1,10,8,1,2,1,4,5,1,2,5,1,8,
        4,1,3,4,2,1,6,2,1,3,4,1,2,1,1,1,1,12,5,7,2,4,3,1,1,1,3,3,6,1,2,2,3,3,3,2,1,2,12,14,11,6,6,4,12,2,8,1,7,10,1,35,7,4,13,15,4,3,23,21,28,52,5,
        26,5,6,1,7,10,2,7,53,3,2,1,1,1,2,163,532,1,10,11,1,3,3,4,8,2,8,6,2,2,23,22,4,2,2,4,2,1,3,1,3,3,5,9,8,2,1,2,8,1,10,2,12,21,20,15,105,2,3,1,1,
        3,2,3,1,1,2,5,1,4,15,11,19,1,1,1,1,5,4,5,1,1,2,5,3,5,12,1,2,5,1,11,1,1,15,9,1,4,5,3,26,8,2,1,3,1,1,15,19,2,12,1,2,5,2,7,2,19,2,20,6,26,7,5,
        2,2,7,34,21,13,70,2,128,1,1,2,1,1,2,1,1,3,2,2,2,15,1,4,1,3,4,42,10,6,1,49,85,8,1,2,1,1,4,4,2,3,6,1,5,7,4,3,211,4,1,2,1,2,5,1,2,4,2,2,6,5,6,
        10,3,4,48,100,6,2,16,296,5,27,387,2,2,3,7,16,8,5,38,15,39,21,9,10,3,7,59,13,27,21,47,5,21,6
    };
    static ImWchar base_ranges[] = // not zero-terminated
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x2000, 0x206F, // General Punctuation
        0x3000, 0x30FF, // CJK Symbols and Punctuations, Hiragana, Katakana
        0x31F0, 0x31FF, // Katakana Phonetic Extensions
        0xFF00, 0xFFEF  // Half-width characters
    };
    static ImWchar full_ranges[IM_ARRAYSIZE(base_ranges) + IM_ARRAYSIZE(accumulative_offsets_from_0x4E00) * 2 + 1] = { 0 };
    if (!full_ranges[0])
    {
        memcpy(full_ranges, base_ranges, sizeof(base_ranges));
        UnpackAccumulativeOffsetsIntoRanges(0x4E00, accumulative_offsets_from_0x4E00, IM_ARRAYSIZE(accumulative_offsets_from_0x4E00), full_ranges + IM_ARRAYSIZE(base_ranges));
    }
    return &full_ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesJapanese()
{
    // 1946 common ideograms code points for Japanese
    // Sourced from http://theinstructionlimit.com/common-kanji-character-ranges-for-xna-spritefont-rendering
    // FIXME: Source a list of the revised 2136 Joyo Kanji list from 2010 and rebuild this.
    // You can use ImFontGlyphRangesBuilder to create your own ranges derived from this, by merging existing ranges or adding new characters.
    // (Stored as accumulative offsets from the initial unicode codepoint 0x4E00. This encoding is designed to helps us compact the source code size.)
    static const short accumulative_offsets_from_0x4E00[] =
    {
        0,1,2,4,1,1,1,1,2,1,6,2,2,1,8,5,7,11,1,2,10,10,8,2,4,20,2,11,8,2,1,2,1,6,2,1,7,5,3,7,1,1,13,7,9,1,4,6,1,2,1,10,1,1,9,2,2,4,5,6,14,1,1,9,3,18,
        5,4,2,2,10,7,1,1,1,3,2,4,3,23,2,10,12,2,14,2,4,13,1,6,10,3,1,7,13,6,4,13,5,2,3,17,2,2,5,7,6,4,1,7,14,16,6,13,9,15,1,1,7,16,4,7,1,19,9,2,7,15,
        2,6,5,13,25,4,14,13,11,25,1,1,1,2,1,2,2,3,10,11,3,3,1,1,4,4,2,1,4,9,1,4,3,5,5,2,7,12,11,15,7,16,4,5,16,2,1,1,6,3,3,1,1,2,7,6,6,7,1,4,7,6,1,1,
        2,1,12,3,3,9,5,8,1,11,1,2,3,18,20,4,1,3,6,1,7,3,5,5,7,2,2,12,3,1,4,2,3,2,3,11,8,7,4,17,1,9,25,1,1,4,2,2,4,1,2,7,1,1,1,3,1,2,6,16,1,2,1,1,3,12,
        20,2,5,20,8,7,6,2,1,1,1,1,6,2,1,2,10,1,1,6,1,3,1,2,1,4,1,12,4,1,3,1,1,1,1,1,10,4,7,5,13,1,15,1,1,30,11,9,1,15,38,14,1,32,17,20,1,9,31,2,21,9,
        4,49,22,2,1,13,1,11,45,35,43,55,12,19,83,1,3,2,3,13,2,1,7,3,18,3,13,8,1,8,18,5,3,7,25,24,9,24,40,3,17,24,2,1,6,2,3,16,15,6,7,3,12,1,9,7,3,3,
        3,15,21,5,16,4,5,12,11,11,3,6,3,2,31,3,2,1,1,23,6,6,1,4,2,6,5,2,1,1,3,3,22,2,6,2,3,17,3,2,4,5,1,9,5,1,1,6,15,12,3,17,2,14,2,8,1,23,16,4,2,23,
        8,15,23,20,12,25,19,47,11,21,65,46,4,3,1,5,6,1,2,5,26,2,1,1,3,11,1,1,1,2,1,2,3,1,1,10,2,3,1,1,1,3,6,3,2,2,6,6,9,2,2,2,6,2,5,10,2,4,1,2,1,2,2,
        3,1,1,3,1,2,9,23,9,2,1,1,1,1,5,3,2,1,10,9,6,1,10,2,31,25,3,7,5,40,1,15,6,17,7,27,180,1,3,2,2,1,1,1,6,3,10,7,1,3,6,17,8,6,2,2,1,3,5,5,8,16,14,
        15,1,1,4,1,2,1,1,1,3,2,7,5,6,2,5,10,1,4,2,9,1,1,11,6,1,44,1,3,7,9,5,1,3,1,1,10,7,1,10,4,2,7,21,15,7,2,5,1,8,3,4,1,3,1,6,1,4,2,1,4,10,8,1,4,5,
        1,5,10,2,7,1,10,1,1,3,4,11,10,29,4,7,3,5,2,3,33,5,2,19,3,1,4,2,6,31,11,1,3,3,3,1,8,10,9,12,11,12,8,3,14,8,6,11,1,4,41,3,1,2,7,13,1,5,6,2,6,12,
        12,22,5,9,4,8,9,9,34,6,24,1,1,20,9,9,3,4,1,7,2,2,2,6,2,28,5,3,6,1,4,6,7,4,2,1,4,2,13,6,4,4,3,1,8,8,3,2,1,5,1,2,2,3,1,11,11,7,3,6,10,8,6,16,16,
        22,7,12,6,21,5,4,6,6,3,6,1,3,2,1,2,8,29,1,10,1,6,13,6,6,19,31,1,13,4,4,22,17,26,33,10,4,15,12,25,6,67,10,2,3,1,6,10,2,6,2,9,1,9,4,4,1,2,16,2,
        5,9,2,3,8,1,8,3,9,4,8,6,4,8,11,3,2,1,1,3,26,1,7,5,1,11,1,5,3,5,2,13,6,39,5,1,5,2,11,6,10,5,1,15,5,3,6,19,21,22,2,4,1,6,1,8,1,4,8,2,4,2,2,9,2,
        1,1,1,4,3,6,3,12,7,1,14,2,4,10,2,13,1,17,7,3,2,1,3,2,13,7,14,12,3,1,29,2,8,9,15,14,9,14,1,3,1,6,5,9,11,3,38,43,20,7,7,8,5,15,12,19,15,81,8,7,
        1,5,73,13,37,28,8,8,1,15,18,20,165,28,1,6,11,8,4,14,7,15,1,3,3,6,4,1,7,14,1,1,11,30,1,5,1,4,14,1,4,2,7,52,2,6,29,3,1,9,1,21,3,5,1,26,3,11,14,
        11,1,17,5,1,2,1,3,2,8,1,2,9,12,1,1,2,3,8,3,24,12,7,7,5,17,3,3,3,1,23,10,4,4,6,3,1,16,17,22,3,10,21,16,16,6,4,10,2,1,1,2,8,8,6,5,3,3,3,39,25,
        15,1,1,16,6,7,25,15,6,6,12,1,22,13,1,4,9,5,12,2,9,1,12,28,8,3,5,10,22,60,1,2,40,4,61,63,4,1,13,12,1,4,31,12,1,14,89,5,16,6,29,14,2,5,49,18,18,
        5,29,33,47,1,17,1,19,12,2,9,7,39,12,3,7,12,39,3,1,46,4,12,3,8,9,5,31,15,18,3,2,2,66,19,13,17,5,3,46,124,13,57,34,2,5,4,5,8,1,1,1,4,3,1,17,5,
        3,5,3,1,8,5,6,3,27,3,26,7,12,7,2,17,3,7,18,78,16,4,36,1,2,1,6,2,1,39,17,7,4,13,4,4,4,1,10,4,2,4,6,3,10,1,19,1,26,2,4,33,2,73,47,7,3,8,2,4,15,
        18,1,29,2,41,14,1,21,16,41,7,39,25,13,44,2,2,10,1,13,7,1,7,3,5,20,4,8,2,49,1,10,6,1,6,7,10,7,11,16,3,12,20,4,10,3,1,2,11,2,28,9,2,4,7,2,15,1,
        27,1,28,17,4,5,10,7,3,24,10,11,6,26,3,2,7,2,2,49,16,10,16,15,4,5,27,61,30,14,38,22,2,7,5,1,3,12,23,24,17,17,3,3,2,4,1,6,2,7,5,1,1,5,1,1,9,4,
        1,3,6,1,8,2,8,4,14,3,5,11,4,1,3,32,1,19,4,1,13,11,5,2,1,8,6,8,1,6,5,13,3,23,11,5,3,16,3,9,10,1,24,3,198,52,4,2,2,5,14,5,4,22,5,20,4,11,6,41,
        1,5,2,2,11,5,2,28,35,8,22,3,18,3,10,7,5,3,4,1,5,3,8,9,3,6,2,16,22,4,5,5,3,3,18,23,2,6,23,5,27,8,1,33,2,12,43,16,5,2,3,6,1,20,4,2,9,7,1,11,2,
        10,3,14,31,9,3,25,18,20,2,5,5,26,14,1,11,17,12,40,19,9,6,31,83,2,7,9,19,78,12,14,21,76,12,113,79,34,4,1,1,61,18,85,10,2,2,13,31,11,50,6,33,159,
        179,6,6,7,4,4,2,4,2,5,8,7,20,32,22,1,3,10,6,7,28,5,10,9,2,77,19,13,2,5,1,4,4,7,4,13,3,9,31,17,3,26,2,6,6,5,4,1,7,11,3,4,2,1,6,2,20,4,1,9,2,6,
        3,7,1,1,1,20,2,3,1,6,2,3,6,2,4,8,1,5,13,8,4,11,23,1,10,6,2,1,3,21,2,2,4,24,31,4,10,10,2,5,192,15,4,16,7,9,51,1,2,1,1,5,1,1,2,1,3,5,3,1,3,4,1,
        3,1,3,3,9,8,1,2,2,2,4,4,18,12,92,2,10,4,3,14,5,25,16,42,4,14,4,2,21,5,126,30,31,2,1,5,13,3,22,5,6,6,20,12,1,14,12,87,3,19,1,8,2,9,9,3,3,23,2,
        3,7,6,3,1,2,3,9,1,3,1,6,3,2,1,3,11,3,1,6,10,3,2,3,1,2,1,5,1,1,11,3,6,4,1,7,2,1,2,5,5,34,4,14,18,4,19,7,5,8,2,6,79,1,5,2,14,8,2,9,2,1,36,28,16,
        4,1,1,1,2,12,6,42,39,16,23,7,15,15,3,2,12,7,21,64,6,9,28,8,12,3,3,41,59,24,51,55,57,294,9,9,2,6,2,15,1,2,13,38,90,9,9,9,3,11,7,1,1,1,5,6,3,2,
        1,2,2,3,8,1,4,4,1,5,7,1,4,3,20,4,9,1,1,1,5,5,17,1,5,2,6,2,4,1,4,5,7,3,18,11,11,32,7,5,4,7,11,127,8,4,3,3,1,10,1,1,6,21,14,1,16,1,7,1,3,6,9,65,
        51,4,3,13,3,10,1,1,12,9,21,110,3,19,24,1,1,10,62,4,1,29,42,78,28,20,18,82,6,3,15,6,84,58,253,15,155,264,15,21,9,14,7,58,40,39,
    };
    static ImWchar base_ranges[] = // not zero-terminated
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x3000, 0x30FF, // CJK Symbols and Punctuations, Hiragana, Katakana
        0x31F0, 0x31FF, // Katakana Phonetic Extensions
        0xFF00, 0xFFEF  // Half-width characters
    };
    static ImWchar full_ranges[IM_ARRAYSIZE(base_ranges) + IM_ARRAYSIZE(accumulative_offsets_from_0x4E00) * 2 + 1] = { 0 };
    if (!full_ranges[0])
    {
        memcpy(full_ranges, base_ranges, sizeof(base_ranges));
        UnpackAccumulativeOffsetsIntoRanges(0x4E00, accumulative_offsets_from_0x4E00, IM_ARRAYSIZE(accumulative_offsets_from_0x4E00), full_ranges + IM_ARRAYSIZE(base_ranges));
    }
    return &full_ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesCyrillic()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
        0x2DE0, 0x2DFF, // Cyrillic Extended-A
        0xA640, 0xA69F, // Cyrillic Extended-B
        0,
    };
    return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesThai()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin
        0x2010, 0x205E, // Punctuations
        0x0E00, 0x0E7F, // Thai
        0,
    };
    return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesVietnamese()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin
        0x0102, 0x0103,
        0x0110, 0x0111,
        0x0128, 0x0129,
        0x0168, 0x0169,
        0x01A0, 0x01A1,
        0x01AF, 0x01B0,
        0x1EA0, 0x1EF9,
        0,
    };
    return &ranges[0];
}

//-----------------------------------------------------------------------------
// [SECTION] ImFontGlyphRangesBuilder
//-----------------------------------------------------------------------------

void ImFontGlyphRangesBuilder::AddText(const char* text, const char* text_end)
{
    while (text_end ? (text < text_end) : *text)
    {
        unsigned int c = 0;
        int c_len = ImTextCharFromUtf8(&c, text, text_end);
        text += c_len;
        if (c_len == 0)
            break;
        AddChar((ImWchar)c);
    }
}

void ImFontGlyphRangesBuilder::AddRanges(const ImWchar* ranges)
{
    for (; ranges[0]; ranges += 2)
        for (ImWchar c = ranges[0]; c <= ranges[1]; c++)
            AddChar(c);
}

void ImFontGlyphRangesBuilder::BuildRanges(ImVector<ImWchar>* out_ranges)
{
    const int max_codepoint = IM_UNICODE_CODEPOINT_MAX;
    for (int n = 0; n <= max_codepoint; n++)
        if (GetBit(n))
        {
            out_ranges->push_back((ImWchar)n);
            while (n < max_codepoint && GetBit(n + 1))
                n++;
            out_ranges->push_back((ImWchar)n);
        }
    out_ranges->push_back(0);
}

//-----------------------------------------------------------------------------
// [SECTION] ImFont
//-----------------------------------------------------------------------------

ImFont::ImFont()
{
    FontSize = 0.0f;
    FallbackAdvanceX = 0.0f;
    FallbackChar = (ImWchar)'?';
    EllipsisChar = (ImWchar)-1;
    DisplayOffset = ImVec2(0.0f, 0.0f);
    FallbackGlyph = NULL;
    ContainerAtlas = NULL;
    ConfigData = NULL;
    ConfigDataCount = 0;
    DirtyLookupTables = false;
    Scale = 1.0f;
    Ascent = Descent = 0.0f;
    MetricsTotalSurface = 0;
    memset(Used4kPagesMap, 0, sizeof(Used4kPagesMap));
}

ImFont::~ImFont()
{
    ClearOutputData();
}

void    ImFont::ClearOutputData()
{
    FontSize = 0.0f;
    FallbackAdvanceX = 0.0f;
    Glyphs.clear();
    IndexAdvanceX.clear();
    IndexLookup.clear();
    FallbackGlyph = NULL;
    ContainerAtlas = NULL;
    DirtyLookupTables = true;
    Ascent = Descent = 0.0f;
    MetricsTotalSurface = 0;
}

void ImFont::BuildLookupTable()
{
    int max_codepoint = 0;
    for (int i = 0; i != Glyphs.Size; i++)
        max_codepoint = ImMax(max_codepoint, (int)Glyphs[i].Codepoint);

    // Build lookup table
    IM_ASSERT(Glyphs.Size < 0xFFFF); // -1 is reserved
    IndexAdvanceX.clear();
    IndexLookup.clear();
    DirtyLookupTables = false;
    memset(Used4kPagesMap, 0, sizeof(Used4kPagesMap));
    GrowIndex(max_codepoint + 1);
    for (int i = 0; i < Glyphs.Size; i++)
    {
        int codepoint = (int)Glyphs[i].Codepoint;
        IndexAdvanceX[codepoint] = Glyphs[i].AdvanceX;
        IndexLookup[codepoint] = (ImWchar)i;

        // Mark 4K page as used
        const int page_n = codepoint / 4096;
        Used4kPagesMap[page_n >> 3] |= 1 << (page_n & 7);
    }

    // Create a glyph to handle TAB
    // FIXME: Needs proper TAB handling but it needs to be contextualized (or we could arbitrary say that each string starts at "column 0" ?)
    if (FindGlyph((ImWchar)' '))
    {
        if (Glyphs.back().Codepoint != '\t')   // So we can call this function multiple times (FIXME: Flaky)
            Glyphs.resize(Glyphs.Size + 1);
        ImFontGlyph& tab_glyph = Glyphs.back();
        tab_glyph = *FindGlyph((ImWchar)' ');
        tab_glyph.Codepoint = '\t';
        tab_glyph.AdvanceX *= IM_TABSIZE;
        IndexAdvanceX[(int)tab_glyph.Codepoint] = (float)tab_glyph.AdvanceX;
        IndexLookup[(int)tab_glyph.Codepoint] = (ImWchar)(Glyphs.Size - 1);
    }

    // Mark special glyphs as not visible (note that AddGlyph already mark as non-visible glyphs with zero-size polygons)
    SetGlyphVisible((ImWchar)' ', false);
    SetGlyphVisible((ImWchar)'\t', false);

    // Setup fall-backs
    FallbackGlyph = FindGlyphNoFallback(FallbackChar);
    FallbackAdvanceX = FallbackGlyph ? FallbackGlyph->AdvanceX : 0.0f;
    for (int i = 0; i < max_codepoint + 1; i++)
        if (IndexAdvanceX[i] < 0.0f)
            IndexAdvanceX[i] = FallbackAdvanceX;
}

// API is designed this way to avoid exposing the 4K page size
// e.g. use with IsGlyphRangeUnused(0, 255)
bool ImFont::IsGlyphRangeUnused(unsigned int c_begin, unsigned int c_last)
{
    unsigned int page_begin = (c_begin / 4096);
    unsigned int page_last = (c_last / 4096);
    for (unsigned int page_n = page_begin; page_n <= page_last; page_n++)
        if ((page_n >> 3) < sizeof(Used4kPagesMap))
            if (Used4kPagesMap[page_n >> 3] & (1 << (page_n & 7)))
                return false;
    return true;
}

void ImFont::SetGlyphVisible(ImWchar c, bool visible)
{
    if (ImFontGlyph* glyph = (ImFontGlyph*)(void*)FindGlyph((ImWchar)c))
        glyph->Visible = visible ? 1 : 0;
}

void ImFont::SetFallbackChar(ImWchar c)
{
    FallbackChar = c;
    BuildLookupTable();
}

void ImFont::GrowIndex(int new_size)
{
    IM_ASSERT(IndexAdvanceX.Size == IndexLookup.Size);
    if (new_size <= IndexLookup.Size)
        return;
    IndexAdvanceX.resize(new_size, -1.0f);
    IndexLookup.resize(new_size, (ImWchar)-1);
}

// x0/y0/x1/y1 are offset from the character upper-left layout position, in pixels. Therefore x0/y0 are often fairly close to zero.
// Not to be mistaken with texture coordinates, which are held by u0/v0/u1/v1 in normalized format (0.0..1.0 on each texture axis).
void ImFont::AddGlyph(ImWchar codepoint, float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, float advance_x)
{
    Glyphs.resize(Glyphs.Size + 1);
    ImFontGlyph& glyph = Glyphs.back();
    glyph.Codepoint = (unsigned int)codepoint;
    glyph.Visible = (x0 != x1) && (y0 != y1);
    glyph.X0 = x0;
    glyph.Y0 = y0;
    glyph.X1 = x1;
    glyph.Y1 = y1;
    glyph.U0 = u0;
    glyph.V0 = v0;
    glyph.U1 = u1;
    glyph.V1 = v1;
    glyph.AdvanceX = advance_x + ConfigData->GlyphExtraSpacing.x;  // Bake spacing into AdvanceX

    if (ConfigData->PixelSnapH)
        glyph.AdvanceX = IM_ROUND(glyph.AdvanceX);

    // Compute rough surface usage metrics (+1 to account for average padding, +0.99 to round)
    DirtyLookupTables = true;
    MetricsTotalSurface += (int)((glyph.U1 - glyph.U0) * ContainerAtlas->TexWidth + 1.99f) * (int)((glyph.V1 - glyph.V0) * ContainerAtlas->TexHeight + 1.99f);
}

void ImFont::AddRemapChar(ImWchar dst, ImWchar src, bool overwrite_dst)
{
    IM_ASSERT(IndexLookup.Size > 0);    // Currently this can only be called AFTER the font has been built, aka after calling ImFontAtlas::GetTexDataAs*() function.
    unsigned int index_size = (unsigned int)IndexLookup.Size;

    if (dst < index_size && IndexLookup.Data[dst] == (ImWchar)-1 && !overwrite_dst) // 'dst' already exists
        return;
    if (src >= index_size && dst >= index_size) // both 'dst' and 'src' don't exist -> no-op
        return;

    GrowIndex(dst + 1);
    IndexLookup[dst] = (src < index_size) ? IndexLookup.Data[src] : (ImWchar)-1;
    IndexAdvanceX[dst] = (src < index_size) ? IndexAdvanceX.Data[src] : 1.0f;
}

const ImFontGlyph* ImFont::FindGlyph(ImWchar c) const
{
    if (c >= (size_t)IndexLookup.Size)
        return FallbackGlyph;
    const ImWchar i = IndexLookup.Data[c];
    if (i == (ImWchar)-1)
        return FallbackGlyph;
    return &Glyphs.Data[i];
}

const ImFontGlyph* ImFont::FindGlyphNoFallback(ImWchar c) const
{
    if (c >= (size_t)IndexLookup.Size)
        return NULL;
    const ImWchar i = IndexLookup.Data[c];
    if (i == (ImWchar)-1)
        return NULL;
    return &Glyphs.Data[i];
}

const char* ImFont::CalcWordWrapPositionA(float scale, const char* text, const char* text_end, float wrap_width) const
{
    // Simple word-wrapping for English, not full-featured. Please submit failing cases!
    // FIXME: Much possible improvements (don't cut things like "word !", "word!!!" but cut within "word,,,,", more sensible support for punctuations, support for Unicode punctuations, etc.)

    // For references, possible wrap point marked with ^
    //  "aaa bbb, ccc,ddd. eee   fff. ggg!"
    //      ^    ^    ^   ^   ^__    ^    ^

    // List of hardcoded separators: .,;!?'"

    // Skip extra blanks after a line returns (that includes not counting them in width computation)
    // e.g. "Hello    world" --> "Hello" "World"

    // Cut words that cannot possibly fit within one line.
    // e.g.: "The tropical fish" with ~5 characters worth of width --> "The tr" "opical" "fish"

    float line_width = 0.0f;
    float word_width = 0.0f;
    float blank_width = 0.0f;
    wrap_width /= scale; // We work with unscaled widths to avoid scaling every characters

    const char* word_end = text;
    const char* prev_word_end = NULL;
    bool inside_word = true;

    const char* s = text;
    while (s < text_end)
    {
        unsigned int c = (unsigned int)*s;
        const char* next_s;
        if (c < 0x80)
            next_s = s + 1;
        else
            next_s = s + ImTextCharFromUtf8(&c, s, text_end);
        if (c == 0)
            break;

        if (c < 32)
        {
            if (c == '\n')
            {
                line_width = word_width = blank_width = 0.0f;
                inside_word = true;
                s = next_s;
                continue;
            }
            if (c == '\r')
            {
                s = next_s;
                continue;
            }
        }

        const float char_width = ((int)c < IndexAdvanceX.Size ? IndexAdvanceX.Data[c] : FallbackAdvanceX);
        if (ImCharIsBlankW(c))
        {
            if (inside_word)
            {
                line_width += blank_width;
                blank_width = 0.0f;
                word_end = s;
            }
            blank_width += char_width;
            inside_word = false;
        }
        else
        {
            word_width += char_width;
            if (inside_word)
            {
                word_end = next_s;
            }
            else
            {
                prev_word_end = word_end;
                line_width += word_width + blank_width;
                word_width = blank_width = 0.0f;
            }

            // Allow wrapping after punctuation.
            inside_word = !(c == '.' || c == ',' || c == ';' || c == '!' || c == '?' || c == '\"');
        }

        // We ignore blank width at the end of the line (they can be skipped)
        if (line_width + word_width > wrap_width)
        {
            // Words that cannot possibly fit within an entire line will be cut anywhere.
            if (word_width < wrap_width)
                s = prev_word_end ? prev_word_end : word_end;
            break;
        }

        s = next_s;
    }

    return s;
}

ImVec2 ImFont::CalcTextSizeA(float size, float max_width, float wrap_width, const char* text_begin, const char* text_end, const char** remaining) const
{
    if (!text_end)
        text_end = text_begin + strlen(text_begin); // FIXME-OPT: Need to avoid this.

    const float line_height = size;
    const float scale = size / FontSize;

    ImVec2 text_size = ImVec2(0, 0);
    float line_width = 0.0f;

    const bool word_wrap_enabled = (wrap_width > 0.0f);
    const char* word_wrap_eol = NULL;

    const char* s = text_begin;
    while (s < text_end)
    {
        if (word_wrap_enabled)
        {
            // Calculate how far we can render. Requires two passes on the string data but keeps the code simple and not intrusive for what's essentially an uncommon feature.
            if (!word_wrap_eol)
            {
                word_wrap_eol = CalcWordWrapPositionA(scale, s, text_end, wrap_width - line_width);
                if (word_wrap_eol == s) // Wrap_width is too small to fit anything. Force displaying 1 character to minimize the height discontinuity.
                    word_wrap_eol++;    // +1 may not be a character start point in UTF-8 but it's ok because we use s >= word_wrap_eol below
            }

            if (s >= word_wrap_eol)
            {
                if (text_size.x < line_width)
                    text_size.x = line_width;
                text_size.y += line_height;
                line_width = 0.0f;
                word_wrap_eol = NULL;

                // Wrapping skips upcoming blanks
                while (s < text_end)
                {
                    const char c = *s;
                    if (ImCharIsBlankA(c)) { s++; }
                    else if (c == '\n') { s++; break; }
                    else { break; }
                }
                continue;
            }
        }

        // Decode and advance source
        const char* prev_s = s;
        unsigned int c = (unsigned int)*s;
        if (c < 0x80)
        {
            s += 1;
        }
        else
        {
            s += ImTextCharFromUtf8(&c, s, text_end);
            if (c == 0) // Malformed UTF-8?
                break;
        }

        if (c < 32)
        {
            if (c == '\n')
            {
                text_size.x = ImMax(text_size.x, line_width);
                text_size.y += line_height;
                line_width = 0.0f;
                continue;
            }
            if (c == '\r')
                continue;
        }

        const float char_width = ((int)c < IndexAdvanceX.Size ? IndexAdvanceX.Data[c] : FallbackAdvanceX) * scale;
        if (line_width + char_width >= max_width)
        {
            s = prev_s;
            break;
        }

        line_width += char_width;
    }

    if (text_size.x < line_width)
        text_size.x = line_width;

    if (line_width > 0 || text_size.y == 0.0f)
        text_size.y += line_height;

    if (remaining)
        *remaining = s;

    return text_size;
}

void ImFont::RenderChar(ImDrawList* draw_list, float size, ImVec2 pos, ImU32 col, ImWchar c) const
{
    const ImFontGlyph* glyph = FindGlyph(c);
    if (!glyph || !glyph->Visible)
        return;
    float scale = (size >= 0.0f) ? (size / FontSize) : 1.0f;
    pos.x = IM_FLOOR(pos.x + DisplayOffset.x);
    pos.y = IM_FLOOR(pos.y + DisplayOffset.y);
    draw_list->PrimReserve(6, 4);
    draw_list->PrimRectUV(ImVec2(pos.x + glyph->X0 * scale, pos.y + glyph->Y0 * scale), ImVec2(pos.x + glyph->X1 * scale, pos.y + glyph->Y1 * scale), ImVec2(glyph->U0, glyph->V0), ImVec2(glyph->U1, glyph->V1), col);
}

void ImFont::RenderText(ImDrawList* draw_list, float size, ImVec2 pos, ImU32 col, const ImVec4& clip_rect, const char* text_begin, const char* text_end, float wrap_width, bool cpu_fine_clip) const
{
    if (!text_end)
        text_end = text_begin + strlen(text_begin); // ImGui:: functions generally already provides a valid text_end, so this is merely to handle direct calls.

    // Align to be pixel perfect
    pos.x = IM_FLOOR(pos.x + DisplayOffset.x);
    pos.y = IM_FLOOR(pos.y + DisplayOffset.y);
    float x = pos.x;
    float y = pos.y;
    if (y > clip_rect.w)
        return;

    const float scale = size / FontSize;
    const float line_height = FontSize * scale;
    const bool word_wrap_enabled = (wrap_width > 0.0f);
    const char* word_wrap_eol = NULL;

    // Fast-forward to first visible line
    const char* s = text_begin;
    if (y + line_height < clip_rect.y && !word_wrap_enabled)
        while (y + line_height < clip_rect.y && s < text_end)
        {
            s = (const char*)memchr(s, '\n', text_end - s);
            s = s ? s + 1 : text_end;
            y += line_height;
        }

    // For large text, scan for the last visible line in order to avoid over-reserving in the call to PrimReserve()
    // Note that very large horizontal line will still be affected by the issue (e.g. a one megabyte string buffer without a newline will likely crash atm)
    if (text_end - s > 10000 && !word_wrap_enabled)
    {
        const char* s_end = s;
        float y_end = y;
        while (y_end < clip_rect.w && s_end < text_end)
        {
            s_end = (const char*)memchr(s_end, '\n', text_end - s_end);
            s_end = s_end ? s_end + 1 : text_end;
            y_end += line_height;
        }
        text_end = s_end;
    }
    if (s == text_end)
        return;

    // Reserve vertices for remaining worse case (over-reserving is useful and easily amortized)
    const int vtx_count_max = (int)(text_end - s) * 4;
    const int idx_count_max = (int)(text_end - s) * 6;
    const int idx_expected_size = draw_list->IdxBuffer.Size + idx_count_max;
    draw_list->PrimReserve(idx_count_max, vtx_count_max);

    ImDrawVert* vtx_write = draw_list->_VtxWritePtr;
    ImDrawIdx* idx_write = draw_list->_IdxWritePtr;
    unsigned int vtx_current_idx = draw_list->_VtxCurrentIdx;

    while (s < text_end)
    {
        if (word_wrap_enabled)
        {
            // Calculate how far we can render. Requires two passes on the string data but keeps the code simple and not intrusive for what's essentially an uncommon feature.
            if (!word_wrap_eol)
            {
                word_wrap_eol = CalcWordWrapPositionA(scale, s, text_end, wrap_width - (x - pos.x));
                if (word_wrap_eol == s) // Wrap_width is too small to fit anything. Force displaying 1 character to minimize the height discontinuity.
                    word_wrap_eol++;    // +1 may not be a character start point in UTF-8 but it's ok because we use s >= word_wrap_eol below
            }

            if (s >= word_wrap_eol)
            {
                x = pos.x;
                y += line_height;
                word_wrap_eol = NULL;

                // Wrapping skips upcoming blanks
                while (s < text_end)
                {
                    const char c = *s;
                    if (ImCharIsBlankA(c)) { s++; }
                    else if (c == '\n') { s++; break; }
                    else { break; }
                }
                continue;
            }
        }

        // Decode and advance source
        unsigned int c = (unsigned int)*s;
        if (c < 0x80)
        {
            s += 1;
        }
        else
        {
            s += ImTextCharFromUtf8(&c, s, text_end);
            if (c == 0) // Malformed UTF-8?
                break;
        }

        if (c < 32)
        {
            if (c == '\n')
            {
                x = pos.x;
                y += line_height;
                if (y > clip_rect.w)
                    break; // break out of main loop
                continue;
            }
            if (c == '\r')
                continue;
        }

        const ImFontGlyph* glyph = FindGlyph((ImWchar)c);
        if (glyph == NULL)
            continue;

        float char_width = glyph->AdvanceX * scale;
        if (glyph->Visible)
        {
            // We don't do a second finer clipping test on the Y axis as we've already skipped anything before clip_rect.y and exit once we pass clip_rect.w
            float x1 = x + glyph->X0 * scale;
            float x2 = x + glyph->X1 * scale;
            float y1 = y + glyph->Y0 * scale;
            float y2 = y + glyph->Y1 * scale;
            if (x1 <= clip_rect.z && x2 >= clip_rect.x)
            {
                // Render a character
                float u1 = glyph->U0;
                float v1 = glyph->V0;
                float u2 = glyph->U1;
                float v2 = glyph->V1;

                // CPU side clipping used to fit text in their frame when the frame is too small. Only does clipping for axis aligned quads.
                if (cpu_fine_clip)
                {
                    if (x1 < clip_rect.x)
                    {
                        u1 = u1 + (1.0f - (x2 - clip_rect.x) / (x2 - x1)) * (u2 - u1);
                        x1 = clip_rect.x;
                    }
                    if (y1 < clip_rect.y)
                    {
                        v1 = v1 + (1.0f - (y2 - clip_rect.y) / (y2 - y1)) * (v2 - v1);
                        y1 = clip_rect.y;
                    }
                    if (x2 > clip_rect.z)
                    {
                        u2 = u1 + ((clip_rect.z - x1) / (x2 - x1)) * (u2 - u1);
                        x2 = clip_rect.z;
                    }
                    if (y2 > clip_rect.w)
                    {
                        v2 = v1 + ((clip_rect.w - y1) / (y2 - y1)) * (v2 - v1);
                        y2 = clip_rect.w;
                    }
                    if (y1 >= y2)
                    {
                        x += char_width;
                        continue;
                    }
                }

                // We are NOT calling PrimRectUV() here because non-inlined causes too much overhead in a debug builds. Inlined here:
                {
                    idx_write[0] = (ImDrawIdx)(vtx_current_idx); idx_write[1] = (ImDrawIdx)(vtx_current_idx + 1); idx_write[2] = (ImDrawIdx)(vtx_current_idx + 2);
                    idx_write[3] = (ImDrawIdx)(vtx_current_idx); idx_write[4] = (ImDrawIdx)(vtx_current_idx + 2); idx_write[5] = (ImDrawIdx)(vtx_current_idx + 3);
                    vtx_write[0].pos.x = x1; vtx_write[0].pos.y = y1; vtx_write[0].col = col; vtx_write[0].uv.x = u1; vtx_write[0].uv.y = v1;
                    vtx_write[1].pos.x = x2; vtx_write[1].pos.y = y1; vtx_write[1].col = col; vtx_write[1].uv.x = u2; vtx_write[1].uv.y = v1;
                    vtx_write[2].pos.x = x2; vtx_write[2].pos.y = y2; vtx_write[2].col = col; vtx_write[2].uv.x = u2; vtx_write[2].uv.y = v2;
                    vtx_write[3].pos.x = x1; vtx_write[3].pos.y = y2; vtx_write[3].col = col; vtx_write[3].uv.x = u1; vtx_write[3].uv.y = v2;
                    vtx_write += 4;
                    vtx_current_idx += 4;
                    idx_write += 6;
                }
            }
        }
        x += char_width;
    }

    // Give back unused vertices (clipped ones, blanks) ~ this is essentially a PrimUnreserve() action.
    draw_list->VtxBuffer.Size = (int)(vtx_write - draw_list->VtxBuffer.Data); // Same as calling shrink()
    draw_list->IdxBuffer.Size = (int)(idx_write - draw_list->IdxBuffer.Data);
    draw_list->CmdBuffer[draw_list->CmdBuffer.Size - 1].ElemCount -= (idx_expected_size - draw_list->IdxBuffer.Size);
    draw_list->_VtxWritePtr = vtx_write;
    draw_list->_IdxWritePtr = idx_write;
    draw_list->_VtxCurrentIdx = vtx_current_idx;
}

//-----------------------------------------------------------------------------
// [SECTION] ImGui Internal Render Helpers
//-----------------------------------------------------------------------------
// Vaguely redesigned to stop accessing ImGui global state:
// - RenderArrow()
// - RenderBullet()
// - RenderCheckMark()
// - RenderMouseCursor()
// - RenderArrowPointingAt()
// - RenderRectFilledRangeH()
//-----------------------------------------------------------------------------
// Function in need of a redesign (legacy mess)
// - RenderColorRectWithAlphaCheckerboard()
//-----------------------------------------------------------------------------

// Render an arrow aimed to be aligned with text (p_min is a position in the same space text would be positioned). To e.g. denote expanded/collapsed state
void ImGui::RenderArrow(ImDrawList* draw_list, ImVec2 pos, ImU32 col, ImGuiDir dir, float scale)
{
    const float h = draw_list->_Data->FontSize * 1.00f;
    float r = h * 0.40f * scale;
    ImVec2 center = pos + ImVec2(h * 0.50f, h * 0.50f * scale);

    ImVec2 a, b, c;
    switch (dir)
    {
    case ImGuiDir_Up:
    case ImGuiDir_Down:
        if (dir == ImGuiDir_Up) r = -r;
        a = ImVec2(+0.000f, +0.750f) * r;
        b = ImVec2(-0.866f, -0.750f) * r;
        c = ImVec2(+0.866f, -0.750f) * r;
        break;
    case ImGuiDir_Left:
    case ImGuiDir_Right:
        if (dir == ImGuiDir_Left) r = -r;
        a = ImVec2(+0.750f, +0.000f) * r;
        b = ImVec2(-0.750f, +0.866f) * r;
        c = ImVec2(-0.750f, -0.866f) * r;
        break;
    case ImGuiDir_None:
    case ImGuiDir_COUNT:
        IM_ASSERT(0);
        break;
    }
    draw_list->AddTriangleFilled(center + a, center + b, center + c, col);
}

void ImGui::RenderBullet(ImDrawList* draw_list, ImVec2 pos, ImU32 col)
{
    draw_list->AddCircleFilled(pos, draw_list->_Data->FontSize * 0.20f, col, 8);
}

void ImGui::RenderCheckMark(ImDrawList* draw_list, ImVec2 pos, ImU32 col, float sz)
{
    float thickness = ImMax(sz / 5.0f, 1.0f);
    sz -= thickness * 0.5f;
    pos += ImVec2(thickness * 0.25f, thickness * 0.25f);

    float third = sz / 3.0f;
    float bx = pos.x + third;
    float by = pos.y + sz - third * 0.5f;
    draw_list->PathLineTo(ImVec2(bx - third, by - third));
    draw_list->PathLineTo(ImVec2(bx, by));
    draw_list->PathLineTo(ImVec2(bx + third * 2.0f, by - third * 2.0f));
    draw_list->PathStroke(col, false, thickness);
}

void ImGui::RenderMouseCursor(ImDrawList* draw_list, ImVec2 pos, float scale, ImGuiMouseCursor mouse_cursor, ImU32 col_fill, ImU32 col_border, ImU32 col_shadow)
{
    if (mouse_cursor == ImGuiMouseCursor_None)
        return;
    IM_ASSERT(mouse_cursor > ImGuiMouseCursor_None && mouse_cursor < ImGuiMouseCursor_COUNT);

    ImFontAtlas* font_atlas = draw_list->_Data->Font->ContainerAtlas;
    ImVec2 offset, size, uv[4];
    if (font_atlas->GetMouseCursorTexData(mouse_cursor, &offset, &size, &uv[0], &uv[2]))
    {
        pos -= offset;
        const ImTextureID tex_id = font_atlas->TexID;
        draw_list->PushTextureID(tex_id);
        draw_list->AddImage(tex_id, pos + ImVec2(1, 0)*scale, pos + ImVec2(1, 0)*scale + size * scale, uv[2], uv[3], col_shadow);
        draw_list->AddImage(tex_id, pos + ImVec2(2, 0)*scale, pos + ImVec2(2, 0)*scale + size * scale, uv[2], uv[3], col_shadow);
        draw_list->AddImage(tex_id, pos, pos + size * scale, uv[2], uv[3], col_border);
        draw_list->AddImage(tex_id, pos, pos + size * scale, uv[0], uv[1], col_fill);
        draw_list->PopTextureID();
    }
}

// Render an arrow. 'pos' is position of the arrow tip. half_sz.x is length from base to tip. half_sz.y is length on each side.
void ImGui::RenderArrowPointingAt(ImDrawList* draw_list, ImVec2 pos, ImVec2 half_sz, ImGuiDir direction, ImU32 col)
{
    switch (direction)
    {
    case ImGuiDir_Left:  draw_list->AddTriangleFilled(ImVec2(pos.x + half_sz.x, pos.y - half_sz.y), ImVec2(pos.x + half_sz.x, pos.y + half_sz.y), pos, col); return;
    case ImGuiDir_Right: draw_list->AddTriangleFilled(ImVec2(pos.x - half_sz.x, pos.y + half_sz.y), ImVec2(pos.x - half_sz.x, pos.y - half_sz.y), pos, col); return;
    case ImGuiDir_Up:    draw_list->AddTriangleFilled(ImVec2(pos.x + half_sz.x, pos.y + half_sz.y), ImVec2(pos.x - half_sz.x, pos.y + half_sz.y), pos, col); return;
    case ImGuiDir_Down:  draw_list->AddTriangleFilled(ImVec2(pos.x - half_sz.x, pos.y - half_sz.y), ImVec2(pos.x + half_sz.x, pos.y - half_sz.y), pos, col); return;
    case ImGuiDir_None: case ImGuiDir_COUNT: break; // Fix warnings
    }
}

static inline float ImAcos01(float x)
{
    if (x <= 0.0f) return IM_PI * 0.5f;
    if (x >= 1.0f) return 0.0f;
    return ImAcos(x);
    //return (-0.69813170079773212f * x * x - 0.87266462599716477f) * x + 1.5707963267948966f; // Cheap approximation, may be enough for what we do.
}

// FIXME: Cleanup and move code to ImDrawList.
void ImGui::RenderRectFilledRangeH(ImDrawList* draw_list, const ImRect& rect, ImU32 col, float x_start_norm, float x_end_norm, float rounding)
{
    if (x_end_norm == x_start_norm)
        return;
    if (x_start_norm > x_end_norm)
        ImSwap(x_start_norm, x_end_norm);

    ImVec2 p0 = ImVec2(ImLerp(rect.Min.x, rect.Max.x, x_start_norm), rect.Min.y);
    ImVec2 p1 = ImVec2(ImLerp(rect.Min.x, rect.Max.x, x_end_norm), rect.Max.y);
    if (rounding == 0.0f)
    {
        draw_list->AddRectFilled(p0, p1, col, 0.0f);
        return;
    }

    rounding = ImClamp(ImMin((rect.Max.x - rect.Min.x) * 0.5f, (rect.Max.y - rect.Min.y) * 0.5f) - 1.0f, 0.0f, rounding);
    const float inv_rounding = 1.0f / rounding;
    const float arc0_b = ImAcos01(1.0f - (p0.x - rect.Min.x) * inv_rounding);
    const float arc0_e = ImAcos01(1.0f - (p1.x - rect.Min.x) * inv_rounding);
    const float half_pi = IM_PI * 0.5f; // We will == compare to this because we know this is the exact value ImAcos01 can return.
    const float x0 = ImMax(p0.x, rect.Min.x + rounding);
    if (arc0_b == arc0_e)
    {
        draw_list->PathLineTo(ImVec2(x0, p1.y));
        draw_list->PathLineTo(ImVec2(x0, p0.y));
    }
    else if (arc0_b == 0.0f && arc0_e == half_pi)
    {
        draw_list->PathArcToFast(ImVec2(x0, p1.y - rounding), rounding, 3, 6); // BL
        draw_list->PathArcToFast(ImVec2(x0, p0.y + rounding), rounding, 6, 9); // TR
    }
    else
    {
        draw_list->PathArcTo(ImVec2(x0, p1.y - rounding), rounding, IM_PI - arc0_e, IM_PI - arc0_b, 3); // BL
        draw_list->PathArcTo(ImVec2(x0, p0.y + rounding), rounding, IM_PI + arc0_b, IM_PI + arc0_e, 3); // TR
    }
    if (p1.x > rect.Min.x + rounding)
    {
        const float arc1_b = ImAcos01(1.0f - (rect.Max.x - p1.x) * inv_rounding);
        const float arc1_e = ImAcos01(1.0f - (rect.Max.x - p0.x) * inv_rounding);
        const float x1 = ImMin(p1.x, rect.Max.x - rounding);
        if (arc1_b == arc1_e)
        {
            draw_list->PathLineTo(ImVec2(x1, p0.y));
            draw_list->PathLineTo(ImVec2(x1, p1.y));
        }
        else if (arc1_b == 0.0f && arc1_e == half_pi)
        {
            draw_list->PathArcToFast(ImVec2(x1, p0.y + rounding), rounding, 9, 12); // TR
            draw_list->PathArcToFast(ImVec2(x1, p1.y - rounding), rounding, 0, 3);  // BR
        }
        else
        {
            draw_list->PathArcTo(ImVec2(x1, p0.y + rounding), rounding, -arc1_e, -arc1_b, 3); // TR
            draw_list->PathArcTo(ImVec2(x1, p1.y - rounding), rounding, +arc1_b, +arc1_e, 3); // BR
        }
    }
    draw_list->PathFillConvex(col);
}

// Helper for ColorPicker4()
// NB: This is rather brittle and will show artifact when rounding this enabled if rounded corners overlap multiple cells. Caller currently responsible for avoiding that.
// Spent a non reasonable amount of time trying to getting this right for ColorButton with rounding+anti-aliasing+ImGuiColorEditFlags_HalfAlphaPreview flag + various grid sizes and offsets, and eventually gave up... probably more reasonable to disable rounding alltogether.
// FIXME: uses ImGui::GetColorU32
void ImGui::RenderColorRectWithAlphaCheckerboard(ImDrawList* draw_list, ImVec2 p_min, ImVec2 p_max, ImU32 col, float grid_step, ImVec2 grid_off, float rounding, int rounding_corners_flags)
{
    if (((col & IM_COL32_A_MASK) >> IM_COL32_A_SHIFT) < 0xFF)
    {
        ImU32 col_bg1 = ImGui::GetColorU32(ImAlphaBlendColors(IM_COL32(204, 204, 204, 255), col));
        ImU32 col_bg2 = ImGui::GetColorU32(ImAlphaBlendColors(IM_COL32(128, 128, 128, 255), col));
        draw_list->AddRectFilled(p_min, p_max, col_bg1, rounding, rounding_corners_flags);

        int yi = 0;
        for (float y = p_min.y + grid_off.y; y < p_max.y; y += grid_step, yi++)
        {
            float y1 = ImClamp(y, p_min.y, p_max.y), y2 = ImMin(y + grid_step, p_max.y);
            if (y2 <= y1)
                continue;
            for (float x = p_min.x + grid_off.x + (yi & 1) * grid_step; x < p_max.x; x += grid_step * 2.0f)
            {
                float x1 = ImClamp(x, p_min.x, p_max.x), x2 = ImMin(x + grid_step, p_max.x);
                if (x2 <= x1)
                    continue;
                int rounding_corners_flags_cell = 0;
                if (y1 <= p_min.y) { if (x1 <= p_min.x) rounding_corners_flags_cell |= ImDrawCornerFlags_TopLeft; if (x2 >= p_max.x) rounding_corners_flags_cell |= ImDrawCornerFlags_TopRight; }
                if (y2 >= p_max.y) { if (x1 <= p_min.x) rounding_corners_flags_cell |= ImDrawCornerFlags_BotLeft; if (x2 >= p_max.x) rounding_corners_flags_cell |= ImDrawCornerFlags_BotRight; }
                rounding_corners_flags_cell &= rounding_corners_flags;
                draw_list->AddRectFilled(ImVec2(x1, y1), ImVec2(x2, y2), col_bg2, rounding_corners_flags_cell ? rounding : 0.0f, rounding_corners_flags_cell);
            }
        }
    }
    else
    {
        draw_list->AddRectFilled(p_min, p_max, col, rounding, rounding_corners_flags);
    }
}

//-----------------------------------------------------------------------------
// [SECTION] Decompression code
//-----------------------------------------------------------------------------
// Compressed with stb_compress() then converted to a C array and encoded as base85.
// Use the program in misc/fonts/binary_to_compressed_c.cpp to create the array from a TTF file.
// The purpose of encoding as base85 instead of "0x00,0x01,..." style is only save on _source code_ size.
// Decompression from stb.h (public domain) by Sean Barrett https://github.com/nothings/stb/blob/master/stb.h
//-----------------------------------------------------------------------------

static unsigned int stb_decompress_length(const unsigned char *input)
{
    return (input[8] << 24) + (input[9] << 16) + (input[10] << 8) + input[11];
}

static unsigned char *stb__barrier_out_e, *stb__barrier_out_b;
static const unsigned char *stb__barrier_in_b;
static unsigned char *stb__dout;
static void stb__match(const unsigned char *data, unsigned int length)
{
    // INVERSE of memmove... write each byte before copying the next...
    IM_ASSERT(stb__dout + length <= stb__barrier_out_e);
    if (stb__dout + length > stb__barrier_out_e) { stb__dout += length; return; }
    if (data < stb__barrier_out_b) { stb__dout = stb__barrier_out_e + 1; return; }
    while (length--) *stb__dout++ = *data++;
}

static void stb__lit(const unsigned char *data, unsigned int length)
{
    IM_ASSERT(stb__dout + length <= stb__barrier_out_e);
    if (stb__dout + length > stb__barrier_out_e) { stb__dout += length; return; }
    if (data < stb__barrier_in_b) { stb__dout = stb__barrier_out_e + 1; return; }
    memcpy(stb__dout, data, length);
    stb__dout += length;
}

#define stb__in2(x)   ((i[x] << 8) + i[(x)+1])
#define stb__in3(x)   ((i[x] << 16) + stb__in2((x)+1))
#define stb__in4(x)   ((i[x] << 24) + stb__in3((x)+1))

static const unsigned char *stb_decompress_token(const unsigned char *i)
{
    if (*i >= 0x20) { // use fewer if's for cases that expand small
        if (*i >= 0x80)       stb__match(stb__dout - i[1] - 1, i[0] - 0x80 + 1), i += 2;
        else if (*i >= 0x40)  stb__match(stb__dout - (stb__in2(0) - 0x4000 + 1), i[2] + 1), i += 3;
        else /* *i >= 0x20 */ stb__lit(i + 1, i[0] - 0x20 + 1), i += 1 + (i[0] - 0x20 + 1);
    }
    else { // more ifs for cases that expand large, since overhead is amortized
        if (*i >= 0x18)       stb__match(stb__dout - (stb__in3(0) - 0x180000 + 1), i[3] + 1), i += 4;
        else if (*i >= 0x10)  stb__match(stb__dout - (stb__in3(0) - 0x100000 + 1), stb__in2(3) + 1), i += 5;
        else if (*i >= 0x08)  stb__lit(i + 2, stb__in2(0) - 0x0800 + 1), i += 2 + (stb__in2(0) - 0x0800 + 1);
        else if (*i == 0x07)  stb__lit(i + 3, stb__in2(1) + 1), i += 3 + (stb__in2(1) + 1);
        else if (*i == 0x06)  stb__match(stb__dout - (stb__in3(1) + 1), i[4] + 1), i += 5;
        else if (*i == 0x04)  stb__match(stb__dout - (stb__in3(1) + 1), stb__in2(4) + 1), i += 6;
    }
    return i;
}

static unsigned int stb_adler32(unsigned int adler32, unsigned char *buffer, unsigned int buflen)
{
    const unsigned long ADLER_MOD = 65521;
    unsigned long s1 = adler32 & 0xffff, s2 = adler32 >> 16;
    unsigned long blocklen = buflen % 5552;

    unsigned long i;
    while (buflen) {
        for (i = 0; i + 7 < blocklen; i += 8) {
            s1 += buffer[0], s2 += s1;
            s1 += buffer[1], s2 += s1;
            s1 += buffer[2], s2 += s1;
            s1 += buffer[3], s2 += s1;
            s1 += buffer[4], s2 += s1;
            s1 += buffer[5], s2 += s1;
            s1 += buffer[6], s2 += s1;
            s1 += buffer[7], s2 += s1;

            buffer += 8;
        }

        for (; i < blocklen; ++i)
            s1 += *buffer++, s2 += s1;

        s1 %= ADLER_MOD, s2 %= ADLER_MOD;
        buflen -= blocklen;
        blocklen = 5552;
    }
    return (unsigned int)(s2 << 16) + (unsigned int)s1;
}

static unsigned int stb_decompress(unsigned char *output, const unsigned char *i, unsigned int /*length*/)
{
    if (stb__in4(0) != 0x57bC0000) return 0;
    if (stb__in4(4) != 0)          return 0; // error! stream is > 4GB
    const unsigned int olen = stb_decompress_length(i);
    stb__barrier_in_b = i;
    stb__barrier_out_e = output + olen;
    stb__barrier_out_b = output;
    i += 16;

    stb__dout = output;
    for (;;) {
        const unsigned char *old_i = i;
        i = stb_decompress_token(i);
        if (i == old_i) {
            if (*i == 0x05 && i[1] == 0xfa) {
                IM_ASSERT(stb__dout == output + olen);
                if (stb__dout != output + olen) return 0;
                if (stb_adler32(1, output, olen) != (unsigned int)stb__in4(2))
                    return 0;
                return olen;
            }
            else {
                IM_ASSERT(0); /* NOTREACHED */
                return 0;
            }
        }
        IM_ASSERT(stb__dout <= output + olen);
        if (stb__dout > output + olen)
            return 0;
    }
}

static const char* get_glacial_indifference()
{
    static const char glacial_indifference_base_85[42825 + 1] =
    {
        "7])#######pfX,K'/###[H7+>)322'Ql#v#RxMT9[8Dioe@*##d-&##HnmT9_0$rLx6$##]l&##v5:t8FN;loG:$##9C[w'ar+h<:`gNUc'cw'c^&##no&e=tDw2vmhdw'sJf--x*re="
        "39^%@hY&##K'^w'&vA0Fjt,Zs'aFO1=l$##J.=UCQ;EQ&q6Nk+P=r-$f'TqLIXLp7G-t92>M[w'E3n0F/D#jX-gDwK(#r92dY[=Bu$>c`0l%>-/Vtv/g=00F:gaL8OW+41R(2>5`%S+H"
        "wba1#l#5@M/KG&#TCoN15;*_l+I$t'alVP&M5DX(wWl/]`+^F%+r>7v-Cx1$T;Mt-Zr[fL0SafL:g'2^)8###Bde--$s<>Y8Ow3+<l>g`tZP+#rBF['WS1v#eXZV$Ze5AOep^w'qhIF%"
        "S^%-#_Vl##U@w2LZI93(HEbKNBjCt8laIS7)&g9vWv*REtL-2^gqTk+uQj-$XTf._C@:;-%%h^#;QZ`*$@,GM=x(t-j9D6NS@-##Sd_ANTF6##l<?*NUL?##6DN3NVRH##88m=NWXQ##"
        "oeK)NX_Z##n#C1NYed##%XCAN[qv##ej'1N[t)$#WMnk=L%u<6riaR*v8<A+[6xu#1:<p%)`ZPK9___&q:dl/,a`R*WRJM'`]q_&A17G;xIi_&.*mx4BVm_&ofkr-D^p_&.t5A4%Pi_&"
        "rv9SI$Nl_&#Sdl/1ri_&QC5YYq/l_&DUpVIt5l_&H$U`3f+CP8XhdxFeBEYGIYo+DUw%;HULL-Q'm,20Z8#-3/pYc29%e'&I1He-U%co7KYSfCVTlMC?Ai34`<Sc;j#(@'6$Y_&w3F_&"
        "nk:_85``f1,<)@09e/F%@L-#H>b'NCQf8JCpU(vH/(=2CVL9N0F+U.G5E+7DHX5p/W%C,3/on@-aI3s7#EFVCeB$g;ui#hFY<:9CTnEkL:4cF-snL$H9CM*H,,lVCaY$##MWT,.kg$qL"
        "KgipLqPG&#^S3rLLZ5.#;Ts+M?)8qL8r%qLeL[i9iT*nDZkbM:qgeqCrAFVCZ[i-#hl3O+x'AVH)fg]G=e'NC#*N=BTd&g2=pfJ2Rs]K2Qaw.:UNDP8;W>#6vdu8EUaCM>2*7L2]N:5:"
        "UL^+6?RwUMK`iLN+^sP-rH2BRskgcNs#ZoLHB2/#-6[qL#'d.#OX%RjA*D)lVPHY>->hoDNuqiLC3I@-Ag/+4ux4$Y,;3S-6VS/.'ZhpL^GD/#?N#<-Ch(P-<aYs-.BnqLs'm.#SvX58"
        "4),44s1L`EKItaeOEZe2@8OG-6HcfGg8T0F-$]F-^q%?IEt@Y-dMBe6SvI&#ZK[HMQ(Vq'cEJr7J;b>-sVGp%uCH]F[3&;Hol,<-T(MT.B0f-#]Lx>-HxRu-03&[8HE;K*/dDE-8ec5&"
        ":$O787hr22/a<hF3f`PB_Zq?-ulgoDnZ`9CXN*IMxhqfDjeZL2c3>h>sqQ/MqiR[/<Cel/Gxg%F73.&GYLe5/NZO.#M)IqLb%B-#*TKs-#Z<rL*8D'#Y`($#RSrt--0RqL-#e+#o.;t-"
        "vk3w7Hj;,<+sZMCALPgNq1CkLUr)$#?h:#8QkP:;o/$L>vq%;HKlD5BtfNk=VpmcE^X&mBeadDFtgNe$@*&RE+i-AFkpF_&CEG_&.'4X:Io>a=dY9G;&?jG<Cmkw0l2aJ;=8K21C`:dk"
        "3Y7(#fFK@To?K^?0r%qL1mFrLNEe>%m%L`EOSK;Ir&?D-8B5E8<'mcE<>o`FBY`5B0)fo7enR5B7Zp92Lo<WA0fipLY6L*#:bbOM:LD/#?MOgL8HD/#[H@2.e,KkLiK;/#N)1/#>)m<-"
        "6cpC-n`+p7h6i`FBt9/D6pjG<=;tjt]CFk4'.AJ1DJ`8&B7I'&20&29kKYEe4m5D<q[ix=us)K;Z^DP80jo`=Ara>-LB(q)JtnW_DHu(3c<Oq2H37JC;+D3krGw1BD;#sIa>T,3,#5L#"
        "65;-m+W-9&qO2A=xAms7^a)58Pke'&Rhi;-v>Z-.'Z<rL[mP.#Mbf_%<g7WSrthf-9Cel/+L5l$,-@e63g'<-6cpC-9$v[.VH4.#KnS(%P,+58Nf[aFWLL`ET0x7IEk=2C8$W8&8OkMC"
        "%?g'&hYi34f/q92<g_,tj<eq$>sVq)d5Qq2_j;MBA^e]G(;0L>mEc>-A$ls%62%L>u1l(EqA6mB>.N'J*`lKPsPxYHm97L#VXHe-Q]jq)eQi3Olm#mBDnXMC;.l`FK_Z8&K3uiC@Uh]G"
        "3?Z>-N=xiCcaT5B.X`'/?>J)Fp)G_&H`%@'G.3q`.wtWUgcIk4mQ5'o)2KqrX^p92CD8)F6wUDFj7G>H.+XMCGO?KE,4ZtMbD?&8gL*L>i'm]G[_]MCFGsw0SHZ9MGv?X(Ep?X(1BaE["
        ")mZ-?x3x7I#;,_-^U5.#YrT;IT7r?TUtr'/]eL>HW][>-T6T5B2Mw9)$5P3t_3a'/3ch;-<0&'./5[qLH^A%9vC[k=rHtdm.@&;HO@b-60EKq;CU1&GMdO<-R(A>-u-0597FW9`].>,M"
        "q#6x8&^*#H4Mhm88L=kO/i5dMaE6lNB5urL'ItJ-X4)=-SsO(.*n-qL8W`t-/msRMRRTw81m@5B-$C2C1/k]Gk0B2CrG8R3?w/F%sn@VHF$_3klEj;-n^BK-?%?D-3nVE-8i;..,aAkN"
        "sBD/##?1.-2?Z>-oLU?pi+7X1cp9X1'DGdM_kbRMYmP.#aYU).G'BkLd))PM&l-68kp<#H<wTq)wHRw9iC#F7jUOLG6gi+M>#/qLH:2/#scEB-6V)>MX5eu%0/2?7Y4T-#j`N^-TQtVf"
        "N=X3.#n-qLw8):%n.h%FGk=2CU>K:):8bo7icSj2$P%(#qv>aO;-Ej&1,0#HPDl68u@=2C%<N,MPFOJMRWRBMUTVQ&:hfQaN[i;-rB*;&:csE@Ip_w'#kTEnDZxp7UjGjD7`rq%:+YDF"
        "u?3GRQL;/#Y`($#K,Yc-_;dpK=wXrLvGG&#LpWDN]mrpL*HG&#FGscE0o@5BegF=-VmLAMZlYp%3H#;Hc0exFBh+Ecd?8K'3WOvRi9)/#1H<RMCY+rLGUwq$wt<dO;OKL([t)F.j/w1B"
        "f]NDFk`q0:ELk>-WpJq;>i2q`Pn7<-tJm-.-HwqLxYPgL8g=rL19MP-U'l?-$E55OtcFs%u%h;-/2'C-%@EU-1u3o(QHO/:>cpfD+BSwK2O/F%jvxW-^UVQ1?6:*#YuJ*#==fnLslGoL"
        "nMpnLN/n+#'3*jLEbGl$Xn;J:)F9v-L94W-eMa-?lhMW-f>k,k>'crLkd`Y9.P0jCNW5'o-uH-v:]e*<+hTL5^Q':.v*^*#$0k;-Dh(P-3oWB--vK#0:'Fs-+#[QMQgipLFlP.#SxgM;"
        "A8.K<.xIQMRA;/#ceF?-e0IY%XG&3`/XPgLMskRMULM/#)5gh8bmaYH#T$mBv$CLlOWQ'ArE3X:KptKG5qF_&?ldN9]5v]ma(B-#J6J'.B^J%8W1Bv-xPL>.EFR78Z8VDFUO?2&)udxF"
        "*k'mBOD<EYF9CSMu6EFMu/AqL^)8qLi4oiL5:SqLTXPgL-9I^(V@1L,R-4@0S7Qp7SIn,=mVY&#Xij#.w)IqLJ`4rL'cI7;cZK#_e=`/:%7XMC&L>K*oPG&#&$@qLtlfw+lf3)FuB-aF"
        "It;^dOL%f$o.75&`VwR*$+S>-m6>t(UX(g;@G$=-(wl1''m2WonlRZ(7%V58Y%XwT8JQ9`TTHEY?-lrL&YOG%JT]3F#8PcDsbS/Dai:-m'@Ws-D^bT:uwa>HvI+DN1KG&#I)B;-qiQX."
        "_rt.#hXX,%?[odbh$m.#^FgX1FE4<-G?(A&$,$r7&^dDc?k-68bGnp0DKCsLS5T-#uKn:MEO&d9Y]w]G(;Wq)YW$ed6v?EP)p5X$E^:@'xb]kFWM)?[n`G59j&89&LV8JC;VU-H4bf'&"
        "'>PcDfDKq;I>`KsMDFO%Q`?VHb,VJDn5hDc*dgl<R?s&dK5%?8X2XMCXB<:)NZ8RE2CNe$AE=kFProR*C9g'&gP4GDlW1gDP8eWqgo65&/)k]G>hw3+9BI-ZQLrM9KR.&G#Y,#HPHq>-"
        "ZnBe6]9A_/:Hqw'FUl?BLJ>J:%@4wn]R`Z-wG+e<f.2A=eJ=J:O76L#5v5D<OVrE@Dmk-$lnW_&L`S90&gYOMjY+rLdwGM=YfpfDF(X>-n?^MCdw]MCx+(sIg99kt6gi+M,=^-#X4)=-"
        "K.A>-WC1X%@;Cs-**IqLgP9w%(+e>P+h=rLeFD/#IGuG-;N5s-mMOgL`V,p8R=oj2M_,@OO4urL8<oQMMm-Q%N#;@'Bj&29/g'@0U6[DNZ>33&U/&aFB^nKP.Ni+M^S/@+<gK<-*1s2/"
        "QZO.#/9r3;)QG&#D:<(R@OQx%5e&=-hd,t$RFlj`RbCH-5cpC-7,Kg%#MLp7/?tiCKc7eQ,Rx7IZKEp7b,MDFn?M>H=73T.&-3)#(;#s-U^jjL%JgnLjYIR;60i3=l(Q29'qMDFJ$i]G"
        "ailm80ew?9UcwfDxej2CX6]rH-x<2C.qX5B`(j3i@'(SM_*8qL]/Kc$$E@kXq.t&Hd)r]$(8WJ;%);)F'(5D'hVkdMn)8qLwoPt9wgA,3+r/9p<'DA8,=@:9XR<v'6eT+<=fEXAPA`'#"
        "F[&:BuLg-#=$S-#kX'-%0hXoILQl[$K#FP80wx'H`1B-#Jmk.#MOV58GR^>&#[wq$+Nc3=9#(68j`c'?R^./&;vi+MBTZa&Mbn<-xMdh-kPc2V[%B-#E/F,M^)d.#7w8kLggT@8v]Z-H"
        "#==SIUZh>-UIQI'%+AVH<Ex?0d)q92XY6eHa]*L59J6<-ES`t-AMLY9E7o3OKfhs-6j]%8aA.&GTaqfD3k+rTLbB%8Sd,gDYEr?9dZMk+<T?X(-(n-$'[5'Q9X+rLDe;>MF75dMr)8qL"
        "I4fQMYeH5NS#>dOLg=rLEr)$#E(A>-U/WU%n5`>-M54#HZm(q)7A[-?FYOk+p5L:DWl9-M4)crL(M-a$E?g'&x3x7I<7SRUuJG&#5PKC-p37#%g$`l8B5i`F;e8QU5QIH3$+s>I5pG.+"
        "LXn(<Gxg%FXUkw@=RxqLH'crLB4oiLrdi/#-HwqLXt%qL=-lrLlV`/#->h;-Ur.>-C)m<-KAGA+5s^<-Dgps$-HK<-0JKC-AZ+(.G)Px8XIrDF?`65&c/`e;&+B&vVaen;po<#Ha[r?T"
        "@'`k;EDOJMXh0h$BObKcI8XeO=FF4'r@=L5YYc;-fXiI&4A9r0FexZ$kmn?9?_w;-t(Vn;D,v:;Z,g;-Ms-?:%,5d3+.XFPE?5x7(-Mjr;L,&+1il-M8t?2&Xn;J:v1Z/?^_n=-;.&J."
        "Bn@-#CL+a<P3<edwG$Yfm9v.#Zmk.#66T;-2U-.%q7-AFo>_w'OV7'dEgrm$v']rH^i4^GV7PcDo.=Wfrgr/#^gW8pg%1kLpGg-#,h$qLvkFrLtt(.&jl0DEO')aFtPI-ZK26eHN@He-"
        "DA?K:g%qfDxA[DF1C^-)ltVGN90`t%+U@p7ofK9rn[dp7)]#gD?$).M3T/d+<JBP8B2Ud4kn`:8M9vQ:bO*?'Ea*583>xjMiX12)5eCYAt[^@%Y_xN0Y`($#[;L/#a6LJ>'QVR:m@g-#"
        "h-f/MLu4O%@AvN9Ooew%d:=@8icu3`%I[g=2[OLs*TG&#P0f-#d3p?-5kGF=)txPh`lfn$a1.X-8xNLs[n)$#n+278(Ifv7>.5P;i'W,<bx1A=6jA;K`;.;)uO9MB/VM'JxLh%FD#O#H"
        "]g*AF%_:-+&d59%/uH]FF>s.CKH$gDpJug$wdg&dtdo,(/[/X-KY@)ZSuMZ>(w^dk8p?*0[1g*#ec/*#u$50.>I=OMBmSN-ncPW-iaU)-L)n+#$/;t-QtboLAslOMs@EY.>[P+#1fr=-"
        "boWB-uLM=-a+NM-_?f>-.#T,M1WY&#PY5<-<:xu-Bo.nLm5*,#+/m<-<dq@-?eF?-#)m<->`Y-MCEOJM8uY<-BQqw-8V`mLaXPgLtAMPMohjmLm(?D-N&@A-Z3S>-(MM=-oM#<-B?x/M"
        "'ZGOM6uPoLJHh*#;i%nL1l'*#1L?,<p2wEIC,xf;Ajs34bB,_J:r_'/4:8X1&THqiT<dM:jKYkF@Q55&LYl;-0<Y@MGmGoL3v'*#=C4OMmGVPMG,TNMwYGOM3s-A-*/PG-$ni&.lHxnL"
        "Ga5oL'+9nLhtPoL>b&NMYoQ+#xM#<-$.RW@P@/s7<:0p8^A1eZlZh+M%psmL_JrP-;F[*.HbFoLvo4[$@EHk4D:FV?0w(m9]P9&>p-OqDH(^9MrDJ21fk6&>(FBs7PE>X(Cuo9M8rq'/"
        "x-e;-cIKC-5:M.Mib5oL(#XB-78L1MY3oiLMOpnL@*C*#rt>%.oZ=oLoOEmL)P5OM[5RA-LfRQ-a-2uOn3oiL:NpnL30#0NdUNmLB2oiLt)9nLD4>g$X?<p7_`;,<TT_>-Wf]J;.3rM:"
        "?ri9VkeE_&i#.m9t+^,MT_sM-nL8j-)M`2M2XPgL7AeJ(x5W,MbZGOMp_lO-_ZlO-&ufN-=(c8MVO5OM1+9nLSdamL^a5oLYq0*#>-@,M1n4<8ve;,<(`me6R5)a<C9o`=H]K)=6cEW/"
        "ug#`-+mmLc<ZF#?gI(F.wk`e$wsm2DpX)..]CDmL#>a(?*ta>-Q=-2&7'WD=8FA_85&Ok+JFuQNv)Zj)*)doLVg>oLZv^c$'X9@'w?&T.ttJ*#qDZu7JjVG<]:(dMe*=J-qt-78aDd9i"
        ")Y8WA_MgS8xRXA>Wf9HN0W0a+L9[_JQ6$XLcC4p8^<Gs7BCO3X0<]i99WZ>-FxW>-ClCm8]24j1O1ip(cb;Emh[3p/%JD.GpZ3nDO-f0F387r17#d=0:q2pDsZVD='d)N0hxtLFht9:B"
        "&S[MCP8bj1I>.PDUF&(/uVErC82ddG;&x<(AditB%dOVC6*TMF*'FG-07%F-,B@C-bfpKFAp[J;l/S218$5(&)+@hM`i-2M`$wgFo.V$9+)9kEVbt?-0nL6M+[hjE2MD5BGuFg.djeUC"
        "[+qP5,x4w7k<AEHD$+FHr(622F1vdG:C+,Hu(L9iAr'8D4ibN;ojFK1]@V8&<UVe<sIeP95,4D-AZ299@SadObjfM1www@-NVldG;@g%J+9IL2bS$82ktf:9@J=N;/?.PD*4EhF7`/?-"
        "83I>H:STp9G]2p/aZCQ/@*adG7-giF?FqoD)*N=B?KE;I+<J:C#OrTCn3I'I*SV.G.]eFH);WnBxdM=Bt;/>B2gkVCF8@rLCG>fGqEDtBg3kKMqJ5/G?=4GHb;CL,+;FVCfrAaFh>=qC"
        "D`i=-;e803Xw:9CngitBxi[:C2r>LFAB`qLLsfE-&5H1F:Y7FHlk/U;-ddERD**vHtn=?-(I8eGM'Zq21*S.G1L(@'$mnUC6/SHE;pwF-%XitBwPeu10/T=B/g]TC+a)=B9[hQM'1w0F"
        "$pa$'j=5'G/rMMF,5LSDFFPv[.AlFHL_lH2evl#>a6ZG<78C5B8=kCIt9UI8[n)edG^i`FINjZH7?c91K^bcH'lSb%3>f:C)ff'&5WvKG[e:^G.(OGHHk/b[Rf0H-/hpcEA*i6MEurqL"
        "7BDVC@'oFH=C?lE.q'$JP-)*F4c3(IJb0@-T$[8&aeL21VG2mBNND6Mkx?%J*f1eGd*wAHqio3/F?#mBZ6=hP8FVMFUODnN(^rXB*OL6M9g&?PIlHKFCW5fGNIpMrpT^C-U?YD%?3o;-"
        "]Une-@+G0cXwwn2vc0BFv4M*H/Bj*[&LS*[v;Q$^iS/gL@41D05=8HD;oI'I-b<eZt@nPB6nr&.DRdZ8;p@5B$s8bF-s1eGLQ]/G)iCrCU5G)%L`>*eFF$pD#$oUCAiLwBNH3:DUc@rL"
        "[7T=BglbW/_P3v%9w)hFK/MnE57`.G@gSgLXAfB.%AraH')<nB&dCEH-6eYH+HX7D84*9.C*5fG=q#gDi`1x2T:dm8Fr^0<YO$L<F9ts-7@Z6N>+%<-p^Nb.*ln+HT8mpLEa+#H?PW6a"
        ",M^kEdsr]G6l528ImW>-'U#30Ql=lE2pux/e^?;`&j[:Ccc*L5MN0%J/icdG8o9`06Q.#H=dhq1--UqL_gT=B)J;LFx`F;C6'^iF)eiTC`bH]/GKfaH(=rE-MUr;-A'gKNfJAEHBN7rL"
        "vnY59w-%lb:'B%J'uHq$:`4VCNwR3%i:x>-O/G>97U`8&++gb%?SA>B%$oUCTkZEe%IDH<%0OnDN<8x8g0j&-**wgFpKDtBubo>-9$.Ht)*wgF4MvLF4LZ#HxXMA&o8K<&(_j<-wV<Z%"
        "2O$g;a>#<-*nPg$r8wAZv9T=B:YqiLf.99CrGX7DSXr:1$/9kE6Uc$'J&L^FI^UcH,GqoDK-.=(l*1O1e0Yw$U*2O+KPg;Ix;fUC@LS5BH@vj;Ys9E9]NV8&@Ph<-hCL`.j5KKFDp9[/"
        "6R7fG/l*,HWDuQNBq7fGSK[>H>Jhs&HfgiFsh2BF04]MC9`VeG)?oUCwE;$ptnMGHTCIcQ_YeoD&/2eG-ZFkL=2S6M6D>fG#$/>B7=P_&:dDtBrjitBs0(9I.ZXVC0]m^8/de&G3u`iF"
        "eIE6j:qvgFV6@T^AQr2%5vjgLh^R:9`_;e=^I?1<x`gS8H^ZW8`I]d<'p0oDK<`qLpvo*H.r1eGkuw'MRBcFH_wb69wsnaFv1cQDo&8UC@j-EPTjJZ0vv@>B(#?LF#$ipLxu^%8771`J"
        "sUlYH%g8v.:#G;Cw82@0iVoiLKsG%.H-N<9LJ,sIEBP1'8#-T.*^#lE9;/c&I)TMFBXOmB'-JUCZeIN%GOZ8&uVUm/X]UcHdEDtBmp'P%5r>lEN/4sLXe$U2);7FH3rI+HM9_d2vAQRC"
        "BV5l$c#u9.>EFG-lw.T.?ZOG-lGau.25%bHb.;_8-m>T.1I;LFr5[Y%Bm[5hH'crL1/S3%$x,gLhw6kEwE:6&UVtN5tA`>-$gPj9w=va5V8Er&/<aE[)$>Y-;2gN2p[a<B=FZEe@U/B&"
        ".%fFHN&Gu9<$f5B&#lVC]ut^G&>.FH@'oFH.RU6M4fF>H0N/,&@h5&><NerL?hHKFOjMw835F2C/PB%%JvNG-0ZKKF-rhjE1I%F-?CkGH6oj,2%0W=BACDVCWhYWB(oWJD8X0NCC-m`F"
        "45emLH])u0;iViF&e2=B5F5)F+B2gDY0?u17=iEHtXP>H%+D=/x+D*HQG`qLx%1oMn4pv.jg`=BTt&7EV']<-8;si-UKDR3pC*N0r`goDJ)+vH,SM2FWp,@H19eYHGPnr1>=O70w#*G-"
        ".9Fq73j+O4P6_dXV8u)<HH/X9NI4fE:WOG-3YHlE#87FHAF_oD5spKFx:$6LGSl)%MG@g13I7fG4V%/G$-4nDIgGN9;MB99u'q%%^oX>H4tNTT:_4rL9s)-G9M[MC#&<K12&$1F;#1H-"
        "1%fFHn/4hP<3&*HUfi348[eFHeR[N25Kck-,9'qrq.V?-ujqQMujKE944T*#;wC.&Xmo;-e/Sn$NU5KN>h+#HqP#lE$NI#%:MI2CfNx728JZ1FCsl91&v[:C3pOVC$Z$6B+G2eG:4c$'"
        ":'B%JI7Ev'me-WI&iwFHrSZ6B%A2iF&U`9C/0I>H?i>au&EWhF9=3(8TU:O+A#7rL7gDFO:gCG-6vTSDELDp7PnV8&/u29.E^v&4tO6m89DQaP.,5fG.Y?<-+_u9)Ejk&FK^XG-RC+m9"
        "4mBNC7YVeGb%r;:&uYB$x7?u7e(7w@=KNb$B%<K:A4k>-kg'XJ,#HZGs5u&/(@%XJWTAK:Gohv%?q,:&oQQH=I#*x7_I?1<5=$_4iA#hF4-OG-58$lExi[:C=1CoDxCcqCFQt8.07%F-"
        ")vg29b4'?-g8$@&,[L)F0vsE@1C;e==&8Q:h=v=&RVtj;Bo?c=eZb(?r2VB-0_uc<eXLS:gZLB-]u?oLUR]2180W*>eao+>Dg-W%6fk/<@hYG<o#j/<13<R:AUq9&dKaI=/5+D-.raM:"
        "VuFR:Nrr?-JSFN;Tl^0<OUNmLv=PL<-9ER:;>IoL<M1M<^Drt:U8lmL&B]<&KA/Y8tST81<S'O;FtR81'RV*>iq,D-tu:M3l^:x7V83X8Xx,H=;Q8J=[xo:9UGwp8/X?1<SFZH=vOx59"
        "Xn.+>oL%B0`pt_?N8S6:Z:Kn3.<;U)Db5&>@H;99O(_4;?GHL-_6Eq-<*t*.D>N3X+3m>-iC@_/ZYj7:sJ@C-BBW8&8^V?-KDjt8s1M88xg_h.8&/:9.N[uL,AEN;'`:71l1Oj;F8R3b"
        "<:mfM1<ER:A2^8&9)1<-DfOO1C;A:9/A171,rS:9;`Rb%%>&u8GpdW8ON$@0T2cmLlrchLvLAK<[F6-=8P5hLP'i)NKgq?-7HLNMAX]0<>(K?-E(:S:f%QJM[JgnLHEg?.nNS+>lYu$?"
        "1MG61vk'S:UYxq9kZvL<pofG<:4S>-IFHP.14p:9;TiM:ubS/.lRROMD:0L(MoLe6_NX0lg2-w7de,`-JQW6N.d4j1,P9t3]l4(H5%g+Mt-gfL#8T;-)F4u$E8(v#Fle&,^oGxkmo5d3"
        "$;d'&8mc'&iWE$#B?)4#&?S5#3d46#=&Y6#VYQC#BBno#R'^R-;<o=-Msk>-$Ig^-se$bsPoe+M#5$##j[W#.LPsROgMu$M$6Ec#s93Y.96f=-aX9Y%J'@(su]1]bA+Z._8MBl]2/b4]"
        "Grm4JcS_Rnut^IhupD+Uj*L1SBI:oH$&ZUYa-Sj=uNpIYPq%s$bUe]-a2B2#_4Z;$=R<T%ME#n&^8`0(n+FI)(u,c*8hi%,HZO>-XM6W.i@sp/#4Y313'@L2Cp&f3Scc(5dUIA6tH0Z7"
        ".<ms8>/S6:Nx9O;_kvh<o^]+>)QCD?9D*^@I7gvAcR.iFYAx;LLQG1PaMRfQtIB)S.=)BTBB+wUVPQUW+aUfCpRpm^4Wa9CS]V-#P3)@'b^H>Q6oud2lQ,1FW#n+]K;Td]#Zu=Y*snO+"
        "7tNFHCk0DI_pM>K`HETM/%BiZAGOV.<c&##&O#<-.VjfL$LuCN.Gh*%4%UB#+GG##B)&#,^AJV%K25##U8(fs[s[U-@?9^#:cF`sVGJ*'uhcUQifH+)E%$##m9(hLJ4$##2SGs->_>hL"
        "tWEM0I`l8JrLRS%Be,Z%pE_c)aLPV-Wqp.C868SR`ICO;2;Q1FwctnD&HFVCCo;MFKGRR2&x)7B+?fUC96eh22EK)>4bP2XT/(8/$,>>#bIvtLa'w_#*P:;$.iqr$=:1GD6C35&:[jl&"
        ">tJM'B6,/(FNcf(JgCG)N)%)*RA[`*VY<A+GBaQj/5TY,cL5;-%5fv-k'MS.%cU^4qd0'#upB'##'U'#'3h'#+?$(#/K6(#3WH(#7dZ(#;pm(#?&*)#C2<)#G>N)#KJa)#5f60#Sc/*#"
        "WoA*#[%T*#`1g*#d=#+#hI5+#lUG+#pbY+#tnl+#x$),#&1;,#*=M,#.I`,#2Ur,#7kIH#0;w0#t^V4#Bm?0#rF31#(-85#+Kf5#+E]5#`Po5#6Wx5#6d46#>pF6#>&Y6##2l6#H8u6#"
        "JPC7#4qr4#vnq7#Rc_7#Wu$8#VRp2#c1@8#fOn8#fIe8#-Uw8#q[*9#qh<9##uN9##+b9#M6t9#-=':#/UK:#H:v3#Cs#;#7hg:#<$-;#T)6;#C(02#6sH0#0A*1#hmk.#CWs)#?G4.#"
        "e5C/#U$S-#em%P#nUhn#(J:;$4=35&aQ=G2ar*?GThMw$41<-&iwfe&m(oh&0b&2']2)o&9v$['UE*X(YP>n(XxZS)Tp]2+gbg'+blL,+i/WL+qCbM,,v-d-@<0+.FYxe.[JZk.FZx*/"
        "*foM/?#U7/En<20b65;1lZs]1,CsT1f&K^1onbb1s]pf1u,.l1_fg32JHwh2npJ^3[2?W4GG5(588SM5'h`5641kU69mQ-7mxa:8tI:88(hPk8dMvW8dw$=9?F3Q98A4/:#cZh:2VKC<"
        "<+8Y<&].m<]`5s<&sa>=Ul*F=x5NM>o4MP>VQ$T>?V,W>+0(w>$dL)?sav-?r_I2?@'x6@:qT0@1VY4@3^69@^KtNAY:#8AW8L<An]X?B=%>KCSi)QCpxBWCf'KZCPSF$DpI_%DoirED"
        "uvN/D4thlDhU]jDhY9oDlZu8EfWl8E8OkZFjYgKF<X+0GS+]'G.[$*G]Sp,Gl6wEG:FfJG*-11IBGa]'B&^h#0C:btS<$DaN'5>#jdcc)MD5>#o<oT%XwJGL'v(aODX#-figP^tX`l't"
        "&8xbOa7(8aiT4FD:Vd$'r1w$'jwjWs@3Uk)O/L+I<YD%4u&lnKjs`*s[&YDWYYn%R>ulftq>?C&C?,C&e`0lOrirIHG'D#$*VWkq*:E.Cjb+OXZ_w$4)+eLB:0-xL2$@]On;UjL>^*Zs"
        "FO.CbD:5>#`G(-vQ`7+MVKuR0JmOJ(vE#W-U?4I$=,voL&W%C>1d98.v7R207#J[`BXnr6tQQe$$&Y<#bLvV>rUV:vtR_[-9A[w'K]JraOTwRnu@Uw-Flu$MHiv>>f]Gs-5wo(MUQ88%"
        "ip0+v?UBraYjFF%Dq9>#D+Yc-RW8I$(YbgLSsT=u&r0'#L83&v]--iLo_.iLCUS7n[vIv$$6kn/n,=b%^=P:v8X-A/)D$%Mf<_hL$TaFrVF+0vm#r>LR9LF=7#8>#ZK=v-nTbqLiBWZ#"
        "aM@8>$?P:vD>uTVKkp<LtCZAMc#EEMW*A9cchU]l[ai=-530_--7g<CxR>L,^4S]7tmi=c<:Oe$>,c0#7Z=3veQ-RNa]Y5KjW?.Md_9tY5wU:v6]:jMQL6>#BGj+MoCE%tZl8bth?P:v"
        "='o3v<]F.#Hd98.6sZKrX=7>#D]KZ%Y85>#<u4G-i^rX-]eW-?W$#)Mi#hi'Y.o+vUUCkL<+(=skRci9+/OJ--.f/2o$m3vT5(9LQ,d?$[WWE(DWAp&3HQT/qfk1TRrDGq@ptm'Rk'+v"
        "2%I&M))RHLVf5.v$i@(#F@^v-RT9>#wVwxu(/'UD<**/Mf'nBXJo.a+*3-L-:%d<-A>^;-Vwud0/L4)#vXcqtEq_*8oVKj(V_9C8[Qkxu;^4?-;3t+.bkfsLS*o.Mq*iqYp$U:v<%CmL"
        "$3B@W?PlIq@[lxuiCAF-Oe94.C^KpLRXAA*vCDpot*^*vTKm<C_*S0oSV8bt*)>N'=^'@bMUVwuq*g/L-56j(/;RfruM[&#.0l63L`Jra6[9>#u>%u/='KRPoEAX>ho(T.McL+rVlK4."
        "L2-6MMTGs-M`[FMlI`^<J@o+sJA;npi']-v[TT]7KCE%t>:AtLECi1NjRI#15J^bKltejOP9cY#@k:'#^pQOHW(aQJUV;Jr8D]&#C$uH?Gn],j,v[&#k.+wp5Y]q)INL]7rYoi'EBWu."
        "n;8@VY1cB]O`($#M.8e*X7Hc%h`XY#=A1tLtf)ZYIV3DtO1T&#U4%BM/?KVMnDYDN.nNvN,[/6e>YxxuK#P*IYvgWht+NDO_'p+s_Uw1qfBp/r[P=&J8^<onT1$I3K[#kO<F>dhNAU:v"
        "oUT2V]f9>#BGj%u2sxs%nQ$.veG#f$40>-md(n924X2bN%SkxuNX'4.J7KcM2@KVMn]q.vkS`da2L-Z$&bGs-[UXgL&EZY#Hu]&#+t4bI6-]T&#WT:v&R,GM%8Q:v*-4Y%,Zr,#7NrIL"
        "#@Ywup/?l7%RXV-coP2(u7IH*qda1v0MsaeVHOl9W#wo%TF+^,94N6.+)G?MA8.Jrg.:$pRei;-4EsY0`$l3v`w)#bt;3jL0Y_@kk7mM(wUtxun@m630JCWbZTmxujp%>-sk8,MJPC%k"
        "1lHe6x$/DV<v*GM$5<ulh.p44:>*0LbxT:vxdVoR0RsFLn_U:vm`,naGv92#;5V:vt.xR4KcB]OrFj,S'v>7t*jpkt^d+^$<[A0M]#Su-U'4GMsqdX.p^:>#*4'C-9)Ml-t(5kOV=p3c"
        "p9L&#[Nm6a'RT6j^kc4cQSDW8c]-/8S-_#vAY'-v0]CDLK9>d$=%D(lZwBN'A+i-Ul+,FsK<R`$&[T:v1ou$MniF00XKT]7xIV.hN*m<-7upg-=&IU)&fYh,C$bX78X,]kG9Oe$Am6k7"
        "_k0-M/StrP?GiD+I:1@-_O>s-KP<uLp&ndhqVAuLtCh`7Vv:>#M$Z6N>d9I$Jr#vaVgAI$3al'&<8iD+(2]Y-CiT$9e5k*.=Q3ta[c9>#(fcc)4txCMFMM]PUX1fq22]&#w1nBJG+:A5"
        ".j5pLJ76>#;b.u-W<LfLbH40M9o,/(<N0j(+qU&#5WU-/xML]79(#f$2.lxu<<;iLovHTMhlA,M,LL1pp^6.v9-t4Me6@m/oEAX>-3T:vf8K4M<_q58'Sm+sxlZfr9F`&0r.n.LrW]X>"
        "/v=p7I15b@]xWq):oNcsro7NMQs@jLv:]0#[F1-/`'v7vuR;-mv..^C.9P:v(W*WJsHT:vR]feJ?@;Gsp55>#0E).SRunG2C4/k7s5WP&NBf3v9>M]7sTGs-]X;uL*hxJ.W63$#4P#R0"
        "<6V:vHV2&va%W*MCCvx<4tU:vp]OlSS^YN[&HZpL'Nt'Mu'E+rdnV:v7D=KMXr5<--6]Y-/-@0udlCg7``OkLN-b0vE_(3LR<+VdOmK&8%Skxu1pO?-d+KV-P$bB-i@^;-:n:p.r%'-v"
        "N:>L,-)ItU^hHO?6pWB-Sv@Y-<fZt1Ex6I$BNa/M*R>L81%YPp1CRwL>-m<-8D#W-[]$)&iV?i7LV@+MqZpu,*uEc%0q&K1;CQJrZ>sl&2(5>#3)6I$>7<laf?q-Mr,dvcCP`D+7jP,*"
        "wKVX-`>>_/43AX%ChXVPMU`e1Z?')3Gw37/A;8&vokJfLopC^>V&Fs%w#47/n:;b%gPod:JJhR'^Ve$OQZsk9fI[7nO)P:vhP9-m#[cHd@GdAMk]LgKTw.YcriZ*v*m_)M`[?HL%j7#Q"
        "_][Qsd,,@MSPwtLF/df(5w@Y-htf3FV_c*79x$mLs7V-MC1.AMc^J@#4402h)7@e-vAc'&W9,(#?-&OFmI#iLHik$Q3N4;nT&@A-pr=c-_K,L5A&>V7m9/X>nIV:v+kr=-dhr=-s_5<-"
        "gYfF-+r5<-,ewQ/GiM]7#;$fdt>l5/1e_hR0V>1MLZD[cEabPKt38b-pe/_J[R0tLh]FC5'eR&>hJX,MnD],vk%c0#xeW4o-XiX-XJ)qi3)s05CNFkFIV5#MrpfKW9vL$%T9pFa[/68f"
        "opWB-P(GG-S+n=.#Av6S).QJ`v@;=-;C;=-4tor/Jj8Z@.W'SeXlq9MnbR#1Up5k7*cFr;JD`IL,#q]c>hEtLpPqw-s#MuLLnN*I@0Vcj'BWSS^PDTfmVT:v7e6b%cWCd-[X[?Tc[e&M"
        "@vJ@>xe;>#oRaw'<UMkL7klgLCO*20+UrS&Q[l'#TE).S&BgOQ,K^sOUCAF-E.ad%YSbda2xux4$?@N'ZJgisb-H1k'0&gWhG#cOK@qbtuZ>J5e<qhtO9)2gjLvq-<F:5g1PNDs3Q^E<"
        "P`ht>1Ya=5KA`et6L-G=1][=5vK=>%/i)WsE_md)XuP^tVF0HMIrCY=B`s20G5qLLsDQN'B8[4t1%1dO-h%G;Cg'N'&E*)<5^&LpW>xu,UpV:vb9E$T_Z-tLGSFf0K+Dm'QmAr-N<j'&"
        "N;<*eurQDC-R;s%W::%vbjjqO-=ICq7FENO>QGHL%C2,v1r(mLv)LH87vaB-(r-A-%kj#.JM2rL.?qB)qTnump_i=-5=[A-]fDE-^sG%.pe3tLr$(bCGN7DkJwP&#NW9S/iO)1[vs-cr"
        "ucp;-UEFg$h$Lh5()/:)BVr(&O(t05I&adaj1LG),B@m/KFMi0.bU:vXiOkL0r5<-LMU[-#EL'J:v;f2R+Xe$HA(v#q>R&#4Yvh-0>Qt1_/Um9/Y-R<+1$Z$L12_A5u'4+r<?dMTR6>#"
        "J2-JU.kCZY`Ile$5J:>#uMjq-?78R3Xx7R3>WK/)3C_>$Txv,*D/2.v+=@m/LXk3vIM.JLO]_@-E?hH1BW%-v@4o.L3OW`<pii*/5TBuu'):I$CbQ@LtI=;-NNGv6C^Oq2/J0g;[Tlk-"
        ")K2O=xXtxuN0?[B[:;$%w$g+MF&gFiY/[]-c6eX:F$aSX52se$2.lxurAV.M*LjiM<`e@b>,k;MA-xO]Pu[>/_8uma:l&GMStF8M#KnUMFUel/1ke%v+[;YM`^Ih%boP^t,sC-*9<:$%"
        "aI9jOEeOf$Rq[>*^MDstnCMeLW;ti^t&oqO#x9>#laxNF3@c9M`>K4L,B8>#wb4&vWm[4L's/kLQQ>rdZ'O,*br/#v1rMfL%)jBq?[3>dL-vxus;-L5+Tg$##c2BM5QY)qw@;ig;>eA-"
        ":c$]-,?$:)Xvk+`KqM-a[,lMg&g]=ubk;>#im:e?T8tjLK`k/M[+J21Qhpv-$>7>#UXt9)PI':))*[*7Y$Js-jsx$M[$xjUYl8g1Z5'+KMI7P^QtM1v@h2$Mb*D-MO_UqY?[uxut@Ye$"
        "F>^xLLf(`j>QF1FpA0.v+.-I-')S/L8f5>#84ji-a)9I$PZL]7`a:>#0'Q5_b`W&#SJ]48hvSe$DJY?^0$&kL5^-lLLL3ig39W211c0o-KFJ0Ym#`sLC.r%MsM%)*wUtxusLsNO?Eq'#"
        "`VaPsh[j1u,)p@9mTrktrUFna5Ff1Z_3rpO'v5D-Y+q<0?Ud0#Q>XrEd3a9`9T6k71ne+M0qCp.R%n3v`rY-H36%#vOAY-H8dPc(&sqMqb_b,vZF'4++^KkFx?K0ckD7FM/o5<-i'd<-"
        ";d98/TBoqOYu*GM*YGs-ZRb,Mf:Mt-d3_-M7Dg,MEa[`*Ex7Jrav2'#5DFZ%g8&gLKRd6#l;WQH?#&#,/cIe-`B$3UPRv>m7@-_Ke6UhLJ2A(sb)Ok4kA%+M_M%)*C8D)+Lhg%u0)S&#"
        "ldwI-%Sg;-jYp_-047I$Q1AT%W%s#l)9$IqNU7`tGU,Xl1&=+h7:ins=qoktqe)ssJBsktRnNf$pw(.:+E9L_=dD[O^?EstRnVctWlcqL3rIiLwE`c)Vrjv-jl;>#9R@q.82B]OXvfe$"
        "P/*daBsLEMji*VnZ@l%lYRx],.O?$vd$)-vs>L]7k#U:vsx-tL]cNNRK4%<-9WFG%EJR]*UX1fqvq2WJvj.,v11Kra#MbxuT5Bp7mo0B>LP6vu;tn*%$bs;-5^+%&xhZ&H#Y8K%5U?.M"
        "[X9tY2,Sj9Z9rl-%RDqD_a@p7=w(T/@)Kt:CkM9i_'>rLj_iA.$V*R0*u[iqxXb&#&L&%M&Bb5CaAg;-UEDA.OX/k7,s:Mq5DgM1VHI21N9nM1>WvX$(e>A4NIX$0P?nWMQek_7dSP)v"
        "eb9vL-%4W>whZ+vW.q)5M=6>#1VVI8HT:C&*/`:m&2QD-@9fY-bs<hG#woe$I+[nLSW/+M1^_@kR%#h3bFFZ%afTY#Hj.`s?f5>#;2B]O@Uu)%/+HwLVLYd$Hc'&l[UI#1.o%%D/]7fh"
        "p,e@BA0Yw9QI@I$ubUe-;9V)F(KS^AJpoP'xWqxu3T3S8[QC&l@VN1v#$t3Mcp)*MD(MS.B`bQj=[Faa*P5W-G,&CAqgqV.VO=#-8AZp.GnBraZ,6wgS8nQ:_8`]$N6n--(w8I$ZK$tL"
        "8=5,M,)K[YxQ6vusn0b79p`hL?5]8^L]<on?^A+9/WX&#bBdWqO#cmdv2QP/xVT:vxOoMVl/iQlf8cY#Ocl8IK%QNsi-pktCNGn$ennFigsA,vw3WoL_V>rds5P<h%`A*M[KihL.]Y7e"
        "H2WM-KC8/.28sgLSuUn'=Fblgqbmrnvei21_<*kO3]xwV=Q;>#e>FgMaUj?>IMM/v)dL]7d,*ZQA1=A>+_#6//vL]7LVaOs#N:_a=R#S7x#3Kst[mbV'L+N'@rETs'i5>#+ov$Md@+jL"
        "soD+rHu]&#@YqKG[oe*@:T0I$6E'j&caW5'BfxxuWL_$0i0d';uPwxui$Hb.#KtaeHtuxu(n]w'2bFi7VMO]<xXkxuN[N#.dtNfLWLg;-`9;X-)'n--FhP&#8HpS%3Ymq;Qc=nN((%5L"
        ".o1T.YU9>#WkF8Mpd['M5v<A+T;R:MOS?p-Y1cB]*i0-MSM3f/%(=M$-xVVR42J&#weWfr0,]&#k&,_S*:7F.@M@['Pw:T-ku9T&,PLrLC`7:lFnlxun&JY-0V(FRd3EVne-arnBIBI$"
        "XjDWSM@Q:vrIY2MirA,Me?OP&RhSvn_@eA->rr6Mv+-xL.HGA>-dIQ-fnUd-po`e-;7]$lRrFtUG@C,Mw-wi$>/9W%';saepKgqL=bU`3okO?-8uU5/TO]h[/W^e;`wtlTeRUlLdmj0v"
        "*4%Q/>,7&vUBCra]l[>-m-L&#((U#vZ)idag,T1psN>#-oRT(&/<(:28N0I$/QTU>p>w3vfsCraO/FG2:YZ'8MMZOM1[Ps-@jLaMh.3waUQXV-n@R:v3T1tLEerd0*UU:vY>AkFB3<X("
        "aeCL,q&8*vdSDqaXEmxu<w8S3Ov/$#%W'Se`'(1v31S]7dJV.h7GtBJh;T:vS@##v=qNq2j36#vLFFZ%A)&3r<,%M..FaFL[9-KNVK8Cqd:hc)P`28f8?3m0&-.,j>IVX-b&[3X%KM<L"
        "_0`w'@Lc_7H2:-MMSrILO38#*BrGBVN5D-d]xVq26kH-#q,*/M-lK(M$cpu,-)P:v0e&Z[K6[Q0['R@^`n'hL/xLS.>Vw],ib.T.mfu`TI>]>-G,Xf-:92O+dpLEMKux1_9I^U;7Tb$B"
        "c*hdaq;().8Z?gLeE5h8Kv#2q1Q@GjZ>[Ze5QQ)MNj9xtMU.8o<ne;-^c[m->(=h>n>4ha:]R@>d'8>#RjI>/D#6&v]-7lLE7KKi%V9>#<-31I0%ja,7rxXuuuV,VnUigts5_Rs?BGn$"
        "N<ml/Mp&0v*KWxL$x6A42ZhTiismgL/E?)Mngq%4daclgYc8'#,/8&vj60*MB8hKM*i<r$j>xjOp@H`H7IS:v<AFgM@CbsY,-r,vtLJraWk8gMZ#?rY#jE#vvK,i%n%`3rQOQY#@v+B$"
        "9D3=(&t2e)oVe4vafNS7rAQN'u)bUC?Ss8uZ`6gs::`mLEk]ct`:q^$7ARF)?8vqtXWQDs#'=kMDpREN*CY_O@BLJs;Nj8uo9lT8`%0l8_B[,vWDPA@'%g+MEfcCL^5FO8l]0)25B%+T"
        "f2uO04q:>#Ym@7>q'A9&;dZ)S^e'-M#_c,M/%ms-=kRfL^^4?-:_-x-;H=mLtYDP0'E^$.3]JfLVJ14h;,T1pW-6eHbT8>#BWx,vpXd%#HJV.hNZ;E-M#Ql0j2pY7bBP:vt0#kOAVWm/"
        "xud,v]i(FLq<0`j7*P:v?>2&vO^hda+U&]t;YT:vjd,i%^:5>#+4#h%rG9X:8n[0#(Zoi'odme$_^)&+#-s;%0goHH]>>mLq9=JM@gM1LQ]9>#tYfeJ/OV8&evvxuA6%_]QUA(M?3J,v"
        "t]$pa/p><cn];GsGD#W-NZc<LnTJSoA,)X->nTn*W.?#M.?S/T/Wi(v0pbkameR+ii8$N0g2p]t&:`ZYs2wiL1DFl]]e,+<nc7p&/aM`t@$$LG_W#]TU44,M4v-q7g8f$RP21[-n7I['"
        "9Q`daopBwLC*vW-&5DU2TV2U2kP*_Nk7g,M+nD+rr16pJ11]x%>[ldMbCf>-A@TgLD_-2M?K/jOeaD@',$EtC)6AW%I@lwLi02c`':KGsPVF$K9Z#oLi];E-n1aF&<]8/LtL7&*?kEWo"
        "SQ'(Mm$kr$],r)#-*[M_l$Me$MKZq)3P)W>9[7fh@FFPgVJ[ihJmhlgnB;=-vdi=-G8lg.J^M9b(6&70UF0=WmgB`h%l8U2;GCuuI^c;Lj*;>5/TBSokV&(#g#2dOQn=Cb;:vuuC^XAM"
        "[1XBMC0M-Mx=2x3mEY>-Ug;GjwRX&#B7;$^J87k7<`(l-G6sKPivdnT-#:m'iianq?/x.MG15m']PSM'L5cn*XI3'MMc,KhY`:>#b$se$^Td`*3S9?$AA6oOeDBC-_;C[-s>k$0RB2#v"
        "3L,i%^(>,MO76>#A0J>-W=>S.t+T1p,+('#E=9dFe*@[[unS+M84<ul'+Xf-BN9C&k?AI$YARk7cR)W>0<T:vcE%QL`@dV>&n7^lGE4.vE;)rL)+3?>'t8>#b]Qt.6)ToR31Xk4np-kX"
        "tY9>#@b.u-5A>mLk`l)N)2-uYSrY=U7V-3(dk;>#(2WbMd/7aNo0MT./W`eSr)tX%d+qB%tD=>#0GeCsI<s8&g%efRX1ur)S+ogUcR:>#wfcc)E7vJ2f=c<L<84p/vOkxuSWg_-u(eNb"
        "HdevL%x-v?Z@l%l:X1'#dSBuu<lf-M*W)xL?vV[cX,<5&f#d<-Gh.Y-H%ChPh6u9)P9M1L7n0)mUHkl&>nq#$'WW/v*4%Q/:,7&vJI/k7Uv^,*odme$@T&]tP^K,*f8a#vPY4jVd%Dul"
        "/_=?-vdjB%:qZiLe:wIUI6Hc%Pw0V60&#vu:l-4MB%wLM;%H`-]ZPq2xWKF,bLj?>US:>#Ihr=-<ID=-rx#*OSVd>>FnE2Lt:R*.,D$%MI6c-v%rWS7oeW_OTSTU>+YAZ$X,:dHJ%e9D"
        "YJ2E+,j=7%C;-s@;Y,j1m5nxuYwU5/>lc&-(o,AM/qY<O?xd8.s8%VmJBr#vle3&vP1]*#9<PDMZ8xe0-,r.r'iSMhDtEn<df-IU+nEn<cRRAMW7+/M-2N*Mdu<A+j1.a$Q=sl&9#=sR"
        "D)`EM;QP,MDp)*MQk[f$)rB#$LsE%v3=3dOSK9EsuV<Ls6fP#%bM)sYCJS:v+Ld$@w$@Mhw=Frp1xPW-]Ai<C'Dbn*(/'F7R9&M^CjB6seE5R3HNBp.3?p3v?N&[^Al6I$E1)xdxikum"
        ",[^W7p^6.vOLm1#nTdJ;%3$9/u_vYe.GpkL8hOZ-C+w929T9I$E^IwB.nOO0e?:?-1S(e%Wso*%G+;>#:[`=-]ClG-#Jg;-G7;X-QT8I$cZ[[Yib9>#m``<U#)WrL5lQ1M'w<BMS=uYd"
        "K%:c`(ddGiC@rvmaT9>#8*'-vJM$FI^PJwBO1iFps/wS/W0[]-$e5L,Z-Qn*2E'%M[2),)qHmi_9+>0.eUd%MUf%S[Bmc81@UJnPWTii_i-rAlB&-+H,C/CM6m2EMW*OBM/o5<-;u*3."
        "OB@0MYqFpYoRZJV;^H$gIOh7[6:P&#dVY8.30FELCDR88)khxur>R&#+JD=-TO>s-=/TlL^`$29>/2.v>m:e?O(@p.d8>uuCvn--p^P7MpXZ)Mk*MB>dP,d3^e1l-Y:e'&d#;baE*cb)"
        "BSHnnl1RA-%?Uw-sU;'MorX3LH[*5p:8O50uIbxuj03;n:70C-fsKs.p@JraVrfe$WZ8>#homt.+,W5UX#pe$8LRt:%JX,M6;5T>H,u]tFE:Jsp85>#,l9o-'_]0>?[u&#U/=CUxIg;-"
        "oP5w.-pL]7X(#f$F)'UD4jb3FB5F,Mcs5<-i8@uL7CN'1jh__OJu*NHt9%Vm%anxu^8@e-uk?I$uj<lLaK[]MZ/6seWK.'v,F*W-kQmBSH:Un[64`BS0J@naq?84+NgnUJ;me+Mo76>#"
        "RRE3MG&X]+rw(dMpP]X%Km4:HwMQN'T6JlsSr.8uWB08g_I>biiCrZsbH9Es7t=QsSV7Csn:t.%HdS8m2)CAcLH[e-o2k'&Xb%%MMOYSlSofcjenEqDIn<>d1Smh--o1I$M%+^7$cbM:"
        ":r>g`6V9>#ohXq)DVC1#6-Mi2lJN(&pZA%M^xgm(Yu9>#fF5tLnA^nLuAjM-e1G[MKqsfLO8`P-D)_s.b?5>#,^N$BV#`ejP,hlpR/0_-Q%tE@oaQeaa]L(&cQCsL=v'b(Li)h)5AR:v"
        "Qx.i%vT^K/2Z1&vIZhfL-NVHKt[[>6#&JY-<)$:)s7cjL?xPW-8&Y-?G$VhL$V')TnQd#vDBoqO%1eCsv@:@-H9&/.>7_xL_=MT.13:aGMSfn$qCYh,EK8U)57cxuHru05EnBQg/o/.v"
        "_cIra`s0^=ddo6*L%L,M+*df(0,f'&cDm)PiIpu,HZfAOLFAQ0V+9>#:[sn%XBeM1[IIp.kw9>#1*>kF,ePq9p>HE+KU9>#Tk70uK8qnL:xPW-I8Sn*dEhfLnM]WZO$oG=rVBN'1b(-*"
        "LdPqdZ0g;th3utcRWTtcTub>%q8'%MFF99&TEoHs284j)Hnf)ucwVEel3l^7>RPdhV:1Al9+U`ka@SKr^7%VmHpV:vF>o(EpjJs-qI$nL?OaYPvwBGjc^5<-+[:E3tASoR)D0?FHZoi'"
        "6v_s60]$s$W'1DtwRX&#rJ<b7pLAe?Y6%[YDhAcM>I<0v1Gd;-sT6T-wO>s-`tCkLG`BiB1R;>#742X-<[3U;'b/nLM4U0MHC[+sm/h-*@x:qOo^-<&A.0q@+ClbO'2_,+*A?,v5Jlxa"
        "ud-uP)Nq*MZESbO<-DeMKio<9u33V66](V$'A$$*VI/Euo,=sZe'kkO>9*N'Xjf[=>oukt`m@htkvEttgjEtt<eUtgkrjnIp8ubVksObVb=A[s5`Fr)ZP9EsBlXIsU[J9&>eS9&8Y54/"
        "e#?u8D't<YodC9r_8M]7[2V:v%TYO-?qoj-;h7I$+ZL(&?1Pn*oq9i8/$U:vcdne$i5g,vTNgoa%B@rmU@R:vXgw$MK47b%:G;'M-Vn^r/_]Ds[,#g)SMH#6'<]>-s^rX-$(7U;q3A[0"
        "*kY,a$kvlh;k62v1Xgda0eMxb<`Cc%s22V61A'UsQ85>#.<#%M4S-1Ml;HO0cE220B&5>#-YE^-*2:I$eN(sLDN%.Me7(4L;5]N107HZ%k206t?\?I&*IhQT9wsAJ1Kwi58$'+^GvGEjL"
        "_uZ=lhTW&OW=4bh^oU:vrRZQ8AcCPg;A>t%Jw,gO`#U:vTLYa8om4R*p%G)NKI9[8RLvV%f'EQ8dQc]Y7JvD-iB6I%5x_vR62`iNYW&xCj:=w(Jh6wg:=5@pWBeM1a][-s+9=qOG#&#,"
        "Lp/.vNrh$bXmj87k-w,v?UBra@6UG)vf`./gRwRnM*P:vxd%-v>aeuaRK*j1Z[6>mDbwQ/]^ntWher#ls-+-*p>7>#g%i'&L[TON?5+/MnE#W-<XAQ1iG-i$E^G#[*9BN0gHeN1k3O:`"
        ";gX+`c9>W/#dOj)c(El]NvxvRMxVb9UnNYdxEhM:>K+v-ravwM]lA,MgMGd$GkP&##fLH-+r5<-e-)X-lH]t1kB4OM-uF=,FdK[0kPcq;Up3W%*5m92#P@#N_NmPMr1#A>`o=qMipwW%"
        "G164/cf+Fsi&vt&0,j3vGn%Q8eQl'&3*^5/6F4.v;pC3Nx;f,v&J#0#WP6]/GAR:v2=o3vpOd;-m(Vl-H<AI$.ePpag0WhB0AT_O<]wFk='@gL*h/fhBq1'#A:%-vuXJi8jTkxuWj.Y-"
        "h;0I$).qOHX1Q]%LXs%=L$$vut3j8L-#D)MXriBY1LA2'#5suuC9wva:ChcDQN*31V?7>#g#l'&P0)hLA>v`ObkQdFw;x,v1mf(%MNrQNGAJ=LP2r3v%[IxawOfM`Nh5>#V7+[T-?Uq2"
        "FcW7#T-scUDuE#-&$$h-#cj-6kA%:)Q1%nLUYS,re?WVn8ra9MY:v:NK=Q:v>]u?0TStwL,;/R7SFP.qhE#W-DWf3=VF;$MpfaujfK/8ok)A>-`0R]-&Ln05bd?*#rlhFrU=-k%en?/7"
        "8h`0v3jToLUNll;-_f8.Tv,^TdQc88UJw^][96gL;#46fwarM1#dPW-e25I$Tk^fL['p5'@P.a+$f2i-9p5I$5R-iLFxBE>35oxucePW-;2i'&B8llLoD1_,Gn02hf;.6/>dm3vKWJ%M"
        ":KihL?wU4fjt?]-:K=_/@ih$MOiA0qJ>S(&E>gxLWeHBXpj/8oZi>%.MS>mLFW7ELFDQ,*Mg,`-E9f'&'I<mLw&^f1,jW50:gCd-,@8X1_?5)MW.*C=)IDs%`F#W-2gT-HM?rhLianJr"
        "v]oM1VF#W-oeb-6FW;ra]5:s@xL,W-gftB8Ti^xLcSusQWCDAc>fdx/Q1CBLgRwRnrDC@-D(]u-&ZHsLUZ@C>iwMTJ&p/.viV/ALB'9;eGjt?%o1[6C4xPW-+-D6s)EtlLm)&crFWAda"
        "1@R:vS`%:)I%HkLndOH=DX%&cTqia-CQi'&b$CCi`P=w5$sg(*MLuwt7dZ_s_%'Y5KopktPe'N',BPYsWtLR@'j,C%+=.?mk?_-vxXJra9pUp$:tnl%OV;fFsF6Vs%7fY-WcXe$hngda"
        "EbGxkMBrwC%>-YQ_J15pjP:d->2k0,C9F++4P]eCD9U5MiN/0@j?@4`IXI<XgjIJ1r=q)vvMDraSM7.vvdJX1nrM]77.d11=^;>#=d2&v&YFS7ngGA50;,i%*TjDsg1'Fs5]vB7$E_/u"
        "N'PmsRP<$^DVh-U,+el)fV9:Hgws%uPxGjs(Mi,ulQs*Msh]_3Ue):Hjt*N'F_O7*fUWFM)Nq*MsD#cO*4mh^&rGh^`DNbM3sZFMumGuO`Q4U%frr#lX>XY#jvOj)-Iuvc)8L#*8SOwt"
        "tb5lOYh@_O@ltwttn5lOThP^tTJDot[b5qtg9q^s,0/f:9+8N'DJ<ltU-R%+5u2dO*k%G;+,xuQQ1Xm/fV+f:%*P>uul6W?S5muskWn&*G?+UmoOH+;i-KtOQYb-_Ur0._#BLV%CkwO/"
        "VJ+o[0mP^t2;_+tEmf9&`;CJs4so9&w(do)XY`&v6WKFL'g]=u6&5>#L=?uu5f7k70k%T&T2ip.&m;>#[i%:)d;MrL>U._Y[aBoeX=WH*d@R:vnDkqOH^]q)[?YDMf&uZ-Bxt--WBP['"
        "P+v#M:QQ)MC[Y7eKFL[-fXue$Z0LG)m6;iL6Ani$EZ0@0T1<?Q(TKe6PM51M;Ei=/fkIi7;?=cM#1>cM2%g+Mq'c?-[SK,.h:RqLup;i-3lIn3w.xfLXQvu#0`7Q-cW;i/e_(-*;tk$."
        ".BKO+s&t$kw8[t16UUbr8Pk(M.m9xt^u6RS6urH-4'<6.rP9nL(j>-0C]M]7AORV61W,W-QBE-v6F:#vg%JraF#Y?-xM#M*E2)B?$-k4)EsHpas98f.5f7k70T(L.3=.%#tv[fj,GPY#"
        "7U]?9NGo`s[pco)I3/$P=:p6[+bL+ri=uc-(3Ih5[1d0#Zx3ciC*Wi-B2=b7]dYqLS,fiL+P3igFJr5%UQ.gS*t$#vL[Ue$MpRxRB[0n-S-GPgT8%2hJmhlgcxc<-?%7A-0M]j/m5Bla"
        "qQm@b[UT&#Q?oY-H&uH?M?CI?^$w+v(=^q)qA)=(7_?iL@vjb$cEZJXP(Y@tmk1oOPTn0v<u.k7=asY$Y]bZI(L4d*H-$L-8fW^-(Y8kXnw<FL0vMEGn0>#%rBw6*N)c;-%'JY-GX'CA"
        "_q@I$wJ$)&?jO*R@VftY)vL0Y*0x4LQ]8Gj/&:m'9N/^l?)J1vd>:U20:RA,UX1fq987?&W*JBPGqY&#7=CqMYcv92oIn929kQiLf*uZ-h]l'&c_k05`78R30Wl'&Ar&:);*L]7^@0DN"
        "O76>#QHk7/cR-:X7rY.?mRkxu&@2=-9D#W-rr_q2o5o343P)W>s'J`%Dk&q.:Rb&v<TB^Z5$Wqs8Xip)d'E&d]uf2%pukf(w2w.vur`daH:'GH%a0Dt,JBD3Gn%3vRb/k7bU]]$/w9>#"
        "5@We$bMadaj$&#,34U+v'R.f7uP7&EYc1j(H0M#vp]q3v]@ofL4:-LSapP^t(O1;tJW0m?Nq#Pfp^6.vS$n'/PJ7k7sm6]-odme$<'I(s.veX$$:unghIr]tn2XBmKMDvsQ$-TesNN[="
        "Ih_d-;FY6N(Zg;-.lYW-pIc6EXlZh,9xXn*PWL]73v]^bsc+=G-nG%.'.RI8eRkxu(Vh[-Ukh*.a6*J01'Dul=/g*v?-hdatRrIh6;Q-.pKuD>5h6nT$;,SH'i$+vq0nkaGnm*MD>6>#"
        "dtKfLRBg;-t;)=-K'+,3xoO`O:-;>#N&x'Um=TZdO<BgLt9X'Mc7MS.`%K,v?Ue&Ms89>,r/[*I.TFKh>3kCjK(Ll3mE8vuXXUia6ZXwunhQ2#0f?8%wTHI%e>s,#-kh.52*ceBqWkxu"
        "YSlk->RA['[<G)M1cfr6$Qre$Z.)$%QRdHNP[_%O>p*+Nx&,%M/Gu`svOR:v6=qE%NfeFMYNaYPcwp&udk;>#DW&-vb:8:)vOkxu8iIY%-SQW-Je_Nk6dp[MU3VDL5D?9TprcQ93/He6"
        "=4?HMN`rR90'HV'J_e&MeT:D3s5R&#nV4C.hD7;65eeG3#a>9TwCDR9uHv92.pH['_u.k7RX[ARGo3N.w%@cV4.0aO%l2'MUZdIqOW;,j(a#rLsr$i$CP0$f5l&gLkL6>#uwL`EdpsOo"
        "6_/g)HrDMq-00d5P(4&vn>Uoa>uw@>Ol=0LN*.FsW-0.vlQrkLBXPgL/I^f1DBD0v%o#va[UXgL7wGJ(/$4m'd^Os-d3_-M3[c,M/o5<-N#rh.owCoOe80C-)kL90/S7k7R#9loodme$"
        "K]$s$usa;n/o/.vY4nBJkmSCgU2/^Opckg$E^jjL4M8rm,'?#-vOkxu;XfrL8QH.qvLO&#@qnq.f>dhRX#pe$nYeq)rxh3=<>Jra,jie$'C3C/UV7k7Q,FO4.p0^<*QlIqG(wT-$t7Y-"
        ";mk'&i^0pLYm^P@WHPY#5#)29dW9N'J$RO/lXZwuKHf;-)K2a-KI]*@9`cM97M1_AwZDXC2.lxu79S>-A*6IPF0)iL)mFkLR:Y(Nj3KOMW#7A-+o5<-Fo.RMQ._o-:(#f$#&u05X0:I$"
        "*cT%O%q#D-QM0dNWn(-M#whdH58wwB;-X1uU*,:&,VdVs/o`x)jbX9`tm7LcU5wBS]jo9;Fo+#v0JY2MPtA,M]nY[O>iZiMo2>cM3;#%O>V20CW,p*%G&qmV%=5,M>vVFO/_YHM=XPgL"
        "t46>#[J'X%%qAf)T<g)uL:I0Yu&Qn3h[cdh&tS:vt_HT%%&JU)1_T]7M-Y:vVakr$,,U:vZF$%Mo4wHL(S_V$)1h#vD0c>R%*+vuC5U8gqND9Nf?q-M_oS+M2.ms-&)$NMU@D*SW?:>#"
        "VwO$H5'o5'_(jwL`Gg;-SxIj1ehjqOLtKB>LO8n/.^ihL2LRmgTp.48-W:*kj]vkt5BSPsR.*r15_)%vZt$>#Vb8]tLW^dO4`Se$?YsG28@+Eu^H9Es?`I?PTT_wXfns#.FbPfL6`;E-"
        "0aTg-<k7I$LSQYQ1/Xj.ZPlIqlZ@Q-F6fY-h9pMs3r5<-mE;=-BWtr.&=3dOX#pe$uHhdaqR]tYX4T:v-N=v-ro4lL1DOgp$ZuPp#YT:viDg'&.SsrLEde;#4V&]tixra-#5qKGZc@k7"
        "a/?E34xAdsgU9>#Yv)i%..koomi4c-L8rBA5/T<h*nb&Wn>_iqqevM4W6WS%1(aE,]>0c;,rl=ls(-vuh>b$M?9'dWM;:>#oq*C&lnDmLTRchq$I0%%ob_<Jmka?>6p7ZU#)1,)JSi.v"
        "M*Crais&gLfLQ:vwpGvRKSIFMgpIiL7BhLp0?$p-+jlEI'_%kLDT3ig-(Sm8Yc'#vn]0%MZqJ+iB'.Z$mA&j-rWsHHO6HtLOi`V?8t2W%o>@$TObs'&,Hc'&pNonL4<eAOv/dR'm3a**"
        "j9D?Y#,ctGDSb2g^.+R-i$#k-6h/_Jl7bdaQ^f8_>?`%%.%lxu=LACN>vC`h0Aa$0$Pq0#rle+MN#lZ-S`vh,&d>nLAD21L;x=b7Q.v#M@nCnB<Ehlg#TI&#[F5U2.7#:)f+9n7b?5>#"
        "xObxup(MKaRcvT/fsCra^a./1'6x(b#i^Q^GL9Es=1ur)b,rj;C%p'#VT]?9h<m`sVN,o)PXdx%oZ,o)[pre=-TO]uCZwE%56ZNs9'O'*vLTdQk-sW>G,U:vaBoqOa=:sbwI,W-Bs-aQ"
        "C`8WnKAR:v5FFZ%BO6+n'L81uXw7lOr*q]Oo`:T/VpZS%d=7>#oSBuu_`-W-Ys-:)Ns[bT8r]mAHVooS&aXA>gfc8.*:9*WVveZ$dk;>#<a=mLmxCp.hk?uui=vHHxbQt-(FkHQ]oP^t"
        "c[,AOkt;U/L=Gq)YG/-:5%)T&C2]Y-f&g'&K9i1L4MxuQ2E?',qF8vu#`,=#)T2E-_LM=-UL0g-;l3Or]$QfCOFk3.Os,)M3S%)*$N_[-&u6I$O_krL<E8,V(tTYZpY/Z$wdPW-A@7I$"
        "HXBsLb5CU%+[%T&mD#W-5nE_/bpQtLpUhasLIBv-`-'_-Dv1O=W=jpLK=pP@`3Uv-^H-A=cb5g)jdPW-x2x?0)EuoLS%)#t?'[;%<Sxxu#)h'&7,XS7[)prn3e><-KskZ->.Z$0TTvtL"
        "8]e;Ya4Pv$XYq[-3Sq'&=cs9)-W5&M^J:pRdiN&#L^Es%IIVX-c^#:)dL)mLMGVR@X`cQ'^F4.v?Vdi7[/6th#)]+MXttp[O5>m'$fPW-g5Fb.$llwL3[c,MZ>`-MBDDG`(:8g)u-R]-"
        "d/r05U3_sLX$8Cs#/_G3AB$Hs.H8vucS(5Mp1N*MAIdjBk#clpD8fY-Rm7kO#2srLZDs+;E9&vu.g%vL-C(_GBK05px<oY-Dkr<:<++)M/vZ'=)nrxlN^5<-bdPW-35B_/B5h%M,&90S"
        "9Jxxu%2+u&OAL[-w1k--*n7kLlB/mIfl=Alq+)X-uS4I$[itwLor>LoYxT:v.tv$M_1Cg0f;/Z$@%uZ-PMc'&sKJnLK]Mmn]nt?07BX2(6*A>-EtBo-*I#:)Nd7;Mfdj3T+GE,sv(5>#"
        "Ufn2.qCm&M1,RF<<2<DtdBQh-wi/L,e+lwL<YKnfIX9DtPIVX-#<v9)brXwLtL?-jwSCMq7cPW-0gUq2FTR;MrP&&*V+sofXZFZ-X#)UMgljgLO68_qVN<>m5FxY--UU$9Z$^(MaMw,C"
        "SK8U&V)>N'aN*qs=G#u#NP.T%d)irtKOj89]6Iu#[kcms(x$,;3hTwu3[bpa*N5:9J#7gLpQeheROAJrB8@e-XI1I$N.7nLx%DIT<hL(&Qq8pL-m2EMR9mrFIF/rL9<hGLDRv8&eMwsL"
        "txL]7%Ytxua@fq)`^-tLh^tx+9T_Ds_r(xXd2e(v27*'MU+9k7k$0a+d80C-7up)OJP4NLV?klpO2]Y-m]j-6R$g`al`;>#J4=X(D`1DtFwux4RUBN'hJ7=B2_5jsa^pkt_xNttIrRO7"
        "eB9Es^SWl=93vjsmWpktqkgut8?8xt<K8xtXR4p6aeI-ud;DGsr<mQ8Ne/=m&AfHsg:s-$m5pktZiZct7BR/3&Y'btde'btR,_RsbbF^$`7SJ*Icre2qOFwRFHbZe4LLDs.=Y#uXVLu:"
        "^wvKG^@;GsLI0(*c/#<-mc0o%e@S]7J(i?%dLa@t&iSN-Bbi=-SW6T-&l9%%OHo;-f<U7AQ&12q,G:[9LcJraOwrD%sFV'8PS_V$dnV:vK,-eYOIQ:vPdw,vGN`'8Nwk)M3`,KhWkhFr"
        "*9,HMjJs*MGI^k$4*oP'*okxuY[E^-OnqwIGDS2L)85>#<7ggL]=*#.PgOoLk]c,MuqJfL,?=L%nrg%MG/&crieI_6]9McD(oQ3:'T_Ds0ZYF[@g:EMQmrLhuOax=MHQfrtOYn3%w7UD"
        "69;]f(:#gLrr<A+Bi`Nt7#(LhEJi/vwJL]7<b1tKj1p[Y#plf%s4*F@rgNiCHL:&5WW;#vbFFZ%l@kJ-dXm-.IG7FMMi]j-*iNk4NAP:v*oem-#Of$'$W,]k8Clxu:%7A-J:sq//%S]7"
        "'IT:v0RM=-,B]>-tK<(6J`oPs_D;p)*%g*>SYxU-662_s)PwH%lukf(=5]t1-x9hLq4n/vd-'_-E?_q)xPR&#DQp4Mg.P8M8_Bg-)vS:'#u]+Mb;cA%jmkNbj9D,Mk4aFrV%`:D=^uan"
        "/`uNt3>dV>SPU:vVbvwMPI6>#s#j;JldQ.MW)NEMYT[0B&xB#vY.j3vPTAI$%v.:)Y4KrawosOo.d>Pp6A,#mBa)F@V6cKcfpK?DexJk$TkoCMcx_%MxKOP&u.Rs?bE@s%X.IW-S[d'/"
        "&_U&#<:4S&6#`$'#pE+-#.gfLsQ02'L'Q&#)9YHO0hu,Mrw96Ai]5W-]PvAQ]p,/(rB;=-&nT@1cIf8#.NMM09T_Ds&O9g-UA=[9ip<[901XL><-9aY$.,GM3$;-MDOg/LjO:>#S,kW%"
        "O@P:v*#<pTZbA#Z)rixF1QaJ2.=wQ'tP%)t%kP:vJDL3b;1Z$9?u5g^Ib:HGV=jPe=Pfxuaqmum(0ix0USBuu#.<&#^u-crI'm&#i(((L4bn;--xDU%s/:MqG0`^#jYBjO%vN'J5wh?B"
        "HE`uLMHC;,(*FcMW=39d)WDI$ouCla@g:EMv@sEMu7R8dTrmwDq/n>V6JwjtQ&Re$x%)R<Ju=uY&IU:v:]:jMM@6>#v:Z/Nkw,$A7ZN^OO?2iL,?4gMDoWB-E'co&;`J^PSd6vuI<k'M"
        "<J#L:(hDA+4#u0v77R&M2>2=.[nDN'tIqj^l5;Ofq*.^tkCP:&DJolsDgVx)#<P:v,O)kF&NlOebOT:v)%?mLj_Gs-'9&%M?OvL+DnM9i$ICra:ExcMW'X]+Q+1gLL^5<-W`IQ-w@^;-"
        "@]5<-:C;=-w@jU/qspqOmY1fqN(&e*6@R:v$L,i%'7rx0?L?uuB?g5#=LGdhC.nJ)wUtxuAbhTifSq$b.DQN(1kU:vH,,[TjH*F.7/_mLFOR1gpN,]B@d(*Mp]_@k0)JN1,e&xDn)`@E"
        "_FT:vS]<wg_sq)#+8vLgCZ2wLmTCra@J=;.dYE>d%=:X1f8ae6_0Rb4xQ6vucR^2L&C*d3DqB#-E=#W.qwDY0UDFZ%6EUF3%wc+Mpv3x)h#/m00K6GsvLO&#n,UW.(lJra>S]ZM_>6)M"
        "v[PV-s43qib#&#,G6V(&auqT`2@hhLD<Gq<HpU(&;WeTr$A=gLRVN.h,WPDXR@>O-S=1@-Pp5`->NZNtOZY4r^)$DMnGQDMgiU]M12@_%%Vkxue46h-p<2@9[`_s-YLX,M..F'MawGJ("
        "1CYfiG6V(&)#_t:0DqLj,pr&.cx[5NX78G;S1^&#sodBSw.q^Ypw9>#Kqs3Mt>IP/#]Q2(:fX(&1x=[p:tm.M&;5]YudT%OK$Pw$a/0F.f]]^e>Cg,MqMel/?[AdF#,@N',cGF;a$V_O"
        "j(ssYjhT:vm%RiKpKJX>f]Gs-CvC*MNj9xt/f7I-:88T@-*&vu=Zim$p3X<U#5TSScZhe$FK3C8*]1X%r@2p)&R,Z$jT-iLe7_`$wYKt:1TFKh%_KdRkKC;$'Zb5'<EWq-Hv4bRfQU<L"
        "Dw1ehw1n1*dc2aM#7.mg$qsOoV/)0vKQhdaG9%VmbtEc%*$w`_K]R0%aG^Y$Ff$(3_%;J-$5N6.8>RsLgh=f0[32.v=5]b7AdW'%./]/LPmnFiUn&0v.uu:N'l8k$fbkL8j4ks/kh2vu"
        "RoB*+j:U'j]8ehLx']uOA<$)tu<^;-_<si-/WC'oJMo`7rXftYg_T:v5Bm3+gZ<t=,XtxuuDk$0Fa%bGkxDEMq,-Y>Pp<m$LLCb77SPM9ChQC>8Q9>#>,^5LN?6pe%X9>#f-swM0)Y-v"
        "j<U:#uq`]+cX?gLaBebsj]$s$o8R&#n_,u9LS03(+'5>#._(I6HNL]7'-<5&Xh0Z-05R6LJIEu$pCMi0;'M*Rv0j8.RXUQRVO)B>].`:m#k;q/h>a.#rIGM9's%'.P?1tLe&bi0G:sD["
        "U$&#,'jwxuioVn*%@Cra?G82')b^5gP]6.v$cc8.%cL+rhlZe-62QV>$nS:v&l;PJ**we0j%/2h-cMa-3t<tU7.7w^M=s'M=U(k$;vblL0xkuH)$9>#0jp,.oH5tL<1ae39IwQNHv47*"
        "*i0-MaY260-QSM''WW/vm@*=(LI2:^23DhLhr5<-X/'G.xML]7ssgs-,DFZ%^(x[s3<We)%aDctE:Nf$;Rjb2Rqn`tH>P:vwAPfLT6%&MP`3r$xn%$b6G_W$D[uKYUB5qLjMZDMZCIAM"
        "Vi*]b<EL[-cd+9Co&rOfhe5p/wUtxuP?I3kaLlS7Cq#F:H0v/1_0mnG^#I,=p/eCs54p]t1?X$t@7hR%f`LeUK:b<s>,&dt`55>#SLjDP7(NY$d^3[9,uB-MLEHYYxe_(&nUp#vdgK]7"
        "Fq9>#jvjHQk:x]A]GASoK9M#v:[)-vcC@(M<QN.L$308o3F#W-m.1I$e[d0#_`@Mh6+p%u&1]Y-9wx9)rstoLVKt'MfU;;$$'.,j4G:-/ah7b%R7nKPb#jhL+.w8H[g*j1b8@Paa&$XR"
        "YtLv$E0<q-=+3I$4<O$Mb3oiLwD`c)]ZZ&#fdu9)/pm)M_kU97Ax&Z-`Q*B-ZVBg-7B.L,J1='*.'hA,<_hD.vc%+T5^V;?1SH/rx8*/v,I[20fx]uP>RPdh&$(f$*,wqL=OHuYowT:v"
        "lFFZ%aVI7/>0(+MX%VpR=J;Ftj>D:S(9K:HTM7(4I(p%u2$?D-,_8>->$KO&YKj$5F`fSQ:-5>Yt]nuuBCFqa4coiB_c4/($;.6//J3&v[.J+M)rwpI8^T/)%ePW-]Zk'&P13'MfxS[Y"
        "pw9>#fo>mL/L4YHfA),se3S>-M']u-=3XoLZ7k'4&-T1pf(KgL<qO4oB9<s%wUtxu6bj638Yp+_(fhq)i5i*M9FL1pAXT:vAJ40MqBhLpvh?2M+uO4oQv):/h-9^O6'#)Mq%g+M[(P4o"
        "N'5>#[X##vmnOwBR)XjLnNi#tD)13(RBR:vd'F0c96f#McYXL^k85uulKDuuuF*<LR&1)0V`gCsFSVM0wXo]%@dlmq2pq_$Pf./L@(Hw)kP9Ese?bxsRZBLpAuODsW/Wl$#_^v)u&I3%"
        "lG&lOh^vEibKWstTh><-l>,-2*)pkta7`iim4ik8>RIf$,Tj7MxJ=OJ_k;>#kV8aG09sZ^iNA?/$`)-v_p;wLNU#ZenX,]kWcPW-J#i'&:B0DLUQrIhi*P:vqqal8UImEI_2vM-^Lk)9"
        ",Y6$vV:;Y7fdE^,>75>#[%/C&m9ZsaMDnrS>/HK)oS9>#W0g'&M,c1#*`jc6$mGoLPq$T'q2(W-m(X<&1(Z<-WgNr$5`D8A6vA)Npdu-9T(I7rM0t#Nj1vW-%G]e62?6;[$&K@'OxQFO"
        ">tcb)S0Fnn*f7C3[5APsC8S7@#=$Ju^/-?*H51?/AWrH=uQBkHbjQc6loF[9N;G]8'd&+i3Yvk0v&?q.<*/F`f)[V)u$EOSf&RV)Z>SvLF$(`P#WcRID(4:^G?Phs@fC_txMaDs/+=mB"
        "`Bb,MIXrn%=_XR%4lj-$3.+qL`rXDs8_-Z$q+?-.H3g;GV2OOq,(5>#x,2&v6VF@Ll[$Mh8gW4ox4kn/ip)i%5L&wQg%rP/:ML?]j#R9Gs[+NhVEluuMW8%Mnh#`4qt(Yl'^d9/5)Guu"
        "wc<wL,0ZZc?PPA>(9'<-Na/40.SwRn?U^C-8=n]-ho2O4)QC(&<RrEHuW1^Y(7>^%;5XU%)dFgLE_KM'^bBK-@i^g-'AP9ievGqL6hI.2mOq>mVp3vu4ID$MN56Kf%G(*vi.YiaebF59"
        "O5a&#^Nl34KrqmLE]A5'ig+5p9dPW-Q;K9iF<VuLAk:w%RRJT.kw9>#eu4G-=j:l-Y%OUMTbPs.$DWOG6PO&#Uc;AG[Zkxu7']u-d3_-MHMR]-<3S<_9p4ias78vu;&7+MsFa5g9oPY#"
        "]f%-v^qeBMo5r#ZVrvreoobZ-C_j'&/-A/#m-U(aPxLe$4>KtC.3ZrY-^R:vTODHM9(c7'liDDMB9Nr=x0LG)j+P:v/NJn<?*?l7Yv3cio#001]x:x)c9>$ElA')3B7g_/oKDuuLBK:L"
        "V7=p8cDg;-Ye<1=(B/s7[>YD4V&cn*sM$em8fl052'.hYc$t(PhS58oq6J'.[v2tLZrgea)3@$tr:;p)xIq_E2%D$#Xlf9&:I_#$Dr+wLjHSFO?Ph.v1>[t(kQLu(_%mu#dT-C=l3>x8"
        "Hl#u#bkW20;A[osA]<.deoGDI=K(L.:FS]7GuX)3W,u]tZYx.CG3`&vE@^saIDVAHqQb]YR[W+0l,7&vwx`da&p]#'xpJW-<k4hlBDFZ%6^rt,ir@x$<TA5<'U6O.xQT:vqaTRG%K[7n"
        "(CQufGGYr$Fooxu2cl@0`(Ax$Txr-LAl4[B5$i`71r+_S,TBuuD])oa8Q;>#-W4jV)$'Ohgkfj-:iVe$<S:X:eBXoLURFjLPCsUm*lO&#rg(@..`5<-kDpK1OX/k7@Yoi'_wt0voF*=("
        "_iQt14b=O0kQ+@U#qZS%CM`D+@d?^,o1DgLbk_O1f*T1pC(R2v*pl0>ehw&MAA1+(&kRfLf:Q:vwIC@-%tLt/ckNHLd1a1^HFg;-kI'_%/ei3LJmNT>Ihdp-Dp+C&(M'4+>*jB]B_S)9"
        "a:I(<+7o=%DH,d*xMI&#8v(d*m>w&#^ikJW$Pg;-BqchL#R*vUUZ:>#^%(V79?kM(1I8t^0iPW-hsF$TC'j*.xuV#p(P,W-dX$_]W2e?p[b^9MNcJra]#9lo'0&4`2[S7nvXtW_oLg1L"
        "g4K#v6Wl3vAH_3XntS]7f$uv-,2bsM&;Q:v)?M.M&D[=lI6Hc%PWE/%%toGjXaHP)k4aEMW,qRf&.)w]<pIiL&^S7ntJjb>+xZ=l[Sfb-ETWe$<N<wLw)NEMnQ'(M[:W:mg'$#vT[s9)"
        "Ee0*MY)$DMiaKCMh(NEMPB1FCis6N9@)Gr$gob2g4Yf[5/,Tas]mf9&E%3W@N3lb3;rx`tj9-d)v<]U.Ks6t)t'&u-PFmf93/scs3qco)T6w%v0&^0#oOf_4?#:c`h;Jo)oP9EsqX>Fs"
        "bqU+v<KZ0YS-Z0Yg3AI$>;Jra)3b(NUPFf0>EdDtN*P:v^)Nb$a'EVnJ9D'#9#FIJ/pC?>GNvV?d*C'#VapqO;M2,<#Jg;-'Vni$)0N,#DCXA>ukOJ(s%ZDscA$vuM3(m7alNe^AqZS%"
        "+B=?///'-vw%:%MH@k(VWHPY#<w5X1j`JS7.,?&MOL>gLa#MS.2?0j(f.AB,/AW:WH&kk=qHT9VBNa/MjW,W-a^Z^$k:tx+X(f+v7H9j(6t4G-MLs@>UgIJjln3vuWwo.L+8UwuBp=g7"
        "aNRV6Tf@+vk>P4#OhlQaWlg6EXf;U2$awxu)BpQaGS@&,V9KqD`<.[Y29cv5pNlIqh'qxu.jXZ-xbJh5gOPdh-3T:vN'se$-SwRn,P/Dt)aJs]NOQkICET*v9oVq20=utL4d@.M&;N`)"
        ".a:d*(Y_=3'[enY_SE48Hupo.,0v`stH1gLL(GGOwrKVngvQ&#3#7A-)US70ED82L9aU:v85>[0qC/k7H5WP&odme$'s(YlL?%Z$k)Pp.9bGxklQQe$9S4I6Z1Kra,Y1fqC5ie$[=i8."
        "pv1^H+)1B>3AE1v8>Qk=[MtjLB%Ni-w`sE[ag8>#'+][D7Pn<:f3hdaU@KoLPJg;-+N_lL`e#X$iRS]7mYXc$/#%<?/Tkm'@*v/v2Qa*.(]t,M?o5<-CR@q.2At$Vc<VEnCp`daOi[+M"
        "UQ02'34lxusF#W-.;1I$6evvt*Rn:LXe'chE2<n3X)XS%a`MD%].HV>&Q$&Jhr`sYr.[qMEgKX>u,#SEvh+3h-T9>#F,7&vtBv?97_?iL^lS7nGMc#LawB-Ms=a*MZ)df(.;/9(1.29."
        "jh]=uJ>_/)P`/bR7TgJ)_e,^#4QgJ):X_>$^u]#v9Iqc7<4,nAbWfD4vF#W-:7T3X_$5rLe_Rj:D5$S`EP5faDYpc$^nL;/`L?uuMs0pLZ?N_=v18DtBqCl-9%=e?N8j_7VZ7fhk?R:v"
        "Xog<(InRM0@Cp3v+*tla8b4/(pjYW-nL6I$?v19L)TC;?1OSM'DD#W-P)H['#Ec;-pmixG&)@['=9M`7i:,C-+B-RS'.LJ,LvV;&Xm/.vBwdS%N=wk09<IP+u7BO+7'7O+xf]P+DP=O+"
        "%5l<Jb1r@-lp%>-No*C&K%EWsF+,:&g-;e$%#<Gs8dcd$16quZk]=)N''Y_3XCKE3mXNh^LDcKs^?8l/q:RLsurq+;fsx1/gtM/1kei-cT8OL^C%OC476$n/kGP:v18GQs2h/k$PEW,M"
        "_<'5#gcsm)4Z(@-Me7u-wQt_P$QGrHrWDJ`rE;8SOW[;%1uOa3ePCbVE5i;&B(pf)p[KY=?SoPs>j/[5bKMXP.<W_Ocm'r])HP.qK(5>#ZU0&v*>'xLWF$-N+>5>#l/-C&&Xt4#:DV:v"
        "`*OfLZQ-a7UmnFif[rX-Vxe34g`IsL63M-MUWbYdZn@=B5%:h,UU#ea2*V?Yw7nofdM3^-x'f'&2=1>LM_BoeSs]?0HU@I]gR^`3V14I$,]?kFZkFTS#XM&lM=7>#s#uHH,LmiLPmEl("
        "L-%XEf$)#vZpVwL@Ua2kNh5>#Q0$C8%nC_7:tkf(os>`-*H;nNP0-tL/f5lRop2)kFd>'fF>^xLO0bW>'Nc;-V+P4%f_l%lUPs+jika*.SHTt1pn(/M:b.ukX.Ls%I/[]-4_C']A,h2#"
        "#qLJirV?]/)ZTj)HdS8m[[]a+QVT:vX+vHH,L#l7_5ev-_O5x^C0+W-'96b@+;[]GKvU(&GsOm'uZC?M.tgCM*^RumYr%,s>e,`-Q#Ln<nbs[7U[Nq%wRx&4#LlaT.nf9&6_A)XZJCpL"
        "nZ4(PhapI(vA3jLpa=(PaUmu#2JjO$056)%$lp*D.rVGsA#]i)@3etp6GU.v4)8vL.LF>&6NI#vlmpS^'Gwf5*16k%Xxm4MNi+,+'[(L5-#h;-cV_@-Kd`%%9UoY6792N=?C;=-6glg("
        "gEp63MAP:v#VkxuO5x=&<>[*RCi+uY`Mc6WK)IMhtgC1.O,RUM7[e5JAIO]uMcil%*Z*m^Bpw&?;iDY$v-jtGu?Annou9>#<M4&v;Le;-Yf9[%pn%&M^uoUJU#g,3]A:5vOtP)3B#nqO"
        "xV]xt-_6s$PX5dJi39kM,eY7eil;>#Fquxum'5kXYbGe6'$Ks-)[=jMOINj9#&###bvFc-0=:XCO)tjL:LZmqvuxW=&Vu'&d-3qipUh*.h4FA>Up@cMCGD#;x<@Q0_LF,v#Jg;-.[KnC"
        "cWjJ2c$hK-(J)U15wKtlr*x[sHfO0f+On*%])f2%`@U:vG5>e$Ha<T>WLpxulEoRMUs/fh3tA,v3:tiC-&=#v23>:F$Pg;-06.I-=j@=C&qEwT#^m+s=b-kbSl&GM1A?N'<kLL9Wn`cs"
        "a0,Rhg*p;/^lM7#c*ChLa99>,L'Q&#S-OR/:qp3vF9Vga/sU6/0FA(]P,@G;0$(B>-09>#CuCp.6Y(-vq[4I68X8q`+)<g1+8Yqtkf5>#7]@jDla?iL4M8rmRV<v-,W#5puFF&#Vw(9/"
        "N*P:v2?xu-uF*%D#qJe$iMKb.Bd`daBVd`*C5ie$'f;RNqG`]-]*l<CC2>4IW_G'v0<R51#hW4o:'+d3dqpxuJXY8.xGJ0#S4w=-._BO.>%/k7Crh;-7u0LM-whd-,Ja=?`Uu'&WM`-?"
        "W$#)Mt1lOoM;V*v`FPRC.CsUm57P&#Gf8,M#7Ge%%dY8I&dk&#f%XiB).5BGHudJV<KY)M4b[`*vOkxu3'[&/T?Cra.(#f$.u/k7ApIc%_w9>#r-l8M[A6>#$<8mUJ%+b+9?7>#iR?uu"
        "Pa;b7n@XA>voP^ta:v?\?_78dspY%rQ;#6_@fOg*u4Zc2_^@7I`(#>uuDR;p&[_.<-s]Mx$4>:>#aR$-vb2h;-4caf%ZAE-CbZV8&9__#vt^g>RqR]tY9Lg;-W?tY$,]9O+$A=gL/3A(s"
        "9[V8&46ixu%>TSSfdhe$Wtkf(6s?X:c^)&+xMJc%&]txuJ3h-6M'-tL%N3b.,r`]+O*j2/B*q3vL#4[9?93jLEl9xtJNgk0($*i%e*0.va*<#bs<$FIAD3O4oYI+M-^=%t#@R:v-F;=-"
        ",0sYM5SadaJMZh,P7niL(b/mKH`vB8hr)L>v(ofLAM=%ts7-aN(Q=-vt6u3+T.Kra+(#%Ml7W*MBQM=-BqS7.Ft;dMQt/Z$p5dID875b@46#=1mPIFM-PtW>7QT:v65CwT^j$/LAaP-L"
        "X:P)kxS9>#[@*kb+G',#KThC*a:>I:=mHLp,H9Esc&p0MgV[%+3I1btgk%.L;JRN'/E*.:+XiX-Y/pG3R#2:)5p>nt$s%>-p<x:.L5TkS_]]&GEk&Ooj$Ou$g)V*M?t<A+Sd#Ti6KihL"
        "pt5<-?K6p-`#0I$uxBv^K-DIMCc%$QrdpC-An9sI$[Bh5C=C,MY46>#w[)snju_(N.LpC/d:3dOYrn+MI9MIM?P4%Qq4t;M2+r]2X+hk/v9Kq)NT_n@#&###VIST%x8ifs;6+j)ORVO]"
        "bG?sOqvNkS(_ru#ql;x))]nH@<d.qb+Fq`sj<Zw)8x0Q76f?:Z;PHJ73.HOo0NxWsh^uk]NqW[sELtk]gdB%4@.RX>UJ=4].QTasCDZVsVRKhTY)9(/iS2&v.)T)Rwk]FMO0gfL0_q@O"
        ")maehe/G>#K&`kOoMC6U%F1`N$#-EO;wIu-cF$%MkwptYltRFM)v]+Mw<<jMWrJfL+kk-v]W$#QQ=*W-/G2U2$@AgQV:=,M/E<?>*,vq$Mflh:ts-cr-xkxu?IoRM=R>-MZYXKM4%g+M"
        "F7_2%cNdxu`'kfMEmKmKv:+%++LK$.uLiiL/cW?PNA_LM7?9(/0DeH%O.WtC6f*73f;`n@$7*uLVFNehKCX(M)2G@Mw/Cl8deVW%J).`*%HJQ([S3:MG/2PM%K-X.:FBra_aAcMkrXDs"
        "N$H&#U##<-YCMx.=OBra8rfe$P%.B6^12#Qk<9C-9I4KM=`qlLlFGL%U4<l&l1B;-OhOoMLVn=.GkfAQb_]20ivK]7<F7`tA.V?^8dIra$JV.hsV6,v:FS]7YE%`Z(P(`j(8(rLr'#GM"
        "LH@bNs5fjLAM_lL]IwmL14)=-M&bB-%t&gL9M6>#(xL`EBvbeYcaQ&#Vfb$/cjCra%l]e$vY9>#R>NfLWS.C>dJg;-n;)=-=lRfMfhEB-U+@A-*X/GMFhGhLGHfjLTV$2M>i2vu#tanL"
        "JcRjNZDSrL%uWF.Q>XrEN3hNkk3C0l_;bHdaedX%3ocRaDrch0KdV:v`2mP_AQI&G=Dqm9cObxu8]h;-cZPh%:K6k7'A8>#'`vwMWos(vB1/k7>j[fOk1tB-xINjMI7Q:vei8&G_q$q$"
        "?+rm_>O,hLa/tCO/mbkLQ<)pLg;DPM,3`iNVAnJXcOalOjEjKYGl_w^B;#ed(HoGaWQ`qO#9'C-?&suMP@=R-rijNMbqsfL8ElHO-gXkLL)gU`/kDN'M&=bsAWmERMUqSs/#=rO?]TlO"
        "U*&w#@Y'2'5KP6&Qiuuu^Wx%#,2PV-&),##Q(@S#drQK#t]4Q#Yqap#wSe<$EofA$7NBu$eKK:%C]Om%lf]&&Lp3a&t2a)'+SsL',SS&(U]oA(M-MK(A4WP&%KE/v>%/k70vL?>Dv9>#"
        "#`vwMJ:Q:vk`qd%(IT]7?3b(N<<tW>P^RZ$N*P:vN:q-N*JO09O9r.5``OkL9RCraivmxujJ)a-KW+f$?PoxuA*PsLR*6b%*i;2L8?eq2qkwkOegW5'37c&#B)Y[MhwSd%6t]4=Mr_pO"
        "WFfXB&rJfLcnuDS7NGMY0Nb(M.aJjOAG`%FodI?7(4$a$WABaN(2=aE8=Eic`Q2nanVRFM[Ba?>$;BgLv+nMt>;<5vv:_kaRi-Q&Cae_EH&4`E-iR=-Jj*c'oT'BulGT&t.Ar*%bfC$u"
        "nB0+D'L81u_N,bssdpFD;K+Q&S9(@-gbTg-5/mF<DH6seQ5[c;FeG&#>(je*+2-KMgRA$M<rBQ&,4W?-L=V:viPaB%b(K,j9?7>#7;^;-b%k:0;FcO'?A:OF'q5rY'OqX-tr,c*&$9d*"
        "%39pAXWc#5gjIJ1UsL:`oZn3S0dD0tH7pkt2J]ptdp8w2=FG/v+[kk7oBH%bf>`sO]&#Is_8XRsIpGZ$@a.[%n6dK/kB8700Ps'U4so(2,hPe$8T2&vDh[EL?S5h0pn9s.M4,#vO9+'M"
        ";ac8.]VFKh]PTq)d,A>,H`./M6;9>,/$580^-0k7?Wii0GO6l9I_ZB]NwE&@^@Q:vN@NfLXN9g-%Fs'&N.R-H3?XCL^ujg;eL*W%lZw6%WvL]7D%]C]>7k+H=6I0PY;9asS?0c;TA)Zs"
        "/AoO9,kf&#<JOfLVp*Zsqd$lbX=o/LO_./1FSrKZ.kCZYfoZ1%U;tjL2fGKhq<3'%^,f/&?52d*5rIu-V/5u*mj^?-GIc22UZ5U2_qq9DjF)bRR[J(MhIpu,G6V(&p2H['f=0dku5`B>"
        "N`Nt.o@6.vs3&O=aKR_8Nf7REuiFwKSMc^%:$G:t.Wpkt>b+qt37n36EmS*uG?FatxOja*5:va*M2-u-Mux=>EY2Gs.E6Gs?Ucxu.`4at9BR:v],``*@B'X-RBmq)Hdj=&jnMFO2JF-="
        "eQCraw2vO9E'5l9*>tS&d%^2_+YT:vW$<M-*=6L-M25k-BHlKYwMUFLvhR:v/4F3.T%.tL<J[WE9%;Db,rO&#>%rP/[?_>LWaBoeXNZ&#^N<3txdC*eV_gS0bd$Q9^h(eZ-66;LjT-iL"
        "+Hju5Sj]G3Yr3@08oIIEUBQ&#Ue>[.+UY>Ln07i.V[)kObsZ]/M`A2L[IBD3xSexuVCT&#tRW>.i$]'#b%#N_icAQ&,0f&%Li&n-*rKu#`59nLmeuR.<K+Q&L65Q#]9],=irYcaa)IH2"
        "6oZS%L`%/vn]`dan4'`S]a5<-eSM=-`iW/1h+j3vEhd0#;u_=LXa5<-MHPS.Dw1ehl4Dt-qbre8:7WgrtvZ.v11KraJ@,+%faH].)RGgLTe@mOiKC;$xK'>>3uV*IpVja*mi7]-uLb@>"
        "5fjV7i9;k5@i:>#RHdHd6[E-X>Jn<&590u*6[x0:Y.E(=CGn<&Pb0&v=p$R9mTkxu32V@.oIka*SN<hLUEar9s%<*MDaxkrDDar9DqFr*NvUn/lAP:v=ZwE%#4aq+3`[DC$`'#v<X3'M"
        "h@a*Mj+x%+M_6.vc+EILBWSq)4;:>#pHvPU*cwi$:JwmL@n;73df`C=a^kV7'8gC=5b@2L<-/`?4_YgLZs^EY5f0&v.v^EYVb0&v:io(']+Bn*KCU,MK9a?>l5IEtN'5>#FY>CL&4C/v"
        "^&dD<j(B:Bw/f,vE/_s.YNT]7/(#f$V7Kra/M_q^<wch%]p&TK[Xk,Mw7H*O7TCQ=W=U0G'n+E<5NRQ0>U_V$)o-WMWl&gL+9]0#qZE]$pVja*5rqjL.'DIT1G]fiVpwJ&52:o8ENwY-"
        "w&?o83nXU&X)lXsWiJf$/,>>#1LJM'8);d*vlKe$,mZw'XvtF`$o^>$*cF&#=LcA#7xs-$8%&/1SkZY#Ner*>TFJ=BtI_`*p3Lk+E)P:v=kL(&nVXk4,I=G2s)u:2kON:2Ux@r2B@,HE"
        "jjf.#G*DIMW[;;$`SU_&PD>>#?J)@'WR$##m]Q(#aC,+#t([0#ZWM4#9VL7#jTI@#l8$C#F]/E#YpPN#di+u#F3QtLB6Twus^ewuBjwwu0v3xuvnRfL'.#9#TaB#v#&h#vS@gkLVU$lL"
        "ib6lLT@4%vV*E%vipYlLARb%vm*uoL^Vs(vxk?x-]JZ&M5K=jLHRF%vv6W%vk<a%v+H)=-N6)=-B6)=-Pe2q/TwX6#LT0#v13'C-AZ`=-:;X7/QBj%v/7+gLNHl$M>=/#M(h)7#Z:S5#"
        ">vW9#$=dD-Ber=-]gr=-Lgr=-^54V.qt2%v&KwA-K_nI-kKwA-5-:w-sv7nLqDW(v1S0K-wwQx-sOqPML<F%v(g[I-Un+G-_Rqw-St[fL*7Z6#(f'S-e$kB-;B;=-(7r1.66UhL.OvwL"
        "<Wo0#N<T2#'$suLkFD/#s@Rm/E7w0#I+e0#p5T;-')m<-vM#<--A;=-(/r5/dNp2#1I)uLC+jxLn=05#WrhxLvOB5#(mr4#,bpC-f[lO-T]lO-3PD&.b3TkLag[$vZ7`P-<1&F-p0&F-"
        "p0&F-WT=_.];5'va0&F-K66X06m((ver1(vfcFoL.sPoLM>N(v%1&F-9&?D-bclO-(xQx-8LTp7[c,8f%aDMh#Ndlg#aVMhAg$XLXw`'/K&*XL;K)XLpuv(3$Qh?Kp8;A4RI+XLJx&XL"
        "i6j9;/&&XLIu&XL;J&XL,t(XL,t(XLIu&XL,t(XLAV.>>_5LY>2bfu>/=e>?Imm?B9/o?Bm_o^o0,0XCnZJ'SBxC>#H*$/i5^,g)U>D&,f=Ed%_qEM0+W#,20#v(35>V`3;cR]4A1OY5"
        "EI0;69'h+Ms*p+MHw/;ONd6F%ah^Afix:T.H11Q#CCGdMuwSfLEv0/1D(Q]F5cE>5?T+)<&](DjWG]o[hY3VZ//``3:Yl-$X>wf(>G3Se2txRnF17MTUTXP&5RPA+LS`s-0TQC#wcv_#"
        "w]Aa#ai)`#<;7[#'^m_#?$4A#1Bk]#2d8KMc`OkLPD%_#,v8kL?P6LMQ7_=#2k0'#2N4.#xd?iLqX1:#^KvtL'DM'#d6s<#aOqhL2I<,##As'MowY.#17a'Mf[^vLAw(hLO;)pL7E`hL"
        "K-;hL2'2hLB2oiLs'Y)#W'Y6#W4:/#4r?0#0NF_/3-_w'RKp92Q(#&+<g;MB9<2F%/KOJ:=tUN:JP>f_`;`Y5xYu]5dJ=s%SHwM1mYmQWoa38.?8$,2$XKwKxPFM9LlvRn0xTkODjf=#"
        "5(vhL0.:kLL6KnLwL.C#CvgK-NFj6.Y=q-M`0$>Yh,M2hDf?`aUUG_&:p^%M'080#AIX&#$x+F%,ltgLXqugLN&S(#V`)<#QYlS.wJe8#K2`]0AYl##qwn%#><[qLw=2p.hAhi'-$M-Z"
        "/Z[p0&Lu(N,Y1`jCd8`jl0qxub?%q.AaN1#@hBW/e]%d)3`h.$&r'j:[m3N1*H2;608)s@J0oM(GcFp/vcS9i3fK#$O`CH-'5T;-V@iP/IurOoYUV`<]_->mbNx4pJkDW/b4*U/:L25/"
        "wpai0U?q.$DZHwKvpWf:1xS,MG*_#$XZu'$R=-C#d.@<$H,>>#RT'B#I7$C#nl*]#AZI%#7^H.M_HVcDoP'.$TCMS.bHw+;dGGJ1IsZ`*^`05/pV1>>^#VY5if'##" };

    return glacial_indifference_base_85;
}

static const char* get_glacial_indifference_bold()
{
    static const char glacial_indifference_bold_base_85[27295 + 1] =
    {
        "7])#######^iMT)'/###[H7+>)322'Ql#v#RxMT9k=uWS*b$##>t*##c]nT9P^ZbdxH)##N<###v5:t8EEvOoG:$##/&bw'dr+h<NmC9]m`###d0H50ji/e=tDw2v'Nm--i.l--x*re="
    "M9F@&Ee&##K'^w'7vA0FL-=;GrU$##fC)##RF=UCVnF,`InRk+P=r-$f'TqLN5NJqW]q92xlE'#<_[FH6`5C*pB(tL]$e.q0iR/G^+)oLR2wx4-X2w$qmnUC&+Mj*LdbKVo#8M)@C=GH"
    "sBx(3<lOk=g7?>#O^7C#jBsmVh3ts'alVP&M5DX(wWl/]d7^F%R.Jlu_Ap.$9+,##-/5##0UQC#2NAM^)8###9k(VZ`2&#P<q:eMD.`5/0NN`<c)O$MbwvY#VB(7#*5YY#5+loL#QGkM"
    "ZC+5A_^]S%*V/GM3o8gLxD.q0j*32<dO5;-cH5MpSR[>d-ug@tGk(B#`m_=MhUZY#5qbf(`t+xKNi;SI-rtA#,elR*MM:;$jnbR*ni08.U9k_&d^Z`*V=n_&YR/2'63p_&fdZ`*wCi_&"
    "L%u<6,Vn_&v8<A+x:vu#1:<p%_#+vH9___&q:dl/v;`R*QrQS%S8q_&;P>M9c`h_&&7=G2$Ql_&gs;A+p3o_&8l4R*d^p_&DUpVI::j_&>$U`30Zbi9F,w1BEYOcD?K`dE7n;J:KMs.C"
    "?wTjCD>8JCqQn-$Q11L,DGo3+xq`uG,c@5B=^E>H>CA5BRHlMCH]i34#D1DEHai`FmKl>-4[Iq;dS%44o=H]F(3wf;JnxlB#0sc<i;rfD]Q*B48HL=Bi&kjE`vZ:CjQ.UCxm@uB0eo*H"
    "9#I>Hl6rTC%m>PE9Q.F-T.;eG'h*G-R#fuB0U#oL5*[kEj<BgRU5$##C9',.wm-qL(K<lEUsJg2,a>G2W5,h2_+.>>qneu>Rqe'&uX-q7GleM1Tn.h28<eh2VQf'?fpKnj.ouM1[qU_["
    "/q`t/40f-#_/:/#Uj*N.]:F&#Fm@'#pkn+H5-xF-T[[m198FGT(]WmLx.v.#i.wA/I<x-#-#krL=-;'#2_jjL)c@(#'Rw*.]p/kLIv4Q/=qAJ1V<T5Bj07L#krv&-fH<mLl.Z.#CH4.#"
    "@'BkL?f=rL=D`'#'40sLK-lrL2LG&#mvAV->H5s-9/'sL/x.qLQnsmL'skRMPIgnLmX+rLfDK88nR+j1-`l8&SV;e?/#>&Oeg>oL,/AqL=@1sL84:SM=If>-g_nI-922X-ECDqMkJ4ZH"
    "KMs.C$i]>-^vUJDJo>a=dY9G;^;oG<Cmkw0C[_%OvAe*P3&1kLp1MB)Lv4-vM%(GI)H8_8s=+L5$4P3XA?G_&WI'XLsO9MBHOpcEOSK;Ir&?D-(^5TN%)B-#9P/N-kj?x-<8>&8be@5B"
    "dJncEMo<WA0fipLY6L*#:bbOM<RD/#h`($#[54R-[H@2.e,KkLiK;/#N)1/#>)m<-6cpC-n`+p7f6i`FBt9/D4dWG<S[cJ;Z5X8&L6nM192bV-C(HpKKsQ+#9=fnL[>L*#lO>+#')MT."
    "d1g*#vRM=-uJ,W-Wlg8Kw8hh$XM7A4Aa2Q9@v;MBqZ%ZHb0Ne$P;@X(Rm>G2S/u`4.W-9&qO2A=xAms7^a)58Pke'&c@d;-v>Z-.'Z<rL[mP.#j`639^jN>HWQbg$9-c;.lj9'#KUi;-"
    ":i?2M^gpC-6cpC-=H7u/VH4.#[;L/#<bDE-o$%#%'%EYG;RO2C8:Pk+=dv9)'8f+MVKZA8EB.&G[acMCa/Uh$c_&mB(DocElv^PBA^e]G&)&gDMSqw-B>8[82I.&G74(aFU9r?9sh%;H"
    "l6M21@[nqM+%exF6*i`F[cF>HtC&Rjlm#mBDnXMC;.l`FK_Z8&rPviC@Uh]G3?Z>-<]wiCcaT5B.X`'/9,J)Fp)G_&H`%@'G.3q`.wtWUgcIk4mQ5'o)2KqrWht,F;kbRM4b4rLj-lrL"
    "2LO3Ns_]C%6uo9``Bep'0Qp4Mh#YrL[)8qL90l?-S97I-G5)=-E5)=-1DsM-+C2&.-50sL14q`$1Wi+M+X`/#upTK-d3f/M[/lrLW3oiL')B-#RA;=-jV3B-L4)=-=+#O-CgG<-<0&'."
    "/5[qLH^A%9vC[k=dmkw0(f-AFh%g]GoY.gD<@wpBX3Qx8<T6L,l_c,M?[;Y$[4'dE+Bjb$$A-QLx=SC']-n>RJa4?-Bc/N-lrO8M0'1kLJ'crL(MM/#A=fVNoqY.#F/m<-ZWbk%*w)eZ"
    "#%pQWb7q>R=wXrLp.K-#3&wiM#+2e$nf(_S?M%L>3rk?B6d)RWqio+DQ8X>HYl&qr8T<Qh9_4rLR@LSMff=rLSh]j-(Sm--`;3)F3g.dEQxi34<u`f1AS5&>iYX8&91es-Eq/kLD@]qL"
    "ts%qL7,@YNRrkRMtx.qLH:2/#scEB-6V)>M2(8qL1P-Q%n/6mBHOY'8P%co7Q'qWhnMs.CG5DQ(]@g-#Px.>-QF@FNbn)$##:Wr7SGE/2l3m]GhGdvnt@6L-,]xG(R*]<-[mt4%:]2q`"
    ":2AkXP;@X(F`]KN0XPgLC^Gl$6l:p7$5kYHIH(=%^Q<p7UjGjD7`rq%:+YDFu?3GRQL;/#Y`($#UsEj.U0f-#?6T;-J>DX-58u>@XCqP%?,8R*=%g+M%TxqLJO#t9);A5B.QI-mA;1<-"
    "9v]7.<s&SMnnYl$=f1'OYDOg$.FAVHwd?XCgJ2R35ak;-]fe+%U:%=-^_Xv-uZhpL:xc.#G:+5:?2Y?I#KG&#xIKC->CGO-JU)>M84oiLvkFrLUsP.#)NOgL8g=rL?d(AMTr%qL*$p@9"
    "w=v&6@3urL#A]qL9c#=9fqK99CWQlNH@]qLt&iH-T,l58E$4#HD-G[$*b%29^5=J:L:m-$sLm-$tte'&YsF'SRXTq)Qe.W-A7*XAbP^88YB'jCB#%v8FuN>HKV4o$P`8R*#$PWfaLp-#"
    "&HwqLv;ts7Zj_,bpGG&#^Q':.v*^*#$0k;-Dh(P-?d&V--vK#0:'Fs-+#[QMQgipLFlP.#T+-j;w5.K<,fipL93oiL>3urLm_gw8A^pfDNb@<-<ZlO-(29$.*n-qLkI_w-5)trL%/]QM"
    "g0:S-OT2E-Afr=-K#>G-_.NK:TERW]c*8,(qb%;HE_(aFi;MvRbLR/;K3#KN$D9@%K#FP8Rt<#Hm;I-Z9F-XC=1I<-D>mtM_1$L-_R[I-X54R-BM#<-^?:@-q>EU-E`BM;gr*#HDUA5B"
    "dX],X^Nbg$3kEs-mBonLEY5.#CT]F-a.%I-v,g4(Il4?7DScD%S%f?Kvt`/:sO.kror-A-8JA/.Dp+]8jA=KNkE;/#jl-qL2%qa9(^QpKpDK88$_)Fn(<QdtLCcl;JXfGnI)GQ8C_]32"
    "*$Eb$q'F5BMt>q`8ch;-Y8lc-g,Hpg7RxqLR.lrLcjr=--:-5.Fvo%8&p[PBIJh8.c;L/#dwk>-)p4L#mI*:2-$bxFb2`?R,+]4:+Ir`F0,DMNS-&$&CHEE>mGG&#-(oh:nbV8&#RfS8"
    "B:#sI+s6mBFjnKPvAKqM>Z?X(c8g;-HMM=-Y6`P-r'x+%C;c;-Y..'Of/lrL4R[3<)h(?[c`G59j&89&LV8JCJ2:X14bf'&'>PcDfDKq;I>`KsMDFO%Q`?VHb,VJDn5hDc]'crL#G+RM"
    "<+ml$6tF_&5gk;-^jtk%NZ8RE2CNe$AE=kFProR*C9g'&BC+kbx3;_8q8eWqgo65&/)k]GODu3+9BI-ZQLrM9KR.&G#Y,#HPHq>-ZnBe6]9A_/:Hqw'FUl?BLJ>J:%@4wn]R`Z-wG+e<"
    "f.2A=eJ=J:O76L#5v5D<OVrE@Dmk-$lnW_&L`S90&gYOMjY+rLbegl<LfpfDF(X>-n?^MCdw]MCx+(sIg99kt6gi+M,=^-#X4)=-K.A>-WC1X%@;Cs-**IqLgP9w%(+e>P+h=rLeFD/#"
    "IGuG-;N5s-mMOgLij0h$RV#/:1j-W]p**,+VEeYH6E_MCvB9wnH^$TM`1T-#KG=,MLrAk$.$H9pD6fu-$n-qLe/AqL:5X0;.:s3rD'crLKwP.#Z:F&#J,vU.d:L/#<a[c<_=f_6usQT-"
    "BT<C(aj.#H$Q_PBnt;Q8WFS^-3.AqLHPB,.K>G&8oj&E>;qkRM/g`Y9ij<#Hm+ZKl-RXc25KO9`/laKc/kKeO8Ej#&<[xL:sOe]Gailm80ew?9UcwfDxej2CX6]rH-x<2C.qX5BaRK&m"
    "b*&,($)tQNsQsM9,Wv&66^t1-oQ/W-2C,FYD5u<'q?^MC]hxiC6]q;-8qWe_-IG&#^A5q7=BLxT@:oH;)7h21:38G;+B:NC=sViBe;.WSh0v.#l<=k2ctc.#U3^)<%+S5BO5eWqMOV58"
    ".OG'?#[wq$+Nc3=9#(68j`c'?R^./&;vi+MBTZa&Mbn<-xMdh-kPc2V[%B-#E/F,M^)d.#7w8kLQTxqL)skRM0.lrLCE:sL(JG&#$NuT.V;L/#]qX?-d?:@-[Lx>->IVc=NXTJDBbkMC"
    "<279^(N&`8j#S5B&0i,=<kFrL#0'1=T-2)F*O3W-<u,XLdZMk+<T?X(-(n-$'[5'Q9X+rL5f;>MF75dMr)8qLI4fQMYeH5NTSTW$Soo`Fs.l8&%c0L,nfu4)U=xQ-OIVX-dl;:KJe=rL"
    "-)8qL#AQ*O;3DM'5>0#HPwF_?A9(sL_@g-#lt:V<h@k>-kY^q;4&$'6(fI(#OSW;?=upX8%xc9'F+nhMR^U&&Dbk7/&O>+#uM*rL6JJZ&AW_w'@&B']B)-F%0.exFA%-qi,%n-$/N8;-"
    "0P-<-Ur.>-C)m<-KAGA+5s^<-Dgps$.QgW-/J.4Vf-v.#$,[6%sio+D0vaX&nj#A'7s'<-<J)uM(.lrLCO.7;JGMDFWQ+<-W6MP-1g74:+V.&GG.5.+jC)N=/&X>->+r58&NV8&//@RE"
    "DGo3+4`>3t>QK:pubN082Bx_Q-oR(#3_Xc%Uva,F*&M2'g3f?P&qT.<S)b>-'.5d&g`Dq'.ZG&#TB+.#Or@3MVq<?'3^ILl,(EYGULh%F&gC3ts?iWhq7-AFo>_w'OV7'dEgrm$N%R`<"
    "?qW>How4_AbQx8.Qa($#D6ffMoqi/#A0f-#I,,&8`k@5BB6QGEO')aFtPI-ZK26eHEUj&ZHV,d$FS-=-sg^g-74ap'LFKa-c]HS1XcR$8ofK9roe)68eY#gD2XiX-_5V^6BXPgLe*8qL"
    "8Jp8'rVg;-3Nar&?QuL:3npfDF1@j9OFvE5>w1C>&4b>-5=9d).X45&[9AVHaB&;H3Cb)<askMC9$a'/tGX4iVLIu'72`h:*5,rDDe+*<8i.`A8<2C=xl0?-Pc8JCCs+.1Xo`q2[6[k;"
    "C;1liD[Xk'bj0A=So`8&+DIF))Ia783A4&>gK+9.^Vs)#c'm8@oeoPB/VM'JxLh%FD#O#H]g*AF%_:-+&d59%/uH]FF>s.CKH$gDpJug$wdg&dtdo,(2w+U.Om@-#F$jE-G1WU%@H(@'"
    "KfFxRENh*#ec/*#u$50.>I=OMBmSN-ncPW-iaU)-L)n+#$/;t-QtboLAslOMs@EY.>[P+#544V.n1g*#uLM=-a+NM-_?f>-.#T,M1WY&#PY5<-<:xu-Bo.nLm5*,#+/m<-<dq@-?eF?-"
    "#)m<->`Y-MCEOJM8uY<-@?:@-fh>%.1MOgLtAMPMohjmLm(?D-N&@A-Z3S>-(MM=-oM#<-B?x/M'ZGOM6uPoLJHh*#;i%nL1l'*#1L?,<p2wEIC,xf;Ajs34bB,_J:r_'/4:8X1&THqi"
    "T<dM:jKYkF<f=qMKSc;-0<Y@MGmGoL3v'*#=C4OMmGVPMG,TNMwYGOM3s-A-*/PG-$ni&.lHxnLGa5oL'+9nLhtPoL>b&NMYoQ+#xM#<-$.RW@P@/s7<:0p8^A1eZlZh+M%psmLa]R2."
    "87]nLBb?+#r?W(8bQfS8Ol0d<xwYe-Oa'_SLFPqM/q7&>uDJ21fk6&>(FBs7PE>X(Cuo9M8rq'/x-e;-cIKC-5:M.Mib5oL(#XB-78L1MY3oiLMOpnL@*C*#rt>%.oZ=oLoOEmL)P5OM"
    "[5RA-LfRQ-a-2uOn3oiL:NpnL30#0NdUNmLB2oiLt)9nLD4>g$X?<p7_`;,<TT_>-Wf]J;.3rM:?ri9VkeE_&i#.m9t+^,MT_sM-nL8j-)M`2M2XPgL7AeJ(x5W,MbZGOMp_lO-_ZlO-"
    "&ufN-=(c8MVO5OM1+9nLSdamL^a5oLYq0*#>-@,M1n4<8ve;,<(`me6R5)a<C9o`=H]K)=6cEW/ug#`-+mmLc<ZF#?gI(F.wk`e$wsm2DpX)..]CDmL#>a(?*ta>-Q=-2&7'WD=8FA_8"
    "5&Ok+JFuQNv)Zj)*)doLVg>oLZv^c$'X9@'w?&T.ttJ*#qDZu7JjVG<]:(dMe*=J-qt-78aDd9i)Y8WA_MgS8xRXA>Wf9HN0W0a+L9[_JQ6$XLcC4p8^<Gs7BCO3X0<]i99WZ>-FxW>-"
    "ClCm8qU?l1+NQdF>M_oD2<XG-MhW?-Ag/+49ZEJ)R_fM1*`G1FwN;9CK;a2Ds>^kE<rY1F:q2pDsZVD=)>2eG.(rE-g$fFHa2/0F?PbNN?[,:&SJWnDjTdp/lIrEH)6FVCP[Rq.mHiTC"
    "ru#F7GP]:C:WI>HeZr`=(@YG<BK3oMAWPkM[]q?-2aPW-T+IU;Yg>[947V.G'fC90rxuhFH-B5B8tDU)5:7+H5Ox&I1*l5BxB%F-GP*8Br[g,=Sgqf5)3FGH,C%12+a^oDT.xr1p4;e0"
    "F>%72er>=&t/hN_'k1HMR[R:9'K]K<2=^J;CP90<[GI7an`$@-GVdP9(uNcH;#1H-Bs]b44C8:&L^-w7D&/:9O&@UCG;@rL$@j`FK.lrLs$`6(ecbjL]F760'5FVCGA+G-7,$1F0#DEH"
    ":?,jF3P[BIad#@&tYkVC2Gd1FD#48MkHwgFk*ZWB05DEH3b%F-;BWq)HhViFvoUEH4q/C8It`.GS8OSMT@xu-6w:qLQjsLF`VIqCQ>+#H)8<7B]6rTC(.iE-xBrE-$HX7DTmiBJ5:%mB"
    "8xwbH.)qoD0os3M,g3vLD:(sLRqA5B,DFVCbsdX.sR4VCC/;t-/h:qL)HYF-=gE;IldeQD,JY$H6Rw#J+$%F-(ZcdG(2(@BI>I7M#8ggLH_p=B3WxUC0TXnD7ENj$<1USDhfgk1I>.PD"
    ">0eU9^IhjE?RF,H8H*EPPYu%MHnX7M9v1,H9(t?-:#G;C<&DT.#04nDD8,HM3ESrLBM[oDG/orLDIqD05lD5B=RshF.]8F%MU_w'HT%F-@QXG-,3J:CMra8M@?>+H&pkVCh^oiLVl1HM"
    "_&1kL@Z$kEpan05Rl0H-tf^oDT'#d$4Ka#H4GN<BsN^c.#Q]:Cl3B(/iN`TC#fo'&bPf7N'j[:Ce+XoM#/[q$3:-#H#NOp7c*=2CYCN5BV*i1.s=Q]8t'A9&qmnUCI5+3N&%3FMpeeUM"
    "E[(*%DI[LYK8s9&-)uVCKZ<SMj@Nq-wcoBA#9d<Bs&9<-l;@Y1*3AUCN2w%J3Xl`Fa/9^H2WgoLRkCcH'dT0FCpnrLJFq4N:0giF(VX)F'qwF-/+Im8T-k>-/?Y?^Dcr5B?I0%J_PF0M"
    "(+*hFqh.@95;FVCnm`9C@FhoD8&@dX7eLG-?Ul$'VOPiOM)-:&R``T9XkV*>^-je=ZrndFv--?[D>+jLA5,UCV.riL@q_L.;OiEHCv6jCPMFwKMDR3N0`Oo$[n%w@Qg&0:tWrTCZqA>0"
    "MKl5B$>%mB#oP^GA-6)F:ErqLuR&*H:l+vB.i5&>%'?A3v(/B&vA'kE-+%F-C=ofG;lQJM]pq?-&&_kEV42KaB;4/M`aAn.'>ZhFT<iP-9A4[(L^c5BmI3s7]nE#HIUZ?8Ugb$K.iopC"
    "q%ff$MfoW9B=t#H(BXVC.]eFHR?a'Avx]oDog`=BbZXO%c*Fq.$&$lEPO@kO;hCrCAfDq.)PD.G$(k9;=XD5BHQ&[8GY%?7_;0s7bcZ-HQa&9K`@]lDn8:`f@XWT.g6rTCB&942bd3nD"
    "86I>HCH>2C$;iEH`5b31/DFG-7;2*H@@bcHOIt>-7)-p8@nW>-L3)C&A@QhF93&7*9'B%JGmfAO,ada$;1Ta#W.L0M.>cQD2wLqL6KuiC)'W=B5tl3=k)E&GIXs6E22AUC]jdE33MD5B"
    "#[eBI8nE(I2SvLF.]eFHOoes7:8QhF01<M/+^,LFTt3kLZ5/F-<GIQ/2jb7D'aG1FU%$@HnP5LFijjjE+F_oDHQw7MZkSc$<42eG?-xF-p].?-.%fFHhtm58&jwVoaVhT2XF?l;]R1S:"
    "](S8/$7?1<pG)u$s(1eG6(x+H)iu</3CXJD8w:qL$<na.%n`9CN`jj$CQ.F-nYcdG<+t?-+YG)NDc,YB&Q>LFm86@&T4.I-6J)^%96an3Z(frLmc:GHMfw;1tJS21&(=CMiqQ0FYfnb%"
    ".WGq8lnw,t%-MVCbFJ59l4k>-H$%]'67V.GQW&&8J'W*IpXtGkIrq?-.?=3ENjf]8@`KjC08vhFLSmxLQ+Z:C/>ZhFI8G;&R%V?-/6sw8K5A#e4g/iF)$^j2+#5FHMA]qL`g*pDvJkjE"
    "sVrPq/Ip9&^1N88mS2U2jvo<-LbRg9wLF03t&MX/4sUq1=7F70`n'rik>vNDERSv$AIMp7=<=2Cv/P32U]q?-u?S9.aKeQDaw1'G$/vgF&-omDm*`]$%/pgFEBJR*?3Bp.4;qoDuDa*@"
    "BBRSDBXjp..,/PDQO]$07WwF-^<2Ni%YoE-APW)36Q.#Hwj[:C8luhF60I>He],#9ZX2LD+EKKF4Kd<BcjsjE>;SqL(Q/eGC*5fG$$0[BV^@'G;fV.G*pCEHCY+rLm=<LFcahw-BTnrL"
    "R[>>B)/<NC2*OG-raG'SBLl$BpA6QB$mcdG)d#u9)Oi`F/9-jC6j.o99wgNC#v^kEGX*pDWpJt9Fl#@-9>2eGCK?E3*GbeFcJg)(>e2iL46$F9Z'&9&PmnUC(aeA41rw+H<tR/Gqm*nD"
    "LZ(gG0&YVCf1?]8m#cvI%F?&%DqvgF6DM*H+?]:Cm86@&<Vq29_)9*>DQFv-dg8=9R0MDFKpP5BD$jE-[^bk%G1_oD3@g%J2.vLFS`4rLWkA5B>)FrLN$nKFS*f09DI/q`WuerLFBY70"
    "f&@UC_u%[^$i2j92RUgaA5,)%*cj2iw%$U8RO9?-x#kjE5`/_6128b-E5V0GOhViF/mXVCCtmKF24o+H(x1=/pVkVCK?`qLxdHKF_KS#IXD$X.*QP-G/YU51mcPLF1TfUCu3%F-`rw(9"
    "al,gDhq`RN&[M-M._HKF/$n63O+C^GCt7FHmNI#%2(n<-lGd*&3E:6&ecMY$D,:n8JEr`F+OmE&8<tmL@Xp9&CU=GH+$%F-l3XQ/*8qoD6(0%JkMu39:%]n3H64j9j1S$7Shn'&kc(t-"
    "er+U:56j8&kg'XJ'/`M:4-W8&2pbp7>LKk1&Wv:9Z1_0<QO$L<L2qH'0-DtB.5M*H>XvlE*]HKF89#jF%usfD4l;B%32AUCYGcD%1o;T.'5FVCtS&7.BTnrL90RQ:GZmw7Q=QH=eCa99"
    "<9.^4a[;*>SGf:9Ul*D-lWob>:N)t8OVFd=V-?w-f%TS:e0SF>;kEe-EGMx7?]bD-pY1s76CB/;+8>61q/si<b-Af=ZxwC-VIhO;xr=61t4aX8RSSQ:qfL88S(1-<3=^J;WhV*>p6@&>"
    "cxv.NsZh99I)EQ9h<QH=53;e?r`@Q:gvB`?TjdQ:GkKk=aT^0>^'uc<TU:S:I;nw7;iZF>i6se=VUus/%&PL<OP]U9-^X$/Io90<DAD'S/Ivh<`VJq9on2kLNmGoLY%r?-OaZs8I%-H="
    "kp?qV^Z)tq;1QJMl1CkL[v&$.b=[oL9Fp`=EcBO;X'A>-xp:90'4_O;uYwC-@^1q`m%=N;FPChPct'U8GYBk;Jkxv7'6C3M#DvC-nLgJ;+klO-#?#p%7U4U2+]90<HS4N;iLCi<#HU3M"
    ">f]0<H`f:9j:HxLPQLt8l,bq)S7OT8Yuo:9]q#=(LaW8&]_q21(Ma99x]nM:mM:l/rtu)>*aRC-^*D,<Rf`w9<9WI=ipK`2]fo:9XG+H=WC_0<c#XH=pF/j-jMg--FiD-metQf=7-([-"
    "Fh.F[_I1M<c'06(wc<;MftLdM[D:99.CFn<:xpiL1[1[-&Nb:_sWO`<%1G&#tUB#$>A@&,&Ye8.+Juu#Fle&,fbwRno+mD4$;d'&xJB%#B?)4#sv%5#-Kf5#9pF6#A2l6#q1PY#T$2G#"
    "hBno#Tv@=-DT=>-V5:?-x2Lv-.mX:vZLwA-uu-A-]Kex-`@r*MOa5oLorU%M]$8=-Z(v@P@sd$vq]q3vE8:3v:pb2v2W=2v.K+2vx&J1vI=i(v;.HVuVi:3lnIc2laux1lC.#Qk^;?hI"
    "6rF$KCVtA#[]4L6ZlgY#5.Is$Ew/6&UjlN'f]Rh(vO9+*0CvC+@6]],P)Cv-ar)9/qefQ0+XLk1;K3.3K>pF4[1V`5l$=#7&n#<86a`T9FSFn:VF-1<g9jI=w,Pc>1v6&@Ais>AaR.iF"
    "YAx;LLQG1PaMRfQtIB)S.=)BTBB+wUTA-:W(T:JCRKVQ^1H<tBPSM-#P3)@')d-6M%PP8.j9`rV3ljh#,O-3Eg[a1ZE5#E^4B)##b6-wMJ<P)3:+fbHTQH&JQ2.9`-x$:MXFO>MpU,.#"
    "[+2F%wWj-$vU'#vG?gkL9L%m/,`_;$&5G>#vbei*d0eY#S$x+2%p*`sXd%u,@<B#$0YK]tvJm]4(MtU-=<M#,*gW$,O+Y$,O)f$,Z^q(4h1VV$]8FW%#+v.:mm*[HKGRR2&x)7B+?fUC"
    "'3>=-;QI#/BD5N1P2J:C<k;'o]8YY#*P:;$.iqr$=:1GD6C35&:[jl&SJUjB@(V$#D4i$#H@%%#LL7%#PXI%#Te[%#Xqn%#]'+&#a3=&#e?O&#iKb&#mWt&#qd0'#upB'##'U'#'3h'#"
    "+?$(#/K6(#3WH(#7dZ(#;pm(#?&*)#C2<)#G>N)#KJa)#5f60#Sc/*#WoA*#[%T*#`1g*#d=#+#hI5+#lUG+#pbY+#tnl+#x$),#&1;,#*=M,#.I`,#2Ur,#7kIH#e*]-#Q&/5#kE24#"
    "j/:/#8.92#(-85#+Kf5#+E]5#`Po5#6Wx5#6d46#>pF6#>&Y6##2l6#H8u6#JPC7#4qr4#vnq7#Rc_7#Wu$8#VRp2#c1@8#fOn8#fIe8#-Uw8#q[*9#qh<9##uN9##+b9#M6t9#-=':#"
    "/UK:#<G^2#9b^:#9np:#SL;4#C;K2#FLg2#isH0#+#R0#r_W1#mSl##YG_/#gSq/#vj>3#S]N1#84>>#s^*2003<3'o>9U#60Yu#-2(n#&$=x#<Aq+$S^r0$NeaP$v=s@$YNLr$H^Yf$"
    "n6Is$T=oP%WV2<%7A?C%dZ$o%`eSa%^dUv%S8F*&w'/9&I`2@&j(fL&d-lU&=lM[&dvbq&?h9J'^i.5'^,.@''meH'^.]P'&5mV']c^^'uhK+(:WUv'A=?G(P6Z?(p5>f(pcNX(3Ml+)"
    "Mg/m(X$6C)Q<)<)9Cm_)EluN)^=2W)G+pi)j6B>*m7b**60W-*SB2[*3n;H*WX'j*aR#v*jW))+oJ-S+8LG7,?EZ,,1aZ4,2r/Y,gLnD,.CGn,Afq_,Gn*i,Wv7x,c3>N-?_cr-(3m_-"
    "Bt_0.:TT+.?^79.[A'j.+j,g/p$e[/h_x.0w&Ds/VaG$0xi*20S/2@0U4Je0[w2t0-3[H1>]971YNa]1ZjUO1kAWx1q]P$2m(fA2Vop32PTu72L_Z<2cbbB27+8I2<-Pn2h::+eBPeS#"
    ">]atPOEfe(o,&Fs[t]]t(Hb3fV/&+MV+J+Mkx5=lr;t_L1vlEa3v0f1ZN9ba,c%u?^^b?`WIe?`_v@d,K4XB@h-/.1iXY]OUOpmtAe'N'7s5FV32m+V00,,V*OM=-@cY=-AGctT8jiZo"
    "FcBF;s'0[kc--Ds?t1h)lvg=#A3_q$<R'7oEjtTsRQq/HcAv9)x@[C&eY0lOGU,]KwsOb%L%PvOFh<a$+AGUsbMG]O82$9oN#YeCr_j_oIWaN2^;TGswbtoV]e1Zs+C(pV3L7h%kh=Ws"
    ",ZGF23xgms*c=.);XsP%]s`vq8;%x+;1T2uMmWRs9Nr<au]>+e%pmDRAc@F2s$#0LAk@C4b8Pe%VTFKlO>i,)j20>%68.O:AGV^Op`eX,N8M%+gXj8K-:>dO,tXnj5i3O4(/>0Poc`m7"
    "TmAa%8DW#G_G9EsEG(V>=g$/lCokh0erLs(=_@%sqh[@sh*Xts_<4j)T.4V6SEVRsYW+^$Mh47e;hvo[c;I/)IUKXExnvO&;SJht)T6=]d/xY,RgRP]6O7uB;2mLFVau5Y>Iu2-N5Zts"
    "8qT(*kuo`pkRU@-YOJ709CC;?mXf)0+l7R3Vq4R3@L)6E)lq,Z@touLlaM`'8Rnh'@+h-h9&FtZ-EUMs$A7.(3A0[t5Xw$GqNrtBMDh@BUe-qOPT;mYsg?Vs#KcXVch]`O:LtcH<=(11"
    "R`/8V[.mAX#0rk,Uu;.(ZVB_t#P+^$nUSOYY3Na3F/SMF@u%/GVXvq$W;%Z`AZ$k%E8&&2U`AX/vSYMWniRtK)>YwKvaGn]5DM->uZpktQKXIspNgsOlFn?`sEx*dtol#uI#O$%'C^p9"
    "`%oP'@W[CE&]XrZ6L%+2@mW_O`5vX5eirneacR77nT>6%eaCHst6GYs;+TEsHHFGNL0Y_3B?@>M0_KI(&q][bKYvI(iW+:2Ia&EstBYYsIo&EslfWX%5O:.(AISucuHAVm#`$(F=lNbF"
    "tof=#qs4Zs/FHu,n)j:$CO@F2rpkutx*<UsXSjRWeY$N',@?I:B(#.:@j/LTq8&FsoRb=52X/ctrgs(52`[=5gkP^tltYFsM;rRsAWObDH-<_tw'Get]JCuL4N*:MmD5TsnVR'jlBq/1"
    "V/O85LiZ[b6WSE#xp='$G[lX--F'et+f1G#M<r0$(5Kn)h2sns,:pet`k5Z%G=DQssYqJs<P?Vss_URsgf.Ud4L5OpahVF3$;W.(8]kOsL[a=X<d)9-<[:Ds[b?ICD<0.(0s'jt2jBq:"
    "N[W.(l-,ttL:C)-(7hYVG_I>I(eD[9_WdU6C7Ga79IVtO1K&I?'x0j:O&%`%?h57.2Vtq-m-QR.Q,25*@1'iB>4-p$Y[-/f%W_RsNNDut>`uO/CG*V$rC,V%&^sYP_#(Y`IMH.ZW&xfO"
    "[vf,G2-.(Wu6+N'nq$x+agsn%K2%%+UA.[sJm&Es8FnfTu8ktQNtVR%fqR_Oag;7sQ%97.aN7cd2Ww,)f>eUW]L_pO%c4^c`vnkt#m^T8nI5V%1/9vP8?OX`kw=mY(&R^IW[pf;L1B2%"
    "?f@.]LokSCTBIA>5Fa?LIBkdVK>C@+`Eje1wKGtluKgfL[1GIW4jb`OB=TeCP'ekt>Gst,qTtatPRAi'&(Q=#O@i@+['2lOGd#N'x?<:-d.][+HPnFMt%eRV50*FsG5r_t1cqbtlvG`$"
    "PY4kPt%Z8e.L7W8MY/(-be[vS[tw]t:;85fq/ZcC_Rlk/G,Fwl^Q$bOeYtkq?%Ma%'`TQsoRpKIHn210Cvjk/rCZVs:)MC_6#*>A2j)YAlq%(_feG`tE[qZ^7LL^suKcf%CUNkq#1od%"
    "[^i$F?lpktCOHk$].].>Wxr]tau>p.e[]P'H>clsB#XA#o4h3$Y=4j)]a$LgBjq7uE,BdOV/`U6*:OrLWmr'<g@110H.@VsKm&Es17fT%4(xO&JL5hq7B-Ofi:V^C>=YWB=)pv]#gp*^"
    "V.'Gs[O;noPA_UCr2fUCPXX+]3td+]gl8Q$$MNF)>puDMq2?@db_YDZ()IvXW*`pO9Xqn%cbRv-WoHct5v.`A3N[:E7HlQ@)7_eO8h)]+Q/#Is/4'nJ0:$j_**q&$[gu'*H]QW&Eb]'-"
    "U#S]tf=Zp.;uY`O4Hrl.W/pa%dv7%F8JXRs?CHk$EodRbwi_Ds1bCJqhX]P'A<VQsCWqJs,[PYs0klusL/a8&6A'W-K02iLZm(UsD+AY-$5EZRMgUisrvpd36(0js%F9lm>>M@ArRrk&"
    "j2ok/#YjfL3KpF2PIqktZfP^t[l&.pZtb/aa1#u)V/O85>Z_rt*JGdM;ITI(Fna^-_iS$9E6pF2QUsu.LAestwEs-$=s<Z$.,3nW<e37I6sEtZYN;Rs2:]Hs2tKF)28`_3u)L>]^hpH]"
    "^Q%CCTw8;@f4P)-E;lh0qXY_O^S%dW<5M^Olx@&c21=i'bZiHO=lXDs3T-LcvY4rO:Q(HJ*,R%9_3`lOIRq]t`qJ:%F1`Rj[`_hB]l*4mxWH8$@.RX>H?3lO)'C.dbM'I<sF?@+DZf*j"
    "Y+]bt%VHu#d0R10F^fHs)8PR.$&dv$K9fMn8AKJou1S7W90.Q3&C`dO=XI&dmc%]Tse#ATPx.5%?Qh-I?-0.(xP&K#A?A&$Dul0T]+lDsStOr)vgsaERUjDsZ#KEsjBqg(@e0j%.#PR."
    "1uu0)w_GFsW,8TC2.:21D4q5%RN/[AUvhn@AsrnAqXld,eQ@IaMlqYD%gGjBipufta*NFnl3nPUKA(Caf8fqO&WL28t[OZ,-d2V6lRKDs80$[sQ::EsGHWU-mnTU-L<JI1(4jWssw&P<"
    "H+bdO]HTD3ZPaDsFfj]sKQPsON^b:<eR=#%>SI`tU4nF2=ScA>))5L#BfrX>Ww@T%t'()-#aWRs3fVWspXaaai2Dcsc'bA#Xg.Hs$:m$$@6vt,g't2_Y>0diSm]]t=:EbtEmpX>gVehL"
    "=a:B%8H34/Thr'><gsO/>,U=>#9qdOLSWX,;D7a+UY4[9xxF`sQvff)GitW6-1%@XEYPe<#R)Fs]x=QsbRCRs2kdiLhQ9Hs+I9EsVh&C/TSuq$x[$kkeV1kk0Qg*`b&s_f6_eZ%b6#T@"
    "SiEx+e]&kLbf$ftQD>8.k%kLsw4sGsi%'MsS#gkLmZ@ntUJEDsK;B_.G.<nLSgYc2]JWDsJx&^s$JgjgMLRGWE1dt2g-x$4`.w,tjfe_W/pC_WW@TnNbSOitDl'+tqlP^t%M@^$7u%Ys"
    "YDo*DwMQN's#RQsi^0sH7IJs(`1hh0<Rwn%TNQRA0Y1*JhRG+%C]cF;AsA5Y4)v*[;Vpw42(nt>u`Zct-/S=%]XKC-4QpoOh.o<cBsRhtu0JgONnEnn@OEnnPIqktedu`OIR<*n8JuHQ"
    "?e'N'FB@=,`jR$^4O8>*NP<=,UKr0^^G-N'$W07<TZ@x+WIP,*nl`U6BupktYP4it3]s7M=W%iLaZGcOYG)OsEQ)Os<'D@+_ue9utQfjgMA@Gs4L0`2?eR7/.In$=k%]dskdgB#&O#<-"
    "?-(V%t#d3ob:lDskW'o/;21QVgM:Rs%OgMsC<#Ys&?LJsRDo*DW1,Irg$p1pd*)GsB3DutG7v`Ol5Hr$p0%i.)'sx)KN)E-*9=Z/Bhnk&B;SU-=b3j(Cwa/)Lf;XC77EvuE4->#wmS]="
    "#g5>#b@%-v=5kxLShD*O6:8=@]/2.voma2#p$^et.3qbtn2^V-6wFF%i1`E4G#</)%q`]+l&)C/uuq$bJ)qoI9/M)+2H(a3M-HZ%>ESY#81=Xs'v-N'J>cls^=;l-YR8[9%uR[=O80Ms"
    "9)&`VO[^Rs3cO$grT.&g=v3eJ)K5:?E(%Daptj*ME+,DsH*)r72HmVN0biI1Am?u5rk`JsE^<:-;7w]c&ES1;'^Z`c0u/uhl2?DR7^04/,4EF2LMMrt0CtI(`4bL:18s,;_]5WIwwBN'"
    "v-`g%liA=,pBTMs:.5eq3**Fs5JI%u7*mFs#m)Ws3B/3gk%]dsZij._->LhT@FBdsw<4j)^Gt)s#QGp.A?A&$GEYt(eI3cOOsY+)]H@3sJW5+)(JsGsRF:Jsa:Y58/?_w0S,].6I0h@+"
    "SY9l/lD.%uS8e'&X&XRsoRon0e#97.Q#sq-%sah9863AlsXhe<IlkGI)S,o.a-V-*+YDnnv<eM0G.qktlRhjtCU5tL);^;-j<iT.0jGeO$7)=--q7Y-J$h-6aKMPs'MrCsx9N'1JVH11"
    "D)]jO]6NI1wjF2Vh@K`sJwff)Ca[Rs)9]5urCskt`O)nO,2B1^YN0_J<R<ltgD::Q,?#YsGH$4A64Z't8)vvtOlpMs?vHg:q-G*ttO1(*4LYX,fmuBtgUmaN$QsOsVK^#$S(do)K8`OA"
    "ZuN8o8l[w$5/Qqmq:Ms((w#@)7`SXsallU?G'kC3TcPftv4LaO)2lTs$^TUs51Fi$76?u5F3rns^wRqdb6HhqkH0<%dQw4t;pxE#`lf.$ulIi)?%YKL%GGu#0I)=GQ&,F%$3FtZX0q*D"
    "p=LbEXj*=GiOlDsXh&K1P7,&%bq9:&F+xLshFxR3,kCwcPNCjCqaa0_vr>W685>:&qnQ=GW=o,)Z2kAeJYJc%d<Y]swrAEsW1_X#LOJD=8<'<C)h]9vd%j,)#r4F%n1TQs>SoPs)(J'J"
    "fX,Dspwq%*6x-5Gg1[Dst+tWqSE&S./36l/'nP+)<aiP.`#+Gs0ltCiZUx]t93nwB@Q<gO_CJDedb=ee88c@Tc-eVIEQmdsZv2i04+Y/Cwa&n=EmPP(IT8]trt`0_iWqutJ.L[,2%p`U"
    "K-x,)-rD2/d8,hg^[H$T<sDR@:`+Ds.[Eu)%Zkn.*AP:&=`vRVc6rZsPQXoLa);Yn@r+Ds49]q)VYSftj<3i0>oBsdqYkh05%^^s`;%%+gCN>UZXHYs<iP^tO13Ds4g/Es5OA]tkL,q7"
    "rACW-lQAXs=<F=A(LfC_+[xC_;Rlc<Fm%c,$vju)1XVm`Oxd$^O78i''n-B#%jEJMo+hN_ox#H;60Db,moadYhuv]t70XltFdcF<f.Dc%:CVR%U^s#-iugb;_M6N'G=ug.^hh?#*(qK-"
    "kF?656e_tPB8E9@$ibZ,#GqGGK@[DsGa9Bur9dP0ixAi[KXX?V5#_iLj#pV/Nbf-Ik`OMkdMsA4:cY0)]N8Um3P/-)8D?(u[Nc:&sD9%M4eNcsggTd3MdnEaBRtU6'#R$c*?]q)xY`UN"
    "8n;d%m$kWs;,+v,f3F%4M+>gC<E1EH3OJi,FjH<t41hY]]k,e]^H`'CU$9;@e4P)-+jc*I/Pes-'0dNsb4AF2nL2=>_=dNs6dWLs`0deO*(1l8rptit=Z[:M?bHf$E;p]Vtj-x-9qc9M"
    "-86N'9^jKuH2c:$s'Kk$&gqdmEliL)^90ts242p)cH_Rc;FrvAH<)uLD,#dN'*P#>-VFZ,rqVAIW[jn`UG8fO0`?HGg1u,)=)9i0kn_'/xm/j^7/btNXw`B%PuG^4.cY0)aDRSs[Sl:&"
    "OP&;&.x^Qs<MfPs)P?VsHh5NsPRHk]na(;&F^C#5[)C4Jd;uQs:miOs#^DjCO7=noHgKp.197Ds:w-<A/FC/)Up;'%[*+9BJP<noekBAuP`78<N]u;-rhrU%uvlo)O1#&%nvSK*8G)u$"
    "GdBp.NbvwbknHq;x]CZ7n_^/)X2>YsEvWnO^F)*JxG@=,mT;&L:`L]KIP)YCwJq_%ox.</wdxma+tIo,:*;9rj'u)Q)uox=.v7>#DNULsqtN))5;OatTwIHsthc0)dAvNsp-x;qj'Fo%"
    "]?LaO*3Mr2,fFIsd7Uu165vXKCPR*j;_[(fnGDw$^@iNpn-o_OJcQIm]wPn%;V^RslK6Z;TJP4XG+>oO#d-;YqSZHQCpP^t=e[h%t##o%5;PL0NqqRs;I/[sErNU6+VsU6>^gDjpPOL0"
    "H0V>9?Bl`kX-Qe89p@Z5O)$Xs#l:;&HFG;&%Va-I[6gEsS3,LL5Ph$GC[)(<Ac-bO3/&3.h]T.1x30U[L/K(<FkIf$bA$6MJpTHs0@RqIkBj$G/I*=1?t0V2JM5HsZp^O./jsv2[M:&5"
    ")jJ21DThW7/.`wXKr^Usc]d>7w=cls`q+8I`shcMvD&Fsn;oht&0&Ps/JK]s?qh=,<6Rctx#&Psj/ohtw5Zbt02xw+eYCbtYtC_WWm9@IPkfe%'K.FMSh,x5E.`aO-M8IC^T%;Q'*I`s"
    "Oevd)chYRsWW<9u/CsktkkOhOtD]_j%DmEIim7ht]xe)t+RKB#.Tg#$S-jUHV2;uUp.+cRdV]sR*qgk/`v6uL#UchOF*TkOG9PHQ,obCS+TNA%*[<GZAKq.[bbT=_l*:?%q(l6adv(<%"
    "remIPKG`p.#&S%-[7,^OB=i<Ci:L(*P.ThYUle`%ps#B4]w<t,KsqrtMwwH#[ZEx#>I.rH=K`gtMcpB%+UulN/K;7%8Y+DsXl;d%>;Fhm.XPbDZQekYF^wc%FnY/CXa1mO+^/=>_Y0Mk"
    "bZ2iOl@qLOQ,)<%e`>2>17:?%rmGDBsf7JG7Oa:D/aNA%&[TrD9e&4OSd0j%ROc@Km3Qc%Ka?#Y3?tkXEnFKhIDqh'/U6h(XEZ@)JRqfsf/GsOIRoratb4l$96AuB4Ov,)J1Ju:G<2]9"
    "@XqIEeJ?;>ba%oXugIc%KmZV=x;mZ%+7Kf'.'YbVvd&Da9Qh[s1G'Ofh[K=6/-R9*pP9EsZl2od1admiqIM`WPsG[sSTM`s7jvd)ec7A*Hk#]4*`[gYn)Dl&h]Ca47pDakgu1GsV5E3m"
    "P9j&4hx7UCVXGc%O()qbxTG6c241lX]_4a%pjcGsJjHndK`7>/K_BZj.kKhLiVJx$M3q'X'ubcHPWx+-f@MDsw#bu)(-vZ$tlD3Y%;,^bF'Zb,PPs+h63>.%aG^Y$Nqkq6xaav5GLMGs"
    "-kpq6VR?Vs?q@Hs;I)=GuQfb<j:^)^W/M7c:`cWBoAJ(--OV5/@o.<.2J%*r^umdqo:%$-MreT%Q(/Ttb9r;&,la-I;/@S#+PpRco)b_3k8=_tg<hC*>*]`t6QoktEhnHsWJt]tL,d'/"
    "TRZu#`H,nA9Ov[F,xvo%#jTwu.$;faVg<FHrOSM'=U9>#FOVV$.K0.;)%L#$H'=7/TZh3v/:N'M?-?OKag]=ubU/;/B(%-vIY[fLZ[LZF4Y,,aw_N^-VlQn*lw@%#:$O3JJ-2^#Nh.Y-"
    "rHCb7Awi*.%svqLLFnxkVBP:v4WRh,g^]$0#<>&M%L'>E.S,W%<e,`-FVZ'JhTqfL.flx<@ebMCbD#W-E+2I$i.LkLL;*'Q9gpJ)KePW-0drE@_0XoLL*rCc37KMhedPW-kIi'&Po:mL"
    "]w@S[sf2GsHqP&#6dPW-`Fv<1ac4wL?fai0UnbU%2gYlada3-vNj29GmZQ8'qn^oI045>#dWfK#>Tf;iUG<D<%K9>#kS^HMe-[uuG`?i7(;48l-?%<&gau5lHi-<&Ho9?5Z`Fn(;bd(u"
    "aQo:)WHXEtOxx9&@&PL0EU,l;GA>%&i[aD+D:XcsO:1@-5Khw-.eCBS&e<k<h$MT/tI>78fw.E/3NRV6Vp7>#Ui0wpwettL_T:^[X$Ed%*w;7vs3Mp>YU6m2^)AwuF'Kj7l<`R8:[mm8"
    "C?&49F<mo-vmK)Jr//19/2$bb9Qh[s4PHbs.2.&*`K&c$m1QActtfP462B582vXm:^h&08Y$I$?+Z3;$S+Hq).mFQY=QFab#AR:vSRU[M[FZ`ORlqGBSiDA+x/F7/:3@uu`ItuL6w1gg"
    "k85uu?2'-vMP?aaC]hU`sPOkS/N,Y,K/Pn*-G$-v#<16#O1QP/<QE#.JI0tLjksY7fb<^GDv9>#,b=b%oFP'?:>]&#n.X'AmO-4L4`:J:]VoS7.0PY>te;uuRrJfL8?;EsxB@=,Ot1nS"
    "JAww+0;oCswAK#$>t>=,?<)uL4Qvm<)c6N:wU`Ou[*ri5ABskt(T]RsB%uxtG20v)d2%?%WgkesFW:?=O16at1oJ(3_C=uLM=Q:vEQwae)lGga3wM4<TOJ>Z_`(l-0snWh:vp%Me6>cM"
    "_,g2`Q)MY?7Xnr6O3suulS%uLfWJW$_x#5J#)5>#e+f'&YQ;hLSpnh,Den[[%P*Zs<q#YsE7C[O+s3k8])X#H^JgU.^AG?-xqbZ-LN(?R`Sww+*D?&0St6=&m@WVt-L7<&J^4g2MM+n("
    "d3Edt[,svm7Q@C+WHed4/_Gxs4B_5Mgelneq1dU6M2]9Vw#E[OV_dh0>V2lMWe-Ys6wIi)@wMBOXpCl-PEX<UDG1[k9).pfu$X0P.d9K-agTg-x/3M,H/P*RhX(19r)`HE8):d3(ZxeQ"
    "n:d*`(-AX::j]RsGWqEk+e?S1pxZ:t5;<=co]<rtDT]f+_;I+uh5AF2,9dtt4_v(>^WhLNp:XW)>:qg;[8mNsdFH`E*D$<-2HdE&^WI<&U8S<&pqfRu>W%l`';%YshP?Vsd=G[Od<eh0"
    "o9pItOxx9&@.D=lFM3B=tI?=1TxuFsNEv<17#?&Mijr;&q/9f1jQYMCkU*6/+vZl)HnOdRn76>#W]ZU(=Qd]P[uij0+Y1vuEeY0)0D7VmTjx;-ZreQ-qR^K/u'wbVkxQ^=+*BIZT*.19"
    "#OP</dG,El[fDsLPWgdsS&jF&KrE=5pNpO42%'j_r54J_]s2Ds-wlo)+r?o%:5tn%34c'&5bubMNOrn%wcPW-al$@'ESK.=<++g(eB1;?voV:vJ>Tj)(rf6_ow$,;2tpuu,Rg8#V;<&`"
    "1#BL;bBR:v,a1&vMFbm7-1veOls]D4&f'#v[c3hL@tN(Vk6Eu.<oV:vXeGb.hc[ca&OW`<(Q`X-nm;U)ZEY:LBB5]QSY-S[5&rP/giL^$f64(huibJ2EMso]p7fY-In1[BMFwDL1=veO"
    "2G%]4&f'#vY8oxLm?S],]LTP^)[we-`[Db.CY_1#3t)P]Lsus-[pOfLGA4<siwpl^IT(1.?BtoL+DLJDM;)/rhGLP1`v5dOj1^;eO$aNo:s0C&QQI;#FI>s.6UBg-6JLn<fn*'MtE)Q7"
    "2jj$vj7?uu0E.gaM6UG)'_TY#bXv$MQ5,Lq'Uv%l#FT_-MRV0GM]$7L2x3ci&:d+.u`+jL5xta_n;wxuRth9;JQniLgJH4IbP6H*&?@N'j1Ls6Z`Fn(Kq.h;xIwZs+_&Hs,kIi)B2Y0h"
    "RlfXPQ;BZP,N$Ysb].C/I92uL+C3mL@^IJ9Y9YMCNY26/DuZl)P,EfOL<;^s'8U@/wF,Eli^`kL]Wgds`i&W&+@dvt/vke=kkHYmfeFHs.AXatuOo:)]X6uMMCiut+D]q)dS-X$>n[V'"
    "GprBsKM%d<^PAs78qji*fKxO&'U*j^/pln:g[(*-R3UA7nsxHsi69v)Z$*-]F(D)v<?deahNg@kImOG<uF>1P=l0lOmS'vus?`0#MQf7LS,>4&BI)6(d4eLi[#Z=5w7_:-E3QNs+0lF%"
    "*d4/(dvH2vnP@saIBJ[AWIV.huVMqL:ZYCLlT0^=qD@Mh6DJxZRB&S%#P*[sFVqDj_9XL1H#RNsE6Cj=<T&sR<gHC=v`vekq'0?.C7&YCVqXcsVV3B-29Lw-#SCrX;wsK=d-&X%v-%uG"
    "B(0-)H/u+71wGwKhK6#L48gl8$crt-D8#m8/r'#vNPc)M1>h/Lri.BfH+Dh)37XX$Z/0-)QKV<&vONp9T)/fZDL@mLVpn^7lU2eav0HtZ&a,c29Qh[s-U#L>JTeCs;((4)6uM9L#fHo%"
    "_hcVR@Yp_-?5W8<w6bc2,PP-MO;Q:v+7#5(BT&]tZrX0GDZ$hcxA?i768R'l8iP:vW8q3vh9P+#Ge#v?l-A>,DEIQ/;]j3v1m#i7Ekc'&B(B+MI9($HY>':)(CX=XF&Dulv9giL1LO)#"
    "&J:U)uVT:v).Ia,sdAW-Y7^*7r(TfaTm-R7ANn/8)^uIs7cXg0c5Heg@*k))h2;1)]SH_s;g5>#dng<C11C1L*`><->K2a-E19nNbPJvLf9NpHZwLv$a#47/L2FuuU&^%MT^q;,`fIMh"
    "UF#W-63B['R.:/JKn9hOu4@(]3<fvUhdSp@EV<.)n%AOYQZIc%p+$a@9[3uGUcNVC.-VeO_b))mrVm%Fv$Me$WV_nL4V4>H6R;j12u2>0bj9b%Mm8>#r;b6EdJ%sL84(9TatQYmSCLV%"
    "R3d_7;/D^OqmIUs]h.f14_:4EvbFc%[I6=a+tw5%fuN(Sr:[Rsu>9Djpi0.vX=7'#8x()b_S<^-Px*hY%(CuL-Wc9Y3aGn]J`[WtI`xS/P?DX-RK'XLD/^6E`7M(aXl?f6ZSb[Aw^-4A"
    "]d(11/q8'FTJ>4&T/$%4)wMO/WBlR/PUr@04UT+)m0PAP*4d`-*pY3FR%PvLd0W^oxmtmp/06vu^ZH?LJUuQsen1@[_?J]GJfRw6Cspuu3G;:L.rhv#$TfH<T?7>#Z:p3v-j<jLdbxPD"
    "7HbC(6jNS7(e'6--IV]Is%ni$ZoZIs%Qqn.DD#u#(Kg8.ZD<l8B@w>PwB=Ws7Q`w0OLU#9eRJ8o/^aUQKU^`tpuZb&YC]MCNSMQ/G/vo)u*nk]RAn_/J)Z20_M#Fs0[.19xTb?K0TO$k"
    "JpTT.mtL_tlTlG-%>o#%%W+.:o:ew7'D`lM9c-YsT.el)fpxF.<0kh0`hB[O9lHtlot/pf>2s3P#G,I#CwU#$Iko6WF-3M,cQ.19S>dw7#)%IOHR-Ys?$=j$(>71'M34[k1%GWkkZ/<-"
    "Yg8;%bkQ]t`eNM*V9UnJ?%]dse/v[<eK3i0sPC%4kGvl&DOtk0K>Aw6mHVGsRX-S[9t8T.b]TY#bvQQ%&X@$9(Oc0L+4Z'X(US(35V/-)$6JG5tOWZsn-LD0YwC[Of^P0)godv7AQFQd"
    "i<[O9&Y4_$m8O)NvB=Ws<ZD[0BJS%8i.@2hBIcxQ+NP*?$k=Pp1[e;-tUu#?KWnk=5A,29jD67/T_^[sNa`K;JjjfM?0*(-J#qX>D(ADJCO-19@BZ`thu<_$#TE-ZOm$6:t=3ZsIw8HQ"
    "$`D/7re_?@X`Gxk3QRwu?b8(#D@KDl(55>#:v'-vKHs$MrT):o-#DMqs,ST%.l[i7Rek`O8sQuOs(1.v_;JU7d]M8ZnS^`3InV:vp6e'&jB7U7l9#w.hS9>#nK?U)I80xLxMO/MsxGF'"
    "dNe,a^spuux*3oL5DWS&H4wudUdPW-QPj?B5Hf/#TeX`O-Jg0ML=Q:v11o3+1K>oLS%DDLK_Y8/G/VX/sO&i%g?OChZvb'c?Q<Dsc&[wujL`FLF,T1p1DSb-w-&O=K_=)MY2%*3Cc#6T"
    "L?7>#Rc<mLY=lSRJiHPp=^FZ-I;a*7<&($#Eim5p*Z<l=ZdZcOkYPGWYU(1.-qAmLMlu'Ee1;j(KF/j-A13U2F)u$M5QDG)LtcOb@7h'MaxHV%bJ,W-8^Sw9T'G)M-WhE<=lRJrLqq;?"
    "VQW0r2m5e`E_)V$:/KTsX0PwlV`;j0VbZ(uhl:;&:rF;&PkrMUQ4Hr$D:Xcs>bJc$E$`UZrOn;2hWuoC_kLEs3/vo)/tn<ZXk7=1DRGc@aQ.19u_xY-Y%d;-N#/'.Jr3h8%fXAYLOs/M"
    ">A]qLnSZp%kP_NL)-21gxCJI)x//IQ)Y4FujK##@w,]5'UGj2%TDo*DR7XU@)^+*=x'[L<%p[o,e2ft?+`hPiw=[1KfH4C&WR1_J4`sRMxs'+*ba;+[9Nw*D_6:UMw^8m8072HE0&RN'"
    "n,40b?VUHsvwG$$5hut,CBa]tCH-]sT,kci&Ykv,Ml`X,-],%n?6I=P),1aOXpX]t@44at0i.T%`B34/?sKX>4s]RsV7(=PeB9EsQS/7.510ggnvej0a0#dP@#O1)S,KEsBLZ6:/>Qxu"
    "-B#5#pHQ%b`)P:vw&1&vae:ia%_6I0v0[tHEa%EF52QP/xS'8/@j%-v/`h%M:'9'h3-u]ts<b,2JX#[-Is4Isd.6>#^68[9iTGAQa77g)fI4k1i^h3vT-3$#B$p1vQ:ljL2oP<kmXW;7"
    "s_Ma-H>c'&KF32:_@wV%&]kA404wl)vI5`H_VO]HjsdXWBC6[WF;loAo/Inpjp`Es'L43DWRXOs8imbsLAY@&GtZU-G9B111GIi$^kqCsPvZl)wYTas4(9x=(cow8#Yc`Oaf]^4=CVY#"
    "U6>,tiR@<&Q&T:vDc&kONqR[WH)C.3F@R:vas(kO1bHo['XT:vKlUEnjXH4LqTj4%UB:>#tO`nYJHgD9h&eLDCD&V#[Q_v#c@>I:p>p7%S4SZl.Y8)t-r3(3.l4Auos(.2;x;:?5>]rZ"
    "CnlrZ%.rk9%S(Y(,wj5tcqn<&p@:F.TQJF;HFhXMaD4@t8Xew0&;KsRb'-DsvB_<.wPB_a_[t20$#j]sExwSsL@FTs4Hn)c3nSsb3$ms=o&QH==(_P'DPSQsk%*Wsechh#ZTFb`<1a8&"
    "Jj.+NE@DW'N``-I'^&K#BCv$$[Q.FsO_Qctn(/%Xqx$N'MfwErvY3$kNk_RsBh7gsr%tF7AuQJs^K5`8>#[uO7EmxgnDdtO7Hsx^^7uxtd/Zw)IUqh^g-4+`fkP^tDdq%)]eKtORPPnZ"
    "P1wP#ao-(*mBwqYI/T&$LjeqQ:.rER.fh9MY0qhToujVMt1=,R;x__*#_l3+/LDh,$@mTM=3C*R)-GVe>'JY-W8AO+6dnYL9^sn3x+'sRD#Y41IKltQ[Q0WMp5<l&nC#s-%;+vLt-25O"
    "SK#POlb?>5wQQc%oBOkOE_37N;4`_tJ^VqMEhWVM]2U_&TCwBS1B3d%P.PR.-=petI)B;-#mu8.=gO[=I*eM1Lv=R*0`U;*ej:r6eZ$fCZlvY6-x8gL2`;Js9KWfLP'9XsXAUas]DxBj"
    "HW]psxW#4JI&)uP3ID8&f<4q%PoD^O>.31'x%ijT^@dh0]xEq&,<hN'Qiuuu^Wx%#E%*20&),##K'+&#1DW)#-[%-#,x&2#;D17#?DAX#T9OA#PbcF#IgkI#,,lQ#_b<T#^o4u#ev^^#"
    "omRd#$:^i#>Yq3$@/+*$0@p.$)si8$.%r;$#=cXYqCgTs8X39@^^b?`WLwZ`D<L&@e+5d,K09dO3gxe$x1H7tOwI_O$pUT.Tl0qt2PAx>mFX^,un'u,:JQt1Uc/V63FRm/`9i]t([Qct"
    "*3w1%mjZU6(^)V6,J[21j:]219cdNobRP8.EMaS&TI]:.D(W[4os.d<8M)7qNP%4ATEQZ%`.]NoFSjS&r>,K12P]s.ZKNs,`93i0Vth]t0x[HsDEu2rm-Jb,bM:Q&mxS'jS'.Ss?^#eO"
    "mX<.Li@n-L/+,IsVq'r$<4r_tGfl]OiC#.:e-oZ7(s`C_;<cxcN-S(-<D`+,d1(wIu-FG<&#x4-(Qp4kkQlM5^L<%<1#latwknf1Y7#V-npEI1q`T=-b`39@X5AbBXUmv?f4P)-L$fnL"
    "uFJ@%Gi;M'at^#]Ei;M'B`c^?rT2`H.JkS&1cjK16u>:$n<DGs+iE8n1?.L,TeIn5_D%Acd3TOY=+;?>:e%LG#YDD%9Ve]@_8S:apKEltKt,Qo3pQU6f&9<95;NU6H+mnO/Klf$Qgt,="
    "l#k_tc1^:-=@Dr$YE?iL%cl8.O.2Gs_6uF+7M>I:l6ik/b`B(<pN7l/VveI:tt:T.YM9C<98M.27nbS&f2okti+IKsnH+Q&:emf<Z7=#-v%qA.hnQ:vk`w,v1OkIL0,666.q_f:/r1I$"
    "5ef'&1+Y&#R_O[',7i*.Ru[n/S4wO&fs:_t;RH&1;vs4S3b46vT@H6LNPIRB<VC&37rxXuLAR:vf@Ma,>g_TKVN4Z-hf4I$&OY-2ed>A4uG4.v:%Z##`C$5dDq>69A5Hc%5:ae+>W<m0"
    "KL2e.rAs6l<^ul&BC2`H-DbS&S;'riJSLcOWCQ10R2a_aZw19.$,>>#,nnP'8);d*vlKe$,mZw'WmX+`$o^>$*cF&#=LcA#7xs-$8%&/1SkZY#Ner*>TFJ=BtI_`*p3Lk+e_Oe$Dh_$'"
    "cS'9&<:gr6o_aD+Q'dA#n0E_A6%2eGrA=gLB=V9.)AYY#4hDW%WCt(N<`o/18L7%#HuJ*#;AU/#.(02#kd`4#c90U#K_[@#*^ZC#NiAE#O`;W#2nw[#0&CL2o$6;#>Lo5#,)d3#aV*9#"
    "1q&01Qb($#s+k9#rc<9#RM#<-mK9o/RYV4#8[W1#gL.U.vcb.#V,ex-%Wm&MY@O%v2'kB-_^pO0r^_7#hkF6#4xmqNN@7&MBMu$M06&#MAmuxu5CP##?8?&Mt#5$M4*jxLQ=R3#7>0:#"
    ";l+G-Jn+G-Ds;Y0hUZ$vls]&v'&loL-rugL]en&MLY1%MYRR&MwlM7#Q10sLVZ,oL`*>$MIbamLGL(:#6qX?-G.%I-vCSf.?kq7#CN@2.cI+wNn5%&MG6P$Mv<95#K?[*.4l&kLN*^fL"
    "pmx#Mtn>WM2$>9#Her=-6YKo/=aA&vNmS&v_[V&.acrmLEQ$'vISY'vvConLoS#oL&Z,oLu`5oL,g>oLSJ<(vd.M(vX4V(v(vFg.'Ai(vLYaB/+M%)vc;_hLJQwtLTB7&MD7P$M*'l1#"
    "4WT,.K,-&MK1G$MpcgvLpcgvLSehsLF,@3##U;uLANu$Mt)N7#bg>3#5XU7#+A;=-x`Ys-IG(xLI)i%MJ@'_M^#4'MFS]8#a+Yl(t$Z3O%:Z3OueBqM5=CA+`E6)FD&ruZ;VliU?tU/;"
    "vcU9Vn@wIU-J0fhJ2P>?I)5#?NEpY6I)5#?;UcM:,$)#Q,$)#QI)5#?,$)#QE<iN:[.l:Q/EMiB`ipW_7+ElS7'4c`7Hei_b0>;[=gEJ`Y<XDX%V0wgM@Re$s;[`*$arJl8r2'#x,_'#"
    ")9q'#/K6(#4^Q(#:pm(#?&*)#C2<)#sCbA#BtjYd&='1M?GXmH^JEF%/ptk]X>387tl+##qAU`3>@FG)TB45&eAWY,%FJ_&nj6_ALQHG)hk18.-=q^uo_h._)WI(s&*px=>48R3PMY##"
    "j4RqLfu'kL<1nuu2$o%#;W.)v)KkxuSxm(#:[^C-[o8gL/W()3?il-$&?;,)YExE7aqdumU+e0)dl]o[QIZP/Idp7nPlYSJ$c'^#?CBdss%I/(;vNYP40oP'uNtxYnWufDb6arnoJ[YQ"
    "o=6R*w%]fLed7iL2L>gLqwQ2MPL=jLB,u1#Gq,D-NYvp/sZ:Z#ugLZ#uArP-%k5pLE>AG;*S?)4dS>)4e36N:cT),D<b9/D_K2p/ICg;.c^2wg$wVY,adI21YJvd*qIf`*sw*j'XUP#$"
    "KH+kLEf]V63=S>6EklA#_r.>-#&jE-.=Qx12ZZC#af=:$$wiW#=Z(EMM9k:mF?JYGa2UM'%9@JC9&0;QP:c@t7((,DAo<B,1fg+inJt-$+k*$M+O9g-UGdw'D$<YPmproS_ra/$0+:hL"
    "D;-)M`2uo7(['^#Pt:T/OX]:dj;U>6D0KVexdK>Z1'MwBhhi8.Kf08.N*:kFXF]w'K-nxO5iq.$ACWS%o?X-?Epk-$Slc;-VQt4P&=gfLvgwl/,/V>61xKB#_S3T#TCDmLI_e&M'ZxYM"
    "'C:=%.rVw0`gAg27Q7AO8H_1p%A7R3`.7F%lJs#$ATA&$P<ClL*;EJ:x<X/(VC0DN;Ke]G;QSMU4?Om'v3p+Mb$r=#^mo=#<%M$#Cu:$#qd_7#1QA=##wu+#kma1#81]UH&0m$#" };

    return glacial_indifference_bold_base_85;
}

#endif // #ifndef IMGUI_DISABLE
