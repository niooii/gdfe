#include <gdfe/render/shader.h>
#include <i_render/types.h>
#include <i_subsystems.h>
#include <shaderc/shaderc.h>

#include "gdfe/render/vk/utils.h"
#include "i_render/renderer.h"

/// Contains the global state related to rendering.
typedef struct gdfe_shader_state {
    shaderc_compiler_t        shaderc_compiler;
    shaderc_compile_options_t shaderc_options;
} gdfe_shader_state;

static gdfe_shader_state STATE;

void gdfe_shaders_init()
{
    // initialize shader compiler for hot reloading shaders and first time compilations
    // and stuff
    STATE.shaderc_compiler = shaderc_compiler_initialize();
    STATE.shaderc_options  = shaderc_compile_options_initialize();

    shaderc_compile_options_set_optimization_level(
        STATE.shaderc_options, shaderc_optimization_level_performance);
}

void gdfe_shaders_shutdown()
{
    shaderc_compile_options_release(STATE.shaderc_options);
    shaderc_compiler_release(STATE.shaderc_compiler);
}

// utility to make a vk shader module from a spirv buffer
static FORCEINLINE VkShaderModule mk_module(const char* buf, u64 len)
{
    VkShaderModuleCreateInfo create_info = {
        .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = len,
        .pCode    = (u32*)buf,
    };

    VkShaderModule shader_module = VK_NULL_HANDLE;
    VkResult       res = vkCreateShaderModule(GDFE_INTERNAL_VK_CTX->device.handle, &create_info,
              GDFE_INTERNAL_VK_CTX->device.allocator, &shader_module);

    if (res != VK_SUCCESS)
        return VK_NULL_HANDLE;

    return shader_module;
}

GDF_Shader GDF_ShaderLoadStrSPIRV(const char* buf, u64 len)
{
    TODO("unimplemented");
}

GDF_Shader GDF_ShaderLoadStrGLSL(const char* buf, u64 len) { TODO("unimplemented"); }

GDF_Shader GDF_ShaderLoadSPIRV(const char* path) { TODO("unimplemented"); }

GDF_Shader GDF_ShaderLoadGLSL(const char* path) { TODO("unimplemented"); }

GDF_BOOL GDF_ShaderReload(GDF_Shader shader) { TODO("unimplemented"); }

void GDF_ShaderDestroy(GDF_Shader shader) { TODO("unimplemented"); }
