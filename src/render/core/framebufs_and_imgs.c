#include <i_render/core_renderer.h>
#include <gdfe/render/vk/utils.h>
#include <gdfe/render/vk/image.h>

GDF_BOOL create_framebufs_and_imgs(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx)
{
    GDF_VkPhysicalDeviceInfo* pdevice = vk_ctx->device.physical_info;
    for (int i = 0; i < vk_ctx->max_concurrent_frames; i++)
    {
        CoreRendererPerFrame* per_frame = &ctx->per_frame[i];
        VkImageCreateInfo color_img_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .extent = {
                .width = vk_ctx->swapchain.extent.width,
                .height = vk_ctx->swapchain.extent.height,
                .depth = 1
            },
            .mipLevels = vk_ctx->mip_levels,
            .arrayLayers = 1,
            .format = pdevice->formats.image_format,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .samples = vk_ctx->msaa_samples,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE
        };

        VkImageViewCreateInfo color_view_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = pdevice->formats.image_format,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = vk_ctx->mip_levels,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        if (!GDF_VkImageCreate(
            &color_img_info,
            &color_view_info,
            &per_frame->msaa_image
        ))
        {
            return GDF_FALSE;
        }

        VkImageCreateInfo depth_image_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .extent.width = vk_ctx->swapchain.extent.width,
            .extent.height = vk_ctx->swapchain.extent.height,
            .extent.depth = 1,
            .mipLevels = vk_ctx->mip_levels,
            .arrayLayers = 1,
            .format = pdevice->formats.depth_format,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            .samples = vk_ctx->msaa_samples,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE
        };

        VkImageViewCreateInfo depth_view_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = ctx->per_frame[i].depth_image.handle,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = pdevice->formats.depth_format,
            .subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .subresourceRange.baseMipLevel = 0,
            .subresourceRange.levelCount = vk_ctx->mip_levels,
            .subresourceRange.baseArrayLayer = 0,
            .subresourceRange.layerCount = 1
        };

        if (!GDF_VkImageCreate(
            &depth_image_info,
            &depth_view_info,
            &per_frame->depth_image
        ))
        {
            return GDF_FALSE;
        }

        // framebuffers
        VkImageView swapchain_img_view = vk_ctx->swapchain.image_views[i];

        VkImageView image_views[3] = {
            per_frame->msaa_image.view,
            per_frame->depth_image.view,
            swapchain_img_view
        };

        VkFramebufferCreateInfo framebuffer_info = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .attachmentCount = 3,
            .pAttachments = image_views,
            .renderPass = ctx->geometry_pass,
            .width = vk_ctx->swapchain.extent.width,
            .height = vk_ctx->swapchain.extent.height,
            .layers = 1
        };

        VK_RETURN_FALSE_ASSERT(
            vkCreateFramebuffer(
                vk_ctx->device.handle,
                &framebuffer_info,
                vk_ctx->device.allocator,
                &per_frame->geometry_framebuffer
            )
        );
    }

    return GDF_TRUE;
}

// TODO! destroy framebuffers
void destroy_framebufs_and_imgs(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx)
{
    for (int i = 0; i < vk_ctx->max_concurrent_frames; i++)
    {
        CoreRendererPerFrame* per_frame = &ctx->per_frame[i];
        GDF_VkImageDestroy(vk_ctx, &per_frame->msaa_image);
        GDF_VkImageDestroy(vk_ctx, &per_frame->depth_image);
        vkDestroyFramebuffer(vk_ctx->device.handle, per_frame->geometry_framebuffer, vk_ctx->device.allocator);
    }
}