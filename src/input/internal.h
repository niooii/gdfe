#pragma once

#include <input.h>

void __input_process_key(GDF_KEYCODE key, bool pressed);
void __input_process_button(GDF_MBUTTON button, bool pressed);
void __input_process_mouse_move(i16 x, i16 y);
void __input_process_raw_mouse_move(i32 dx, i32 dy);
void __input_process_mouse_wheel(i8 z_delta);