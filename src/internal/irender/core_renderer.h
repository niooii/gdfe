#pragma once

#include <render/renderer.h>

#include "gdfe.h"

typedef struct ui_pipeline {
    VkPipeline handle;
    VkPipelineLayout layout;

    VkShaderModule vert;
    VkShaderModule frag;
} ui_pipeline;

typedef struct grid_pipeline {
    VkPipeline handle;
    VkPipelineLayout layout;

    VkShaderModule vert;
    VkShaderModule frag;
} grid_pipeline;

typedef struct post_process_pipeline {
    VkPipeline handle;
    VkPipelineLayout layout;

    VkShaderModule vert;
    VkShaderModule frag;
} post_process_pipeline;

typedef struct CoreRendererPerFrame {
    GDF_VkImage depth_image;
    GDF_VkImage msaa_image;

    VkFramebuffer geometry_framebuffer;
} CoreRendererPerFrame;

typedef struct CoreRendererContext {
    VkRenderPass geometry_pass;

    GDF_LIST(CoreRendererPerFrame) per_frame;

    grid_pipeline grid_pipeline;
    ui_pipeline ui_pipeline;

    GDF_VkBuffer up_facing_plane_vbo;
    GDF_VkBuffer up_facing_plane_index_buffer;

    // TODO! add a configuration structure for these effects
    struct {

    } post_process_pipelines;
} CoreRendererContext;

bool core_renderer_init(GDF_VkRenderContext* vk_ctx, CoreRendererContext* ctx);
bool core_renderer_draw(GDF_VkRenderContext* vk_ctx, GDF_AppCallbacks* callbacks, CoreRendererContext* ctx);
bool core_renderer_resize(GDF_VkRenderContext* vk_ctx, GDF_AppCallbacks* callbacks, CoreRendererContext* ctx);
bool core_renderer_destroy(GDF_VkRenderContext* vk_ctx, CoreRendererContext* ctx);