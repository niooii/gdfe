#pragma once

#include <core.h>
#include <math/math_types.h>

typedef struct PointLight {
    vec3 pos;
    f32 strength;
} PointLight;