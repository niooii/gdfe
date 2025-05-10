#pragma once

#if defined _WIN32 || defined _WIN64
    #define OS_WINDOWS
#elif defined __linux__
    #define OS_LINUX
// me when i cant test if my stuff works on a mac without a mac
#endif

#include <stdint.h>

typedef unsigned char GDF_BOOL;
#define GDF_TRUE  1
#define GDF_FALSE 0

typedef uint8_t       u8;
typedef uint16_t      u16;
typedef uint32_t      u32;
typedef uint64_t      u64;
typedef int8_t        i8;
typedef int16_t       i16;
typedef int32_t       i32;
typedef int64_t       i64;
typedef float         f32;
typedef double        f64;
typedef unsigned char byte;
#define i32_MAX 2147483647

// TODO! move some of these defines into a private defs.h

#ifdef __cplusplus
    #define EXTERN_C_BEGIN extern "C" {
    #define EXTERN_C_END   }
#else
    #define EXTERN_C_BEGIN
    #define EXTERN_C_END
#endif

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define FLOOR(x) \
    ((x) >= 0.0 ? (long long)(x) : (((long long)(x) == (x)) ? (x) : ((long long)(x)-1)))

#define RET_FALSE(expr)       \
    {                         \
        if (!(expr))          \
            return GDF_FALSE; \
    }


#define MB_TO_KB(mb) (mb * 1000)
#define MB_TO_B(mb)  (mb * 1000000)
#define MB_TO_GB(mb) (mb / 1000.0)
#define KB_TO_B(kb)  (kb * 1000)
#define KB_TO_MB(kb) (kb / 1000.0)
#define KB_TO_GB(kb) (kb / 1000000.0)
#define B_TO_KB(b)   (b / 1000.0)
#define B_TO_MB(b)   (b / 1000000.0)
#define B_TO_GB(b)   (b / 1000000000.0)
#define GB_TO_MB(gb) (gb * 1000)
#define GB_TO_KB(gb) (gb * 1000000)
#define GB_TO_B(gb)  (gb * 1000000000)

#define GET_BITS(data, offset, bit_width) ((data >> offset) & (((TYPEOF(data))1 << bit_width) - 1))

#define SET_BITS(data, offset, bit_width, bits)                          \
    data = ((data & ~((((TYPEOF(data))1 << bit_width) - 1) << offset)) | \
        ((bits & (((TYPEOF(data))1 << bit_width) - 1)) << offset))

#if defined(__clang__) || defined(__gcc__)
    #define STATIC_ASSERT _Static_assert
#else
    #define STATIC_ASSERT static_assert
#endif

STATIC_ASSERT(sizeof(u8) == 1, "expected u8 to be 1 byte.");
STATIC_ASSERT(sizeof(u16) == 2, "expected u16 to be 2 bytes.");
STATIC_ASSERT(sizeof(u32) == 4, "expected u32 to be 4 bytes.");
STATIC_ASSERT(sizeof(u64) == 8, "expected u64 to be 8 bytes.");
STATIC_ASSERT(sizeof(i8) == 1, "expected i8 to be 1 byte.");
STATIC_ASSERT(sizeof(i16) == 2, "expected i16 to be 2 bytes.");
STATIC_ASSERT(sizeof(i32) == 4, "expected i32 to be 4 bytes.");
STATIC_ASSERT(sizeof(i64) == 8, "expected i64 to be 8 bytes.");
STATIC_ASSERT(sizeof(f32) == 4, "expected f32 to be 4 bytes.");
STATIC_ASSERT(sizeof(f64) == 8, "expected f64 to be 8 bytes.");

#define CLAMP(value, min, max) ((value <= min) ? min : (value >= max) ? max : value)
#define SIGN(x)                ((x > 0) ? 1.0f : ((x < 0) ? -1.0f : 0.0f))

// Microsoft Visual C compiler
#ifdef _MSC_VER

    // typeof is a keyword since C23
    #define TYPEOF      typeof
    #define FORCEINLINE __forceinline
    #define NOINLINE    __declspec(noinline)

    #define PACKED_STRUCT __pragma(pack(push, 1)) struct
    #define END_PACKED_STRUCT \
        ;                     \
        __pragma(pack(pop))

    #define CTZ(x)        _tzcnt_u32(x) // Count trailing zeros (32-bit)
    #define CTZ64(x)      _tzcnt_u64(x) // Count trailing zeros (64-bit)
    #define CLZ(x)        _lzcnt_u32(x) // Count leading zeros (32-bit)
    #define CLZ64(x)      _lzcnt_u64(x) // Count leading zeros (64-bit)
    #define POPCOUNT(x)   __popcnt(x) // Population count (32-bit)
    #define POPCOUNT64(x) __popcnt64(x) // Population count (64-bit)
    #define BYTESWAP16(x) _byteswap_ushort(x) // Byte swap (16-bit)
    #define BYTESWAP32(x) _byteswap_ulong(x) // Byte swap (32-bit)
    #define BYTESWAP64(x) _byteswap_uint64(x) // Byte swap (64-bit)

    #define LIKELY(x)   (x) // No direct equivalent in MSVC
    #define UNLIKELY(x) (x) // No direct equivalent in MSVC

    #define ALIGNED(x)       __declspec(align(x))
    #define UNREACHABLE()    __assume(0)
    #define PREFETCH(x)      _mm_prefetch((const char*)(x), _MM_HINT_T0)
    #define MEMORY_BARRIER() _ReadWriteBarrier()

#else
// GCC, Clang, and other compilers

    #define TYPEOF      typeof
    #define FORCEINLINE __attribute__((always_inline)) inline
    #define NOINLINE    __attribute__((noinline))

    #define PACKED_STRUCT     struct __attribute__((packed))
    #define END_PACKED_STRUCT ;

    #define CTZ(x)        __builtin_ctz(x) // Count trailing zeros (32-bit)
    #define CTZ64(x)      __builtin_ctzll(x) // Count trailing zeros (64-bit)
    #define CLZ(x)        __builtin_clz(x) // Count leading zeros (32-bit)
    #define CLZ64(x)      __builtin_clzll(x) // Count leading zeros (64-bit)
    #define POPCOUNT(x)   __builtin_popcount(x) // Population count (32-bit)
    #define POPCOUNT64(x) __builtin_popcountll(x) // Population count (64-bit)
    #define BYTESWAP16(x) __builtin_bswap16(x) // Byte swap (16-bit) - fixed typo
    #define BYTESWAP32(x) __builtin_bswap32(x) // Byte swap (32-bit)
    #define BYTESWAP64(x) __builtin_bswap64(x) // Byte swap (64-bit)

    #define LIKELY(x)   __builtin_expect(!!(x), 1)
    #define UNLIKELY(x) __builtin_expect(!!(x), 0)

    #define ALIGNED(x)       __attribute__((aligned(x)))
    #define UNREACHABLE()    __builtin_unreachable()
    #define PREFETCH(x)      __builtin_prefetch(x)
    #define MEMORY_BARRIER() __asm__ volatile("" ::: "memory")
#endif
