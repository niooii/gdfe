#pragma once

#include <gdfe/def.h>

typedef struct GDF_Shader_T* GDF_Shader;

typedef enum GDF_SHADER_RELOAD_MODE {
    GDF_SHADER_RELOAD_NONE = 0,
    GDF_SHADER_RELOAD_ON_FILE_SAVE = 1,
    GDF_SHADER_RELOAD_ON_DEMAND = 2,
} GDF_SHADER_RELOAD_MODE;

EXTERN_C_BEGIN

GDF_Shader GDF_ShaderLoadStrSPIRV(const char* buf, u64 len);
GDF_Shader GDF_ShaderLoadStrGLSL(const char* buf, u64 len);

GDF_Shader GDF_ShaderLoadGLSL(const char* path, GDF_SHADER_RELOAD_MODE reload_mode);

/// @note Shader hot reloading is disabled in release builds
GDF_BOOL GDF_ShaderReload(GDF_Shader shader);

/// Destroys a GDF_Shader object loaded by a GDF_ShaderLoad function.
///
/// @param shader The handle of the shader to destroy.
void GDF_ShaderDestroy(GDF_Shader shader);

// hm
// void GDF_ShaderLinkPipelineForReload(GDF_Shader shader);

EXTERN_C_END
