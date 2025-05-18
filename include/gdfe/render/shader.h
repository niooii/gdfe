#pragma once

#include <gdfe/def.h>

typedef struct GDF_Shader_T* GDF_Shader;

/// Specify how shaders get hot reloaded if loaded through a file.
///
/// @note Shaders will not reload on release builds.
typedef enum GDF_SHADER_RELOAD_MODE {
    /// Shaders will not be setup for hot reloading at all.
    ///
    /// @note Shaders will not reload on release builds.
    GDF_SHADER_RELOAD_NONE         = 0,

    /// Shaders will try to reload on any file modification.
    ///
    /// @note Shaders will not reload on release builds.
    GDF_SHADER_RELOAD_ON_FILE_SAVE = 1,

    /// Shaders will not reload unless GDF_ShaderReload is explicitly called.
    ///
    /// @note Shaders will not reload on release builds.
    GDF_SHADER_RELOAD_ON_DEMAND    = 2,
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

/// Reloads the given shader, halting the program and recreating all pipelines that
/// rely on the shader.
///
/// TODO! should be asynchronously loaded probably.
/// @note Shader hot reloading is disabled in release builds
GDF_BOOL GDF_ShaderReload(GDF_Shader shader);

/// Destroys a GDF_Shader object loaded by a GDF_ShaderLoad function.
///
/// @param shader The handle of the shader to destroy.
void GDF_ShaderDestroy(GDF_Shader shader);

// hm
// void GDF_ShaderLinkPipelineForReload(GDF_Shader shader);

EXTERN_C_END
