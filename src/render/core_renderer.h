#pragma once

#include <render/renderer.h>

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

typedef struct CoreRendererContext {
    grid_pipeline grid_pipeline;
    ui_pipeline ui_pipeline;

    // TODO! add a configuration structure for these effects
    struct {

    } post_process_pipelines;
} CoreRendererContext;

bool core_renderer_init(VkRenderContext* vk_ctx, CoreRendererContext* ctx);
bool core_renderer_destroy(VkRenderContext* vk_ctx, CoreRendererContext* ctx);