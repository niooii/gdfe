#include "i_render/vk_utils.h"

#include <gdfe/render/renderer.h>

// should be called on each resize
void gdfe_get_surface_capabilities(VkPhysicalDevice physical_device, VkSurfaceKHR surface, GDF_VkPhysicalDeviceSwapchainSupport* out_support_info)
{
    VK_ASSERT(
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            physical_device,
            surface,
            &out_support_info->capabilities
        )
    );
}

void gdfe_query_sc_info(VkPhysicalDevice physical_device, VkSurfaceKHR surface, GDF_VkPhysicalDeviceSwapchainSupport* out_support_info)
{
    gdfe_get_surface_capabilities(physical_device, surface, out_support_info);

    VK_ASSERT(
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            physical_device,
            surface,
            &out_support_info->format_count,
            NULL
        )
    );

    // Surface formats
    if (out_support_info->format_count != 0)
    {
        if (!out_support_info->formats)
        {
            out_support_info->formats = GDF_Malloc(sizeof(VkSurfaceFormatKHR) * out_support_info->format_count, GDF_MEMTAG_RENDERER);
        }
        VK_ASSERT(
            vkGetPhysicalDeviceSurfaceFormatsKHR(
                physical_device,
                surface,
                &out_support_info->format_count,
                out_support_info->formats
            )
        );
    }

    // Present modes
    VK_ASSERT(
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            physical_device,
            surface,
            &out_support_info->present_mode_count,
            0
        )
    );
    if (out_support_info->present_mode_count != 0)
    {
        if (!out_support_info->present_modes)
        {
            out_support_info->present_modes = GDF_Malloc(sizeof(VkPresentModeKHR) * out_support_info->present_mode_count, GDF_MEMTAG_RENDERER);
        }
        VK_ASSERT(
            vkGetPhysicalDeviceSurfacePresentModesKHR(
                physical_device,
                surface,
                &out_support_info->present_mode_count,
                out_support_info->present_modes
            )
        );
    }
}

GDF_BOOL gdfe_swapchain_init(GDF_VkRenderContext* vk_ctx, u16 w, u16 h)
{
    vk_ctx->swapchain.extent.width = w;
    vk_ctx->swapchain.extent.height = h;
    GDF_VkPhysicalDeviceInfo* pdevice = vk_ctx->device.physical_info;
    // TODO! add a lot more checks during swapchain creation
    VkSwapchainCreateInfoKHR sc_create_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .surface = vk_ctx->surface,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
                        | VK_IMAGE_USAGE_TRANSFER_DST_BIT
    };
    sc_create_info.clipped = VK_TRUE;

    // swapchain.extent should be updated prior to this call
    sc_create_info.imageExtent = vk_ctx->swapchain.extent;
    if (pdevice->formats.depth_format == VK_FORMAT_UNDEFINED) {
        LOG_ERR("Could not find a supported depth format. Cannot continue program.");
        return GDF_FALSE;
    }

    sc_create_info.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
    sc_create_info.imageFormat = pdevice->formats.image_format;
    sc_create_info.imageColorSpace = pdevice->formats.image_color_space;
    sc_create_info.minImageCount = 3;

    VK_ASSERT(
        vkCreateSwapchainKHR(
            vk_ctx->device.handle,
            &sc_create_info,
            vk_ctx->device.allocator,
            &vk_ctx->swapchain.handle
        )
    );

    // Allow triple buffering
    vk_ctx->max_concurrent_frames = MAX_FRAMES_IN_FLIGHT;
    VK_ASSERT(
        vkGetSwapchainImagesKHR(
            vk_ctx->device.handle,
            vk_ctx->swapchain.handle,
            &vk_ctx->swapchain.image_count,
            NULL
        )
    )

    u32 image_count = vk_ctx->swapchain.image_count;
    if (!vk_ctx->recreating_swapchain)
    {
        vk_ctx->swapchain.images = GDF_ListReserve(VkImage, image_count);
        vk_ctx->swapchain.image_views = GDF_ListReserve(VkImageView, image_count);
    }

    VK_ASSERT(
        vkGetSwapchainImagesKHR(
            vk_ctx->device.handle,
            vk_ctx->swapchain.handle,
            &image_count,
            vk_ctx->swapchain.images
        )
    );

    // Create corresponding image views
    for (u32 i = 0; i < image_count; i++)
    {
        VkImageViewCreateInfo image_view_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .format = pdevice->formats.image_format,
            .image = vk_ctx->swapchain.images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            // i dont have to initialize these but why not
            .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
            .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .subresourceRange.baseMipLevel = 0,
            .subresourceRange.levelCount = vk_ctx->mip_levels,
            // some swapchains can have multiple layers, for example
            // in 3d (the blue red glasses stuff)
            // i will NOT be having 3d chat
            .subresourceRange.baseArrayLayer = 0,
            .subresourceRange.layerCount = 1,
        };

        VK_ASSERT(
            vkCreateImageView(
                vk_ctx->device.handle,
                &image_view_info,
                vk_ctx->device.allocator,
                &vk_ctx->swapchain.image_views[i]
            )
        );
    }

    GDF_ListSetLen(vk_ctx->swapchain.images, image_count);

    LOG_TRACE("Fetched %d images from swapchain...", GDF_ListLen(vk_ctx->swapchain.images));

    return GDF_TRUE;
}

void gdfe_swapchain_destroy(GDF_VkRenderContext* vk_ctx)
{
    VkDevice device = vk_ctx->device.handle;
    VkAllocationCallbacks* allocator = vk_ctx->device.allocator;
    for (u32 i = 0; i < vk_ctx->swapchain.image_count; i++)
    {
        vkDestroyImageView(
            device,
            vk_ctx->swapchain.image_views[i],
            allocator
        );
    }

    vkDestroySwapchainKHR(
        device,
        vk_ctx->swapchain.handle,
        allocator
    );

    if (!vk_ctx->recreating_swapchain)
    {
        GDF_ListDestroy(vk_ctx->swapchain.images);
    }
}