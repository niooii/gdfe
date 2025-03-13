#include <gdfe/render/vk/image.h>
#include <gdfe/render/vk_utils.h>

GDF_BOOL GDF_VkImageCreate(VkImageCreateInfo* image_info, VkImageViewCreateInfo* view_info, GDF_VkImage* out_image)
{
    VK_RETURN_FALSE_ASSERT(
        vkCreateImage(
            vk_ctx->device.handle,
            image_info,
            vk_ctx->device.allocator,
            &out_image->handle
        )
    );

    view_info->image = out_image->handle;

    VkMemoryRequirements mem_reqs;
    vkGetImageMemoryRequirements(vk_ctx->device.handle, out_image->handle, &mem_reqs);

    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = mem_reqs.size,
        .memoryTypeIndex = GDF_VkUtilsFindMemTypeIdx(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
    };

    VK_RETURN_FALSE_ASSERT(
        vkAllocateMemory(
            vk_ctx->device.handle,
            &allocInfo,
            vk_ctx->device.allocator,
            &out_image->memory
        )
    );

    vkBindImageMemory(
        vk_ctx->device.handle,
        out_image->handle,
        out_image->memory,
        0
    );

    VK_RETURN_FALSE_ASSERT(
        vkCreateImageView(
            vk_ctx->device.handle,
            view_info,
            vk_ctx->device.allocator,
            &out_image->view
        )
    );

    return GDF_TRUE;
}

void GDF_VkImageDestroy(GDF_VkRenderContext* vk_ctx, GDF_VkImage* image)
{
    vkDestroyImageView(
        vk_ctx->device.handle,
        image->view,
        vk_ctx->device.allocator
    );
    vkDestroyImage(
        vk_ctx->device.handle,
        image->handle,
        vk_ctx->device.allocator
    );
    vkFreeMemory(
        vk_ctx->device.handle,
        image->memory,
        vk_ctx->device.allocator
    );
}