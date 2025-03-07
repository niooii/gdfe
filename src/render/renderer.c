#include <render/vk_types.h>
#include <render/gpu_types.h>
#include <collections/list.h>
#include <render/vk/os.h>
#include <render/renderer.h>
#include <vulkan/vk_enum_string_helper.h>

typedef struct GDF_Renderer_T {
    u64 frame_number;

    // Camera (and view and projection) stuff
    u32 framebuffer_width;
    u32 framebuffer_height;

    // Holds the current state of the game
    // TODO! maybe decouple from game state?
    // GDF_Game* game;
    // GDF_HashMap(ivec3, ChunkMesh*) chunk_meshes;
    // GDF_Set(ivec3) queued_remeshes;

    GDF_RENDER_MODE render_mode;

    GDF_Camera* camera;

    VkRenderContext vk_ctx;
} GDF_Renderer_T;

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

bool __create_shader_modules()
{
    // load all (built in) shaders
    // TODO!

    return true;
}

// TODO! initialize lighting and postprocessing pipelines
// and separate pass for postprocessing
bool __create_renderpasses(VkRenderContext* vk_ctx)
{
    /* ======================================== */
    /* ----- CREATE RENDERPASS AND SUBPASSES ----- */
    /* ======================================== */

    // color
    VkAttachmentDescription color_attachment = {
        .format = vk_ctx->formats.image_format,
        .samples = vk_ctx->msaa_samples,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .flags = 0
    };

    // depth
    VkAttachmentDescription depth_attachment = {
        .format = vk_ctx->formats.depth_format,
        .samples = vk_ctx->msaa_samples,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        .flags = 0
    };

    // resolve
    VkAttachmentDescription resolved_attachment = {
        .format = vk_ctx->formats.image_format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    VkAttachmentReference color_attachment_ref = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };
    
    VkAttachmentReference depth_attachment_ref = {
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };
    
    VkAttachmentReference resolved_attachment_ref = {
        .attachment = 2,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    u32 attachment_counts = 3;
    VkAttachmentDescription attachments[] = {
        color_attachment, depth_attachment, resolved_attachment
    };

    // TODO! multiple subpasses
    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment_ref,
        .pDepthStencilAttachment = &depth_attachment_ref,
        .pResolveAttachments = &resolved_attachment_ref
    };

    VkSubpassDependency dependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dependencyFlags = 0
    };

    VkRenderPassCreateInfo rp_create_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency,
        .attachmentCount = attachment_counts,
        .pAttachments = attachments,
    };

    VK_ASSERT(
        vkCreateRenderPass(
            vk_ctx->device.handle,
            &rp_create_info,
            vk_ctx->device.allocator,
            &vk_ctx->renderpasses[GDF_VK_RENDERPASS_INDEX_MAIN]
        )
    );

    return true;
}

// filters the vk_ctx->physical_device_info list 
void __filter_available_devices(VkRenderContext* vk_ctx, vk_physical_device* phys_list)
{
    u32 list_len = GDF_LIST_GetLength(phys_list);
    for (u32 i = list_len; i > 0; i--)
    {
        vk_physical_device* device = &vk_ctx->physical_device_info_list[i - 1]; 
        LOG_DEBUG("Checking device %s", device->properties.deviceName);

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
            LOG_DEBUG("Device does not have the required queues.");
            // LOG_DEBUG("yes %d", );
            device_suitable = false;
            goto filter_device_skip;
        }
        // TODO! Check if the device supports
        // all the required extensions

        filter_device_skip:
        if (!device_suitable)
        {
            LOG_DEBUG("Marked device \'%s\' as unusable.", device->properties.deviceName);
            LOG_DEBUG("%d", device->queues.graphics_family_index);
            LOG_DEBUG("%d", device->queues.compute_family_index);
            LOG_DEBUG("%d", device->queues.present_family_index);
            LOG_DEBUG("%d", device->queues.transfer_family_index);
            device->usable = false;
        }
    }
}

