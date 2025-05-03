#include <gdfe/logging.h>
#include <gdfe/os/io.h>
// TODO: temporary
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <gdfe/collections/hashmap.h>
#include <gdfe/os/thread.h>
#include <gdfe/os/misc.h>
#include <gdfe/prelude.h>

#define MAX_MSG_LEN 16384
#define ENTRIES_BUFFER_CAPACITY 1024
#define FLUSH_INTERVAL_MS 50 
#define MAX_THREAD_NAME_LEN 128
// timestamp (23) +T thread_name (128) + level_str (7) + message (16384) + newline (1) = 16543
#define MAX_FORMATTED_ENTRY_LEN (MAX_MSG_LEN + MAX_THREAD_NAME_LEN + 48)

typedef struct LogEntry {
    GDF_DateTime time;
    const char* message;
    const char* thread_name;
    LOG_LEVEL level;
} LogEntry;

// perhaps expand... 8mb allocation right here, absolutely diabolical
static char FORMAT_BUFFER[MAX_FORMATTED_ENTRY_LEN * ENTRIES_BUFFER_CAPACITY];
static LogEntry entries[ENTRIES_BUFFER_CAPACITY];

// holy globals
static GDF_BOOL INITIALIZED = GDF_FALSE;
// index based, start from 0
static u32 next_free_entry = 0;
static GDF_HashMap ti_map = NULL;
// Sync
static GDF_Mutex ti_mutex = NULL;
static GDF_Mutex entries_mutex = NULL;
static GDF_Mutex flushing_mutex = NULL;
static GDF_Thread flushing_thread = NULL;

const char* level_strings[7] =
{
    "FATAL: ",
    "ERROR: ",
    "WARN:  ",
    "INFO:  ",
    "TIME:  ",
    "DEBUG: ",
    "TRACE: ",
};

typedef struct ThreadLoggingInfo {
    const char* thread_name;
} ThreadLoggingInfo;

typedef struct ColorCombination {
    u8 fg;
    u8 bg;
} ColorCombination;

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

static const ColorCombination log_level_color_combinations[] = {
    [LOG_LEVEL_FATAL] = {
        .fg = TC_WHITE,
        .bg = BG_RED
    },
    [LOG_LEVEL_ERR] = {
        .fg = TC_RED,
        .bg = TC_RED
    },
    [LOG_LEVEL_DEBUG] = {
        .fg = TC_BLUE,
        .bg = TC_BLUE
    },
    [LOG_LEVEL_INFO] = {
        .fg = TC_GREEN,
        .bg = TC_GREEN
    },
    [LOG_LEVEL_TIME] = {
        .fg = TC_BRIGHT_MAGENTA,
        .bg = TC_BRIGHT_MAGENTA
    },
    [LOG_LEVEL_WARN] = {
        .fg = TC_YELLOW,
        .bg = TC_YELLOW
    },
    [LOG_LEVEL_TRACE] = {
        .fg = TC_WHITE,
        .bg = TC_WHITE
    },
};

void GDF_FlushLogBuffer()
{
    char* write_ptr = FORMAT_BUFFER;
    if (!GDF_LockMutex(entries_mutex))
    {
        GDF_WriteConsole("Failed to flush log buffer, mutex not acquired..\n");
        return;
    }
    if (!GDF_LockMutex(flushing_mutex)) {
        GDF_ReleaseMutex(entries_mutex);
        GDF_WriteConsole("Failed to flush log buffer, mutex not acquired..\n");
        return;
    }
    for (u32 i = 0; i < next_free_entry; i++)
    {
        const LogEntry* const entry = entries + i;
        ColorCombination cc = log_level_color_combinations[entry->level];
        snprintf(
            write_ptr, 
            MAX_FORMATTED_ENTRY_LEN, 
            "\x1B[%dm\x1B[%dm[%s @ %02d:%02d:%02d.%03d] %s %s\n",
            cc.fg,
            cc.bg,
            entry->thread_name,
            entry->time.hour,
            entry->time.minute,
            entry->time.second,
            entry->time.milli,
            level_strings[entry->level],
            entry->message
        );
        write_ptr = strchr(write_ptr, 0);
    }
    GDF_ReleaseMutex(entries_mutex);
    GDF_WriteConsole(FORMAT_BUFFER);
    GDF_ReleaseMutex(flushing_mutex);
    next_free_entry = 0;
    FORMAT_BUFFER[0] = 0;
}

