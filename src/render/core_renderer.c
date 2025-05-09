#include <i_render/core_renderer.h>
#include <i_render/gpu_types.h>
#include <i_render/renderer.h>

#include <gdfe/gdfe.h>
#include <gdfe/render/vk/buffers.h>
#include <gdfe/render/vk/utils.h>
#include <i_render/mesh.h>

GDF_BOOL create_shaders(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx);

GDF_BOOL create_grid_pipeline(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx);
GDF_BOOL create_ui_pipeline(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx);
void     destroy_framebufs_and_imgs(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx);
GDF_BOOL create_framebufs_and_imgs(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx);
GDF_BOOL create_geometry_pass(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx);

// Up facing plane vertices
static const Vertex3d plane_vertices[] = {
    { { -0.5f, 0.5f, -0.5f } },
    { { 0.5f, 0.5f, -0.5f } },
    { { 0.5f, 0.5f, 0.5f } },
    { { -0.5f, 0.5f, 0.5f } },
};

static const u16 plane_indices[] = { 0, 1, 2, 2, 3, 0 };

GDF_BOOL core_renderer_init(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx)
{
    ctx->per_frame = GDF_ListReserve(CoreRendererPerFrame, vk_ctx->max_concurrent_frames);
    GDF_ListSetLen(ctx->per_frame, vk_ctx->max_concurrent_frames);
    gdfe_init_primitive_meshes();
    if (!create_shaders(vk_ctx, ctx))
    {
        LOG_ERR("Failed to load builtin shaders.");
        return GDF_FALSE;
    }
    if (!create_geometry_pass(vk_ctx, ctx))
    {
        LOG_ERR("Failed to create renderpasses.");
        return GDF_FALSE;
    }
    if (!create_framebufs_and_imgs(vk_ctx, ctx))
    {
        LOG_ERR("Failed to create framebuffers and images.");
        return GDF_FALSE;
    }
    if (!create_grid_pipeline(vk_ctx, ctx) || !create_ui_pipeline(vk_ctx, ctx))
    {
        LOG_ERR("Failed to create builtin pipelines.");
        return GDF_FALSE;
    }

    GDF_VkBufferCreateVertex(plane_vertices, sizeof(plane_vertices) / sizeof(*plane_vertices),
        sizeof(*plane_vertices), &ctx->up_facing_plane_vbo);
    GDF_VkBufferCreateIndex(plane_indices, sizeof(plane_indices) / sizeof(*plane_indices),
        &ctx->up_facing_plane_index_buffer);

    return GDF_TRUE;
}

// TODO! config to disable specific passes
GDF_BOOL core_renderer_draw(
    GDF_Renderer renderer, GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx)
{
    // MEMROY LEAK HERE
    GDF_RenderCallbacks*  callbacks      = renderer->callbacks;
    u32                   resource_idx   = vk_ctx->resource_idx;
    PerFrameResources*    vk_per_frame   = &vk_ctx->per_frame[resource_idx];
    CoreRendererPerFrame* core_per_frame = &ctx->per_frame[resource_idx];

    VkClearValue clear_values[2] = { { .color = { 0, 0, 0, 1 } }, { .depthStencil = { 1, 0 } } };

    GDF_Camera camera = ctx->active_camera;
    if (!camera)
        return GDF_TRUE;

    ViewProjUB vp_ubo_data = {
        .view_projection = GDF_CameraGetViewPerspectiveMatrix(camera),
    };

    GDF_Memcpy(vk_per_frame->vp_ubo.mapped_data, &vp_ubo_data, sizeof(ViewProjUB));

    VkRenderPassBeginInfo geometry_pass = {
        .sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pClearValues      = clear_values,
        .clearValueCount   = 2,
        .renderPass        = ctx->geometry_pass,
        .framebuffer       = core_per_frame->geometry_framebuffer,
        .renderArea.offset = { 0, 0 },
        .renderArea.extent = vk_ctx->swapchain.extent,
    };

    VkCommandBuffer cmd_buffer = vk_per_frame->cmd_buffer;
    vkCmdBeginRenderPass(cmd_buffer, &geometry_pass, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->grid_pipeline.layout,
        0, 1, &vk_per_frame->vp_ubo_set, 0, NULL);

    vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->grid_pipeline.handle);

    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(cmd_buffer, 0, 1, &ctx->up_facing_plane_vbo.handle, offsets);
    vkCmdBindIndexBuffer(
        cmd_buffer, ctx->up_facing_plane_index_buffer.handle, 0, VK_INDEX_TYPE_UINT16);

    vec3 camera_pos = GDF_CameraGetPosition(camera);
    vkCmdPushConstants(cmd_buffer, ctx->grid_pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0,
        sizeof(vec3), &camera_pos);

    vkCmdDrawIndexed(cmd_buffer, 6, 1, 0, 0, 0);

    if (callbacks->on_render)
    {
        if (!callbacks->on_render(
                vk_ctx, renderer->render_mode, renderer->app_state, callbacks->on_render_state))
        {
            LOG_ERR("User geometry pass callback failure.");
            return false;
        }
    }

    vkCmdEndRenderPass(cmd_buffer);

    // deferred lighting

    return GDF_TRUE;
}

GDF_BOOL core_renderer_resize(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx)
{
    destroy_framebufs_and_imgs(vk_ctx, ctx);
    if (!create_framebufs_and_imgs(vk_ctx, ctx))
    {
        LOG_ERR("Failed to create framebuffers and images.");
        return GDF_FALSE;
    }

    return GDF_TRUE;
}

GDF_BOOL core_renderer_destroy(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx)
{
    return GDF_TRUE;
}

GDF_BOOL create_shaders(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx)
{
    ctx->ui_pipeline.vert = GDF_VkUtilsLoadShader("resources/shaders/ui.vert.spv");
    // TODO! should destroy all the other created resources.
    VK_RETURN_FALSE_IF_NULLHANDLE(ctx->ui_pipeline.vert);

    ctx->ui_pipeline.frag = GDF_VkUtilsLoadShader("resources/shaders/ui.frag.spv");
    VK_RETURN_FALSE_IF_NULLHANDLE(ctx->ui_pipeline.frag);

    ctx->grid_pipeline.vert = GDF_VkUtilsLoadShader("resources/shaders/grid.vert.spv");
    VK_RETURN_FALSE_IF_NULLHANDLE(ctx->grid_pipeline.vert);

    ctx->grid_pipeline.frag = GDF_VkUtilsLoadShader("resources/shaders/grid.frag.spv");
    VK_RETURN_FALSE_IF_NULLHANDLE(ctx->grid_pipeline.frag);

    return GDF_TRUE;
}
