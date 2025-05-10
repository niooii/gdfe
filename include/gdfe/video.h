#pragma once

#include <gdfe/def.h>

typedef struct GDF_Shader_T* GDF_Shader;

GDF_Shader GDF_ShaderLoadGLSL(const char* path);
GDF_Shader GDF_ShaderLoadSPIRV(const char* path);

GDF_Shader GDF_ShaderLoadStrGLSL(const char* path);
GDF_Shader GDF_ShaderLoadStrSPIRV(const char* path);

/// @note Shader hot reloading is disabled in release builds
GDF_BOOL GDF_ShaderReload(GDF_Shader shader);

/// Destroys a GDF_Shader object loaded by a GDF_ShaderLoad function.
///
/// @param shader The handle of the shader to destroy.
void GDF_ShaderDestroy(GDF_Shader shader);

EXTERN_C_BEGIN

EXTERN_C_END