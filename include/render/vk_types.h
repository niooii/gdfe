#pragma once

#include <core.h>
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <camera.h>
#include <collections/list.h>

#define VK_ASSERT(expr) \
{ \
    if ((expr) != VK_SUCCESS) \
    { \
        LOG_FATAL("%s returned: %s, in file: %s, line: %d\n", #expr, string_VkResult((expr)), __FILE__, __LINE__) \
        debugBreak(); \
    } \
} \

#define VK_RETURN_FALSE_ASSERT(expr) \
{ \
    if ((expr) != VK_SUCCESS) \
    { \
        LOG_ERR("%s returned: %s, in file: %s, line: %d. Returning false..\n", #expr, string_VkResult((expr)), __FILE__, __LINE__) \
        return false; \
    } \
} \

#define VK_RETURN_FALSE_IF_NULLHANDLE(expr) \
{ \
    if ((expr) == VK_NULL_HANDLE) \
    { \
        LOG_ERR("%s returned a VK_NULL_HANDLE, in file: %s, line: %d. Returning false..\n", #expr, __FILE__, __LINE__) \
        return false; \
    } \
} \

typedef enum GDF_VK_SHADER_MODULE_INDEX {
    GDF_VK_SHADER_MODULE_INDEX_BLOCKS_VERT,
    GDF_VK_SHADER_MODULE_INDEX_BLOCKS_FRAG,
    GDF_VK_SHADER_MODULE_INDEX_LIGHTING_VERT,
    GDF_VK_SHADER_MODULE_INDEX_LIGHTING_FRAG,
    GDF_VK_SHADER_MODULE_INDEX_POST_PROCESS_VERT,
    GDF_VK_SHADER_MODULE_INDEX_POST_PROCESS_FRAG,
    GDF_VK_SHADER_MODULE_INDEX_GRID_VERT,
    GDF_VK_SHADER_MODULE_INDEX_GRID_FRAG,
    GDF_VK_SHADER_MODULE_INDEX_UI_VERT,
    GDF_VK_SHADER_MODULE_INDEX_UI_FRAG,

    GDF_VK_SHADER_MODULE_INDEX_MAX,
} GDF_VK_SHADER_MODULE_INDEX;

typedef enum GDF_VK_RENDERPASS_INDEX {
    GDF_VK_RENDERPASS_INDEX_MAIN,

    GDF_VK_RENDERPASS_INDEX_MAX,
} GDF_VK_RENDERPASS_INDEX;

/* ======================================= */
/* ===== DEVICE TYPES ===== */
/* ======================================= */
typedef struct vk_pdevice_swapchain_support {
    VkSurfaceCapabilitiesKHR capabilities;
    u32 format_count;
    VkSurfaceFormatKHR* formats;
    u32 present_mode_count;
    VkPresentModeKHR* present_modes;
} vk_pdevice_swapchain_support;

typedef struct vk_pdevice_queues {
    i32 graphics_family_index;
    i32 present_family_index;
    i32 compute_family_index;
    i32 transfer_family_index;
} vk_pdevice_queues;

typedef struct vk_physical_device {
    VkPhysicalDevice handle;
    vk_pdevice_swapchain_support sc_support_info;
    vk_pdevice_queues queues;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory;
    bool usable;
} vk_physical_device;

typedef struct vk_device {
    vk_physical_device* physical_info;
    VkDevice handle;
    VkQueue graphics_queue;
    VkQueue present_queue;
    VkQueue transfer_queue;
    VkAllocationCallbacks* allocator;
} vk_device;

/* ======================================= */
/* ===== BUFFER TYPES ===== */
/* ======================================= */

typedef struct GDF_VkBuffer {
    u32 mem_property_flags;
    VkBuffer handle;
    VkDeviceMemory memory;

    u64 size;
    // should not be directly accessed bc i will be implementing different shit
    // later.  
    void* mapped_data;
} GDF_VkBuffer;

typedef struct GDF_VkUniformBuffer {
    GDF_VkBuffer buffer;
    void* mapped_data;
} GDF_VkUniformBuffer;

typedef struct GDF_VkImage {
    VkImage handle;
    VkImageView view;
    VkDeviceMemory memory;
} GDF_VkImage;

typedef struct ViewProjUB {
    mat4 view_projection;
} ViewProjUB;

typedef struct vk_swapchain {
    VkSwapchainKHR handle;

    GDF_LIST(VkImage) images;
    GDF_LIST(VkImageView) image_views;

    u32 current_img_idx;
    VkExtent2D extent;
    u32 image_count;
} vk_swapchain;

typedef struct PerFrameResources {
    VkFence in_flight_fence;
    VkSemaphore image_available_semaphore;
    VkSemaphore render_finished_semaphore;
    VkCommandBuffer cmd_buffer;
} PerFrameResources;

typedef struct GDF_VkRenderContext {
    VkInstance instance;
    VkSurfaceKHR surface;
    vk_swapchain swapchain;

    // All default pipelines used in application
    // TODO! disable these if the user requests fully custom rendering.
    
    struct {
        VkFormat image_format;
        VkColorSpaceKHR image_color_space;
        VkFormat depth_format;
    } formats;

    // All vertex shaders will get input from these uniform buffers.
    GDF_LIST(GDF_VkUniformBuffer) uniform_buffers;
    // This field is modified then copied over to vk_uniform_buffer[n].mapped_Data
    ViewProjUB uniform_buffer_data;
    VkDescriptorPool descriptor_pool;
    GDF_LIST(VkDescriptorSet) global_vp_ubo_sets;
    GDF_LIST(VkDescriptorSetLayout) global_vp_ubo_layouts;

    // TODO! multiple command pools (per thread or something)?
    VkCommandPool persistent_command_pool;
    VkCommandPool transient_command_pool;

    GDF_LIST(vk_physical_device) physical_device_info_list;
    vk_device device;

    u32 current_frame;
    u32 max_concurrent_frames;

    GDF_LIST(PerFrameResources) per_frame;
    // Sync objects
    GDF_LIST(VkFence) images_in_flight;
    
    bool pending_resize_event;
    bool recreating_swapchain;
    bool ready_for_use;

    enum VkSampleCountFlagBits msaa_samples;
    u32 mip_levels;

    // The index that should be used for accessing resources due to
    // multiple possible frames in flight.
    u32 resource_idx;

#ifndef GDF_RELEASE
    VkDebugUtilsMessengerEXT debug_messenger;
#endif
} GDF_VkRenderContext;
