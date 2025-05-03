#pragma once

#include <gdfe/prelude.h>
#include <gdfe/math/math.h>

typedef struct PointLight {
    vec3 pos;
    f32 strength;
} PointLight;