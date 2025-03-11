#include <camera.h>

#include <math/math.h>
#include <os/window.h>
#include <render/renderer.h>
#include <gdfe.h>

#include "core_renderer.h"

typedef struct GDF_Renderer_T {
    u64 frame_number;

    // Camera (and view and projection) stuff
    u32 framebuffer_width;
    u32 framebuffer_height;

    GDF_RENDER_MODE render_mode;

    GDF_Camera* camera;

    GDF_VkRenderContext vk_ctx;
    GDF_AppCallbacks* callbacks;
    GDF_AppState* app_state;
    GDF_CoreRendererContext core_renderer;
    bool disable_core;
} GDF_Renderer_T;

GDF_Renderer gdfe_renderer_init(
    GDF_Window window,
    GDF_AppState* app_state,
    bool disable_default,
    GDF_AppCallbacks* callbacks
);

void gdfe_renderer_destroy(GDF_Renderer renderer);