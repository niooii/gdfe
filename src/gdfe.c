#include <event.h>
#include <gdfe.h>
#include <os/socket.h>
#include <os/sysinfo.h>
#include <os/thread.h>
#include <os/window.h>
#include "os/window_internal.h"
#include <input.h>

typedef struct AppState {
    i16 width;
    i16 height;
    f64 last_time;
    GDF_Stopwatch stopwatch;
    bool initialized;

    GDF_AppCallbacks callbacks;
    GDF_Config conf;
    GDF_AppState public;
} AppState;

static AppState APP_STATE;

bool default_events(u16 event_code, void *sender, void *listener_instance, GDF_EventContext ctx)
{
    switch (event_code)
    {
        case GDF_EVENT_INTERNAL_KEY_PRESSED:
        {
            u16 key_code = ctx.data.u16[0];
            LOG_DEBUG("KEY PRESSED: %u", key_code);
            if (key_code == GDF_KEYCODE_ESCAPE)
            {
                GDF_EventContext tmp_ctx = {.data = 0};
                GDF_EventFire(GDF_EVENT_INTERNAL_APP_QUIT, NULL, tmp_ctx);
                return true;
            }
            switch (key_code) {
                case GDF_KEYCODE_GRAVE:
                {
                    // mouse_lock_toggle = !mouse_lock_toggle;
                    // GDF_CURSOR_LOCK_STATE state = mouse_lock_toggle ? GDF_CURSOR_LOCK_STATE_Locked : GDF_CURSOR_LOCK_STATE_Free;
                    // GDF_SetMouseLockState(state);
                    LOG_DEBUG("TOGGLE MOUSE LOCK");
                }
            }
            break;
        }
        case GDF_EVENT_INTERNAL_APP_QUIT:
        {
            LOG_INFO("Shutting down app...");
            APP_STATE.public.alive = false;
            // dont return true as other listeners may want to destroy resources
            // on app quit
            return false;
        }
    }

    return false;
}

bool on_resize(u16 event_code, void *sender, void *listener_instance, GDF_EventContext ctx)
{
    GDF_Renderer renderer = listener_instance;
    u16 width = ctx.data.u16[0];
    u16 height = ctx.data.u16[1];

    // check if different bc i need to resize renderer and whatnot

    LOG_DEBUG("Window resize: %i, %i", width, height);

    // Handle minimization
    if (width == 0 || height == 0)
    {
        LOG_INFO("Window is minimized kinda.");
    }
    GDF_RendererResize(renderer, width, height);
    // TODO! why doesn this work?
    // TODO! im an idiot i know why it doesnt work
    // if (width != old_w || height != old_h)
    // {
    //     GDF_SetWindowSizeInternal(width, height);

    //     LOG_DEBUG("Window resize: %i, %i", width, height);

    //     // Handle minimization
    //     if (width == 0 || height == 0)
    //     {
    //         LOG_INFO("Window is minimized kinda.");
    //     }
    //     renderer_resize(width, height);
    // }
    return false;
}

void set_defaults(GDF_InitInfo* info) {
    GDF_DisplayInfo display_info;
    GDF_GetDisplayInfo(&display_info);

    if (info->window.w == 0)
        info->window.w = display_info.screen_width * 0.45;
    if (info->window.h == 0)
        info->window.h = display_info.screen_height * 0.5;

    if (info->window.x == 0)
        info->window.x = GDF_WIN_CENTERED;
    if (info->window.y == 0)
        info->window.y = GDF_WIN_CENTERED;

    if (!info->window.title)
        info->window.title = "Not a GDF";
}

bool GDF_Init(GDF_InitInfo init_info) {
    if (APP_STATE.initialized)
    {
        return true;
    }

    set_defaults(&init_info);
    APP_STATE.callbacks = init_info.callbacks;
    APP_STATE.conf = init_info.config;

    GDF_InitMemory();
    GDF_InitIO();
    if (!GDF_InitSysinfo())
        return false;
    if (!GDF_InitWindowing())
        return false;
    if (!GDF_InitEvents())
                return false;
    GDF_InitInput();
    if (!GDF_InitSockets() || !GDF_InitLogging())
        return false;
    if (!GDF_InitThreadLogging("Main"))
        return false;

    GDF_AppState* public = &APP_STATE.public;
    public->window = GDF_CreateWindow(
        init_info.window.x,
        init_info.window.y,
        init_info.window.w,
        init_info.window.h,
        init_info.window.title
    );

    if (public->window != NULL)
        LOG_INFO("Created window.");

    if (!APP_STATE.conf.disable_default_events) {
        GDF_EventRegister(GDF_EVENT_INTERNAL_KEY_PRESSED, NULL, default_events);
        GDF_EventRegister(GDF_EVENT_INTERNAL_APP_QUIT, NULL, default_events);
    }

    public->renderer = GDF_RendererInit(public->window);
    if (!public->renderer)
    {
        LOG_ERR("Couldn't initialize renderer unlucky.");
        return false;
    }

    GDF_EventRegister(GDF_EVENT_INTERNAL_WINDOW_RESIZE, public->renderer, on_resize);

    APP_STATE.stopwatch = GDF_StopwatchCreate();

    APP_STATE.initialized = true;

    return true;
}

f64 GDF_Run() {
    if (!APP_STATE.initialized)
    {
        LOG_ERR("App not initialized properly");
        return -1;
    }
    GDF_AppState* public = &APP_STATE.public;
    public->alive = true;

    GDF_Stopwatch running_timer = GDF_StopwatchCreate();
    u8 frame_count = 0;
    u32 fps = 3000;
    f64 secs_per_frame = 1.0/fps;
    GDF_Stopwatch frame_timer = GDF_StopwatchCreate();

    while(public->alive)
    {
        pump_messages();

        f64 current_time = GDF_StopwatchElasped(APP_STATE.stopwatch);
        f64 dt = (current_time - APP_STATE.last_time);
        GDF_StopwatchReset(frame_timer);

        // just testing input
        if (GDF_IsButtonDown(GDF_MBUTTON_LEFT))
        {
            LOG_INFO("heehaw");
        }

        if (APP_STATE.callbacks.on_frame) {
            if (
                !APP_STATE.callbacks.on_frame(
                &APP_STATE.public,
                dt,
                APP_STATE.callbacks.on_frame_state
            )) {
                return -1;
            }
        }

        // GDF_RendererDrawFrame(public->renderer, dt);

        f64 frame_time = GDF_StopwatchElasped(frame_timer);

        // wait a certain amount of time after each frame to cap fps
        // TODO! add uncap fps option probably "bool uncap_fps" or some shit
        f64 wait_secs = secs_per_frame - frame_time;
        if (wait_secs > 0)
        {
            GDF_ThreadSleep((u64)(wait_secs * 1000));
        }

        GDF_INPUT_Update(public->window, dt);

        // only thing that should produce innacuracies is if pumpmessages takes a bit of time
    }

    // CLEAN UP STUFF
    f64 time_ran_for = GDF_StopwatchElasped(running_timer);
    // destroy resources
    GDF_StopwatchDestroy(APP_STATE.stopwatch);
    GDF_StopwatchDestroy(running_timer);

    GDF_RendererDestroy(APP_STATE.public.renderer);

    return time_ran_for;
}