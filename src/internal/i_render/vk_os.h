#pragma once

#include <gdfe/prelude.h>
#include <gdfe/os/video.h>
#include <gdfe/render/vk/types.h>

// Accepts a GDF_LIST of extention names and pushes the required ones.
// Function definition found in implememntations of video.h
void GDF_VK_GetRequiredExtensionNames(const char*** names_list);

// Function definition found in implememntations of video.h
GDF_BOOL GDF_VK_CreateSurface(GDF_Window window, GDF_VkRenderContext* context);