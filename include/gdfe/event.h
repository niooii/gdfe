#pragma once

#include <gdfe/core.h>

typedef struct GDF_EventContext {
    // 196 bytes
    union {
        i64 i64[2];
        u64 u64[2];
        f64 f64[2];

        i32 i32[4];
        u32 u32[4];
        f32 f32[4];

        i16 i16[8];
        u16 u16[8];

        i8 i8[16];
        u8 u8[16];

        GDF_BOOL b;

        char c[16];
    } data;
} GDF_EventContext;

typedef GDF_BOOL (*GDF_EventHandlerFP)(u16 event_code, void* sender, void* listener_instance, GDF_EventContext ctx);

#ifdef __cplusplus
extern "C" {
#endif

GDF_BOOL GDF_InitEvents();
void GDF_ShutdownEvents();

GDF_BOOL GDF_EventRegister(u32 event_code, void* listener, GDF_EventHandlerFP callback);
GDF_BOOL GDF_EventUnregister(u32 event_code, void* listener, GDF_EventHandlerFP callback);
GDF_BOOL GDF_EventFire(u32 event_code, void* sender, GDF_EventContext ctx);

#ifdef __cplusplus
}
#endif


typedef enum GDF_EVENT_INTERNAL {
    
    GDF_EVENT_INTERNAL_APP_QUIT = 0x01,
    /*
    Usage:
    u16 keycode = ctx.data.u16[0]
    */
    GDF_EVENT_INTERNAL_KEY_PRESSED,
    /*
    Usage:
    u16 keycode = ctx.data.u16[0]
    */
    GDF_EVENT_INTERNAL_KEY_RELEASED,
    /*
    Usage:
    u16 button = ctx.data.u16[0]
    */
    GDF_EVENT_INTERNAL_MBUTTON_PRESSED,
    /*
    Usage:
    u16 button = ctx.data.u16[0]
    */
    GDF_EVENT_INTERNAL_MBUTTON_RELEASED,
    /*
    Usage:
    u16 x = ctx.data.u16[0]
    u16 y = ctx.data.u16[1]
    */
    GDF_EVENT_INTERNAL_MOUSE_MOVED,
    /*
    Usage:
    u16 dy = ctx.data.u8[0]
    */
    GDF_EVENT_INTERNAL_MOUSE_WHEEL,
    /*
    Usage:
    u16 dx = ctx.data.i16[0]
    u16 dy = ctx.data.i16[1]
    */
    GDF_EVENT_INTERNAL_MOUSE_RAW_MOVE,
    /*
    Usage:
    u16 w = ctx.data.u16[0]
    u16 h = ctx.data.u16[1]
    */
    GDF_EVENT_INTERNAL_WINDOW_RESIZE,
    /*
    Usage:
    GDF_BOOL focus_gained = ctx.data.b;
    GDF_BOOL focus_lost = !ctx.data.b;
    */
    GDF_EVENT_INTERNAL_WINDOW_FOCUS_CHANGE,
    /*
    Usage:
    i16 x = ctx.data.i16[0];
    i16 y = ctx.data.i16[1];
    */
    GDF_EVENT_INTERNAL_WINDOW_MOVE,

    // Not actaully an event.
    GDF_EVENT_INTERNAL_MAX = 0xFF

} GDF_EVENT_INTERNAL; 