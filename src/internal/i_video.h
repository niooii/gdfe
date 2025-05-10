#pragma once
#include <gdfe/def.h>
#include <gdfe/os/video.h>
#include <shaderc/shaderc.h>

// should be client width and height
// TODO! erm
GDF_BOOL set_internal_size(GDF_Window window, i16 w, i16 h);
GDF_BOOL pump_messages();

#ifndef GDF_RELEASE
// Types for shader hot reloading utilities

typedef enum GDF_SHADER_ORIGIN_FORMAT {
    GDF_SHADER_ORIGIN_FORMAT_GLSL,
    GDF_SHADER_ORIGIN_FORMAT_SPIRV,
} GDF_SHADER_ORIGIN_FORMAT;

#endif

typedef struct GDF_Shader_T {
    // Hot reloading is disabled in release builds
    #ifndef GDF_RELEASE
    GDF_SHADER_ORIGIN_FORMAT origin_format;

    /// Either loaded from a file or not, only file reloading is hot reloadable.
    GDF_BOOL origin_is_file;
    #endif


} GDF_Shader_T;