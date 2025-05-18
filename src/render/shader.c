#include <gdfe/render/shader.h>
#include <i_render/types.h>
#include <i_subsystems.h>
#include <shaderc/shaderc.h>

#include "gdfe/collections/hashmap.h"
#include "gdfe/render/vk/utils.h"
#include "i_render/renderer.h"

/// Contains the global state related to rendering.
typedef struct gdfe_shader_state {
    shaderc_compiler_t        shaderc_compiler;
    shaderc_compile_options_t shaderc_options;

    GDF_HashMap(GDF_Shader, GDF_LIST(VkPipeline));
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

GDF_Shader GDF_ShaderLoadSPIRV(const char* buf, u64 len)
{
    GDF_Shader_T* shader = GDF_Malloc(sizeof(GDF_Shader_T), GDF_MEMTAG_UNKNOWN);
    NONNULL_OR_RET(shader);

    VkShaderModule shader_module = mk_module(buf, len);
    if (shader_module == VK_NULL_HANDLE)
    {
        GDF_Free(shader);
        return GDF_NULL_HANDLE;
    }

    shader->shader_module = shader_module;

    return shader;
}


GDF_Shader GDF_ShaderLoadGLSL(const char* buf, u64 len, GDF_SHADER_TYPE type)
{
    u64 shader_type;
    switch (type)
    {
    case GDF_SHADER_TYPE_VERT:
        shader_type = shaderc_glsl_vertex_shader;
        break;

    case GDF_SHADER_TYPE_FRAG:
        shader_type = shaderc_glsl_fragment_shader;
        break;

    case GDF_SHADER_TYPE_COMP:
        shader_type = shaderc_glsl_compute_shader;
        break;

    default:
        return GDF_NULL_HANDLE;
    }

    shaderc_compilation_result_t result = shaderc_compile_into_spv(
        STATE.shaderc_compiler, buf, len, shader_type, "shader", "main", STATE.shaderc_options);

    if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success)
    {
        const char* error_msg = shaderc_result_get_error_message(result);
        LOG_ERR("GLSL compilation failed: %s\n", error_msg);
        shaderc_result_release(result);
        return GDF_NULL_HANDLE;
    }

    const char* spirv     = shaderc_result_get_bytes(result);
    size_t      spirv_len = shaderc_result_get_length(result);

    GDF_Shader shader = GDF_ShaderLoadSPIRV(spirv, spirv_len);

    shaderc_result_release(result);

    return shader;
}

GDF_Shader GDF_ShaderLoadGLSLFromFile(
    const char* path, GDF_SHADER_TYPE type, GDF_SHADER_RELOAD_MODE reload_mode)
{
    u64 read_bytes;
    const u8* buf = GDF_ReadFileExactLen(path, &read_bytes);
    GDF_Shader shader = GDF_ShaderLoadGLSL(buf, read_bytes, reload_mode);

#ifndef GDF_RELEASE
    shader->origin_format = GDF_SHADER_ORIGIN_FORMAT_GLSL;
    shader->file_path = path;
#endif
}

GDF_BOOL GDF_ShaderReload(GDF_Shader shader) { TODO("unimplemented"); }

void GDF_ShaderDestroy(GDF_Shader shader) { TODO("unimplemented"); }