bool __create_swapchain_and_images(GDF_RendererState* state)
{
    VkRenderContext* vk_ctx = &state->vk_ctx;

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
    vk_ctx->formats.depth_format = utils_find_supported_depth_format(vk_ctx->device.physical_info->handle);
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
        vk_ctx->swapchain.images = GDF_LIST_Reserve(vk_image, image_count);

    // Create corresponding image views
    for (u32 i = 0; i < image_count; i++)
    {
        vk_ctx->swapchain.images[i].handle = images[i];
        VkImageViewCreateInfo image_view_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .format = vk_ctx->formats.image_format,
            .image = vk_ctx->swapchain.images[i].handle,
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
                &vk_ctx->swapchain.images[i].view
            )
        );
    }

    GDF_LIST_SetLength(vk_ctx->swapchain.images, image_count);
    
    LOG_DEBUG("Fetched %d images from swapchain...", GDF_LIST_GetLength(vk_ctx->swapchain.images));

    // create MSAA resources
    VkImageCreateInfo colorImageInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .extent = {
            .width = vk_ctx->swapchain.extent.width,
            .height = vk_ctx->swapchain.extent.height,
            .depth = 1
        },
        .mipLevels = vk_ctx->mip_levels,
        .arrayLayers = 1,
        .format = vk_ctx->formats.image_format,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .samples = vk_ctx->msaa_samples,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VK_ASSERT(
        vkCreateImage(
            vk_ctx->device.handle,
            &colorImageInfo,
            vk_ctx->device.allocator,
            &vk_ctx->msaa_image
        )
    );

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(vk_ctx->device.handle, vk_ctx->msaa_image, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = utils_find_memory_type_idx(vk_ctx, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
    };

    VK_ASSERT(
        vkAllocateMemory(
            vk_ctx->device.handle,
            &allocInfo,
            vk_ctx->device.allocator,
            &vk_ctx->msaa_image_memory
        )
    );

    vkBindImageMemory(
        vk_ctx->device.handle,
        vk_ctx->msaa_image,
        vk_ctx->msaa_image_memory,
        0
    );

    // Create color image view
    VkImageViewCreateInfo colorViewInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = vk_ctx->msaa_image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = vk_ctx->formats.image_format,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = vk_ctx->mip_levels,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    VK_ASSERT(
        vkCreateImageView(
            vk_ctx->device.handle,
            &colorViewInfo,
            vk_ctx->device.allocator,
            &vk_ctx->msaa_image_view
        )
    );

    // Create the global depth image
    VkImageCreateInfo depth_image_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .extent.width = vk_ctx->swapchain.extent.width,
        .extent.height = vk_ctx->swapchain.extent.height,
        .extent.depth = 1,
        .mipLevels = vk_ctx->mip_levels,
        .arrayLayers = 1,
        .format = vk_ctx->formats.depth_format,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .samples = vk_ctx->msaa_samples,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VK_ASSERT(
        vkCreateImage(
            vk_ctx->device.handle, 
            &depth_image_info, 
            vk_ctx->device.allocator, 
            &vk_ctx->depth_image
        )
    );

    // Allocate memory for depth image
    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(vk_ctx->device.handle, vk_ctx->depth_image, &mem_requirements);

    i32 mem_idx = utils_find_memory_type_idx(vk_ctx, mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (mem_idx == -1) 
    {
        LOG_FATAL("Failed to find memory suitable for depth image. Exiting...");
    }

    VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = mem_requirements.size,
        .memoryTypeIndex = mem_idx
    };

    VK_ASSERT(
        vkAllocateMemory(
            vk_ctx->device.handle, 
            &alloc_info, 
            vk_ctx->device.allocator, 
            &vk_ctx->depth_image_memory
        )
    );

    vkBindImageMemory(
        vk_ctx->device.handle, 
        vk_ctx->depth_image, 
        vk_ctx->depth_image_memory, 
        0
    );

    // Create depth image view
    VkImageViewCreateInfo depth_view_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = vk_ctx->depth_image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = vk_ctx->formats.depth_format,
        .subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
        .subresourceRange.baseMipLevel = 0,
        .subresourceRange.levelCount = vk_ctx->mip_levels,
        .subresourceRange.baseArrayLayer = 0,
        .subresourceRange.layerCount = 1
    };

    VK_ASSERT(
        vkCreateImageView(
            vk_ctx->device.handle, 
            &depth_view_info, 
            vk_ctx->device.allocator, 
            &vk_ctx->depth_image_view
        )
    );

    return true;
}

