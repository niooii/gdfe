#pragma once
#include <def.h>
#include <os/window.h>

// should be client width and height
// TODO! erm
bool set_internal_size(GDF_Window window, i16 w, i16 h);
bool pump_messages();