unsigned long flushing_thread_fn(void* args)
{
    GDF_Stopwatch stopwatch = GDF_StopwatchCreate();
    while(1)
    {
        // TODO! create timer abstraction to run functions periodically
        if (GDF_StopwatchElapsed(stopwatch) * 1000 > FLUSH_INTERVAL_MS)
        {
            GDF_StopwatchReset(stopwatch);
            // TODO! optimized IO
            GDF_FlushLogBuffer();
        }
    }
}

// TODO! YIKES HOLY ALLOCATIONS
GDF_BOOL gdfe_logging_init()
{
    entries_mutex = GDF_CreateMutex();
    flushing_mutex = GDF_CreateMutex();
    ti_mutex = GDF_CreateMutex();
    ti_map = GDF_HashmapCreate(u32, ThreadLoggingInfo, GDF_FALSE);

    int i = 0;
    for (int i = 0; i < ENTRIES_BUFFER_CAPACITY; i++)
    {
        // max thread name 128 chars long.. no way right
        (entries+i)->thread_name = GDF_Malloc(128, GDF_MEMTAG_STRING);
        (entries+i)->message = GDF_Malloc(MAX_MSG_LEN, GDF_MEMTAG_STRING);
    }

    flushing_thread = GDF_CreateThread(flushing_thread_fn, NULL);

    INITIALIZED = GDF_TRUE;

    return GDF_TRUE;
}

void gdfe_logging_shutdown()
{

}

GDF_BOOL GDF_InitThreadLogging(const char* thread_name)
{
    ThreadLoggingInfo info = {
        .thread_name = thread_name
    };
    
    u32 thread_id = GDF_GetCurrentThreadId();
    
    GDF_LockMutex(ti_mutex);
    GDF_HashmapInsert(ti_map, &thread_id, &info, NULL);
    GDF_ReleaseMutex(ti_mutex);
    
    // printf("init thread id: %d, name: %s\n", thread_id, thread_name);
    return GDF_TRUE;
}

void GDF_ShutdownLogging() 
{
    INITIALIZED = GDF_FALSE;
    // TODO! cleanup logging/write queued entries.
    GDF_FlushLogBuffer();
    GDF_WriteConsole("\033[0m");
}

void GDF_LogOutput(LOG_LEVEL level, const char* message, ...)
{
    if (!INITIALIZED)
    {
        return;
    }

    if (!GDF_LockMutex(entries_mutex))
    {
        GDF_WriteConsole("Failed to buffer log call, mutex not acquired...\n");
        return;
    }

    // TODO! BAD BAD BAD or is it..
    if (next_free_entry >= ENTRIES_BUFFER_CAPACITY)
    {
        GDF_FlushLogBuffer();
    }

    u32 thread_id = GDF_GetCurrentThreadId();
    GDF_LockMutex(ti_mutex);
    ThreadLoggingInfo* info = (ThreadLoggingInfo*)GDF_HashmapGet(ti_map, &thread_id);
    if (info == NULL)
    {
        printf("Failed thread id: %d\n", thread_id);
        printf("Message: %s", message);
        GDF_ReleaseMutex(entries_mutex);
        GDF_ReleaseMutex(ti_mutex);
        return;
    }
 
    LogEntry* entry = &entries[next_free_entry++];
    entry->thread_name = info->thread_name;
    GDF_ReleaseMutex(ti_mutex);

    entry->level = level;
    __builtin_va_list arg_ptr;
    va_start(arg_ptr, message);
    vsnprintf(entry->message, MAX_MSG_LEN, message, arg_ptr);
    va_end(arg_ptr);

    GDF_GetSystemTime(&entry->time);

    // TODO! this could fail too
    GDF_ReleaseMutex(entries_mutex);

    // TODO! could be optimized more, for now force flush
    if (level == LOG_LEVEL_ERR || level == LOG_LEVEL_FATAL)
        GDF_FlushLogBuffer();
}

void gdfe_report_assert_fail(const char* expression, const char* message, const char* file, i32 line) 
{
    GDF_LogOutput(LOG_LEVEL_FATAL, "Assertion Failure: %s, message: '%s' in file: %s, line: %d\n", expression, message, file, line);
}

void report_todo(const char* message, const char* file, i32 line)
{
    GDF_LogOutput(LOG_LEVEL_FATAL, "Unimplemented: '%s' in file: %s, line: %d\n", message, file, line);
}