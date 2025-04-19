#include <gdfe/render/vk/utils.h>
#include "i_render/vk_utils.h"
#include <gdfe/os/io.h>

VkFormat __find_depth_format(VkPhysicalDevice physical_device)
{
    VkFormat depth_formats[] =
    {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT
    };
    u32 format_count = sizeof(depth_formats) / sizeof(depth_formats[0]);

    for (u32 i = 0; i < format_count; i++)
    {
        VkFormatProperties format_properties;
        vkGetPhysicalDeviceFormatProperties(physical_device, depth_formats[i], &format_properties);

        if (format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            return depth_formats[i];
        }
    }

    return VK_FORMAT_UNDEFINED;
}


void __get_queue_indices(GDF_VkRenderContext* vk_ctx, VkPhysicalDevice physical_device, GDF_VkPhysicalDeviceQueues* queues)
{
    queues->graphics_family_index = -1;
    queues->present_family_index = -1;
    queues->compute_family_index = -1;
    queues->transfer_family_index = -1;

    u32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, 0);
    if (queue_family_count == 0)
        return;
    VkQueueFamilyProperties queue_families[queue_family_count];
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families);

    for (u32 i = 0; i < queue_family_count; i++) {
        if (queue_families[i].queueCount == 0)
            continue;
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            LOG_TRACE("Graphics queue idx: %d", i);
            LOG_TRACE("Graphics queue max count: %d", queue_families[i].queueCount);
            queues->graphics_family_index = i;
        }

        if (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
        {
            LOG_TRACE("Compute queue idx: %d", i);
            LOG_TRACE("Compute queue max count: %d", queue_families[i].queueCount);
            queues->compute_family_index = i;
        }

        if (queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
        {
            LOG_TRACE("Transfer queue idx: %d", i);
            LOG_TRACE("Transfer queue max count: %d", queue_families[i].queueCount);
            queues->transfer_family_index = i;
        }

        VkBool32 supports_present = VK_FALSE;
        VK_ASSERT(vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, vk_ctx->surface, &supports_present));
        if (supports_present) {
            LOG_TRACE("Present queue idx: %d", i);
            queues->present_family_index = i;
        }
    }
}

void gdfe_physical_device_init(GDF_VkRenderContext* vk_ctx, VkPhysicalDevice pdevice,
                               GDF_VkPhysicalDeviceInfo* out_pdevice)
{
    out_pdevice->handle = pdevice;
    // fill properties, features, and memoryinfo field
    vkGetPhysicalDeviceProperties(pdevice, &out_pdevice->properties);
    vkGetPhysicalDeviceFeatures(pdevice, &out_pdevice->features);
    LOG_TRACE("Initializing device %s", out_pdevice->properties.deviceName);
    vkGetPhysicalDeviceMemoryProperties(pdevice, &out_pdevice->memory);
    // fill the swapchain support info field
    // for now if these arent supported FUCK YOU
    out_pdevice->formats.depth_format = __find_depth_format(pdevice);
    out_pdevice->formats.image_format = VK_FORMAT_B8G8R8A8_SRGB;
    out_pdevice->formats.image_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    gdfe_query_sc_info(pdevice, vk_ctx->surface, &out_pdevice->sc_support_info);
    // fill the queue info field
    __get_queue_indices(vk_ctx, pdevice, &out_pdevice->queues);
    out_pdevice->usable = GDF_TRUE;
}