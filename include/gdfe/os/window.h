#pragma once
#include <gdfe/core.h>
#include <gdfe/os/io.h>
#include <gdfe/collections/list.h>

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
typedef struct GDF_Window_T *GDF_Window;

#ifdef __cplusplus
extern "C" {
#endif

// TODO! move this into private headers
GDF_BOOL GDF_InitWindowing();
void GDF_ShutdownWindowing();
GDF_Window GDF_CreateWindow(i16 x, i16 y, i16 w, i16 h, const char* title);
GDF_BOOL GDF_SetWindowPos(GDF_Window window, i16 dest_x, i16 dest_y);
void GDF_GetWindowPos(GDF_Window window, i16* x, i16* y);
void GDF_GetWindowSize(GDF_Window window, u16* w, u16* h);
GDF_BOOL GDF_DestroyWindow(GDF_Window window);

void GDF_SetShowCursor(GDF_Window window);

#ifdef __cplusplus
}
#endif