#include <render/vk_types.h>
#include "../internal/irender/gpu_types.h"
#include <collections/list.h>
#include "irender/renderer.h"
#include "irender/core_renderer.h"
#include <render/renderer.h>
#include <vulkan/vk_enum_string_helper.h>

#include "gdfe.h"
#include "irender/vk_os.h"
#include "../../include/render/vk_utils.h"
#include "../../include/render/vk_utils.h"

VKAPI_ATTR VkBool32 VKAPI_CALL __vk_dbg_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data) {
    switch (message_severity) {
        default:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            LOG_ERR(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            LOG_WARN(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            LOG_INFO(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            LOG_TRACE(callback_data->pMessage);
            break;
    }
    return VK_FALSE;
}

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

void __query_swapchain_support(VkPhysicalDevice physical_device, VkSurfaceKHR surface, vk_pdevice_swapchain_support* out_support_info)
{
    VK_ASSERT(
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            physical_device,
            surface,
            &out_support_info->capabilities
        )
    );

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

// TODO! initialize lighting and postprocessing pipelines
// and separate pass for postprocessing
// bool __create_renderpasses(VkRenderContext* vk_ctx)
// {
//     /* ======================================== */
//     /* ----- CREATE RENDERPASS AND SUBPASSES ----- */
//     /* ======================================== */
//
//     // color
//     VkAttachmentDescription color_attachment = {
//         .format = vk_ctx->formats.image_format,
//         .samples = vk_ctx->msaa_samples,
//         .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
//         .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
//         .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
//         .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
//         .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
//         .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
//         .flags = 0
//     };
//
//     // depth
//     VkAttachmentDescription depth_attachment = {
//         .format = vk_ctx->formats.depth_format,
//         .samples = vk_ctx->msaa_samples,
//         .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
//         .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
//         .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
//         .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
//         .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
//         .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
//         .flags = 0
//     };
//
//     // resolve
//     VkAttachmentDescription resolved_attachment = {
//         .format = vk_ctx->formats.image_format,
//         .samples = VK_SAMPLE_COUNT_1_BIT,
//         .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
//         .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
//         .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
//         .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
//         .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
//         .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
//     };
//
//     VkAttachmentReference color_attachment_ref = {
//         .attachment = 0,
//         .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
//     };
//
//     VkAttachmentReference depth_attachment_ref = {
//         .attachment = 1,
//         .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
//     };
//
//     VkAttachmentReference resolved_attachment_ref = {
//         .attachment = 2,
//         .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
//     };
//
//     u32 attachment_counts = 3;
//     VkAttachmentDescription attachments[] = {
//         color_attachment, depth_attachment, resolved_attachment
//     };
//
//     // TODO! multiple subpasses
//     VkSubpassDescription subpass = {
//         .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
//         .colorAttachmentCount = 1,
//         .pColorAttachments = &color_attachment_ref,
//         .pDepthStencilAttachment = &depth_attachment_ref,
//         .pResolveAttachments = &resolved_attachment_ref
//     };
//
//     VkSubpassDependency dependency = {
//         .srcSubpass = VK_SUBPASS_EXTERNAL,
//         .dstSubpass = 0,
//         .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
//         .srcAccessMask = 0,
//         .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
//         .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
//         .dependencyFlags = 0
//     };
//
//     VkRenderPassCreateInfo rp_create_info = {
//         .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
//         .subpassCount = 1,
//         .pSubpasses = &subpass,
//         .dependencyCount = 1,
//         .pDependencies = &dependency,
//         .attachmentCount = attachment_counts,
//         .pAttachments = attachments,
//     };
//
//     VK_ASSERT(
//         vkCreateRenderPass(
//             vk_ctx->device.handle,
//             &rp_create_info,
//             vk_ctx->device.allocator,
//             &vk_ctx->renderpasses[GDF_VK_RENDERPASS_INDEX_MAIN]
//         )
//     );
//
//     return true;
// }

// filters the vk_ctx->physical_device_info list
void __filter_available_devices(GDF_VkRenderContext* vk_ctx, vk_physical_device* phys_list)
{
    u32 list_len = GDF_LIST_GetLength(phys_list);
    for (u32 i = list_len; i > 0; i--)
    {
        vk_physical_device* device = &vk_ctx->physical_device_info_list[i - 1];
        LOG_TRACE("Checking device %s", device->properties.deviceName);

        bool device_suitable = true;

        // check swapchain support
        if (device->sc_support_info.format_count < 1 || device->sc_support_info.present_mode_count < 1)
        {
            device_suitable = false;
            goto filter_device_skip;
        }
        // check queues support
        if (
            device->queues.compute_family_index == -1 ||
            device->queues.graphics_family_index == -1 ||
            device->queues.present_family_index == -1 ||
            device->queues.transfer_family_index == -1
        )
        {
            LOG_TRACE("Device does not have the required queues.");
            // LOG_TRACE("yes %d", );
            device_suitable = false;
            goto filter_device_skip;
        }
        // TODO! Check if the device supports
        // all the required extensions

        filter_device_skip:
        if (!device_suitable)
        {
            LOG_TRACE("Marked device \'%s\' as unusable.", device->properties.deviceName);
            LOG_TRACE("%d", device->queues.graphics_family_index);
            LOG_TRACE("%d", device->queues.compute_family_index);
            LOG_TRACE("%d", device->queues.present_family_index);
            LOG_TRACE("%d", device->queues.transfer_family_index);
            device->usable = false;
        }
    }
}

bool __create_swapchain(GDF_Renderer state)
{
    GDF_VkRenderContext* vk_ctx = &state->vk_ctx;

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

    // TODO! actually run checks against this
    __query_swapchain_support(
        vk_ctx->device.physical_info->handle,
        vk_ctx->surface,
        &vk_ctx->device.physical_info->sc_support_info
    );

    // Set extent
    vk_ctx->swapchain.extent.width = state->framebuffer_width;
    vk_ctx->swapchain.extent.height = state->framebuffer_height;
    sc_create_info.imageExtent = vk_ctx->swapchain.extent;
    // for now if these arent supported FUCK YOU
    // ! also initiliaze formats here
    vk_ctx->formats.depth_format = __find_depth_format(vk_ctx->device.physical_info->handle);
    if (vk_ctx->formats.depth_format == VK_FORMAT_UNDEFINED) {
        LOG_ERR("Could not find a supported depth format. Cannot continue program.");
        return false;
    }
    vk_ctx->formats.image_format = VK_FORMAT_B8G8R8A8_SRGB;
    vk_ctx->formats.image_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    sc_create_info.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
    sc_create_info.imageFormat = vk_ctx->formats.image_format;
    sc_create_info.imageColorSpace = vk_ctx->formats.image_color_space;
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
    // grab images into an intermediate buffer then transfer to my own structs
    VkImage images[image_count];

    VK_ASSERT(
        vkGetSwapchainImagesKHR(
            vk_ctx->device.handle,
            vk_ctx->swapchain.handle,
            &image_count,
            images
        )
    );

    if (!vk_ctx->recreating_swapchain)
    {
        vk_ctx->swapchain.images = GDF_LIST_Reserve(VkImage, image_count);
        vk_ctx->swapchain.image_views = GDF_LIST_Reserve(VkImageView, image_count);
    }

    // Create corresponding image views
    for (u32 i = 0; i < image_count; i++)
    {
        vk_ctx->swapchain.images[i] = images[i];
        VkImageViewCreateInfo image_view_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .format = vk_ctx->formats.image_format,
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

    GDF_LIST_SetLength(vk_ctx->swapchain.images, image_count);

    LOG_TRACE("Fetched %d images from swapchain...", GDF_LIST_GetLength(vk_ctx->swapchain.images));

    return true;
}

void __destroy_swapchain(GDF_VkRenderContext* vk_ctx)
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
        GDF_LIST_Destroy(vk_ctx->swapchain.images);
    }
}
//
// bool __create_framebuffers(VkRenderContext* vk_ctx)
// {
//     // Create framebuffers
//     u32 image_count = vk_ctx->swapchain.image_count;
//     if (!vk_ctx->recreating_swapchain)
//         vk_ctx->swapchain.framebuffers = GDF_LIST_Reserve(VkFramebuffer, image_count);
//     for (u32 i = 0; i < image_count; i++)
//     {
//         vk_image* img = &vk_ctx->swapchain.images[i];
//
//         VkImageView image_views[3] = {
//             vk_ctx->msaa_image_view,
//             vk_ctx->depth_image_view,
//             img->view
//         };
//
//         VkFramebufferCreateInfo framebuffer_info = {
//             .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
//             .attachmentCount = 3,
//             .pAttachments = image_views,
//             .renderPass = vk_ctx->renderpasses[GDF_VK_RENDERPASS_INDEX_MAIN],
//             .width = vk_ctx->swapchain.extent.width,
//             .height = vk_ctx->swapchain.extent.height,
//             .layers = 1
//         };
//
//         VK_ASSERT(
//             vkCreateFramebuffer(
//                 vk_ctx->device.handle,
//                 &framebuffer_info,
//                 vk_ctx->device.allocator,
//                 &vk_ctx->swapchain.framebuffers[i]
//             )
//         );
//     }
//     return true;
// }

// void __destroy_framebuffers(VkRenderContext* vk_ctx)
// {
//     for (u32 i = 0; i < vk_ctx->swapchain.image_count; i++)
//     {
//         vkDestroyFramebuffer(
//             vk_ctx->device.handle,
//             vk_ctx->swapchain.framebuffers[i],
//             vk_ctx->device.allocator
//         );
//     }
//
//     if (!vk_ctx->recreating_swapchain)
//         GDF_LIST_Destroy(vk_ctx->swapchain.framebuffers);
// }
//
// bool __recreate_sized_resources(GDF_Renderer state)
// {
//     VkRenderContext* vk_ctx = &state->vk_ctx;
//
//     __destroy_framebuffers(vk_ctx);
//     __destroy_swapchain(vk_ctx);
//
//     return __create_swapchain(state)
//         && __create_framebuffers(vk_ctx);
// }

void __get_queue_indices(GDF_VkRenderContext* vk_ctx, VkPhysicalDevice physical_device, vk_pdevice_queues* queues)
{
    queues->graphics_family_index = -1;
    queues->present_family_index = -1;
    queues->compute_family_index = -1;
    queues->transfer_family_index = -1;

    u32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, 0);
    VkQueueFamilyProperties queue_families[queue_family_count];
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families);

    for (u32 i = 0; i < queue_family_count; i++) {

        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            queues->graphics_family_index = i;
        }

        if (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
        {
            queues->compute_family_index = i;
        }

        if (queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
        {
            queues->transfer_family_index = i;
        }

        VkBool32 supports_present = VK_FALSE;
        VK_ASSERT(vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, vk_ctx->surface, &supports_present));
        if (supports_present) {
            queues->present_family_index = i;
        }
    }

    LOG_TRACE("%d", queues->graphics_family_index);
    LOG_TRACE("%d", queues->compute_family_index);
    LOG_TRACE("%d", queues->present_family_index);
    LOG_TRACE("%d", queues->transfer_family_index);
}

