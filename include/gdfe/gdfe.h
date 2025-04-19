#pragma once
#include <gdfe/def.h>
#include <gdfe/render/renderer.h>

#ifdef GDF_C_COLLECTIONS

#include "icollections/hashmap.h"

#endif

typedef struct GDF_AppState {
    /// A handle to the opened window.
    /// @note May be null if rendering was disabled via configuration
    GDF_Window window;
    GDF_Renderer renderer;
    /// The maximum amount of updates per second. Updates may happen with lower frequency under high loads.
    /// @note May be freely changed at any time. Set to 0 to disable.
    u32 fps_cap;
    /// Whether the app is currently running or not.
    /// @note Setting this to \code GDF_FALSE\endcode will cause the application to exit after the current iteration.
    GDF_BOOL alive;
} GDF_AppState;

typedef struct GDF_RenderCallbacks {
    // Called after the core renderer is initialized.
    // Initialization of resources for custom rendering should happen here.
    GDF_BOOL (*on_render_init)(
        const GDF_VkRenderContext* vulkan_ctx,
        const GDF_AppState* app_state,
        void* state
    );
    // State to be passed into the on_render_init callback.
    void* on_render_init_state;

    // Called after the window is resized, and rendering components
    // have been resized.
    // If the core renderer is enabled, then the framebuffers and swapchain will
    // be recreated before calling this function. Otherwise, only
    // the swapchain will be recreated.
    // Recreation of sized resources for custom rendering should happen here.
    GDF_BOOL (*on_render_resize)(
        const GDF_VkRenderContext* vulkan_ctx,
        const GDF_AppState* app_state,
        void* state
    );
    // State to be passed into the on_render_resize callback.
    void* on_render_resize_state;

    // Called before the core renderer is destroyed.
    // Destruction of resources for custom rendering should happen here.
    GDF_BOOL (*on_render_destroy)(
        const GDF_VkRenderContext* vulkan_ctx,
        const GDF_AppState* app_state,
        void* state
    );
    // State to be passed into the on_render_destroy callback.
    void* on_render_destroy_state;

    // Called on the geometry pass, before lighting and postprocessing effects.
    // Custom rendering should happen here.
    GDF_BOOL (*on_render)(
        const GDF_VkRenderContext* vulkan_ctx,
        GDF_RENDER_MODE mode,
        const GDF_AppState* app_state,
        void* state
    );
    // State to be passed into the on_render callback.
    void* on_render_state;

    // Called right after the frame is presented, and the frame number is incremented.
    // However, resource_idx remains the same.
    GDF_BOOL (*on_render_end)(
        const GDF_VkRenderContext* vulkan_ctx,
        const GDF_AppState* app_state,
        void* state
    );
    // State to be passed into the on_render_end callback.
    void* on_render_end_state;
} GDF_RenderCallbacks;

typedef struct GDF_AppCallbacks {
    GDF_RenderCallbacks render_callbacks;
    // Called on each iteration of the application loop, before any rendering and after updating
    // the input subsystem.
    GDF_BOOL (*on_loop)(
        const GDF_AppState* app_state,
        f64 delta_time,
        void* state
    );
    // State to be passed into the on_loop callback.
    void* on_loop_state;

} GDF_AppCallbacks;

typedef struct GDF_Config {
    /// The maximum amount of updates per second. Updates may happen with lower frequency under high loads.
    /// @note May be freely changed at any time. Set to 0 to disable.
    u32 updates_per_sec;
    // Disable the internal quit event when pressing 'Esc' or closing the window.
    // Additionally disables the grave keybind for locking/unlocking the cursor.
    GDF_BOOL disable_default_events;
    // Disables everything video related, including input, windowing, rendering.
    // Good for minimal server applications
    GDF_BOOL disable_video;
} GDF_Config;

// Initialization info for the GDF engine.
// TODO! MSAA mode and mipmapping fields
typedef struct GDF_InitInfo {
    struct {
        u16 w;
        u16 h;
        u16 x;
        u16 y;
        const char* icon_path;
        const char* title;
    } window;

    GDF_AppCallbacks callbacks;
    GDF_Config config;

} GDF_InitInfo;

#ifdef __cplusplus
extern "C" {
#endif

GDF_BOOL GDF_InitSubsystems();

GDF_AppState* GDF_Init(GDF_InitInfo);

// Returns the number of seconds ran for, or -1 if something failed.
f64 GDF_Run();

#ifdef __cplusplus
}
#endif