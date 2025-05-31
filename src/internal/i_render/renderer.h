#pragma once

#include <gdfe/gdfe.h>
#include <gdfe/math/math.h>
#include <gdfe/os/video.h>
#include <gdfe/render/renderer.h>
#include <gdfe/render/geometry.h>
#include "core_renderer.h"

typedef struct gdfe_render_state {
    u64 frame_number;

    // Camera (and view and projection) stuff
    u32 framebuffer_width;
    u32 framebuffer_height;

    GDF_RENDER_MODE render_mode;

    GDF_VkRenderContext     vk_ctx;
    GDF_RenderCallbacks*    callbacks;
    GDF_AppState*           app_state;
    GDF_CoreRendererContext core_ctx;
} gdfe_render_state;

typedef struct GDF_Object_T {
    GDF_Transform transform;

    /// The index to use to index into the instance data buffer.
    /// This does not reflect the instance id in the shader.
    u64 instance_index;
    GDF_Mesh mesh;
} GDF_Object_T;

extern gdfe_render_state        GDFE_RENDER_STATE;
extern GDF_VkRenderContext*     GDFE_VK_CTX;
extern GDF_CoreRendererContext* GDFE_CORE_CTX;
