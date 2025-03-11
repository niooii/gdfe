#pragma once

#include <render/renderer.h>

#include "gdfe.h"

typedef struct gdfe_ui_pipeline {
    VkPipeline handle;
    VkPipelineLayout layout;

    VkShaderModule vert;
    VkShaderModule frag;
} gdfe_ui_pipeline;

typedef struct gdfe_grid_pipeline {
    VkPipeline handle;
    VkPipelineLayout layout;

    VkShaderModule vert;
    VkShaderModule frag;
} gdfe_grid_pipeline;

typedef struct CoreRendererPerFrame {
    GDF_VkImage depth_image;
    GDF_VkImage msaa_image;

    VkFramebuffer geometry_framebuffer;

    GDF_VkUniformBuffer vp_ubo;
    VkDescriptorSet vp_ubo_set;
} CoreRendererPerFrame;

typedef struct GDF_CoreRendererContext {
    VkRenderPass geometry_pass;

    // This field is modified then copied over to vk_uniform_buffer[n].mapped_Data
    ViewProjUB view_proj_ub;
    VkDescriptorPool vp_ubo_pool;
    VkDescriptorSetLayout vp_ubo_layout;
    GDF_LIST(CoreRendererPerFrame) per_frame;

    gdfe_grid_pipeline grid_pipeline;
    gdfe_ui_pipeline ui_pipeline;

    // TODO! why do i even need this for a debug grid plane thingy just generate
    // vertices on the fly
    GDF_VkBuffer up_facing_plane_vbo;
    GDF_VkBuffer up_facing_plane_index_buffer;

    // All vertex shaders will get input from these uniform buffers.


    // TODO! add a configuration structure for these effects
    struct {

    } post_process_pipelines;
} GDF_CoreRendererContext;

bool core_renderer_init(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx);
bool core_renderer_draw(GDF_VkRenderContext* vk_ctx, GDF_AppCallbacks* callbacks, GDF_CoreRendererContext* ctx);
bool core_renderer_resize(GDF_VkRenderContext* vk_ctx, GDF_AppCallbacks* callbacks, GDF_CoreRendererContext* ctx);
bool core_renderer_destroy(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx);