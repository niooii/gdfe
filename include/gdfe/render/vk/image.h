#pragma once

#include <gdfe/prelude.h>
#include "types.h"

#ifdef __cplusplus
    extern "C" {
#endif

/// @note The .image field in the VkImageViewCreateInfo struct does not have to point
/// to a valid image. It will be modified to do so within this function.
GDF_BOOL GDF_VkImageCreate(VkImageCreateInfo* image_info, VkImageViewCreateInfo* view_info, GDF_VkImage* out_image);

void GDF_VkImageDestroy(GDF_VkRenderContext* context, GDF_VkImage* image);

#ifdef __cplusplus
    }
#endif