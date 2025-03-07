#pragma once

#include <def.h>

typedef union vec2_u {
    f32 elements[2];
    struct {
        f32 x;
        f32 y;
    };
} vec2;

typedef union vec3_u {
    f32 elements[3];
    struct {
        f32 x;
        f32 y;
        f32 z;
    };
} vec3;

typedef union vec4_u {
    f32 elements[4];
    struct {
        f32 x;
        f32 y;
        f32 z;
        f32 w;
    };
} vec4;

typedef union ivec2_u {
    i32 elements[2];
    struct {
        i32 x;
        i32 y;
    };
} ivec2;

typedef union ivec3_u {
    i32 elements[3];
    struct {
        i32 x;
        i32 y;
        i32 z;
    };
} ivec3;

typedef union ivec4_u {
    i32 elements[4];
    struct {
        i32 x;
        i32 y;
        i32 z;
        i32 w;
    };
} ivec4;

typedef vec4 Quaternion;

typedef union mat4_u {
    f32 data[16];
} mat4;