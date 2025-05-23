#include <i_render/core_renderer.h>
#include <i_render/gpu_types.h>
#include <i_render/renderer.h>

#include <gdfe/gdfe.h>
#include <gdfe/render/vk/buffers.h>
#include <gdfe/render/vk/utils.h>
#include <i_render/mesh.h>

GDF_BOOL create_grid_pipeline(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx);
void     destroy_grid_pipeline(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx);

GDF_BOOL create_ui_pipeline(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx);

void     gdfe_destroy_imgs(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx);
GDF_BOOL gdfe_init_imgs(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx);
// GDF_BOOL create_geometry_pass(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx);

// The initial transitions for the images used in rendering
void prepare_images(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx);

// Resolves the msaa image to the current swapchain image
// void resolve_msaa_image(
//     GDF_Renderer renderer, GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx);

// The final transitions for rendering to be "finished". For now, only transitions
// the swapchain image to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR.
void finalize_images(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx);

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
    ctx->per_frame = GDF_ListReserve(CoreFrameResources, vk_ctx->fof);
    GDF_ListSetLen(ctx->per_frame, vk_ctx->fof);
    gdfe_init_primitive_meshes();
    // if (!create_geometry_pass(vk_ctx, ctx))
    // {
    //     LOG_ERR("Failed to create renderpasses.");
    //     return GDF_FALSE;
    // }
    if (!gdfe_init_imgs(vk_ctx, ctx))
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
GDF_BOOL core_renderer_draw(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx)
{
    const GDF_RenderCallbacks* callbacks      = GDFE_RENDER_STATE.callbacks;
    const u32                  resource_idx   = vk_ctx->resource_idx;
    const VkFrameResources*    vk_per_frame   = &vk_ctx->per_frame[resource_idx];
    const CoreFrameResources*  core_per_frame = &ctx->per_frame[resource_idx];
    const VkCommandBuffer      cmd_buffer     = vk_per_frame->cmd_buffer;

    VkClearValue clear_values[2] = { { .color = { 0, 0, 0, 1 } }, { .depthStencil = { 1, 0 } } };

    GDF_Camera camera = ctx->active_camera;
    if (!camera)
        return GDF_TRUE;

    ViewProjUB vp_ubo_data = {
        .view_projection = GDF_CameraGetViewPerspectiveMatrix(camera),
    };

    GDF_Memcpy(vk_per_frame->vp_ubo.mapped_data, &vp_ubo_data, sizeof(ViewProjUB));

    VkRenderingAttachmentInfo color_attachment = {
        .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView   = core_per_frame->msaa_image.view,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue  = { .color = { 0, 0, 0, 1 } },

        // msaa resolving stuff
        .resolveMode        = VK_RESOLVE_MODE_AVERAGE_BIT,
        .resolveImageView   = vk_ctx->swapchain.image_views[vk_ctx->swapchain.current_img_idx],
        .resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    // Set up depth attachment for dynamic rendering
    VkRenderingAttachmentInfo depth_attachment = {
        .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView   = core_per_frame->depth_image.view, // Assumes this exists
        .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        .loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue  = { .depthStencil = { 1, 0 } },
    };

    VkRenderingInfo rendering_info = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = {
            .offset = { 0, 0 },
            .extent = vk_ctx->swapchain.extent,
        },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment,
        .pDepthAttachment = &depth_attachment,
        .pStencilAttachment = NULL,
    };

    // transition into right layouts
    prepare_images(vk_ctx, ctx);

    vkCmdBeginRendering(cmd_buffer, &rendering_info);

    vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        ctx->grid_pipeline.base.layout, 0, 1, &vk_per_frame->vp_ubo_set, 0, NULL);

    vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->grid_pipeline.base.handle);

    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(cmd_buffer, 0, 1, &ctx->up_facing_plane_vbo.handle, offsets);
    vkCmdBindIndexBuffer(
        cmd_buffer, ctx->up_facing_plane_index_buffer.handle, 0, VK_INDEX_TYPE_UINT16);

    vec3 camera_pos = GDF_CameraGetPosition(camera);
    vkCmdPushConstants(cmd_buffer, ctx->grid_pipeline.base.layout, VK_SHADER_STAGE_VERTEX_BIT, 0,
        sizeof(vec3), &camera_pos);

    vkCmdDrawIndexed(cmd_buffer, 6, 1, 0, 0, 0);

    if (callbacks->on_render)
    {
        if (!callbacks->on_render(vk_ctx, GDFE_RENDER_STATE.render_mode,
                GDFE_RENDER_STATE.app_state, callbacks->on_render_state))
        {
            LOG_ERR("User geometry pass callback failure.");
            return false;
        }
    }

    vkCmdEndRendering(cmd_buffer);

    // resolve_msaa_image(renderer, vk_ctx, ctx);

    finalize_images(vk_ctx, ctx);

    return GDF_TRUE;
}

GDF_BOOL core_renderer_resize(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx)
{
    gdfe_destroy_imgs(vk_ctx, ctx);
    if (!gdfe_init_imgs(vk_ctx, ctx))
    {
        LOG_ERR("Failed to create framebuffers and images.");
        return GDF_FALSE;
    }

    return GDF_TRUE;
}

GDF_BOOL core_renderer_destroy(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx)
{
    GDF_VkBufferDestroy(&ctx->up_facing_plane_vbo);
    GDF_VkBufferDestroy(&ctx->up_facing_plane_index_buffer);
    destroy_grid_pipeline(vk_ctx, ctx);
    gdfe_destroy_imgs(vk_ctx, ctx);
    return GDF_TRUE;
}

