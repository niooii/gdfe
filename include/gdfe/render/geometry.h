#pragma once

#include <gdfe/prelude.h>
#include <gdfe/math/math.h>

typedef struct GDF_Transform {
    vec3 pos;
    // Rotation is always in radians.
    // Represents pitch, yaw and roll.
    vec3 rot;
    vec3 scale;
} GDF_Transform;

EXTERN_C_BEGIN

FORCEINLINE mat4 GDF_TransformModelMatrix(GDF_Transform* transform)
{
    return mat4_mul(
        mat4_mul(
            mat4_scale(transform->scale), 
            mat4_rotation(transform->rot)
        ),
        mat4_translation(transform->pos)
    );
}

FORCEINLINE GDF_Transform GDF_TransformDefault()
{
    return (GDF_Transform) {
        .scale = vec3_new(1.0f, 1.0f, 1.0f),
    };
}

EXTERN_C_END