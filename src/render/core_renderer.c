#include "irender/core_renderer.h"

#include "gdfe.h"
#include "../../include/render/vk_utils.h"

bool create_shaders(GDF_VkRenderContext* vk_ctx, CoreRendererContext* ctx);

bool create_grid_pipeline(GDF_VkRenderContext* vk_ctx, CoreRendererContext* ctx);
bool create_ui_pipeline(GDF_VkRenderContext* vk_ctx, CoreRendererContext* ctx);
void destroy_framebufs_and_imgs(GDF_VkRenderContext* vk_ctx, CoreRendererContext* ctx);
bool create_framebufs_and_imgs(GDF_VkRenderContext* vk_ctx, CoreRendererContext* ctx);
bool create_geometry_pass(GDF_VkRenderContext* vk_ctx, CoreRendererContext* ctx);

bool core_renderer_init(GDF_VkRenderContext* vk_ctx, CoreRendererContext* ctx) {
    ctx->per_frame = GDF_LIST_Reserve(CoreRendererPerFrame, vk_ctx->max_concurrent_frames);
    GDF_LIST_SetLength(ctx->per_frame, vk_ctx->max_concurrent_frames);
    if (!create_shaders(vk_ctx, ctx))
    {
        LOG_ERR("Failed to load builtin shaders.");
        return false;
    }
    if (!create_framebufs_and_imgs(vk_ctx, ctx))
    {
        LOG_ERR("Failed to create framebuffers and images.");
        return false;
    }
    if (!create_geometry_pass(vk_ctx, ctx))
    {
        LOG_ERR("Failed to create renderpasses.");
        return false;
    }
    if (!create_grid_pipeline(vk_ctx, ctx) || !create_ui_pipeline(vk_ctx, ctx))
    {
        LOG_ERR("Failed to create builtin pipelines.");
        return false;
    }

    return true;
}

// TODO! config to disable specific passes
bool core_renderer_draw(GDF_VkRenderContext* vk_ctx, GDF_AppCallbacks* callbacks, CoreRendererContext* ctx)
{
    u32 resource_idx = vk_ctx->resource_idx;
    PerFrameResources* vk_per_frame = &vk_ctx->per_frame[resource_idx];
    CoreRendererPerFrame* core_per_frame = &ctx->per_frame[resource_idx];

    VkRenderPassBeginInfo geometry_pass = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = ctx->geometry_pass,
        .framebuffer = core_per_frame->geometry_framebuffer,
        .renderArea.offset = {0, 0},
        .renderArea.extent = vk_ctx->swapchain.extent
    };

    // vkCmdBeginRenderPass(vk_per_frame->cmd_buffer, &geometry_pass, VK_SUBPASS_CONTENTS_INLINE);
    // vkCmdEndRenderPass(vk_per_frame->cmd_buffer);
    return true;
}

bool core_renderer_resize(GDF_VkRenderContext* vk_ctx, GDF_AppCallbacks* callbacks, CoreRendererContext* ctx)
{
    destroy_framebufs_and_imgs(vk_ctx, ctx);
    if (!create_framebufs_and_imgs(vk_ctx, ctx))
    {
        LOG_ERR("Failed to create framebuffers and images.");
        return false;
    }
}

bool core_renderer_destroy(GDF_VkRenderContext* vk_ctx, CoreRendererContext* ctx)
{
    return true;
}

bool create_shaders(GDF_VkRenderContext* vk_ctx, CoreRendererContext* ctx)
{
    ctx->ui_pipeline.vert =
        GDF_VkUtilsLoadShader(vk_ctx, "resources/shaders/ui.vert.spv");
    // TODO! should destroy all the other created resources.
    VK_RETURN_FALSE_IF_NULLHANDLE(ctx->ui_pipeline.vert);

    ctx->ui_pipeline.frag =
        GDF_VkUtilsLoadShader(vk_ctx, "resources/shaders/ui.frag.spv");
    VK_RETURN_FALSE_IF_NULLHANDLE(ctx->ui_pipeline.frag);

    ctx->grid_pipeline.vert =
        GDF_VkUtilsLoadShader(vk_ctx, "resources/shaders/grid.vert.spv");
    VK_RETURN_FALSE_IF_NULLHANDLE(ctx->grid_pipeline.vert);

    ctx->grid_pipeline.frag =
        GDF_VkUtilsLoadShader(vk_ctx, "resources/shaders/grid.frag.spv");
    VK_RETURN_FALSE_IF_NULLHANDLE(ctx->grid_pipeline.frag);

    return true;
}
