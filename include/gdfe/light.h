#pragma once

#include <gdfe/core.h>
#include <gdfe/math/math.h>

typedef struct PointLight {
    vec3 pos;
    f32 strength;
} PointLight;