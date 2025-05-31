#pragma once

#include <gdfe/render/renderer.h>

#include "gdfe/gdfe.h"

typedef struct GDF_Shader_T* GDF_Shader;

typedef struct gdfe_ui_pipeline {
    GDF_VkPipelineBase base;

    GDF_Shader vert;
    GDF_Shader frag;
} gdfe_ui_pipeline;

typedef struct gdfe_grid_pipeline {
    GDF_VkPipelineBase base;

    GDF_Shader vert;
    GDF_Shader frag;
} gdfe_grid_pipeline;

typedef struct gdfe_object_pipeline {
    GDF_VkPipelineBase wireframe_base;
    GDF_VkPipelineBase base;

    GDF_Shader vert;
    GDF_Shader frag;
} gdfe_object_pipeline;

typedef struct CoreFrameResources {
    GDF_VkImage depth_image;
    GDF_VkImage msaa_image;
} CoreFrameResources;

typedef struct GDF_CoreRendererContext {
    GDF_LIST(CoreFrameResources) per_frame;

    GDF_Camera active_camera;

    gdfe_grid_pipeline grid_pipeline;
    gdfe_ui_pipeline   ui_pipeline;
    gdfe_object_pipeline object_pipeline;

    // TODO! add a configuration structure for these effects
    struct {

    } post_process_pipelines;

    // TODO! super naive implementation do instanced rendering
    // GDF_LIST(GDF_Object) objects;
    GDF_LIST(GDF_Mesh) meshes;
} GDF_CoreRendererContext;

GDF_BOOL core_renderer_init(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx);

GDF_BOOL core_renderer_draw(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx);

GDF_BOOL core_renderer_resize(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx);

GDF_BOOL core_renderer_destroy(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx);
