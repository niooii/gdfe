#include <gdfe/render/vk_types.h>
#include "irender/gpu_types.h"
#include <gdfe/collections/list.h>
#include "irender/renderer.h"
#include "irender/core_renderer.h"
#include <gdfe/render/renderer.h>
#include <vulkan/vk_enum_string_helper.h>

#include <gdfe/gdfe.h>
#include "irender/vk_os.h"
#include <gdfe/render/vk_utils.h>
#include <gdfe/render/vk_utils.h>
#include "irender/vk_utils.h"
#include <vulkan/vulkan.h>

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

// TODO! initialize lighting and postprocessing pipelines
// and separate pass for postprocessing
// GDF_BOOL __create_renderpasses(VkRenderContext* vk_ctx)
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
//     return GDF_TRUE;
// }

// filters the vk_ctx->physical_device_info list
void __filter_available_devices(GDF_VkRenderContext* vk_ctx, GDF_VkPhysicalDeviceInfo* phys_list)
{
    u32 list_len = GDF_LIST_GetLength(phys_list);
    for (i32 i = (i32)list_len - 1; i >= 0; i--)
    {
        GDF_VkPhysicalDeviceInfo* device = &phys_list[i];
        LOG_TRACE("Checking device %s", device->properties.deviceName);

        GDF_BOOL device_suitable = GDF_TRUE;

        // check swapchain support
        if (device->sc_support_info.format_count < 1 || device->sc_support_info.present_mode_count < 1)
        {
            device_suitable = GDF_FALSE;
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
            device_suitable = GDF_FALSE;
            goto filter_device_skip;
        }
        // TODO! Check if the device supports
        // all the required extensions

        filter_device_skip:
        if (!device_suitable)
        {
            LOG_TRACE("Marked device \'%s\' as unusable.", device->properties.deviceName);
            device->usable = GDF_FALSE;
        }
    }
}

// TODO! removeglobal context
GDF_VkRenderContext* GDFE_INTERNAL_VK_CTX = NULL;
GDF_CoreRendererContext* GDFE_INTERNAL_CORE_CTX = NULL;

GDF_BOOL create_global_buffers(GDF_VkRenderContext* vk_ctx);

// ===== FORWARD DECLARATIONS END =====
GDF_Renderer gdfe_renderer_init(
    GDF_Window window,
    GDF_AppState* app_state,
    GDF_AppCallbacks* callbacks
)
{
    if (GDFE_INTERNAL_VK_CTX)
        return NULL;

    GDF_Renderer renderer = GDF_Malloc(sizeof(GDF_Renderer_T), GDF_MEMTAG_RENDERER);
    u16 w, h;
    GDF_GetWindowSize(window, &w, &h);
    renderer->framebuffer_width = w;
    renderer->framebuffer_height = h;
    renderer->callbacks = &callbacks->render_callbacks;
    renderer->app_state = app_state;

    GDF_VkRenderContext* vk_ctx = &renderer->vk_ctx;
    GDFE_INTERNAL_VK_CTX = vk_ctx;
    GDFE_INTERNAL_CORE_CTX = &renderer->core_renderer;

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

    GDF_VK_CreateSurface(window, vk_ctx);

    vk_ctx->pdevices = GDF_LIST_Reserve(GDF_VkPhysicalDeviceInfo, physical_device_count);
    for (u32 i = 0; i < physical_device_count; i++)
    {
        gdfe_physical_device_init(vk_ctx, physical_devices[i], &vk_ctx->pdevices[i]);
    }

    GDF_LIST_SetLength(vk_ctx->pdevices, physical_device_count);

    __filter_available_devices(vk_ctx, vk_ctx->pdevices);

    /* ======================================== */
    /* ----- DEVICE SELECTION ----- */
    /* ======================================== */
    // TODO! integrated gpus (specifically my laptops) just breaks down
    // and dies. something about
    // vkCreateDevice(): pCreateInfo->pQueueCreateInfos[0].queueCount (2) is not less than or equal to available queue count for this pCreateInfo->pQueueCreateInfos[0].queueFamilyIndex} (0) obtained previously
    GDF_VkPhysicalDeviceInfo* selected_physical_device = NULL;
    u32 device_num = GDF_LIST_GetLength(vk_ctx->pdevices);
    for (u32 i = 0; i < device_num; i++)
    {
        if (vk_ctx->pdevices[i].usable)
        {
            selected_physical_device = &vk_ctx->pdevices[i];
            LOG_TRACE("Selected device: %s", selected_physical_device->properties.deviceName);
            break;
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

    GDF_BOOL present_equ_graphics = selected_physical_device->queues.graphics_family_index == selected_physical_device->queues.present_family_index;
    GDF_BOOL transfer_equ_graphics = selected_physical_device->queues.graphics_family_index == selected_physical_device->queues.transfer_family_index;
    GDF_BOOL transfer_equ_present = selected_physical_device->queues.present_family_index == selected_physical_device->queues.transfer_family_index;

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
    gdfe_swapchain_init(vk_ctx, renderer->framebuffer_width, renderer->framebuffer_height);
    u32 image_count = vk_ctx->swapchain.image_count;

    LOG_TRACE("Created swapchain.");

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

    GDF_ASSERT(create_global_buffers(vk_ctx));

    if (!core_renderer_init(vk_ctx, &renderer->core_renderer))
    {
        LOG_FATAL("Failed to init core renderer.");
        GDF_Free(renderer);
        return NULL;
    }
    LOG_TRACE("Initialized core renderer");

    if (callbacks->render_callbacks.on_render_init)
    {
        if (!callbacks->render_callbacks.on_render_init(vk_ctx, app_state, callbacks->render_callbacks.on_render_init_state))
        {
            GDF_Free(renderer);
            return NULL;
        }
    }

    vk_ctx->ready_for_use = GDF_TRUE;

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
    }
    gdfe_swapchain_destroy(vk_ctx);
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
