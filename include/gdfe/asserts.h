#pragma once
#include <gdfe/def.h>

// Disable assertions by commenting out the below line.
#define GDF_ASSERTS_ENABLED

#ifdef GDF_ASSERTS_ENABLED
#if _MSC_VER
#include <intrin.h>
#define debugBreak() __debugbreak()
#else
#define debugBreak() __builtin_trap()
#endif

EXTERN_C_BEGIN

void gdfe_report_assert_fail(const char* expression, const char* message, const char* file, i32 line);
void report_todo(const char* message, const char* file, i32 line);

EXTERN_C_END

#define TODO(message)                                                \
    {   \
        report_todo(message, __FILE__, __LINE__);    \
        debugBreak();            \
        UNREACHABLE(); \
    }

#define GDF_ASSERT(expr)                                                \
    {   \
        if (!(expr)) {                                                    \
            LOG_INFO("ASSERT FAILED");                                             \
            gdfe_report_assert_fail(#expr, "", __FILE__, __LINE__);    \
            debugBreak();            \
        }                                                               \
    }

#define GDF_ASSERT_RETURN_FALSE(expr)                                                \
    {   \
        if (!(expr)) {                                                    \
            LOG_INFO("NON-FATAL ASSERT FAILED. RETURN");                                             \
            gdfe_report_assert_fail(#expr, "", __FILE__, __LINE__);    \
            return GDF_FALSE;          \
        }                                                               \
    }

#define GDF_ASSERT_MSG(expr, message)                                        \
    {                                                                        \
        if (!(expr)) {                                                         \
            gdfe_report_assert_fail(#expr, message, __FILE__, __LINE__);    \
            debugBreak();                                                    \
        }                                                                    \
    }

#ifndef GDF_RELEASE
#define GDF_ASSERT_DEBUG(expr)                                          \
    {                                                                   \
        if (!expr) {                                                    \
            gdfe_report_assert_fail(#expr, "", __FILE__, __LINE__);    \
            debugBreak();                                               \
        }                                                               \
    }
#else
#define GDF_ASSERT_DEBUG(expr)  // Does nothing at all
#endif

#else
#define GDF_ASSERT(expr)
#define GDF_ASSERT_MSG(expr, message)  
#define GDF_ASSERT(expr)         
#endif