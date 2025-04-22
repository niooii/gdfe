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

#ifdef __cplusplus
extern "C" {
#endif

GDF_BOOL GDF_InitThreadLogging(const char* thread_name);
void GDF_ShutdownLogging();

void GDF_FlushLogBuffer();
void GDF_LogOutput(LOG_LEVEL level, const char* message, ...);

#ifdef __cplusplus
}
#endif

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
    #define LOG_CALL LOG_TRACE("Line %d::%s", __LINE__, __FUNCTION__)
#else
    #define LOG_TRACE(message, ...)
    #define LOG_CALL
#endif