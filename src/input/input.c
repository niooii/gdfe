#include <input.h>
#include <event.h>
#include <os/window.h>
#include <math/math.h>

typedef struct keyboard_state {
    bool key_states[256];
} keyboard_state;

typedef struct mouse_pos {
    i16 x;
    i16 y;
} mouse_pos;

typedef struct input_state {
    keyboard_state keyboard_current;
    keyboard_state keyboard_previous;
    mouse_pos mpos_current;
    mouse_pos mpos_previous;
    mouse_pos mouse_delta; 
    u8 mbutton_states_current[GDF_MBUTTON_MAX];
    u8 mbutton_states_previous[GDF_MBUTTON_MAX];
} input_state;

static bool initialized = false;
static input_state state;

static GDF_CURSOR_LOCK_STATE cursor_lock_state = GDF_CURSOR_LOCK_STATE_Free;
static GDF_CURSOR_LOCK_STATE prev_cursor_lock_state = GDF_CURSOR_LOCK_STATE_Free;
static RECT mouse_confinement_rect;

static void __update_mouse_confinement_rect(GDF_Window window)
{
    // TODO! rename misc.h to misc.h and slap a (CROSS PLATFORM) GDF_ShowCursor & GDF_ClipCursor in there and call that instead.
    i16 screen_offset_x, screen_offset_y;
    u16 w, h;
    GDF_GetWindowPos(window, &screen_offset_x, &screen_offset_y);
    GDF_GetWindowSize(window, &w, &h);
    mouse_confinement_rect.bottom = screen_offset_y + h/2;
    mouse_confinement_rect.top = screen_offset_y + h/2;
    mouse_confinement_rect.right = screen_offset_x + w/2;
    mouse_confinement_rect.left = screen_offset_x + w/2;
}

static bool __input_system_on_event(u16 event_code, void* sender, void* listener_instance, GDF_EventContext ctx)
{
    switch (event_code) {
        case GDF_EVENT_INTERNAL_WINDOW_FOCUS_CHANGE:
        {
            bool focus_gained = ctx.data.b;
            if (focus_gained)
            {
                if (cursor_lock_state == GDF_CURSOR_LOCK_STATE_Locked)
                {
                    ClipCursor(&mouse_confinement_rect);
                }
            }
            break;
        }
        case GDF_EVENT_INTERNAL_WINDOW_MOVE:
        {
            if (cursor_lock_state == GDF_CURSOR_LOCK_STATE_Locked)
            {
                __update_mouse_confinement_rect((GDF_Window)sender);
            }
            break;
        }
    }

    return false;
}

// Relies on the event system being initialized first.
void GDF_InitInput() 
{
    GDF_MemZero(&state, sizeof(input_state));
    initialized = true;
    LOG_INFO("Input subsystem initialized.");

    // register to some important events for the input system
    GDF_EventRegister(GDF_EVENT_INTERNAL_WINDOW_FOCUS_CHANGE, NULL, __input_system_on_event);
    GDF_EventRegister(GDF_EVENT_INTERNAL_WINDOW_MOVE, NULL, __input_system_on_event);
}

void GDF_ShutdownInput() 
{
    // TODO: shutdown routine later
    initialized = false;
}

void GDF_INPUT_Update(GDF_Window active, f64 delta_time)
{
    if (!initialized)
        return;

    // Copy current states to previous states.
    GDF_MemCopy(&state.keyboard_previous, &state.keyboard_current, sizeof(keyboard_state));
    GDF_MemCopy(&state.mbutton_states_previous, &state.mbutton_states_current, sizeof(state.mbutton_states_current));
    GDF_MemZero(&state.mouse_delta, sizeof(state.mouse_delta));
    if (cursor_lock_state != prev_cursor_lock_state)
    {
        switch (cursor_lock_state) {
            case GDF_CURSOR_LOCK_STATE_Free:
                ClipCursor(NULL);
                ShowCursor(TRUE);
                break;
            case GDF_CURSOR_LOCK_STATE_Locked:
                __update_mouse_confinement_rect(active);
                ClipCursor(&mouse_confinement_rect);
                ShowCursor(FALSE);
        }
        prev_cursor_lock_state = cursor_lock_state;
        // return;
    }
    GDF_MemCopy(&state.mpos_previous, &state.mpos_current, sizeof(state.mpos_current));
}

