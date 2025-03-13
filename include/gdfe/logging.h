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
    // for internal profiling use
    LOG_LEVEL_TIME,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_TRACE
} LOG_LEVEL;

// ANSI codes
typedef enum TEXT_COLOR {
    TC_BLACK = 30,
    TC_RED = 31,
    TC_GREEN = 32,
    TC_YELLOW = 33,
    TC_BLUE = 34,
    TC_MAGENTA = 35,
    TC_CYAN = 36,
    TC_WHITE = 37,
    TC_BRIGHT_BLACK = 90,
    TC_BRIGHT_RED = 91,
    TC_BRIGHT_GREEN = 92,
    TC_BRIGHT_YELLOW = 93,
    TC_BRIGHT_BLUE = 94,
    TC_BRIGHT_MAGENTA = 95,
    TC_BRIGHT_CYAN = 96,
    TC_BRIGHT_WHITE = 97
} TEXT_COLOR;

// ANSI codes
typedef enum TEXT_BG_COLOR {
    BG_BLACK = 40,
    BG_RED = 41,
    BG_GREEN = 42,
    BG_YELLOW = 43,
    BG_BLUE = 44,
    BG_MAGENTA = 45,
    BG_CYAN = 46,
    BG_WHITE = 47,
    BG_BRIGHT_BLACK = 100,
    BG_BRIGHT_RED = 101,
    BG_BRIGHT_GREEN = 102,
    BG_BRIGHT_YELLOW = 103,
    BG_BRIGHT_BLUE = 104,
    BG_BRIGHT_MAGENTA = 105,
    BG_BRIGHT_CYAN = 106,
    BG_BRIGHT_WHITE = 107
} TEXT_BG_COLOR;

GDF_BOOL GDF_InitLogging();
GDF_BOOL GDF_InitThreadLogging(const char* thread_name);
void GDF_ShutdownLogging();

void logging_flush_buffer();
void log_output(LOG_LEVEL level, const char* message, ...);

// should be available wherever lol unlucky
#define LOG_FATAL(message, ...) log_output(LOG_LEVEL_FATAL, message, ##__VA_ARGS__);

#ifndef GDF_ERR
    #define LOG_ERR(message, ...) log_output(LOG_LEVEL_ERR, message, ##__VA_ARGS__);
#endif

#ifdef GDF_WARN
    #define LOG_WARN(message, ...) log_output(LOG_LEVEL_WARN, message, ##__VA_ARGS__);
#else
    #define LOG_WARN(message, ...)
#endif

#ifdef GDF_INFO
    #define LOG_INFO(message, ...) log_output(LOG_LEVEL_INFO, message, ##__VA_ARGS__);
#else
    #define LOG_INFO(message, ...)
#endif

#if defined(GDF_DEBUG)
    #define LOG_DEBUG(message, ...) log_output(LOG_LEVEL_DEBUG, message, ##__VA_ARGS__);
#else
    #define LOG_DEBUG(message, ...)
#endif

#ifdef GDF_TRACE
    #define LOG_TRACE(message, ...) log_output(LOG_LEVEL_TRACE, message, ##__VA_ARGS__);
#else
    #define LOG_TRACE(message, ...)
#endif