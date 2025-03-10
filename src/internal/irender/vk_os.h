#pragma once

#include <core.h>
#include <os/window.h>
#include <render/vk_types.h>

// Accepts a GDF_LIST of extention names and pushes the required ones.
// Function definition found in implememntations of window.h
void GDF_VK_GetRequiredExtensionNames(const char*** names_list);

// Function definition found in implememntations of window.h
bool GDF_VK_CreateSurface(GDF_Window window, VkRenderContext* context);