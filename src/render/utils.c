#include <gdfe/os/io.h>
#include <gdfe/render/vk/utils.h>
#include "i_render/vk_utils.h"

#include "i_render/renderer.h"

// Public utility implementations

// Only supports SPIR-V for now. GLSL shaders must be compiled into SPIR-V first.
VkShaderModule GDF_VkUtilsLoadShader(const char* src_rel_path)
{
    u64  src_size = GDF_GetFileSize(src_rel_path);
    char code[src_size];
    GDF_Memzero(code, src_size);
    if (!GDF_ReadFile(src_rel_path, code, src_size))
    {
        LOG_ERR("Failed to read file: %s", src_rel_path);
        return GDF_FALSE;
    }

    VkShaderModuleCreateInfo create_info = {
        .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = src_size,
        .pCode    = ((u32*)code),
    };

    VkShaderModule shader_module = VK_NULL_HANDLE;
    VkResult       res = vkCreateShaderModule(GDFE_INTERNAL_VK_CTX->device.handle, &create_info,
              GDFE_INTERNAL_VK_CTX->device.allocator, &shader_module);

    if (res != VK_SUCCESS)
        return VK_NULL_HANDLE;

    return shader_module;
}

i32 GDF_VkUtilsFindMemTypeIdx(u32 type_filter, u32 property_flags)
{
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(
        GDFE_INTERNAL_VK_CTX->device.physical_info->handle, &memory_properties);

    for (u32 i = 0; i < memory_properties.memoryTypeCount; i++)
    {
        // check each memory type to see if its bit is set to 1.
        if (type_filter & (1 << i) &&
            (memory_properties.memoryTypes[i].propertyFlags & property_flags) == property_flags)
        {
            return i;
        }
    }

    LOG_WARN("Unable to find suitable memory type!");
    return -1;
}

void GDF_VkPipelineInfoFillGeometryPass(VkGraphicsPipelineCreateInfo* info)
{
    info->renderPass = GDFE_INTERNAL_CORE_CTX->geometry_pass;
    info->subpass    = 0;
}
