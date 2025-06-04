#pragma once
#include <gdfe/collections/list.h>
#include <gdfe/os/io.h>
#include <gdfe/prelude.h>

#define GDF_WIN_CENTERED 32767

// TODO! support muiltiple windows later (or completely redesign to only support 1).
// this is a scaffold for multiple windows but
// does not actually support it, the static MAIN_WINDOW in every
// implementation is the only window as of now.
// one possible way is to pass in a GDF_Window* to the GDF_InitRenderer.
// actually thats a fire idea wtf why didnt i do that ill do it later - 5/4/2024

// update: ill do it tommorow. - 7/11/2024
// update: ill do it next week. - 8/7/2024
// update: ill do it after i get chunk meshing working. - 2/24/2025
// update: I DID IT GUYS - 3/6/2025

/// A handle to a window.
typedef struct GDF_Window_T* GDF_Window;

typedef struct GDF_DisplayInfo {
    u16 screen_width;
    u16 screen_height;
} GDF_DisplayInfo;

EXTERN_C_BEGIN

GDF_Window GDF_WinCreate(i16 x, i16 y, i16 w, i16 h, const char* title);
GDF_BOOL   GDF_WinDestroy(GDF_Window window);

GDF_BOOL GDF_WinSetPos(GDF_Window window, i16 dest_x, i16 dest_y);

void GDF_WinGetPos(GDF_Window window, i16* x, i16* y);

void GDF_WinGetSize(GDF_Window window, u16* w, u16* h);

void GDF_WinHideCursor(GDF_Window window);
void GDF_WinShowCursor(GDF_Window window);

void GDF_WinGrabCursor(GDF_Window window, GDF_Rect rect);
void GDF_WinReleaseCursor(GDF_Window window);

/// This does not work on linux systems using wayland. Even though
/// the user may be running XWayland, the display info will be 0 for the
/// width and height.
void GDF_GetDisplayInfo(GDF_DisplayInfo* display_info);

EXTERN_C_END
