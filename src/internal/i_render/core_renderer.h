#pragma once

#include <gdfe/render/renderer.h>

#include "gdfe/gdfe.h"

typedef struct gdfe_ui_pipeline {
    VkPipeline       handle;
    VkPipelineLayout layout;

    VkShaderModule vert;
    VkShaderModule frag;
} gdfe_ui_pipeline;

typedef struct gdfe_grid_pipeline {
    VkPipeline       handle;
    VkPipelineLayout layout;

    VkShaderModule vert;
    VkShaderModule frag;
} gdfe_grid_pipeline;

typedef struct CoreRendererPerFrame {
    GDF_VkImage depth_image;
    GDF_VkImage msaa_image;

    VkFramebuffer geometry_framebuffer;
} CoreRendererPerFrame;

typedef struct GDF_CoreRendererContext {
    GDF_LIST(CoreRendererPerFrame) per_frame;

    GDF_Camera active_camera;

    gdfe_grid_pipeline grid_pipeline;
    gdfe_ui_pipeline   ui_pipeline;

    // TODO! why do i even need this for a debug grid plane thingy just generate
    // vertices on the fly
    GDF_VkBuffer up_facing_plane_vbo;
    GDF_VkBuffer up_facing_plane_index_buffer;

    // TODO! add a configuration structure for these effects
    struct {

    } post_process_pipelines;
} GDF_CoreRendererContext;

GDF_BOOL core_renderer_init(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx);

GDF_BOOL core_renderer_draw(
    GDF_Renderer renderer, GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx);

GDF_BOOL core_renderer_resize(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx);

GDF_BOOL core_renderer_destroy(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx);
