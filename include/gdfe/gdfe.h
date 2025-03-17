#pragma once
#include <gdfe/def.h>
#include <gdfe/render/renderer.h>

#ifdef GDF_C_COLLECTIONS

#include "icollections/hashmap.h"

#endif

typedef struct GDF_AppState {
    GDF_Window window;
    GDF_Renderer renderer;
    u32 fps_cap;
    // Set this to GDF_FALSE to kill the app.
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
    // Called on each frame, before any rendering and after updating
    // the input subsystem.
    GDF_BOOL (*on_frame)(
        const GDF_AppState* app_state,
        f64 delta_time,
        void* state
    );
    // State to be passed into the on_frame callback.
    void* on_frame_state;

} GDF_AppCallbacks;

typedef struct GDF_Config {
    // Set to 0 for no cap
    u32 fps_cap;
    // Disable the internal quit event when pressing 'Esc' or closing the window.
    // Additionally disables the grave keybind for locking/unlocking the cursor.
    GDF_BOOL disable_default_events;
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

GDF_AppState* GDF_Init(GDF_InitInfo);

// Returns the number of seconds ran for, or -1 if something failed.
f64 GDF_Run();

#ifdef __cplusplus
}
#endif