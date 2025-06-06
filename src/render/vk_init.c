#include <gdfe/collections/list.h>
#include <gdfe/gdfe.h>
#include <gdfe/render/renderer.h>
#include <gdfe/render/vk/enum_string_helper.h>
#include <gdfe/render/vk/utils.h>
#include <i_gdfe.h>
#include <i_render/core_renderer.h>
#include <i_render/renderer.h>
#include <i_render/vk_os.h>
#include <i_render/vk_utils.h>
#include <vulkan/vulkan.h>

VKAPI_ATTR VkBool32 VKAPI_CALL __vk_dbg_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT      message_severity,
    VkDebugUtilsMessageTypeFlagsEXT             message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data)
{
    switch (message_severity)
    {
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

// filters the vk_ctx->physical_device_info list
void __filter_available_devices(GDF_VkRenderContext* vk_ctx, GDF_VkPhysicalDeviceInfo* phys_list)
{
    u32 list_len = GDF_ListLen(phys_list);
    for (i32 i = (i32)list_len - 1; i >= 0; i--)
    {
        GDF_VkPhysicalDeviceInfo* device = &phys_list[i];
        LOG_TRACE("Checking device %s", device->properties.deviceName);

        GDF_BOOL device_suitable = GDF_TRUE;

        // check swapchain support
        if (device->sc_support_info.format_count < 1 ||
            device->sc_support_info.present_mode_count < 1)
        {
            device_suitable = GDF_FALSE;
            goto filter_device_skip;
        }
        // check queues support
        if (device->queues.compute_family_index == -1 ||
            device->queues.graphics_family_index == -1 ||
            device->queues.present_family_index == -1 || device->queues.transfer_family_index == -1)
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

GDF_BOOL gdfe_create_global_buffers(GDF_VkRenderContext* vk_ctx);
void     gdfe_destroy_global_buffers(GDF_VkRenderContext* vk_ctx);

// ===== FORWARD DECLARATIONS END =====

gdfe_render_state        GDFE_RENDER_STATE;
GDF_VkRenderContext*     GDFE_VK_CTX   = NULL;
GDF_CoreRendererContext* GDFE_CORE_CTX = NULL;

GDF_BOOL gdfe_renderer_init(GDF_Window window)
{
    if (GDFE_VK_CTX)
        return GDF_FALSE;

    u16 w, h;
    GDF_WinGetSize(window, &w, &h);
    GDFE_RENDER_STATE.framebuffer_width  = w;
    GDFE_RENDER_STATE.framebuffer_height = h;
    GDFE_RENDER_STATE.callbacks          = &APP_STATE.callbacks.render_callbacks;
    GDFE_RENDER_STATE.app_state          = &APP_STATE.public;

    GDF_VkRenderContext* vk_ctx = &GDFE_RENDER_STATE.vk_ctx;
    GDFE_VK_CTX                 = vk_ctx;
    GDFE_CORE_CTX               = &GDFE_RENDER_STATE.core_ctx;

    // TODO! custom allocator.
    vk_ctx->device.allocator = NULL;

    VkApplicationInfo app_info  = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
    app_info.apiVersion         = VK_API_VERSION_1_3;
    app_info.pApplicationName   = "Not a GDFE app...";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName        = "GDFE";
    app_info.engineVersion      = VK_MAKE_VERSION(1, 0, 0);

    VkInstanceCreateInfo create_info = {};
    create_info.sType                = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo     = &app_info;

    // extensions
    const char** extensions = GDF_ListCreate(const char*);
    GDF_ListPush(extensions, &VK_KHR_SURFACE_EXTENSION_NAME); // Generic surface extension
    GDF_VK_GetRequiredExtensionNames(&extensions);

#ifndef GDF_RELEASE
    GDF_ListPush(extensions, &VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    create_info.enabledExtensionCount   = GDF_ListLen(extensions);
    create_info.ppEnabledExtensionNames = extensions;

    // validation layers
#ifndef GDF_RELEASE
    const char** validation_layers = GDF_ListCreate(const char*);
    GDF_ListPush(validation_layers, &"VK_LAYER_KHRONOS_validation");
    create_info.enabledLayerCount   = GDF_ListLen(validation_layers);
    create_info.ppEnabledLayerNames = validation_layers;
#endif
    VK_ASSERT(vkCreateInstance(&create_info, vk_ctx->device.allocator, &vk_ctx->instance));

    LOG_TRACE("Vulkan instance initialized successfully.");

#ifndef GDF_RELEASE
    LOG_TRACE("Creating Vulkan debugger...");
    u32 log_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT; //|
                                                      //    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {
        VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT
    };
    debug_create_info.messageSeverity = log_severity;
    debug_create_info.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debug_create_info.pfnUserCallback = __vk_dbg_callback;

    PFN_vkCreateDebugUtilsMessengerEXT f =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            vk_ctx->instance, "vkCreateDebugUtilsMessengerEXT");
    GDF_ASSERT_MSG((f) != NULL, "Function returned was NULL.");
    VK_ASSERT(f(
        vk_ctx->instance, &debug_create_info, vk_ctx->device.allocator, &vk_ctx->debug_messenger));
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
        return GDF_FALSE;
    }
    VkPhysicalDevice physical_devices[physical_device_count];
    VK_ASSERT(
        vkEnumeratePhysicalDevices(vk_ctx->instance, &physical_device_count, physical_devices));

    GDF_VK_CreateSurface(window, vk_ctx);

    vk_ctx->pdevices = GDF_ListReserve(GDF_VkPhysicalDeviceInfo, physical_device_count);
    for (u32 i = 0; i < physical_device_count; i++)
    {
        gdfe_physical_device_init(vk_ctx, physical_devices[i], &vk_ctx->pdevices[i]);
    }

    GDF_ListSetLen(vk_ctx->pdevices, physical_device_count);

    __filter_available_devices(vk_ctx, vk_ctx->pdevices);

    /* ======================================== */
    /* ----- DEVICE SELECTION ----- */
    /* ======================================== */
    // TODO! integrated gpus (specifically my laptops) just breaks down
    // and dies. something about
    // vkCreateDevice(): pCreateInfo->pQueueCreateInfos[0].queueCount (2) is not less than or equal
    // to available queue count for this pCreateInfo->pQueueCreateInfos[0].queueFamilyIndex} (0)
    // obtained previously
    GDF_VkPhysicalDeviceInfo* selected_physical_device = NULL;
    u32                       device_num               = GDF_ListLen(vk_ctx->pdevices);
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
        return GDF_FALSE;
    }

    /* ======================================== */
    /* ----- FIND QUEUE INDICES ----- */
    /* ======================================== */

    GDF_BOOL present_equ_graphics = selected_physical_device->queues.graphics_family_index ==
        selected_physical_device->queues.present_family_index;
    GDF_BOOL transfer_equ_graphics = selected_physical_device->queues.graphics_family_index ==
        selected_physical_device->queues.transfer_family_index;
    GDF_BOOL transfer_equ_present = selected_physical_device->queues.present_family_index ==
        selected_physical_device->queues.transfer_family_index;

    // see how many unique queue families we need
    u32 index_count = 1;
    if (!present_equ_graphics)
    {
        index_count++;
    }
    if (!transfer_equ_graphics && !transfer_equ_present)
    {
        index_count++;
    }

    u32 unique_indices[index_count];
    u8  index               = 0;
    unique_indices[index++] = selected_physical_device->queues.graphics_family_index;

    if (!present_equ_graphics)
    {
        unique_indices[index++] = selected_physical_device->queues.present_family_index;
    }

    // Only add transfer queue if it's different from BOTH graphics AND present
    if (!transfer_equ_graphics && !transfer_equ_present)
    {
        unique_indices[index++] = selected_physical_device->queues.transfer_family_index;
    }

    /* ======================================== */
    /* ----- CREATE DEVICE & FETCH QUEUES ----- */
    /* ======================================== */

    VkDeviceQueueCreateInfo queue_create_infos[index_count];
    for (u32 i = 0; i < index_count; ++i)
    {

        queue_create_infos[i].sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_infos[i].queueFamilyIndex = unique_indices[i];
        queue_create_infos[i].queueCount       = 1;

        // req 2 queues from the graphics family if needed
        if (unique_indices[i] == selected_physical_device->queues.graphics_family_index)
        {
            queue_create_infos[i].queueCount = 2;
        }

        queue_create_infos[i].flags            = 0;
        queue_create_infos[i].pNext            = 0;
        f32 queue_priority                     = 1.0f;
        queue_create_infos[i].pQueuePriorities = &queue_priority;
    }

    VkPhysicalDeviceFeatures device_features = {
        .samplerAnisotropy = VK_TRUE,
        .fillModeNonSolid  = VK_TRUE,
        .shaderInt64       = VK_TRUE,
    };

    VkPhysicalDeviceDescriptorIndexingFeatures indexing_features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
        .descriptorBindingPartiallyBound               = VK_TRUE,
        .runtimeDescriptorArray                        = VK_TRUE,
        .descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE,
    };

    VkDeviceCreateInfo device_create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &indexing_features,
    };
    device_create_info.queueCreateInfoCount = index_count;
    device_create_info.pQueueCreateInfos    = queue_create_infos;
    const char* extension_names[]           = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
        VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
    };
    device_create_info.enabledExtensionCount   = sizeof(extension_names) / sizeof(*extension_names);
    device_create_info.ppEnabledExtensionNames = extension_names;

    // enable dynamic rendering and stuff
    VkPhysicalDeviceVulkan13Features vk_1_3_features = {
        .sType            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .dynamicRendering = VK_TRUE,
    };

    VkPhysicalDeviceFeatures2 device_features_2 = {
        .sType    = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext    = &vk_1_3_features,
        .features = device_features,
    };

    device_create_info.pNext = &device_features_2;

    vk_ctx->device.physical_info = selected_physical_device;

    VK_ASSERT(vkCreateDevice(selected_physical_device->handle, &device_create_info,
        vk_ctx->device.allocator, &vk_ctx->device.handle));

    LOG_TRACE("Created device.");

    vkGetDeviceQueue(vk_ctx->device.handle,
        vk_ctx->device.physical_info->queues.graphics_family_index, 0,
        &vk_ctx->device.graphics_queue);

    vkGetDeviceQueue(vk_ctx->device.handle,
        vk_ctx->device.physical_info->queues.present_family_index, 0,
        &vk_ctx->device.present_queue);

    vkGetDeviceQueue(vk_ctx->device.handle,
        vk_ctx->device.physical_info->queues.transfer_family_index, 0,
        &vk_ctx->device.transfer_queue);

    LOG_TRACE("Got queues");

    // TODO! maybe generate procedurally idk
    vk_ctx->mip_levels = 1;

    /* ======================================== */
    /* ----- CREATE SWAPCHAIN, IMAGES ----- */
    /* ======================================== */
    // TODO! msaa support checks
    vk_ctx->msaa_samples = VK_SAMPLE_COUNT_4_BIT;
    gdfe_swapchain_init(
        vk_ctx, GDFE_RENDER_STATE.framebuffer_width, GDFE_RENDER_STATE.framebuffer_height);
    u32 image_count = vk_ctx->swapchain.image_count;

    LOG_TRACE("Created swapchain.");

    /* ======================================== */
    /* ----- Allocate command pools & buffers ----- */
    /* ======================================== */
    VkCommandPoolCreateInfo pool_create_info = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    pool_create_info.queueFamilyIndex = vk_ctx->device.physical_info->queues.graphics_family_index;
    pool_create_info.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_ASSERT(vkCreateCommandPool(vk_ctx->device.handle, &pool_create_info,
        vk_ctx->device.allocator, &vk_ctx->persistent_command_pool));
    LOG_TRACE("Persistent command pool created.");
    // change queue index and flags
    // still use graphics because it supports both transfer and graphics operations
    pool_create_info.queueFamilyIndex = vk_ctx->device.physical_info->queues.graphics_family_index;
    pool_create_info.flags            = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    VK_ASSERT(vkCreateCommandPool(vk_ctx->device.handle, &pool_create_info,
        vk_ctx->device.allocator, &vk_ctx->transient_command_pool));
    LOG_TRACE("Transient command pool created.");

    const u32 max_concurrent_frames = vk_ctx->fof;

    /* ======================================== */
    /* ----- Create per frame resources ----- */
    /* ======================================== */
    vk_ctx->per_frame        = GDF_ListReserve(VkFrameResources, max_concurrent_frames);
    vk_ctx->images_in_flight = GDF_ListReserve(VkFence, max_concurrent_frames);

    VkSemaphoreCreateInfo semaphore_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    VkFenceCreateInfo fence_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    for (u32 i = 0; i < max_concurrent_frames; i++)
    {
        VkCommandBufferAllocateInfo command_buf_info = {
            .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool        = vk_ctx->persistent_command_pool,
            .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };
        VK_ASSERT(vkAllocateCommandBuffers(
            vk_ctx->device.handle, &command_buf_info, &vk_ctx->per_frame[i].cmd_buffer));
        VK_ASSERT(vkCreateSemaphore(vk_ctx->device.handle, &semaphore_info,
            vk_ctx->device.allocator, &vk_ctx->per_frame[i].image_available_semaphore));
        VK_ASSERT(vkCreateSemaphore(vk_ctx->device.handle, &semaphore_info,
            vk_ctx->device.allocator, &vk_ctx->per_frame[i].render_finished_semaphore));
        VK_ASSERT(vkCreateFence(vk_ctx->device.handle, &fence_info, vk_ctx->device.allocator,
            &vk_ctx->per_frame[i].in_flight_fence));
    }

    GDF_ASSERT(gdfe_create_global_buffers(vk_ctx));

    if (!core_renderer_init(vk_ctx, &GDFE_RENDER_STATE.core_ctx))
    {
        LOG_FATAL("Failed to init core renderer.");
        return GDF_FALSE;
    }
    LOG_TRACE("Initialized core renderer");

    GDF_RenderCallbacks* callbacks = GDFE_RENDER_STATE.callbacks;

    if (callbacks->on_render_init)
    {
        if (!callbacks->on_render_init(
                vk_ctx, GDFE_RENDER_STATE.app_state, callbacks->on_render_init_state))
        {
            return GDF_FALSE;
        }
    }

    vk_ctx->ready_for_use = GDF_TRUE;

    return GDF_TRUE;
}

