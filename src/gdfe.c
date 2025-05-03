#include <gdfe/event.h>
#include <gdfe/gdfe.h>
#include <gdfe/os/socket.h>
#include <gdfe/os/misc.h>
#include <gdfe/os/thread.h>
#include <gdfe/os/video.h>
#include <i_video.h>
#include <gdfe/input.h>
#include "internal/i_render/renderer.h"
#include <i_subsystems.h>
#include <gdfe/strutils.h>

typedef struct GdfApp {
    i16 width;
    i16 height;
    f64 last_time;
    GDF_Stopwatch stopwatch;
    GDF_BOOL initialized;

    GDF_AppCallbacks callbacks;
    GDF_Config conf;
    GDF_AppState public;

    GDF_BOOL mouse_lock_toggle;
} GdfApp;

static GdfApp APP_STATE;

GDF_BOOL default_events(u16 event_code, void *sender, void *listener_instance, GDF_EventContext ctx)
{
    switch (event_code)
    {
        case GDF_EVENT_INTERNAL_KEY_PRESSED:
        {
            u16 key_code = ctx.data.u16[0];
            if (key_code == GDF_KEYCODE_ESCAPE)
            {
                GDF_EventContext tmp_ctx = {.data = 0};
                GDF_EventFire(GDF_EVENT_INTERNAL_APP_QUIT, NULL, tmp_ctx);
                return GDF_TRUE;
            }
            switch (key_code) {
                case GDF_KEYCODE_GRAVE:
                {
                    APP_STATE.mouse_lock_toggle = !APP_STATE.mouse_lock_toggle;
                    GDF_CURSOR_LOCK_STATE state = APP_STATE.mouse_lock_toggle ? GDF_CURSOR_LOCK_STATE_Locked : GDF_CURSOR_LOCK_STATE_Free;
                    GDF_SetMouseLockState(state);
                    LOG_DEBUG("TOGGLE MOUSE LOCK");
                    break;
                }
                case GDF_KEYCODE_V:
                {
                    GDF_RendererCycleRenderMode(APP_STATE.public.renderer);
                }
            }
            break;
        }

        case GDF_EVENT_INTERNAL_APP_QUIT:
        {
            LOG_INFO("Shutting down app...");
            APP_STATE.public.alive = GDF_FALSE;
            // dont return GDF_TRUE as other listeners may want to destroy resources
            // on app quit
            return GDF_FALSE;
        }
    }

    return GDF_FALSE;
}

GDF_BOOL on_resize(u16 event_code, void *sender, void *listener_instance, GDF_EventContext ctx)
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
    return GDF_FALSE;
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

static GDF_BOOL SUBSYS_INITIALIZED;
GDF_BOOL GDF_InitSubsystems()
{
    gdfe_mem_init();
    gdfe_io_init();
    gdfe_misc_init();
    if (!gdfe_events_init())
        return GDF_FALSE;
    if (!gdfe_sock_init() || !gdfe_logging_init())
        return GDF_FALSE;
    if (!GDF_InitThreadLogging("Main"))
        return GDF_FALSE;

    // create the .gdfe storage directory
    GDF_StringBuilder dir;
    GDF_InitStringBuilder(&dir);
    GDF_PushFormat(&dir, "%s/%s", GDF_GetExecutablePath(), GDFE_STORAGE_ROOT);
    GDF_IO_RESULT res = GDF_MakeDirAbs(dir.str);
    if (res != GDF_IO_RESULT_SUCCESS && res != GDF_IO_RESULT_ALREADY_EXISTS)
    {
        GDF_DestroyStringBuilder(&dir);
        return GDF_FALSE;
    }
    GDF_DestroyStringBuilder(&dir);

    SUBSYS_INITIALIZED = GDF_TRUE;
    return GDF_TRUE;
}