// Up facing plane vertices
static const Vertex3d plane_vertices[] = {
    {{-0.5f, 0.5f, -0.5f}},
    {{0.5f, 0.5f, -0.5f}},
    {{0.5f, 0.5f, 0.5f}},
    {{-0.5f, 0.5f, 0.5f}},
};

static const u16 plane_indices[] = {
    0, 1, 2, 2, 3, 0
};

static void __create_global_vbos(GDF_VkRenderContext* vk_ctx)
{
    // GDF_VkBufferCreateVertex(
    //     vk_ctx,
    //     plane_vertices,
    //     sizeof(plane_vertices) / sizeof(*plane_vertices),
    //     sizeof(*plane_vertices),
    //     &vk_ctx->up_facing_plane_vbo
    // );
    // GDF_VkBufferCreateIndex(
    //     vk_ctx,
    //     plane_indices,
    //     sizeof(plane_indices) / sizeof(*plane_indices),
    //     &vk_ctx->up_facing_plane_index_buffer
    // );
}

// ===== FORWARD DECLARATIONS END =====
GDF_Renderer gdfe_renderer_init(
    GDF_Window window,
    GDF_AppState* app_state,
    bool disable_default,
    GDF_AppCallbacks* callbacks
)
{
    GDF_Renderer renderer = GDF_Malloc(sizeof(GDF_Renderer_T), GDF_MEMTAG_RENDERER);
    u16 w, h;
    GDF_GetWindowSize(window, &w, &h);
    renderer->framebuffer_width = w;
    renderer->framebuffer_height = h;
    renderer->callbacks = callbacks;
    renderer->app_state = app_state;
    renderer->disable_core = disable_default;

    GDF_VkRenderContext* vk_ctx = &renderer->vk_ctx;

    // TODO! custom allocator.
    vk_ctx->device.allocator = NULL;

    VkApplicationInfo app_info = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app_info.apiVersion = VK_API_VERSION_1_2;
    app_info.pApplicationName = "Not a GDFE app...";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "GDFE";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);

    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

    // extensions
    const char** extensions = GDF_LIST_Create(const char*);
    GDF_LIST_Push(extensions, &VK_KHR_SURFACE_EXTENSION_NAME);  // Generic surface extension
    GDF_VK_GetRequiredExtensionNames(&extensions);

