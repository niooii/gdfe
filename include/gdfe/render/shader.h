#pragma once

#include <gdfe/def.h>
#include <vulkan/vulkan_core.h>

typedef struct GDF_Shader_T* GDF_Shader;

/// Specify how shaders get hot reloaded if loaded through a file.
///
/// @note Shaders will not reload on release builds.
typedef enum GDF_SHADER_RELOAD_MODE {
    /// Shaders will not be setup for hot reloading at all.
    ///
    /// @note Shaders will not reload on release builds.
    GDF_SHADER_RELOAD_NONE = 0,

    /// Shaders will try to reload on any file modification.
    ///
    /// @note Shaders will not reload on release builds.
    GDF_SHADER_RELOAD_ON_FILE_SAVE,

    /// Shaders will not reload unless GDF_ShaderReload is explicitly called.
    ///
    /// @note Shaders will not reload on release builds.
    GDF_SHADER_RELOAD_ON_DEMAND,
} GDF_SHADER_RELOAD_MODE;

typedef enum GDF_SHADER_TYPE {
    GDF_SHADER_TYPE_VERT = 0,
    GDF_SHADER_TYPE_FRAG,
    GDF_SHADER_TYPE_COMP
} GDF_SHADER_TYPE;

EXTERN_C_BEGIN

GDF_Shader GDF_ShaderLoadSPIRV(const char* buf, u64 len);
GDF_Shader GDF_ShaderLoadGLSL(const char* buf, u64 len, GDF_SHADER_TYPE type);

GDF_Shader GDF_ShaderLoadGLSLFromFile(
    const char* path, GDF_SHADER_TYPE type, GDF_SHADER_RELOAD_MODE reload_mode);

VkShaderModule GDF_ShaderGetVkModule(GDF_Shader shader);

typedef struct GDF_VkPipelineBase GDF_VkPipelineBase;

#ifndef GDF_RELEASE

/// Reloads the given shader, halting the program and recreating all pipelines that
/// rely on the shader. This function returns GDF_TRUE if it is nonsensical to attempt
/// a shader reload - e.g. shader was loaded from a binary buffer not a file path.
///
/// TODO! should be asynchronously loaded probably.
/// @note Shader hot reloading is disabled in release builds
GDF_BOOL GDF_ShaderReload(GDF_Shader shader);
void     GDF_ShaderLinkPipeline(GDF_Shader shader, GDF_VkPipelineBase* pipeline);
void     GDF_ShaderUnlinkPipeline(GDF_Shader shader, GDF_VkPipelineBase* pipeline);

#else

FORCEINLINE GDF_BOOL GDF_ShaderReload(GDF_Shader shader) { return GDF_TRUE; }
FORCEINLINE void     GDF_ShaderLinkPipeline(GDF_Shader shader, GDF_VkPipelineBase* pipeline) {}
FORCEINLINE void     GDF_ShaderUnlinkPipeline(GDF_Shader shader, GDF_VkPipelineBase* pipeline) {}

#endif

/// Destroys a GDF_Shader object loaded by a GDF_ShaderLoad function.
///
/// @param shader The handle of the shader to destroy.
void GDF_ShaderDestroy(GDF_Shader shader);

EXTERN_C_END
