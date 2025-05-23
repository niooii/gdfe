#pragma once

#include <gdfe/def.h>

#define STYLE_COMBO(first, second) first second

extern const char* level_strings[7];

// LOGGING
#define GDF_WARN
#define GDF_INFO
#define GDF_TRACE
#define GDF_DEBUG

#ifdef GDF_RELEASE
    #undef GDF_DEBUG
    #undef GDF_TRACE
#endif

typedef enum LOG_LEVEL {
    LOG_LEVEL_FATAL,
    LOG_LEVEL_ERR,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    // for profiling use
    LOG_LEVEL_TIME,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_TRACE
} LOG_LEVEL;

EXTERN_C_BEGIN

GDF_BOOL GDF_InitThreadLogging(const char* thread_name);
void     GDF_ShutdownLogging();

void GDF_FlushLogBuffer();
void GDF_LogOutput(LOG_LEVEL level, const char* message, ...);

EXTERN_C_END

/// @param message The format message, followed by variadic arguments.
/// @note This will flush the log buffer immediately.
#define LOG_FATAL(message, ...) GDF_LogOutput(LOG_LEVEL_FATAL, message, ##__VA_ARGS__);

/// @param message The format message, followed by variadic arguments.
/// @note This will flush the log buffer immediately.
#define LOG_ERR(message, ...) GDF_LogOutput(LOG_LEVEL_ERR, message, ##__VA_ARGS__);

#ifdef GDF_WARN
    #define LOG_WARN(message, ...) GDF_LogOutput(LOG_LEVEL_WARN, message, ##__VA_ARGS__);
#else
    #define LOG_WARN(message, ...)
#endif

#ifdef GDF_INFO
/// @param message The format message, followed by variadic arguments.
    #define LOG_INFO(message, ...) GDF_LogOutput(LOG_LEVEL_INFO, message, ##__VA_ARGS__);
#else
    #define LOG_INFO(message, ...)
#endif

#if defined(GDF_DEBUG)
/// @param message The format message, followed by variadic arguments.
    #define LOG_DEBUG(message, ...) GDF_LogOutput(LOG_LEVEL_DEBUG, message, ##__VA_ARGS__);
#else
    #define LOG_DEBUG(message, ...)
#endif

#ifdef GDF_TRACE
    #define LOG_TRACE(message, ...) GDF_LogOutput(LOG_LEVEL_TRACE, message, ##__VA_ARGS__);
    #define LOG_CALL                LOG_TRACE("Line %d::%s", __LINE__, __FUNCTION__)
#else
    #define LOG_TRACE(message, ...)
    #define LOG_CALL
#endif

/* Assertions */

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

void gdfe_report_assert_fail(const char* expression, const char* message, const char* file,
                             i32 line);
void gdfe_report_todo(const char* message, const char* file, i32 line);

EXTERN_C_END

#define TODO(message)                             \
    {                                             \
        gdfe_report_todo(message, __FILE__, __LINE__); \
        debugBreak();                             \
        UNREACHABLE();                            \
    }

#define GDF_ASSERT(expr)                                            \
    {                                                               \
        if (!(expr))                                                \
        {                                                           \
            LOG_INFO("ASSERT FAILED");                              \
            gdfe_report_assert_fail(#expr, "", __FILE__, __LINE__); \
            debugBreak();                                           \
        }                                                           \
    }

#define GDF_ASSERT_RETURN_FALSE(expr)                               \
    {                                                               \
        if (!(expr))                                                \
        {                                                           \
            LOG_INFO("NON-FATAL ASSERT FAILED. RETURN");            \
            gdfe_report_assert_fail(#expr, "", __FILE__, __LINE__); \
            return GDF_FALSE;                                       \
        }                                                           \
    }

#define GDF_ASSERT_MSG(expr, message)                                    \
    {                                                                    \
        if (!(expr))                                                     \
        {                                                                \
            gdfe_report_assert_fail(#expr, message, __FILE__, __LINE__); \
            debugBreak();                                                \
        }                                                                \
    }

#ifndef GDF_RELEASE
#define GDF_ASSERT_DEBUG(expr)                                      \
    {                                                               \
        if (!expr)                                                  \
        {                                                           \
            gdfe_report_assert_fail(#expr, "", __FILE__, __LINE__); \
            debugBreak();                                           \
        }                                                           \
    }
#else
#define GDF_ASSERT_DEBUG(expr) // Does nothing at all
#endif

#else
#define GDF_ASSERT(expr)
#define GDF_ASSERT_MSG(expr, message)
#define GDF_ASSERT(expr)
#endif