#ifndef GDF_RELEASE
    GDF_LIST_Push(extensions, &VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    create_info.enabledExtensionCount = GDF_LIST_GetLength(extensions);
    create_info.ppEnabledExtensionNames = extensions;

    // validation layers
#ifndef GDF_RELEASE
    const char** validation_layers = GDF_LIST_Create(const char*);
    GDF_LIST_Push(validation_layers, &"VK_LAYER_KHRONOS_validation");
    create_info.enabledLayerCount = GDF_LIST_GetLength(validation_layers);
    create_info.ppEnabledLayerNames = validation_layers;
#endif
    VK_ASSERT(vkCreateInstance(&create_info, vk_ctx->device.allocator, &vk_ctx->instance));

    LOG_TRACE("Vulkan instance initialized successfully.");

#ifndef GDF_RELEASE
    LOG_TRACE("Creating Vulkan debugger...");
    u32 log_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;  //|
                                                                      //    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    debug_create_info.messageSeverity = log_severity;
    debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debug_create_info.pfnUserCallback = __vk_dbg_callback;

    PFN_vkCreateDebugUtilsMessengerEXT f =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vk_ctx->instance, "vkCreateDebugUtilsMessengerEXT");
    GDF_ASSERT_MSG((f) != NULL, "Function returned was NULL.");
    VK_ASSERT(f(vk_ctx->instance, &debug_create_info, vk_ctx->device.allocator, &vk_ctx->debug_messenger));
    LOG_TRACE("Vulkan debugger created.");