void __destroy_swapchain_and_images(VkRenderContext* vk_ctx)
{
    VkDevice device = vk_ctx->device.handle;
    VkAllocationCallbacks* allocator = vk_ctx->device.allocator;
    for (u32 i = 0; i < vk_ctx->swapchain.image_count; i++)
    {
        vkDestroyImageView(
            device,
            vk_ctx->swapchain.images[i].view,
            allocator
        );
    }

    // destroy depth image
    vkDestroyImage(
        device,
        vk_ctx->depth_image,
        allocator
    );
    vkDestroyImageView(
        device,
        vk_ctx->depth_image_view,
        allocator
    );
    vkFreeMemory(
        device, 
        vk_ctx->depth_image_memory,
        allocator
    );

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

bool __create_framebuffers(VkRenderContext* vk_ctx)
{
    // Create framebuffers
    u32 image_count = vk_ctx->swapchain.image_count;
    if (!vk_ctx->recreating_swapchain)
        vk_ctx->swapchain.framebuffers = GDF_LIST_Reserve(VkFramebuffer, image_count);
    for (u32 i = 0; i < image_count; i++) 
    {
        vk_image* img = &vk_ctx->swapchain.images[i];

        VkImageView image_views[3] = {
            vk_ctx->msaa_image_view,
            vk_ctx->depth_image_view,
            img->view
        };

        VkFramebufferCreateInfo framebuffer_info = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .attachmentCount = 3,
            .pAttachments = image_views,
            .renderPass = vk_ctx->renderpasses[GDF_VK_RENDERPASS_INDEX_MAIN],
            .width = vk_ctx->swapchain.extent.width,
            .height = vk_ctx->swapchain.extent.height,
            .layers = 1
        };

        VK_ASSERT(
            vkCreateFramebuffer(
                vk_ctx->device.handle,
                &framebuffer_info,
                vk_ctx->device.allocator,
                &vk_ctx->swapchain.framebuffers[i]
            )
        );
    }
    return true;
}

void __destroy_framebuffers(VkRenderContext* vk_ctx)
{
    for (u32 i = 0; i < vk_ctx->swapchain.image_count; i++)
    {
        vkDestroyFramebuffer(
            vk_ctx->device.handle,
            vk_ctx->swapchain.framebuffers[i],
            vk_ctx->device.allocator
        );
    }
    
    if (!vk_ctx->recreating_swapchain)
        GDF_LIST_Destroy(vk_ctx->swapchain.framebuffers);
}

bool __recreate_sized_resources(GDF_RendererState* state)
{
    VkRenderContext* vk_ctx = &state->vk_ctx;
    
    __destroy_framebuffers(vk_ctx);
    __destroy_swapchain_and_images(vk_ctx);

    return __create_swapchain_and_images(state)
        && __create_framebuffers(vk_ctx);
}

void __get_queue_indices(VkRenderContext* vk_ctx, VkPhysicalDevice physical_device, vk_pdevice_queues* queues)
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

    // LOG_DEBUG("%d", queues->graphics_family_index);
    // LOG_DEBUG("%d", queues->compute_family_index);
    // LOG_DEBUG("%d", queues->present_family_index);
    // LOG_DEBUG("%d", queues->transfer_family_index);
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

