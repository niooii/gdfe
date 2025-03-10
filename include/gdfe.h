#pragma once
#include <def.h>
#include <render/renderer.h>

typedef struct GDF_AppState {
    GDF_Window window;
    GDF_Renderer renderer;
    u32 fps_cap;
    // Set this to false to kill the app.
    bool alive;
} GDF_AppState;

typedef struct GDF_AppCallbacks {
    // Called after the core renderer is initialized.
    // Initialization of resources for custom rendering should happen here.
    bool (*on_render_init)(
        const VkRenderContext* vulkan_ctx,
        const GDF_AppState* app_state,
        void* state
    );
    // State to be passed into the on_render_init callback.
    void* on_render_init_state;

    // Called before the core renderer is destroyed.
    // Destruction of resources for custom rendering should happen here.
    bool (*on_render_destroy)(
        const VkRenderContext* vulkan_ctx,
        const GDF_AppState* app_state,
        void* state
    );
    // State to be passed into the on_render_destroy callback.
    void* on_render_destroy_state;

    // Called right before postprocessing effects are rendered.
    // Custom rendering should happen here.
    bool (*on_render)(
        const VkRenderContext* vulkan_ctx,
        const GDF_AppState* app_state,
        void* state
    );
    // State to be passed into the on_render callback.
    void* on_render_state;

    // Called right after the frame is presented, and the frame number is incremented.
    // However, resource_idx remains the same.
    bool (*on_render_end)(
        const VkRenderContext* vulkan_ctx,
        const GDF_AppState* app_state,
        void* state
    );
    // State to be passed into the on_render_end callback.
    void* on_render_end_state;

    // Called on each frame, before any rendering.
    bool (*on_frame)(
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
    bool disable_default_events;
    // If set, the default renderer will not be initialized, including any
    // vulkan stuff. The pointer to the render state passed in many callbacks
    // will be NULL. 
    bool disable_default_renderer;    
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

bool GDF_Init(GDF_InitInfo);

// Returns the number of seconds ran for, or -1 if something failed.
f64 GDF_Run();