#endif

    /* ======================================== */
    /* ----- ENUMERATE PHYSICAL DEVICE INFORMATION ----- */
    /* ======================================== */
    u32 physical_device_count = 0;
    VK_ASSERT(vkEnumeratePhysicalDevices(vk_ctx->instance, &physical_device_count, NULL));
    if (physical_device_count == 0)
    {
        LOG_FATAL("There are no devices supporting vulkan on your system.");
        NULL;
    }
    VkPhysicalDevice physical_devices[physical_device_count];
    VK_ASSERT(vkEnumeratePhysicalDevices(vk_ctx->instance, &physical_device_count, physical_devices));

    /* ======================================== */
    /* ----- CREATE WINDOW SURFACE ----- */
    /* ======================================== */
    GDF_VK_CreateSurface(window, vk_ctx);

    vk_ctx->physical_device_info_list = GDF_LIST_Create(vk_physical_device);
    for (u32 i = 0; i < physical_device_count; i++)
    {
        vk_physical_device info = {
            .handle = physical_devices[i],
            .usable = true
        };
        // fill properties, features, and memoryinfo field
        vkGetPhysicalDeviceProperties(physical_devices[i], &info.properties);
        vkGetPhysicalDeviceFeatures(physical_devices[i], &info.features);
        vkGetPhysicalDeviceMemoryProperties(physical_devices[i], &info.memory);
        // fill the swapchain support info field
        // TODO!
        __query_swapchain_support(info.handle, vk_ctx->surface, &info.sc_support_info);
        // fill the queue info field
        __get_queue_indices(vk_ctx, info.handle, &info.queues);
        // TODO! may be a memory bug here check it out later
        GDF_LIST_Push(vk_ctx->physical_device_info_list, info);
    }

    __filter_available_devices(vk_ctx, vk_ctx->physical_device_info_list);

    /* ======================================== */
    /* ----- DEVICE SELECTION ----- */
    /* ======================================== */

    vk_physical_device* selected_physical_device = NULL;
    u32 device_num = GDF_LIST_GetLength(vk_ctx->physical_device_info_list);
    for (u32 i = 0; i < device_num; i++)
    {
        if (vk_ctx->physical_device_info_list[i].usable)
        {
            selected_physical_device = &vk_ctx->physical_device_info_list[i];
        }
    }

    if (selected_physical_device == NULL)
    {
        LOG_FATAL("Could not find a device to use.");
        GDF_Free(renderer);
        return NULL;
    }

    /* ======================================== */
    /* ----- FIND QUEUE INDICES ----- */
    /* ======================================== */

    bool present_equ_graphics = selected_physical_device->queues.graphics_family_index == selected_physical_device->queues.present_family_index;
    bool transfer_equ_graphics = selected_physical_device->queues.graphics_family_index == selected_physical_device->queues.transfer_family_index;
    bool transfer_equ_present = selected_physical_device->queues.present_family_index == selected_physical_device->queues.transfer_family_index;

    // see how many unique queue families we need
    u32 index_count = 1;
    if (!present_equ_graphics) {
        index_count++;
    }
    if (!transfer_equ_graphics && !transfer_equ_present) {
        index_count++;
    }

    u32 unique_indices[index_count];
    u8 index = 0;
    unique_indices[index++] = selected_physical_device->queues.graphics_family_index;

    if (!present_equ_graphics) {
        unique_indices[index++] = selected_physical_device->queues.present_family_index;
    }

    // Only add transfer queue if it's different from BOTH graphics AND present
    if (!transfer_equ_graphics && !transfer_equ_present) {
        unique_indices[index++] = selected_physical_device->queues.transfer_family_index;
    }

    /* ======================================== */
    /* ----- CREATE DEVICE & FETCH QUEUES ----- */
    /* ======================================== */

    VkDeviceQueueCreateInfo queue_create_infos[index_count];
    for (u32 i = 0; i < index_count; ++i) {

        queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_infos[i].queueFamilyIndex = unique_indices[i];
        queue_create_infos[i].queueCount = 1;

        // req 2 queues from the graphics family if needed
        if (unique_indices[i] == selected_physical_device->queues.graphics_family_index) {
            queue_create_infos[i].queueCount = 2;
        }

        queue_create_infos[i].flags = 0;
        queue_create_infos[i].pNext = 0;
        f32 queue_priority = 1.0f;
        queue_create_infos[i].pQueuePriorities = &queue_priority;
    }

    VkPhysicalDeviceFeatures device_features = {
        .samplerAnisotropy = VK_TRUE,
        .fillModeNonSolid = VK_TRUE,
        .shaderInt64 = VK_TRUE
    };

    VkDeviceCreateInfo device_create_info = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    device_create_info.queueCreateInfoCount = index_count;
    device_create_info.pQueueCreateInfos = queue_create_infos;
    device_create_info.pEnabledFeatures = &device_features;
    device_create_info.enabledExtensionCount = 1;
    const char* extension_names = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    device_create_info.ppEnabledExtensionNames = &extension_names;

    vk_ctx->device.physical_info = selected_physical_device;

    VK_ASSERT(
        vkCreateDevice(
            selected_physical_device->handle,
            &device_create_info,
            vk_ctx->device.allocator,
            &vk_ctx->device.handle
        )
    );

    LOG_TRACE("Created device.");

    vkGetDeviceQueue(
        vk_ctx->device.handle,
        vk_ctx->device.physical_info->queues.graphics_family_index,
        0,
        &vk_ctx->device.graphics_queue
    );

    vkGetDeviceQueue(
        vk_ctx->device.handle,
        vk_ctx->device.physical_info->queues.present_family_index,
        0,
        &vk_ctx->device.present_queue
    );

    vkGetDeviceQueue(
        vk_ctx->device.handle,
        vk_ctx->device.physical_info->queues.transfer_family_index,
        0,
        &vk_ctx->device.transfer_queue
    );

    LOG_TRACE("Got queues");

    // TODO! maybe generate procedurally idk
    vk_ctx->mip_levels = 1;

    /* ======================================== */
    /* ----- CREATE SWAPCHAIN, IMAGES ----- */
    /* ======================================== */
    // TODO! msaa support checks
    vk_ctx->msaa_samples = VK_SAMPLE_COUNT_4_BIT;
    __create_swapchain(renderer);
    u32 image_count = vk_ctx->swapchain.image_count;

    LOG_TRACE("Created swapchain and images.");

    /* ======================================== */
    /* ----- Allocate command pools & buffers ----- */
    /* ======================================== */
    VkCommandPoolCreateInfo pool_create_info = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    pool_create_info.queueFamilyIndex = vk_ctx->device.physical_info->queues.graphics_family_index;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_ASSERT(
        vkCreateCommandPool(
            vk_ctx->device.handle,
            &pool_create_info,
            vk_ctx->device.allocator,
            &vk_ctx->persistent_command_pool
        )
    );
    LOG_TRACE("Persistent command pool created.");
    // change queue index and flags
    // still use graphics because it supports both transfer and graphics operations
    pool_create_info.queueFamilyIndex = vk_ctx->device.physical_info->queues.graphics_family_index;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    VK_ASSERT(
        vkCreateCommandPool(
            vk_ctx->device.handle,
            &pool_create_info,
            vk_ctx->device.allocator,
            &vk_ctx->transient_command_pool
        )
    );
    LOG_TRACE("Transient command pool created.");

    const u32 max_concurrent_frames = vk_ctx->max_concurrent_frames;

    /* ======================================== */
    /* ----- Create and allocate ubos & descriptor things ----- */
    /* ======================================== */
    VkDeviceSize buffer_size = sizeof(ViewProjUB);
    vk_ctx->uniform_buffers = GDF_LIST_Reserve(GDF_VkUniformBuffer, image_count);
    // Create separate ubo for each swapchain image
    for (u32 i = 0; i < image_count; i++)
    {
        if (!GDF_VkBufferCreateUniform(vk_ctx, buffer_size, &vk_ctx->uniform_buffers[i]))
        {
            LOG_ERR("Failed to create a uniform buffer.");
            GDF_Free(renderer);
            return NULL;
        }
    }

    // Create descriptor pool and allocate sets
    vk_ctx->global_vp_ubo_sets = GDF_LIST_Reserve(VkDescriptorSet, image_count);
    vk_ctx->global_vp_ubo_layouts = GDF_LIST_Reserve(VkDescriptorSet, image_count);
    VkDescriptorPoolSize pool_size = {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = image_count
    };

    VkDescriptorPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .poolSizeCount = 1,
        .pPoolSizes = &pool_size,
        .maxSets = image_count
    };

    VK_ASSERT(
        vkCreateDescriptorPool(
            vk_ctx->device.handle,
            &pool_info,
            NULL,
            &vk_ctx->descriptor_pool
        )
    );

    VkDescriptorSetLayoutBinding layout_bindings[1] = {
        {
            .binding = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        }
    };

    for (u32 i = 0; i < image_count; i++)
    {
        VkDescriptorSetLayoutCreateInfo layout_create_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = sizeof(layout_bindings) / sizeof(VkDescriptorSetLayoutBinding),
            .pBindings = layout_bindings,
        };

        VK_ASSERT(
            vkCreateDescriptorSetLayout(
                vk_ctx->device.handle,
                &layout_create_info,
                vk_ctx->device.allocator,
                &vk_ctx->global_vp_ubo_layouts[i]
            )
        );
    }

    VkDescriptorSetAllocateInfo descriptor_sets_alloc_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = vk_ctx->descriptor_pool,
        .descriptorSetCount = image_count,
        .pSetLayouts = vk_ctx->global_vp_ubo_layouts
    };

    vkAllocateDescriptorSets(
        vk_ctx->device.handle,
        &descriptor_sets_alloc_info,
        vk_ctx->global_vp_ubo_sets
    );

    // Update descriptor sets
    for (u32 i = 0; i < image_count; i++)
    {
        // Vertex uniform buffer (geometry pass)
        VkDescriptorBufferInfo buffer_info = {
            .buffer = vk_ctx->uniform_buffers[i].buffer.handle,
            .offset = 0,
            .range = sizeof(ViewProjUB)
        };

        VkWriteDescriptorSet descriptor_writes[1] = {
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = vk_ctx->global_vp_ubo_sets[i],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1,
                .pBufferInfo = &buffer_info
            }
        };

        vkUpdateDescriptorSets(
            vk_ctx->device.handle,
            1,
            descriptor_writes,
            0,
            NULL
        );
    }
    LOG_TRACE("Created descriptor pool and allocated descriptor sets.");

    /* ======================================== */
    /* ----- CREATE RENDERPASSES, SHADERS, PIPELINES, FRAMEBUFFERS ----- */
    /* ======================================== */

    // vkDestroyDevice(vk_ctx->device.handle, vk_ctx->device.allocator);

    // if (!__create_renderpasses(vk_ctx))
    // {
    //     NULL;
    // }
    //
    // if (!__create_shader_modules())
    // {
    //     LOG_ERR("Failed to create shaders gg");
    //     NULL;
    // }
    //
    // LOG_TRACE("Created graphics pipelines & renderpasses");
    //
    // if (!__create_framebuffers(vk_ctx))
    // {
    //     NULL;
    // }
    //
    // LOG_TRACE("Created framebuffers");

    /* ======================================== */
    /* ----- Create per frame resources ----- */
    /* ======================================== */
    vk_ctx->per_frame = GDF_LIST_Reserve(PerFrameResources, max_concurrent_frames);
    vk_ctx->images_in_flight = GDF_LIST_Reserve(VkFence, max_concurrent_frames);

    VkSemaphoreCreateInfo semaphore_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };

    VkFenceCreateInfo fence_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    for (u32 i = 0; i < max_concurrent_frames; i++)
    {
        VkCommandBufferAllocateInfo command_buf_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = vk_ctx->persistent_command_pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };
        VK_ASSERT(
            vkAllocateCommandBuffers(
                vk_ctx->device.handle,
                &command_buf_info,
                &vk_ctx->per_frame[i].cmd_buffer
            )
        );
        VK_ASSERT(
            vkCreateSemaphore(
                vk_ctx->device.handle,
                &semaphore_info,
                vk_ctx->device.allocator,
                &vk_ctx->per_frame[i].image_available_semaphore
            )
        );
        VK_ASSERT(
            vkCreateSemaphore(
                vk_ctx->device.handle,
                &semaphore_info,
                vk_ctx->device.allocator,
                &vk_ctx->per_frame[i].render_finished_semaphore
            )
        );
        VK_ASSERT(
            vkCreateFence(
                vk_ctx->device.handle,
                &fence_info,
                vk_ctx->device.allocator,
                &vk_ctx->per_frame[i].in_flight_fence
            )
        );
    }

    // TODO! remove later
    __create_global_vbos(vk_ctx);

    if (!renderer->disable_core)
    {
        core_renderer_init(vk_ctx, &renderer->core_renderer);
        LOG_TRACE("Initialized core renderer");
    }

    if (callbacks->on_render_init)
    {
        if (!callbacks->on_render_init(vk_ctx, app_state, callbacks->on_render_init))
        {
            GDF_Free(renderer);
            return NULL;
        }
    }

    vk_ctx->ready_for_use = true;

    return renderer;
}

