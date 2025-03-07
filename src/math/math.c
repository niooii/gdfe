#include <math.h>
#include <math/math_types.h>

#include <os/sysinfo.h>

static bool RAND_SEEDED = FALSE;

i32 random() 
{
    if (!RAND_SEEDED) 
    {
        srand((u32)GDF_GetAbsoluteTime());
        RAND_SEEDED = TRUE;
    }
    return rand();
}

i32 random_in_range(i32 min, i32 max) 
{
    if (!RAND_SEEDED)
    {
        srand((u32)GDF_GetAbsoluteTime());
        RAND_SEEDED = TRUE;
    }
    return (rand() % (max - min + 1)) + min;
}

f32 frandom() 
{
    return (float)random() / (f32)RAND_MAX;
}

f32 frandom_in_range(f32 min, f32 max) 
{
    return min + ((float)random() / ((f32)RAND_MAX / (max - min)));
}

ivec3 ivec3_adjacent_offsets[6] = {
    (ivec3) {
        .x = 0,
        .y = 0,
        .z = 1
    },
    (ivec3) {
        .x = 0,
        .y = 0,
        .z = -1
    },
    (ivec3) {
        .x = 0,
        .y = 1,
        .z = 0
    },
    (ivec3) {
        .x = 0,
        .y = -1,
        .z = 0
    },
    (ivec3) {
        .x = 1,
        .y = 0,
        .z = 0
    },
    (ivec3) {
        .x = -1,
        .y = 0,
        .z = 0
    }
};