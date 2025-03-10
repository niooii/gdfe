#include <math.h>
#include <math/math.h>

#include <os/sysinfo.h>

static bool RAND_SEEDED = FALSE;

i32 GDF_Random()
{
    if (!RAND_SEEDED) 
    {
        srand((u32)GDF_GetAbsoluteTime());
        RAND_SEEDED = TRUE;
    }
    return rand();
}

i32 GDF_RandomRange(i32 min, i32 max)
{
    if (!RAND_SEEDED)
    {
        srand((u32)GDF_GetAbsoluteTime());
        RAND_SEEDED = TRUE;
    }
    return (rand() % (max - min + 1)) + min;
}

f32 GDF_FRandom()
{
    return (float)GDF_Random() / (f32)RAND_MAX;
}

f32 GDF_FRandomRange(f32 min, f32 max)
{
    return min + ((float)GDF_Random() / ((f32)RAND_MAX / (max - min)));
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