void gdfe_renderer_destroy(GDF_Renderer renderer)
{
    GDF_VkRenderContext* vk_ctx = &renderer->vk_ctx;
    VkDevice device = vk_ctx->device.handle;
    VkAllocationCallbacks* allocator = vk_ctx->device.allocator;

    if (renderer->callbacks->on_render_destroy)
    {
        renderer->callbacks->on_render_destroy(
            vk_ctx,
            renderer->app_state,
            renderer->callbacks->on_render_destroy_state
        );
    }

    // Destroy a bunch of pipelines
    if (!renderer->disable_core)
        core_renderer_destroy(&renderer->vk_ctx, &renderer->core_renderer);

    // for (u32 i = 0; i < GDF_VK_RENDERPASS_INDEX_MAX; i++)
    // {
    //     vkDestroyRenderPass(
    //         device,
    //         vk_ctx->renderpasses[i],
    //         allocator
    //     );
    // }
    //
    // // Destroy shader modules
    // for (u32 i = 0; i < GDF_VK_SHADER_MODULE_INDEX_MAX; i++)
    // {
    //     vkDestroyShaderModule(
    //         device,
    //         vk_ctx->builtin_shaders[i],
    //         allocator
    //     );
    // }

    u32 swapchain_image_count = vk_ctx->swapchain.image_count;
    for (u32 i = 0; i < swapchain_image_count; i++)
    {
        vkDestroyDescriptorSetLayout(
            device,
            vk_ctx->global_vp_ubo_layouts[i],
            allocator
        );
        vkDestroyFence(
            device,
            vk_ctx->per_frame[i].in_flight_fence,
            allocator
        );
        vkDestroySemaphore(
            device,
            vk_ctx->per_frame[i].image_available_semaphore,
            allocator
        );
        vkDestroySemaphore(
            device,
            vk_ctx->per_frame[i].render_finished_semaphore,
            allocator
        );
        vkFreeCommandBuffers(
            device,
            vk_ctx->persistent_command_pool,
            1,
            &vk_ctx->per_frame[i].cmd_buffer
        );
        GDF_VkBufferDestroyUniform(vk_ctx, &vk_ctx->uniform_buffers[i]);
    }
    __destroy_swapchain(vk_ctx);
    // done automatically i guess

    // vkFreeDescriptorSets(
    //     device,
    //     vk_ctx->descriptor_pool,
    //     swapchain_image_count,
    //     vk_ctx->descriptor_sets
    // );
    vkDestroyDescriptorPool(
        device,
        vk_ctx->descriptor_pool,
        allocator
    );
    vkDestroyCommandPool(
        device,
        vk_ctx->persistent_command_pool,
        allocator
    );
    vkDestroyCommandPool(
        device,
        vk_ctx->transient_command_pool,
        allocator
    );

    // GDF_VkBufferDestroy(vk_ctx, &vk_ctx->up_facing_plane_vbo);
    // GDF_VkBufferDestroy(vk_ctx, &vk_ctx->up_facing_plane_index_buffer);

    vkDestroySwapchainKHR(
        device,
        vk_ctx->swapchain.handle,
        allocator
    );

    LOG_TRACE("Destroying Vulkan device...");
    vkDestroyDevice(vk_ctx->device.handle, vk_ctx->device.allocator);

    LOG_TRACE("Destroying Vulkan surface...");
    if (vk_ctx->surface) {
        vkDestroySurfaceKHR(vk_ctx->instance, vk_ctx->surface, vk_ctx->device.allocator);
        vk_ctx->surface = 0;
    }

    LOG_TRACE("Destroying Vulkan debugger...");
#ifndef GDF_RELEASE
    if (vk_ctx->debug_messenger) {
        PFN_vkDestroyDebugUtilsMessengerEXT f =
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vk_ctx->instance, "vkDestroyDebugUtilsMessengerEXT");
        f(vk_ctx->instance, vk_ctx->debug_messenger, vk_ctx->device.allocator);
    }
#endif
    LOG_TRACE("Destroying Vulkan instance...");
    vkDestroyInstance(vk_ctx->instance, vk_ctx->device.allocator);
}