static void __create_global_vbos(VkRenderContext* vk_ctx)
{
    buffers_create_vertex(
        vk_ctx,
        plane_vertices,
        sizeof(plane_vertices) / sizeof(*plane_vertices),
        sizeof(*plane_vertices),
        &vk_ctx->up_facing_plane_vbo
    );
    buffers_create_index(
        vk_ctx,
        plane_indices,
        sizeof(plane_indices) / sizeof(*plane_indices),
        &vk_ctx->up_facing_plane_index_buffer
    );
}

// ===== FORWARD DECLARATIONS END =====
bool GDF_RendererInit(GDF_Window window)
{
    GDF_Renderer renderer = GDF_Malloc(sizeof(GDF_Renderer_T), GDF_MEMTAG_RENDERER);
    
    VkRenderContext* vk_ctx = &renderer->vk_ctx;

    // TODO! custom allocator.
    vk_ctx->device.allocator = 0;


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

    LOG_DEBUG("Vulkan instance initialized successfully.");

#ifndef GDF_RELEASE
    LOG_DEBUG("Creating Vulkan debugger...");
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
    LOG_DEBUG("Vulkan debugger created.");
#endif
    
    /* ======================================== */
    /* ----- ENUMERATE PHYSICAL DEVICE INFORMATION ----- */
    /* ======================================== */
    u32 physical_device_count = 0;
    VK_ASSERT(vkEnumeratePhysicalDevices(vk_ctx->instance, &physical_device_count, NULL));
    if (physical_device_count == 0)
    {
        LOG_FATAL("There are no devices supporting vulkan on your system.");
        return false;
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
        return false;
    }

    /* ======================================== */
    /* ----- FIND QUEUE INDICES ----- */
    /* ======================================== */

    bool present_shares_graphics_queue = selected_physical_device->queues.graphics_family_index == selected_physical_device->queues.present_family_index;
    bool transfer_shares_graphics_queue = selected_physical_device->queues.graphics_family_index == selected_physical_device->queues.transfer_family_index;
    u32 index_count = 1;
    // don't create queues for shared indices
    if (!present_shares_graphics_queue) {
        index_count++;
    }
    if (!transfer_shares_graphics_queue) {
        index_count++;
    }
    u32 indices[index_count];
    u8 index = 0;
    indices[index++] = selected_physical_device->queues.graphics_family_index;
    if (!present_shares_graphics_queue) {
        indices[index++] = selected_physical_device->queues.present_family_index;
    }
    if (!transfer_shares_graphics_queue) {
        indices[index++] = selected_physical_device->queues.transfer_family_index;
    }

    /* ======================================== */
    /* ----- CREATE DEVICE & FETCH QUEUES ----- */
    /* ======================================== */

    VkDeviceQueueCreateInfo queue_create_infos[index_count];
    for (u32 i = 0; i < index_count; ++i) {
        queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_infos[i].queueFamilyIndex = indices[i];
        queue_create_infos[i].queueCount = 1;
        if (indices[i] == selected_physical_device->queues.graphics_family_index) {
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

    LOG_DEBUG("Created device.");

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
    
    LOG_DEBUG("Got queues");

    // TODO! maybe generate procedurally idk
    vk_ctx->mip_levels = 1;

    /* ======================================== */
    /* ----- CREATE SWAPCHAIN, IMAGES ----- */
    /* ======================================== */
    // TODO! msaa support checks
    vk_ctx->msaa_samples = VK_SAMPLE_COUNT_4_BIT;
    __create_swapchain_and_images(renderer);
    u32 image_count = vk_ctx->swapchain.image_count;

    LOG_DEBUG("Created swapchain and images.");

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
    LOG_DEBUG("Persistent command pool created.");
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
    LOG_DEBUG("Transient command pool created.");

    u32 max_concurrent_frames = vk_ctx->max_concurrent_frames;
    VkCommandBufferAllocateInfo command_buf_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = vk_ctx->persistent_command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = max_concurrent_frames
    };
    vk_ctx->command_buffers = GDF_LIST_Reserve(VkCommandBuffer, max_concurrent_frames);
    VK_ASSERT(
        vkAllocateCommandBuffers(
            vk_ctx->device.handle, 
            &command_buf_info, 
            vk_ctx->command_buffers
        )
    );
    GDF_LIST_SetLength(vk_ctx->command_buffers, max_concurrent_frames);

    /* ======================================== */
    /* ----- Create and allocate ubos & descriptor things ----- */
    /* ======================================== */
    VkDeviceSize buffer_size = sizeof(ViewProjUB);
    vk_ctx->uniform_buffers = GDF_LIST_Reserve(vk_uniform_buffer, image_count);
    // Create separate ubo for each swapchain image
    for (u32 i = 0; i < image_count; i++) 
    {
        if (!buffers_create_uniform(vk_ctx, buffer_size, &vk_ctx->uniform_buffers[i]))
        {
            LOG_ERR("Failed to create a uniform buffer.");
            return false;
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
    LOG_DEBUG("Created descriptor pool and allocated descriptor sets.");

    /* ======================================== */
    /* ----- CREATE RENDERPASSES, SHADERS, PIPELINES, FRAMEBUFFERS ----- */
    /* ======================================== */

    // vkDestroyDevice(vk_ctx->device.handle, vk_ctx->device.allocator);

    if (!__create_renderpasses(vk_ctx))
    {
        return false;
    }

    if (!__create_shader_modules())
    {
        LOG_ERR("Failed to create shaders gg");
        return false;
    }

    // TODO! create builtin pipelines
    // if (!pipelines_create_blocks(vk_ctx))
    // {
    //     LOG_ERR("Failed to create block rendering pipeline");
    //     return false;
    // }
    //
    // if (!pipelines_create_grid(vk_ctx))
    // {
    //     LOG_ERR("Failed to create grid rendering pipeline");
    //     return false;
    // }
    //
    // if (!pipelines_create_ui(vk_ctx))
    // {
    //     LOG_ERR("Failed to create UI rendering pipeline");
    //     return false;
    // }

    LOG_DEBUG("Created graphics pipelines & renderpasses");

    if (!__create_framebuffers(vk_ctx))
    {
        return false;
    }

    LOG_DEBUG("Created framebuffers");

    /* ======================================== */
    /* ----- Create sync objects ----- */
    /* ======================================== */  
    vk_ctx->image_available_semaphores = GDF_LIST_Reserve(VkSemaphore, max_concurrent_frames);
    vk_ctx->render_finished_semaphores = GDF_LIST_Reserve(VkSemaphore, max_concurrent_frames);
    vk_ctx->in_flight_fences = GDF_LIST_Reserve(VkFence, max_concurrent_frames);
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
        VK_ASSERT(
            vkCreateSemaphore(
                vk_ctx->device.handle,
                &semaphore_info,
                vk_ctx->device.allocator,
                &vk_ctx->image_available_semaphores[i]
            )
        );
        VK_ASSERT(
            vkCreateSemaphore(
                vk_ctx->device.handle,
                &semaphore_info,
                vk_ctx->device.allocator,
                &vk_ctx->render_finished_semaphores[i]
            )
        );
        VK_ASSERT(
            vkCreateFence(
                vk_ctx->device.handle,
                &fence_info,
                vk_ctx->device.allocator,
                &vk_ctx->in_flight_fences[i]
            )
        );
    }

    LOG_DEBUG("Created sync objects");

    // TODO! remove later
    __create_global_vbos(vk_ctx);
    LOG_DEBUG("Created example cube buffers (remove later)")

    LOG_INFO("Finished initialization of vulkan stuff...");

    // TODO! render init callback here
    // if (!vk_game_renderer_init(vk_ctx, renderer))
    // {
    //     LOG_ERR("Failed to init game renderer..");
    //     return false;
    // }

    vk_ctx->ready_for_use = true;

    return true;
}

void vk_renderer_destroy(GDF_RendererState* state)
{
    VkRenderContext* vk_ctx = &state->vk_ctx;
    VkDevice device = vk_ctx->device.handle;
    VkAllocationCallbacks* allocator = vk_ctx->device.allocator;
    // Destroy a bunch of pipelines
    vkDestroyPipeline(
        device,
        vk_ctx->block_pipeline.handle,
        allocator
    );
    vkDestroyPipelineLayout(
        device,
        vk_ctx->block_pipeline.layout,
        allocator
    );

    vkDestroyPipeline(
        device,
        vk_ctx->grid_pipeline.handle,
        allocator
    );
    vkDestroyPipelineLayout(
        device,
        vk_ctx->grid_pipeline.layout,
        allocator
    );
    
    vkDestroyPipeline(device, vk_ctx->ui_pipeline.handle, allocator);
    vkDestroyPipelineLayout(device, vk_ctx->ui_pipeline.layout, allocator);

    for (u32 i = 0; i < GDF_VK_RENDERPASS_INDEX_MAX; i++)
    {
        vkDestroyRenderPass(
            device,
            vk_ctx->renderpasses[i],
            allocator
        );
    }

    // Destroy shader modules
    for (u32 i = 0; i < GDF_VK_SHADER_MODULE_INDEX_MAX; i++)
    {
        vkDestroyShaderModule(
            device,
            vk_ctx->builtin_shaders[i],
            allocator
        );
    }

    u32 swapchain_image_count = vk_ctx->swapchain.image_count;
    // Destroy descriptor layouts ubos and sync stuff
    for (u32 i = 0; i < swapchain_image_count; i++)
    {
        vkDestroyDescriptorSetLayout(
            device,
            vk_ctx->global_vp_ubo_layouts[i],
            allocator
        );
        vkDestroyFence(
            device,
            vk_ctx->images_in_flight[i],
            allocator
        );
        vkDestroySemaphore(
            device,
            vk_ctx->image_available_semaphores[i],
            allocator
        );
        vkDestroySemaphore(
            device,
            vk_ctx->render_finished_semaphores[i],
            allocator
        );
        buffers_destroy_uniform(vk_ctx, &vk_ctx->uniform_buffers[i]);
        // swapchain images destroyed automatically i think
        vkDestroyImageView(
            device,
            vk_ctx->swapchain.images[i].view,
            allocator
        );
        vkDestroyFramebuffer(
            device,
            vk_ctx->swapchain.framebuffers[i],
            allocator
        );
    }

    // destroy depth image resources
    vkDestroyImageView(  
        device,
        vk_ctx->depth_image_view,
        allocator
    );
    vkFreeMemory(
        device,
        vk_ctx->depth_image_memory,
        allocator
    );
    vkDestroyImage(
        device,
        vk_ctx->depth_image,
        allocator
    );
    
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
    vkFreeCommandBuffers(
        device,
        vk_ctx->persistent_command_pool,
        vk_ctx->max_concurrent_frames,
        vk_ctx->command_buffers
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

    buffers_destroy(vk_ctx, &vk_ctx->up_facing_plane_vbo);
    buffers_destroy(vk_ctx, &vk_ctx->up_facing_plane_index_buffer);

    vkDestroySwapchainKHR(
        device,
        vk_ctx->swapchain.handle,
        allocator
    );

    LOG_DEBUG("Destroying Vulkan device...");
    vkDestroyDevice(vk_ctx->device.handle, vk_ctx->device.allocator);

    LOG_DEBUG("Destroying Vulkan surface...");
    if (vk_ctx->surface) {
        vkDestroySurfaceKHR(vk_ctx->instance, vk_ctx->surface, vk_ctx->device.allocator);
        vk_ctx->surface = 0;
    }

    LOG_DEBUG("Destroying Vulkan debugger...");
#ifndef GDF_RELEASE
    if (vk_ctx->debug_messenger) {
        PFN_vkDestroyDebugUtilsMessengerEXT f =
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vk_ctx->instance, "vkDestroyDebugUtilsMessengerEXT");
        f(vk_ctx->instance, vk_ctx->debug_messenger, vk_ctx->device.allocator);
    }
#endif
    LOG_DEBUG("Destroying Vulkan instance...");
    vkDestroyInstance(vk_ctx->instance, vk_ctx->device.allocator);
}

void vk_renderer_resize(GDF_RendererState* state, u16 width, u16 height)
{
    state->vk_ctx.pending_resize_event = true;
}

bool vk_renderer_begin_frame(GDF_RendererState* state, f32 delta_time)
{
    VkRenderContext* vk_ctx = &state->vk_ctx;
    vk_device* device = &vk_ctx->device;
    // because there are separate resources for each frame in flight.
    vk_ctx->resource_idx = vk_ctx->current_frame % vk_ctx->max_concurrent_frames;
    u32 resource_idx = vk_ctx->resource_idx;

    if (!vk_ctx->ready_for_use) {
        if (!vk_result_is_success(vkDeviceWaitIdle(device->handle))) {
            LOG_ERR("begin frame vkDeviceWaitIdle (1) failed");
            return false;
        }
        LOG_DEBUG("Something is happening w the renderer");
        return false;
    }

    // Check if the framebuffer has been resized. If so, a new swapchain must be created.
    if (vk_ctx->pending_resize_event) 
    {
        if (!vk_result_is_success(vkDeviceWaitIdle(device->handle)))
        {
            LOG_ERR("begin frame vkDeviceWaitIdle (2) failed");
            return false;
        }

        vk_ctx->ready_for_use = false;
        // TODO! HANDLE MINIMIZES BETTER... 
        if (state->framebuffer_height == 0 && state->framebuffer_width == 0)
        {
            return false;
        }

        vk_ctx->recreating_swapchain = true;

        if (!__recreate_sized_resources(state)) {
            return false;
        }

        LOG_INFO("Resized successfully.");
        vk_ctx->pending_resize_event = false;
        vk_ctx->recreating_swapchain = false;
        vk_ctx->ready_for_use = true;
        return false;
    }

    // Wait for the previous frame to finish
    vkWaitForFences(
        device->handle, 
        1,
        &vk_ctx->in_flight_fences[resource_idx],
        VK_TRUE,
        UINT64_MAX
    );

    // Acquire the next image from the swapchain
    VK_RETURN_FALSE_ASSERT(
        vkAcquireNextImageKHR(
            vk_ctx->device.handle, 
            vk_ctx->swapchain.handle, 
            UINT64_MAX, 
            vk_ctx->image_available_semaphores[resource_idx], 
            VK_NULL_HANDLE, 
            &vk_ctx->swapchain.current_img_idx
        )
    );
    // TODO! handle VK_ERROR_OUT_OF_DATE_KHR and resizing stuff etc etc

    // shorthand i guess i aint typing all that 
    u32 current_img_idx = vk_ctx->swapchain.current_img_idx;

    GDF_Camera* active_camera = state->camera;
    // TODO! remove and extract updating ubo into another function
    ViewProjUB ubo = {
        .view_projection = active_camera->view_perspective
    };
    memcpy(vk_ctx->uniform_buffers[current_img_idx].mapped_data, &ubo, sizeof(ubo));

    // Check if a previous frame is using this image (if there is its fence to wait on)
    if (vk_ctx->images_in_flight[current_img_idx] != VK_NULL_HANDLE) 
    {
        vkWaitForFences(device->handle, 1, &vk_ctx->images_in_flight[current_img_idx], VK_TRUE, UINT64_MAX);
        // basically mark image as free
        vk_ctx->images_in_flight[current_img_idx] = VK_NULL_HANDLE;
    }
    vkResetFences(device->handle, 1, &vk_ctx->in_flight_fences[resource_idx]);
    vk_ctx->images_in_flight[current_img_idx] = vk_ctx->in_flight_fences[resource_idx];

    VkCommandBuffer cmd_buffer = vk_ctx->command_buffers[resource_idx];
    vkResetCommandBuffer(cmd_buffer, 0);

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    vkBeginCommandBuffer(cmd_buffer, &begin_info);

    VkRenderPassBeginInfo render_pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = vk_ctx->renderpasses[GDF_VK_RENDERPASS_INDEX_MAIN],
        .framebuffer = vk_ctx->swapchain.framebuffers[current_img_idx],
        .renderArea.offset = {0, 0},
        .renderArea.extent = vk_ctx->swapchain.extent
    };

    VkClearValue clear_values[2] = {
        {.color = {0, 0, 0, 1}},
        {.depthStencil = {1, 0}}
    };
    render_pass_info.clearValueCount = 2;
    render_pass_info.pClearValues = clear_values;

    vkCmdBeginRenderPass(cmd_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    // make as similar to opengls as possible
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = (f32)state->framebuffer_height;
    viewport.width = (f32)state->framebuffer_width;
    viewport.height = -(f32)state->framebuffer_height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);

    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = vk_ctx->swapchain.extent
    };
    vkCmdSetScissor(cmd_buffer, 0, 1, &scissor);

    // TODO! draw UI

    // TODO! prerender callbacks here.
    // if (!vk_game_renderer_draw(vk_ctx, state, resource_idx, delta_time))
    // {
    //     LOG_ERR("Failed to render the game.");
    //     // TODO! handle some weird sync stuff here
    //     return false;
    // }

    // draw debug grid
    vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, context->grid_pipeline.handle);

    vkCmdBindVertexBuffers(cmd_buffer, 0, 1, &context->up_facing_plane_vbo.handle, offsets);
    vkCmdBindIndexBuffer(cmd_buffer, context->up_facing_plane_index_buffer.handle, 0, VK_INDEX_TYPE_UINT16);

    vkCmdBindDescriptorSets(
        cmd_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        context->grid_pipeline.layout,
        0,
        1,
        &context->global_vp_ubo_sets[resource_idx],
        0,
        NULL
    );

    vkCmdPushConstants(
        cmd_buffer,
        context->grid_pipeline.layout,
        VK_SHADER_STAGE_VERTEX_BIT,
        0,
        sizeof(vec3),
        &active_camera->pos
    );

    vkCmdDrawIndexed(cmd_buffer, 6, 1, 0, 0, 0);

    // TODO! post processing here

    return true;
}