/* Forward declared purpose specific functions */

void prepare_images(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx)
{
    const u32                 resource_idx   = vk_ctx->resource_idx;
    const VkFrameResources*   vk_per_frame   = &vk_ctx->per_frame[resource_idx];
    const CoreFrameResources* core_per_frame = &ctx->per_frame[resource_idx];
    const VkCommandBuffer     cmd_buffer     = vk_per_frame->cmd_buffer;

    // msaa image to COLOR_ATTACHMENT_OPTIMAL
    VkImageMemoryBarrier pre_render_barriers[2] = {
        { .sType                 = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout           = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image               = core_per_frame->msaa_image.handle,
            .subresourceRange    = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
           .baseMipLevel                 = 0,
           .levelCount                   = 1,
           .baseArrayLayer               = 0,
           .layerCount                   = 1,
            },
            .srcAccessMask       = 0,
            .dstAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        },

        // msaa image aka resolve target to COLOR_ATTACHMENT_OPTIMAL
        { .sType                 = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout           = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image               = vk_ctx->swapchain.images[vk_ctx->swapchain.current_img_idx],
            .subresourceRange    = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel                 = 0,
                .levelCount                   = 1,
                .baseArrayLayer               = 0,
                .layerCount                   = 1,
            },
            .srcAccessMask       = 0,
            .dstAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        },
    };

    vkCmdPipelineBarrier(cmd_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, NULL, 0, NULL, 2, pre_render_barriers);

    VkImageMemoryBarrier depth_barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout            = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout            = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        .srcQueueFamilyIndex  = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex  = VK_QUEUE_FAMILY_IGNORED,
        .image                = core_per_frame->depth_image.handle,
        .subresourceRange   = {
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .baseMipLevel                 = 0,
            .levelCount                   = 1,
            .baseArrayLayer               = 0,
            .layerCount                   = 1,
        },
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    };

    vkCmdPipelineBarrier(cmd_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0, 0, NULL, 0, NULL, 1, &depth_barrier);
}

// void resolve_msaa_image(
//     GDF_Renderer renderer, GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx)
// {
//     const u32                   resource_idx   = vk_ctx->resource_idx;
//     const PerFrameResources*    vk_per_frame   = &vk_ctx->per_frame[resource_idx];
//     const CoreRendererPerFrame* core_per_frame = &ctx->per_frame[resource_idx];
//     const VkCommandBuffer       cmd_buffer     = vk_per_frame->cmd_buffer;
//     const VkImage swapchain_img = vk_ctx->swapchain.images[vk_ctx->swapchain.current_img_idx];
//
//     // resolve msaa image manually
//     // first prepare the swapchain image for the resolve
//     VkImageMemoryBarrier msaa_to_resolve_barrier = {
//         .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
//         .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
//         .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
//         .image = core_per_frame->msaa_image.handle,
//         .subresourceRange = {
//             .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
//             .baseMipLevel = 0,
//             .levelCount = 1,
//             .baseArrayLayer = 0,
//             .layerCount = 1,
//         },
//         .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
//         .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
//     };
//
//     VkImageMemoryBarrier swapchain_for_resolve_barrier = {
//         .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
//         .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
//         .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
//         .image = swapchain_img,
//         .subresourceRange = {
//             .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
//             .baseMipLevel = 0,
//             .levelCount = 1,
//             .baseArrayLayer = 0,
//             .layerCount = 1,
//         },
//         .srcAccessMask = 0,
//         .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
//     };
//
//     VkImageMemoryBarrier resolve_prep_barriers[] = { msaa_to_resolve_barrier,
//         swapchain_for_resolve_barrier };
//     vkCmdPipelineBarrier(cmd_buffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
//         VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 2, resolve_prep_barriers);
//
//     // now resolve the thing
//     VkImageResolve resolve_region = {
//         .srcSubresource = {
//             .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
//             .mipLevel = 0,
//             .baseArrayLayer = 0,
//             .layerCount = 1,
//         },
//         .srcOffset = { 0, 0, 0 },
//         .dstSubresource = {
//             .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
//             .mipLevel = 0,
//             .baseArrayLayer = 0,
//             .layerCount = 1,
//         },
//         .dstOffset = { 0, 0, 0 },
//         .extent = {
//             .width = vk_ctx->swapchain.extent.width,
//             .height = vk_ctx->swapchain.extent.height,
//             .depth = 1,
//         },
//     };
//
//     vkCmdResolveImage(cmd_buffer, core_per_frame->msaa_image.handle,
//         VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, swapchain_img,
//         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &resolve_region);
// }

void finalize_images(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx)
{
    const u32               resource_idx = vk_ctx->resource_idx;
    const VkFrameResources* vk_per_frame = &vk_ctx->per_frame[resource_idx];
    const VkCommandBuffer   cmd_buffer   = vk_per_frame->cmd_buffer;
    const VkImage swapchain_img = vk_ctx->swapchain.images[vk_ctx->swapchain.current_img_idx];

    VkImageMemoryBarrier present_barrier = {
        .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .newLayout        = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .image            = swapchain_img,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel                 = 0,
            .levelCount                   = 1,
            .baseArrayLayer               = 0,
            .layerCount                   = 1,
        },
        .srcAccessMask    = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dstAccessMask    = 0,
    };

    vkCmdPipelineBarrier(cmd_buffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL, 0, NULL, 1, &present_barrier);
}
