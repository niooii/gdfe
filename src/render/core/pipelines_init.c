#include "gdfe/render/vk/pipelines_init.h"

#include <gdfe/render/vk/utils.h>
#include <i_render/core_renderer.h>
#include "gdfe/render/shader.h"
#include "gdfe/render/vk/types.h"

GDF_BOOL cr_create_grid_pipeline(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx)
{
    // Vertex input configuration
    VkVertexInputBindingDescription bindings = {
        .binding   = 0,
        .stride    = sizeof(GDF_MeshVertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };

    VkVertexInputAttributeDescription attributes[] = {
        {
            .binding  = 0,
            .location = 0,
            .format   = VK_FORMAT_R32G32B32_SFLOAT,
            .offset   = offsetof(GDF_MeshVertex, pos),
        },
    };

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {
        .sType                         = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions    = &bindings,
        .vertexAttributeDescriptionCount = sizeof(attributes) / sizeof(attributes[0]),
        .pVertexAttributeDescriptions    = attributes,
    };

    // Input assembly configuration
    VkPipelineInputAssemblyStateCreateInfo input_assembly = {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .primitiveRestartEnable = VK_FALSE,
        .topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    };

    // Viewport and scissor configuration (dynamic states)
    VkPipelineViewportStateCreateInfo viewport_state = {
        .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount  = 1,
    };

    // Rasterization configuration
    VkPipelineRasterizationStateCreateInfo rasterizer_state = {
        .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable        = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode             = VK_POLYGON_MODE_FILL,
        .lineWidth               = 1.0f,
        .cullMode                = VK_CULL_MODE_NONE,
        .frontFace               = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable         = VK_FALSE,
    };

    // Multisampling configuration
    VkPipelineMultisampleStateCreateInfo multisampling_state = {
        .sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable  = VK_FALSE,
        .rasterizationSamples = vk_ctx->msaa_samples,
    };

    // Depth stencil configuration
    VkPipelineDepthStencilStateCreateInfo depth_stencil_state = {
        .sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable       = VK_TRUE,
        .depthWriteEnable      = VK_FALSE,
        .depthCompareOp        = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable     = VK_FALSE,
    };

    // Color blending configuration
    VkPipelineColorBlendAttachmentState color_blend_attachment = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable         = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp        = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp        = VK_BLEND_OP_ADD,
    };

    VkPipelineColorBlendStateCreateInfo color_blend_state = {
        .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable   = VK_FALSE,
        .attachmentCount = 1,
        .pAttachments    = &color_blend_attachment,
        .blendConstants  = { 0.0f, 0.0f, 0.0f, 0.0f },
    };

    // for player position
    VkPushConstantRange push_constant_range = {
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .offset     = 0,
        .size       = sizeof(vec3),
    };

    // Pipeline layout
    VkPipelineLayoutCreateInfo layout_info = {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pSetLayouts            = &vk_ctx->vp_ubo_layout,
        .setLayoutCount         = 1,
        .pPushConstantRanges    = &push_constant_range,
        .pushConstantRangeCount = 1,
    };

    VkPipelineLayout pipeline_layout;

    VK_ASSERT(vkCreatePipelineLayout(
        vk_ctx->device.handle, &layout_info, vk_ctx->device.allocator, &pipeline_layout))

    // Create dynamic state for pipeline (viewport & scissor)
    // TODO! eventually if the game is fixed size remove these and bake states
    // into pipelines
    VkDynamicState d_states[2] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };

    VkPipelineDynamicStateCreateInfo dynamic_states = {
        .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 2,
        .pDynamicStates    = d_states,
    };

    ctx->grid_pipeline.vert = GDF_ShaderLoadGLSLFromFile(
        "resources/shaders/grid.vert", GDF_SHADER_TYPE_VERT, GDF_SHADER_RELOAD_NONE);
    ctx->grid_pipeline.frag = GDF_ShaderLoadGLSLFromFile(
        "resources/shaders/grid.frag", GDF_SHADER_TYPE_FRAG, GDF_SHADER_RELOAD_NONE);

    VkPipelineShaderStageCreateInfo grid_shaders[] = {
        {
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage  = VK_SHADER_STAGE_VERTEX_BIT,
            .module = GDF_ShaderGetVkModule(ctx->grid_pipeline.vert),
            .pName  = "main",
        },
        {
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = GDF_ShaderGetVkModule(ctx->grid_pipeline.frag),
            .pName  = "main",
        }
    };

    // Info for dynamic renderign
    VkPipelineRenderingCreateInfo dyn_rendering_info = {};
    GDF_VkPipelineInitRenderingInfo(vk_ctx, &dyn_rendering_info);

    // Put all configuration in graphics pipeline info struct
    VkGraphicsPipelineCreateInfo pipeline_create_info = {
        .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount          = sizeof(grid_shaders) / sizeof(VkPipelineShaderStageCreateInfo),
        .pStages             = grid_shaders,
        .pVertexInputState   = &vertex_input_info,
        .pInputAssemblyState = &input_assembly,
        .pViewportState      = &viewport_state,
        .pRasterizationState = &rasterizer_state,
        .pMultisampleState   = &multisampling_state,
        .pDepthStencilState  = &depth_stencil_state,
        .pColorBlendState    = &color_blend_state,
        .layout              = pipeline_layout,
        .pNext               = &dyn_rendering_info,
        .pDynamicState       = &dynamic_states,
    };

    GDF_VkPipelineCreateInfo info = {
        .create_info.graphics = pipeline_create_info,
        .type                 = GDF_PIPELINE_TYPE_GRAPHICS,
    };

    GDF_VkCreatePipelineBase(&ctx->grid_pipeline.base, info);

    return GDF_TRUE;
}

