#include <gdfe/render/vk/image.h>
#include <gdfe/render/vk_utils.h>

#include "irender/renderer.h"

GDF_BOOL GDF_VkImageCreate(VkImageCreateInfo* image_info, VkImageViewCreateInfo* view_info, GDF_VkImage* out_image)
{
    VK_RETURN_FALSE_ASSERT(
        vkCreateImage(
            GDFE_INTERNAL_VK_CTX->device.handle,
            image_info,
            GDFE_INTERNAL_VK_CTX->device.allocator,
            &out_image->handle
        )
    );

    view_info->image = out_image->handle;

    VkMemoryRequirements mem_reqs;
    vkGetImageMemoryRequirements(GDFE_INTERNAL_VK_CTX->device.handle, out_image->handle, &mem_reqs);

    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = mem_reqs.size,
        .memoryTypeIndex = GDF_VkUtilsFindMemTypeIdx(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
    };

    VK_RETURN_FALSE_ASSERT(
        vkAllocateMemory(
            GDFE_INTERNAL_VK_CTX->device.handle,
            &allocInfo,
            GDFE_INTERNAL_VK_CTX->device.allocator,
            &out_image->memory
        )
    );

    vkBindImageMemory(
        GDFE_INTERNAL_VK_CTX->device.handle,
        out_image->handle,
        out_image->memory,
        0
    );

    VK_RETURN_FALSE_ASSERT(
        vkCreateImageView(
            GDFE_INTERNAL_VK_CTX->device.handle,
            view_info,
            GDFE_INTERNAL_VK_CTX->device.allocator,
            &out_image->view
        )
    );

    return GDF_TRUE;
}

void GDF_VkImageDestroy(GDF_VkRenderContext* GDFE_INTERNAL_VK_CTX, GDF_VkImage* image)
{
    vkDestroyImageView(
        GDFE_INTERNAL_VK_CTX->device.handle,
        image->view,
        GDFE_INTERNAL_VK_CTX->device.allocator
    );
    vkDestroyImage(
        GDFE_INTERNAL_VK_CTX->device.handle,
        image->handle,
        GDFE_INTERNAL_VK_CTX->device.allocator
    );
    vkFreeMemory(
        GDFE_INTERNAL_VK_CTX->device.handle,
        image->memory,
        GDFE_INTERNAL_VK_CTX->device.allocator
    );
}