bool GDF_IsKeyDown(GDF_KEYCODE key)
{
    if (!initialized) 
        return false;
    return state.keyboard_current.key_states[key] == true;
}

bool GDF_IsKeyPressed(GDF_KEYCODE key)
{
    if (!initialized) 
        return false;
    return state.keyboard_current.key_states[key] && !state.keyboard_previous.key_states[key];
}

bool GDF_WasKeyDown(GDF_KEYCODE key)
{
    if (!initialized) 
        return false;
    return state.keyboard_previous.key_states[key];
}

// mouse input
bool GDF_IsButtonDown(GDF_MBUTTON button)
{
    if (!initialized) 
        return false;
    return state.mbutton_states_current[button];
}

bool GDF_IsButtonPressed(GDF_MBUTTON button)
{
    if (!initialized) 
        return false;
    return state.mbutton_states_current[button] && !state.mbutton_states_previous[button];
}

bool GDF_WasButtonDown(GDF_MBUTTON button)
{
    if (!initialized) 
        return false;
    return state.mbutton_states_previous[button];
}

void GDF_GetMousePos(ivec2* pos)
{
    if (!initialized) 
    {
        pos->x = 0;
        pos->y = 0;
        return;
    }
    pos->x = state.mpos_current.x;
    pos->y = state.mpos_current.y;
}

void GDF_GetPrevMousePos(ivec2* prev)
{
    if (!initialized) 
    {
        prev->x = 0;
        prev->y = 0;
        return;
    }

    prev->x = state.mpos_previous.x;
    prev->y = state.mpos_previous.y;
}

void GDF_SetMouseLockState(GDF_CURSOR_LOCK_STATE lock_state)
{
    cursor_lock_state = lock_state;
}

void GDF_GetMouseDelta(ivec2* d)
{
    d->x = state.mouse_delta.x;
    d->y = state.mouse_delta.y;
    
    // reset the accumulated delta... maybe
    // state.mouse_delta_x = 0;
    // state.mouse_delta_y = 0;
}

void __input_process_key(GDF_KEYCODE key, bool pressed) 
{
    // check if state changed
    if (state.keyboard_current.key_states[key] != pressed) 
    {
        state.keyboard_current.key_states[key] = pressed;

        GDF_EventContext context;
        context.data.u16[0] = key;
        GDF_EventFire(pressed ? GDF_EVENT_INTERNAL_KEY_PRESSED : GDF_EVENT_INTERNAL_KEY_RELEASED, NULL, context);
    }
}

void __input_process_button(GDF_MBUTTON button, bool pressed) 
{
    // check if state changed
    if (state.mbutton_states_current[button] != pressed) 
    {
        state.mbutton_states_current[button] = pressed;
        GDF_EventContext context;
        context.data.u16[0] = button;
        GDF_EventFire(pressed ? GDF_EVENT_INTERNAL_MBUTTON_PRESSED : GDF_EVENT_INTERNAL_MBUTTON_RELEASED, NULL, context);
    }
}

void __input_process_mouse_move(i16 x, i16 y) 
{
    // check if different
    if (state.mpos_current.x != x || state.mpos_current.y != y) 
    {
        state.mpos_current.x = x;
        state.mpos_current.y = y;

        GDF_EventContext context;
        context.data.u16[0] = x;
        context.data.u16[1] = y;
        GDF_EventFire(GDF_EVENT_INTERNAL_MOUSE_MOVED, NULL, context);
    }
}

void __input_process_raw_mouse_move(i32 dx, i32 dy) 
{
    state.mouse_delta.x += dx;
    state.mouse_delta.y += dy;

    GDF_EventContext context;
    context.data.i16[0] = dx;
    context.data.i16[1] = dy;
    GDF_EventFire(GDF_EVENT_INTERNAL_MOUSE_RAW_MOVE, NULL, context);
}

void __input_process_mouse_wheel(i8 z_delta) 
{
    GDF_EventContext context;
    context.data.u8[0] = z_delta;
    GDF_EventFire(GDF_EVENT_INTERNAL_MOUSE_WHEEL, NULL, context);
}