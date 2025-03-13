#include <gdfe/math/math.h>
#include <stdlib.h>
#include <gdfe/def.h>

#include <gdfe/os/sysinfo.h>

static GDF_BOOL RAND_SEEDED = GDF_FALSE;

i32 GDF_Random()
{
    if (!RAND_SEEDED) 
    {
        srand((u32)GDF_GetAbsoluteTime());
        RAND_SEEDED = GDF_TRUE;
    }
    return rand();
}

i32 GDF_RandomRange(i32 min, i32 max)
{
    if (!RAND_SEEDED)
    {
        srand((u32)GDF_GetAbsoluteTime());
        RAND_SEEDED = GDF_TRUE;
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