GDF_AppState* GDF_Init(GDF_InitInfo init_info) {
    if (!SUBSYS_INITIALIZED && !GDF_InitSubsystems())
        return NULL;

    if (APP_STATE.initialized)
    {
        LOG_WARN("Cannot initialize twice - this will be added in the future.");
        return &APP_STATE.public;
    }

    set_defaults(&init_info);
    APP_STATE.callbacks = init_info.callbacks;
    APP_STATE.conf = init_info.config;

    APP_STATE.stopwatch = GDF_StopwatchCreate();

    APP_STATE.initialized = GDF_TRUE;

    if (init_info.config.disable_video)
        return &APP_STATE.public;

    if (!gdfe_windowing_init())
        return NULL;
    gdfe_input_init();

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

    public->renderer = gdfe_renderer_init(public->window, &APP_STATE.public, &APP_STATE.callbacks);
    if (!public->renderer)
    {
        LOG_ERR("Couldn't initialize renderer unlucky.");
        return NULL;
    }

    GDF_EventRegister(GDF_EVENT_INTERNAL_WINDOW_RESIZE, public->renderer, on_resize);

    return public;
}

f64 GDF_Run() {
    if (!APP_STATE.initialized)
    {
        LOG_ERR("App not initialized properly");
        return -1;
    }
    GDF_AppState* public = &APP_STATE.public;
    public->alive = GDF_TRUE;

    const GDF_Stopwatch running_timer = GDF_StopwatchCreate();
    u32 fps = APP_STATE.conf.updates_per_sec;
    f64 secs_per_frame = fps != 0 ? 1.0/fps : 0.0;
    const GDF_Stopwatch frame_timer = GDF_StopwatchCreate();

    // periodically print average fps
    const u32 frame_times_sample_size = 10;
    f64 frame_times[frame_times_sample_size] = {};
    u64 last_whole_second = 0;
    u64 frame_count = 0;

    while(public->alive)
    {
        const f64 current_time = GDF_StopwatchElapsed(APP_STATE.stopwatch);
        const f64 dt = (current_time - APP_STATE.last_time);
        APP_STATE.last_time = current_time;
        GDF_StopwatchReset(frame_timer);

        // grabs the latest input states
        pump_messages();

        if (APP_STATE.callbacks.on_loop) {
            if (
                !APP_STATE.callbacks.on_loop(
                &APP_STATE.public,
                dt,
                APP_STATE.callbacks.on_loop_state
            )) {
                return -1;
            }
        }

        if (!APP_STATE.conf.disable_video)
            GDF_RendererDrawFrame(public->renderer, dt);

        gdfe_input_update(public->window, dt);

        // only thing that should produce innacuracies is if pumpmessages takes a bit of time
        frame_count++;

        // wait a certain amount of time after each frame to cap fps
        const f64 frame_time = GDF_StopwatchElapsed(frame_timer);

        f64 overflow = GDF_StopwatchSleepUntil(frame_timer, secs_per_frame);
        if (overflow > 0)
        {
            frame_times[frame_count % frame_times_sample_size]
            = frame_time;
        }
        else
        {
            frame_times[frame_count % frame_times_sample_size]
            = secs_per_frame;
        }

        // // temporary fps diagnostics, remove later.
        // if (last_whole_second != (u32)current_time)
        // {
        //     f64 avg_frametime = 0;
        //     for (u32 i = 0; i < frame_times_sample_size; i++)
        //     {
        //         avg_frametime += frame_times[i];
        //     }
        //     avg_frametime /= frame_times_sample_size;
        //     // LOG_INFO("Avg frame time: %lfs, FPS: %lf", avg_frametime, 1.0/avg_frametime);
        //     last_whole_second = (u32)current_time;
        // }
    }

    // CLEAN UP STUFF
    f64 time_ran_for = GDF_StopwatchElapsed(running_timer);
    GDF_StopwatchDestroy(APP_STATE.stopwatch);
    GDF_StopwatchDestroy(running_timer);
    GDF_StopwatchDestroy(frame_timer);

    gdfe_renderer_destroy(APP_STATE.public.renderer);

    return time_ran_for;
}