#pragma once

#include <gdfe/def.h>
#include <gdfe/math/math.h>
#include <vulkan/vulkan_core.h>

typedef struct GDF_Camera_T {
    vec3 pos;

    GDF_BOOL   constrain_pitch;
    f32        min_pitch;
    f32        max_pitch;
    vec3       euler;
    quaternion rotation;

    vec3 axis;
    vec3 axis_right;
    vec3 axis_forward;

    mat4 view_matrix;
    mat4 perspective_matrix;
    // view_matrix * perspective_matrix
    mat4 view_perspective;

    f32 aspect_ratio;
    f32 fov;
    f32 near_clip;
    f32 far_clip;

    vec3 up;
    vec3 right;
    vec3 forward;

    GDF_BOOL needs_view_recalc;
    GDF_BOOL needs_persp_recalc;
    GDF_BOOL needs_dir_vecs_recalc;
} GDF_Camera_T;

#ifndef GDF_RELEASE
// Types for shader hot reloading utilities

typedef enum GDF_SHADER_ORIGIN_FORMAT {
    GDF_SHADER_ORIGIN_FORMAT_GLSL,
    GDF_SHADER_ORIGIN_FORMAT_SPIRV,
} GDF_SHADER_ORIGIN_FORMAT;

#endif

typedef struct GDF_Shader_T {
    /// Hot reloading is disabled in release builds
    #ifndef GDF_RELEASE
    GDF_SHADER_ORIGIN_FORMAT origin_format;
    /// Used in the context of reloading a shader
    const char* file_path;
    #endif

    VkShaderModule shader_module;
} GDF_Shader_T;
