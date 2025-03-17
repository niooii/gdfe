#include <gdfe/camera.h>

#include <gdfe/math/math.h>
#include <gdfe/os/window.h>
#include <gdfe/render/renderer.h>
#include <gdfe/gdfe.h>

#include "core_renderer.h"

typedef struct GDF_Renderer_T {
    u64 frame_number;

    // Camera (and view and projection) stuff
    u32 framebuffer_width;
    u32 framebuffer_height;

    GDF_RENDER_MODE render_mode;

    GDF_Camera* camera;

    GDF_VkRenderContext vk_ctx;
    GDF_RenderCallbacks* callbacks;
    GDF_AppState* app_state;
    GDF_CoreRendererContext core_renderer;
} GDF_Renderer_T;

typedef enum RENDER_OBJ_TYPE {
    RENDER_OBJ_TYPE_LINE,
    RENDER_OBJ_TYPE_AABB,
} RENDER_OBJ_TYPE;

typedef struct GDF_RenderHandle_T {
    RENDER_OBJ_TYPE type;
    void* data;
} GDF_RenderHandle_T;

extern GDF_VkRenderContext* GDFE_INTERNAL_VK_CTX;
extern GDF_CoreRendererContext* GDFE_INTERNAL_CORE_CTX;

GDF_Renderer gdfe_renderer_init(
    GDF_Window window,
    GDF_AppState* app_state,
    GDF_AppCallbacks* callbacks
);

void gdfe_renderer_destroy(GDF_Renderer renderer);