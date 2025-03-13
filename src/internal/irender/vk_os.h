#pragma once

#include <gdfe/../../../include/gdfe/core.h>
#include <gdfe/os/window.h>
#include <gdfe/render/vk_types.h>

// Accepts a GDF_LIST of extention names and pushes the required ones.
// Function definition found in implememntations of window.h
void GDF_VK_GetRequiredExtensionNames(const char*** names_list);

// Function definition found in implememntations of window.h
GDF_BOOL GDF_VK_CreateSurface(GDF_Window window, GDF_VkRenderContext* context);