bool vk_renderer_end_frame(GDF_RendererState* state, f32 delta_time)
{
    VkRenderContext* vk_ctx = &state->vk_ctx;
    vk_device* device = &vk_ctx->device;
    u32 resource_idx = vk_ctx->resource_idx;
    VkCommandBuffer cmd_buffer = vk_ctx->command_buffers[resource_idx];
    u32 current_img_idx = vk_ctx->swapchain.current_img_idx;

    vkCmdEndRenderPass(cmd_buffer);

    if (vkEndCommandBuffer(cmd_buffer) != VK_SUCCESS) {
        
        return false;
    }

    // Submit the command buffer
    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &vk_ctx->image_available_semaphores[resource_idx],
        .pWaitDstStageMask = (VkPipelineStageFlags[]) {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd_buffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &vk_ctx->render_finished_semaphores[resource_idx]
    };
    vkQueueSubmit(
        device->graphics_queue,
        1,
        &submit_info,
        vk_ctx->in_flight_fences[resource_idx]
    );

    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pImageIndices = &current_img_idx,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &vk_ctx->render_finished_semaphores[resource_idx],
        .swapchainCount = 1,
        .pSwapchains = &vk_ctx->swapchain.handle,
        .pResults = NULL,
    };

    vkQueuePresentKHR(
        vk_ctx->device.present_queue, 
        &present_info
    );

    vk_ctx->current_frame++;

    // call postrender callbacks here

    return true;
}