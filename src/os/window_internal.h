#pragma once
#include <gdfe/../../include/gdfe/def.h>
#include <gdfe/os/window.h>

// should be client width and height
// TODO! erm
GDF_BOOL set_internal_size(GDF_Window window, i16 w, i16 h);
GDF_BOOL pump_messages();
