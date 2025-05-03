#pragma once

#include <gdfe/prelude.h>
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <gdfe/camera.h>
#include <gdfe/collections/list.h>

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
        LOG_ERR("%s returned: %s, in file: %s, line: %d. Returning GDF_FALSE..\n", #expr, string_VkResult((expr)), __FILE__, __LINE__) \
        return GDF_FALSE; \
    } \
} \

#define VK_RETURN_FALSE_IF_NULLHANDLE(expr) \
{ \
    if ((expr) == VK_NULL_HANDLE) \
    { \
        LOG_ERR("%s returned a VK_NULL_HANDLE, in file: %s, line: %d. Returning GDF_FALSE..\n", #expr, __FILE__, __LINE__) \
        return GDF_FALSE; \
    } \
} \

/* ======================================= */
/* ===== DEVICE TYPES ===== */
/* ======================================= */
typedef struct GDF_VkPhysicalDeviceSwapchainSupport {
    VkSurfaceCapabilitiesKHR capabilities;
    u32 format_count;
    VkSurfaceFormatKHR* formats;
    u32 present_mode_count;
    VkPresentModeKHR* present_modes;
} GDF_VkPhysicalDeviceSwapchainSupport;

typedef struct GDF_VkPhysicalDeviceQueues {
    i32 graphics_family_index;
    i32 present_family_index;
    i32 compute_family_index;
    i32 transfer_family_index;
} GDF_VkPhysicalDeviceQueues;

typedef struct GDF_VkPhysicalDevice {
    VkPhysicalDevice handle;
    GDF_VkPhysicalDeviceSwapchainSupport sc_support_info;
    GDF_VkPhysicalDeviceQueues queues;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory;

    struct {
        VkFormat image_format;
        VkColorSpaceKHR image_color_space;
        VkFormat depth_format;
    } formats;

    GDF_BOOL usable;
} GDF_VkPhysicalDeviceInfo;

typedef struct GDF_VkDevice {
    GDF_VkPhysicalDeviceInfo* physical_info;
    VkDevice handle;
    VkQueue graphics_queue;
    VkQueue present_queue;
    VkQueue transfer_queue;
    VkAllocationCallbacks* allocator;
} GDF_VkDevice;

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

    // This uniform buffer will be updated
    // every frame with the camera's view projection matrix
    GDF_VkUniformBuffer vp_ubo;
    // the descriptor set for accessing the current camera's
    // view projection matrix
    VkDescriptorSet vp_ubo_set;
} PerFrameResources;

typedef struct GDF_VkRenderContext {
    VkInstance instance;
    VkSurfaceKHR surface;
    vk_swapchain swapchain;

    // TODO! multiple command pools (per thread or something)?
    VkCommandPool persistent_command_pool;
    VkCommandPool transient_command_pool;

    GDF_LIST(GDF_VkPhysicalDeviceInfo) pdevices;
    GDF_VkDevice device;

    u64 current_frame;
    u32 max_concurrent_frames;

    GDF_LIST(PerFrameResources) per_frame;
    // Sync objects
    GDF_LIST(VkFence) images_in_flight;
    
    GDF_BOOL pending_resize_event;
    GDF_BOOL recreating_swapchain;
    GDF_BOOL ready_for_use;

    enum VkSampleCountFlagBits msaa_samples;
    u32 mip_levels;

    // The index that should be used for accessing resources due to
    // multiple possible frames in flight.
    u32 resource_idx;

    // global view projection camera stuff

    // This field is modified then copied over to vk_uniform_buffer[n].mapped_Data
    ViewProjUB view_proj_ub;
    VkDescriptorPool vp_ubo_pool;
    VkDescriptorSetLayout vp_ubo_layout;

#ifndef GDF_RELEASE
    VkDebugUtilsMessengerEXT debug_messenger;
#endif
} GDF_VkRenderContext;