void gdfe_renderer_shutdown()
{
    GDF_VkRenderContext*   vk_ctx    = &GDFE_RENDER_STATE.vk_ctx;
    VkDevice               device    = vk_ctx->device.handle;
    VkAllocationCallbacks* allocator = vk_ctx->device.allocator;

    vkDeviceWaitIdle(device);

    if (GDFE_RENDER_STATE.callbacks->on_render_destroy)
    {
        GDFE_RENDER_STATE.callbacks->on_render_destroy(vk_ctx, GDFE_RENDER_STATE.app_state,
            GDFE_RENDER_STATE.callbacks->on_render_destroy_state);
    }

    // Destroy a bunch of pipelines
    core_renderer_destroy(&GDFE_RENDER_STATE.vk_ctx, &GDFE_RENDER_STATE.core_ctx);

    gdfe_destroy_global_buffers(vk_ctx);

    u32 swapchain_image_count = vk_ctx->swapchain.image_count;
    for (u32 i = 0; i < swapchain_image_count; i++)
    {
        vkDestroyFence(device, vk_ctx->per_frame[i].in_flight_fence, allocator);
        vkDestroySemaphore(device, vk_ctx->per_frame[i].image_available_semaphore, allocator);
        vkDestroySemaphore(device, vk_ctx->per_frame[i].render_finished_semaphore, allocator);
        vkFreeCommandBuffers(
            device, vk_ctx->persistent_command_pool, 1, &vk_ctx->per_frame[i].cmd_buffer);
    }
    gdfe_swapchain_destroy(vk_ctx);
    vkDestroyCommandPool(device, vk_ctx->persistent_command_pool, allocator);
    vkDestroyCommandPool(device, vk_ctx->transient_command_pool, allocator);

    LOG_TRACE("Destroying Vulkan device...");
    vkDestroyDevice(vk_ctx->device.handle, vk_ctx->device.allocator);

    LOG_TRACE("Destroying Vulkan surface...");
    if (vk_ctx->surface)
    {
        vkDestroySurfaceKHR(vk_ctx->instance, vk_ctx->surface, vk_ctx->device.allocator);
    }

    LOG_TRACE("Destroying Vulkan debugger...");
#ifndef GDF_RELEASE
    if (vk_ctx->debug_messenger)
    {
        PFN_vkDestroyDebugUtilsMessengerEXT f =
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
                vk_ctx->instance, "vkDestroyDebugUtilsMessengerEXT");
        f(vk_ctx->instance, vk_ctx->debug_messenger, vk_ctx->device.allocator);
    }
#endif
    LOG_TRACE("Destroying Vulkan instance...");
    vkDestroyInstance(vk_ctx->instance, vk_ctx->device.allocator);
}
