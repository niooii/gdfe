#pragma once
#include <gdfe/render/vk/types.h>

// Internal utility header.

// Call after each window resize.
void gdfe_get_surface_capabilities(VkPhysicalDevice physical_device, VkSurfaceKHR surface, GDF_VkPhysicalDeviceSwapchainSupport* out_support_info);
void gdfe_query_sc_info(VkPhysicalDevice physical_device, VkSurfaceKHR surface, GDF_VkPhysicalDeviceSwapchainSupport* out_support_info);
GDF_BOOL gdfe_swapchain_init(GDF_VkRenderContext* vk_ctx, u16 w, u16 h);
void gdfe_swapchain_destroy(GDF_VkRenderContext* vk_ctx);

void gdfe_physical_device_init(GDF_VkRenderContext* vk_ctx, VkPhysicalDevice pdevice,
                               GDF_VkPhysicalDeviceInfo* out_pdevice);