void cr_destroy_grid_pipeline(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx)
{
    vkDestroyPipeline(
        vk_ctx->device.handle, ctx->grid_pipeline.base.handle, vk_ctx->device.allocator);
    vkDestroyPipelineLayout(
        vk_ctx->device.handle, ctx->grid_pipeline.base.layout, vk_ctx->device.allocator);
    vkDestroyShaderModule(
        vk_ctx->device.handle, ctx->grid_pipeline.base.vert, vk_ctx->device.allocator);
    vkDestroyShaderModule(
        vk_ctx->device.handle, ctx->grid_pipeline.base.frag, vk_ctx->device.allocator);
}

GDF_BOOL cr_create_obj_pipeline(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx)
{
    // Vertex input configuration
    VkVertexInputBindingDescription bindings = {
        .binding   = 0,
        .stride    = sizeof(GDF_MeshVertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };

    VkVertexInputAttributeDescription attributes[] = {
        {
            .binding  = 0,
            .location = 0,
            .format   = VK_FORMAT_R32G32B32_SFLOAT,
            .offset   = offsetof(GDF_MeshVertex, pos),
        },
        {
            .binding  = 0,
            .location = 1,
            .format   = VK_FORMAT_R32G32B32_SFLOAT,
            .offset   = offsetof(GDF_MeshVertex, normal),
        },
    };

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {
        .sType                         = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions    = &bindings,
        .vertexAttributeDescriptionCount = sizeof(attributes) / sizeof(*attributes),
        .pVertexAttributeDescriptions    = attributes,
    };

    // Input assembly configuration
    VkPipelineInputAssemblyStateCreateInfo input_assembly = {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .primitiveRestartEnable = VK_FALSE,
        .topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    };

    // Viewport and scissor configuration (dynamic states)
    VkPipelineViewportStateCreateInfo viewport_state = {
        .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount  = 1,
    };

    // Rasterization configuration
    VkPipelineRasterizationStateCreateInfo rasterizer_state = {
        .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable        = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode             = VK_POLYGON_MODE_FILL,
        .lineWidth               = 1.0f,
        .cullMode                = VK_CULL_MODE_BACK_BIT,
        .frontFace               = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable         = VK_FALSE,
    };

    // Multisampling configuration
    VkPipelineMultisampleStateCreateInfo multisampling_state = {
        .sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable  = VK_FALSE,
        .rasterizationSamples = vk_ctx->msaa_samples,
    };

    // Depth stencil configuration
    VkPipelineDepthStencilStateCreateInfo depth_stencil_state = {
        .sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable       = VK_TRUE,
        .depthWriteEnable      = VK_TRUE,
        .depthCompareOp        = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable     = VK_FALSE,
    };

    // Color blending configuration
    VkPipelineColorBlendAttachmentState color_blend_attachment = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable         = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp        = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp        = VK_BLEND_OP_ADD,
    };

    VkPipelineColorBlendStateCreateInfo color_blend_state = {
        .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable   = VK_FALSE,
        .attachmentCount = 1,
        .pAttachments    = &color_blend_attachment,
        .blendConstants  = { 0.0f, 0.0f, 0.0f, 0.0f },
    };

    // for player position
    VkPushConstantRange push_constant_range = {
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .offset     = 0,
        .size       = sizeof(mat4),
    };

    // Pipeline layout
    VkPipelineLayoutCreateInfo layout_info = {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pSetLayouts            = &vk_ctx->vp_ubo_layout,
        .setLayoutCount         = 1,
        .pPushConstantRanges    = &push_constant_range,
        .pushConstantRangeCount = 1,
    };

    VkPipelineLayout pipeline_layout;

    VK_ASSERT(vkCreatePipelineLayout(
        vk_ctx->device.handle, &layout_info, vk_ctx->device.allocator, &pipeline_layout))

    // Create dynamic state for pipeline (viewport & scissor)
    // TODO! eventually if the game is fixed size remove these and bake states
    // into pipelines
    VkDynamicState d_states[2] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };

    VkPipelineDynamicStateCreateInfo dynamic_states = {
        .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 2,
        .pDynamicStates    = d_states,
    };

    ctx->object_pipeline.vert = GDF_ShaderLoadGLSLFromFile(
        "resources/shaders/objects.vert", GDF_SHADER_TYPE_VERT, GDF_SHADER_RELOAD_NONE);
    ctx->object_pipeline.frag = GDF_ShaderLoadGLSLFromFile(
        "resources/shaders/objects.frag", GDF_SHADER_TYPE_FRAG, GDF_SHADER_RELOAD_NONE);

    VkPipelineShaderStageCreateInfo grid_shaders[] = {
        {
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage  = VK_SHADER_STAGE_VERTEX_BIT,
            .module = GDF_ShaderGetVkModule(ctx->object_pipeline.vert),
            .pName  = "main",
        },
        {
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = GDF_ShaderGetVkModule(ctx->object_pipeline.frag),
            .pName  = "main",
        }
    };

    // Info for dynamic renderign
    VkPipelineRenderingCreateInfo dyn_rendering_info = {};
    GDF_VkPipelineInitRenderingInfo(vk_ctx, &dyn_rendering_info);

    // Put all configuration in graphics pipeline info struct
    VkGraphicsPipelineCreateInfo pipeline_create_info = {
        .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount          = sizeof(grid_shaders) / sizeof(VkPipelineShaderStageCreateInfo),
        .pStages             = grid_shaders,
        .pVertexInputState   = &vertex_input_info,
        .pInputAssemblyState = &input_assembly,
        .pViewportState      = &viewport_state,
        .pRasterizationState = &rasterizer_state,
        .pMultisampleState   = &multisampling_state,
        .pDepthStencilState  = &depth_stencil_state,
        .pColorBlendState    = &color_blend_state,
        .layout              = pipeline_layout,
        .pNext               = &dyn_rendering_info,
        .pDynamicState       = &dynamic_states,
    };

    GDF_VkPipelineCreateInfo info = {
        .create_info.graphics = pipeline_create_info,
        .type                 = GDF_PIPELINE_TYPE_GRAPHICS,
    };

    GDF_VkCreatePipelineBase(&ctx->object_pipeline.base, info);

    rasterizer_state.polygonMode = VK_POLYGON_MODE_LINE;

    GDF_VkCreatePipelineBase(&ctx->object_pipeline.wireframe_base, info);

    return GDF_TRUE;
}

void cr_destroy_obj_pipeline(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx)
{
    vkDestroyPipeline(
        vk_ctx->device.handle, ctx->object_pipeline.base.handle, vk_ctx->device.allocator);
    vkDestroyPipeline(vk_ctx->device.handle, ctx->object_pipeline.wireframe_base.handle,
        vk_ctx->device.allocator);
    vkDestroyPipelineLayout(
        vk_ctx->device.handle, ctx->object_pipeline.base.layout, vk_ctx->device.allocator);
    vkDestroyShaderModule(
        vk_ctx->device.handle, ctx->object_pipeline.base.vert, vk_ctx->device.allocator);
    vkDestroyShaderModule(
        vk_ctx->device.handle, ctx->object_pipeline.base.frag, vk_ctx->device.allocator);
}

GDF_BOOL cr_create_ui_pipeline(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx)
{
    gdfe_ui_pipeline* pipeline = &ctx->ui_pipeline;


    return GDF_TRUE;
}

void cr_destroy_ui_pipeline(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx) {}
