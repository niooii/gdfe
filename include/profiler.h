#pragma once

#include <time/stopwatch.h>
#include <logging.h>

#ifdef GDFP_DISABLE

#define GDFP_START()

#define GDFP_LOG()

#define GDFP_LOG_MSG(msg)

#define GDFP_LOG_RESET()

#define GDFP_LOG_MSG_RESET(msg) 

#define GDFP_END()

#else

// every call to this must be followed by a call to GDFP_END()
#define GDFP_START() { \
    GDF_Stopwatch ___profiler_sw = GDF_StopwatchCreate();

#define GDFP_LOG() \
    log_output(LOG_LEVEL_TIME, "[%lfs]", GDF_StopwatchElapsed(___profiler_sw)); \

#define GDFP_LOG_MSG(msg) \
    log_output(LOG_LEVEL_TIME, "[%lfs] \"%s\"", GDF_StopwatchElapsed(___profiler_sw), msg);

#define GDFP_LOG_RESET() \
    log_output(LOG_LEVEL_TIME, "[%lfs]", GDF_StopwatchElapsed(___profiler_sw)); \
    GDF_StopwatchReset(___profiler_sw);

#define GDFP_LOG_MSG_RESET(msg) \
    log_output(LOG_LEVEL_TIME, "[%lfs] \"%s\"", GDF_StopwatchElapsed(___profiler_sw), msg); \
    GDF_StopwatchReset(___profiler_sw);

#define GDFP_END() \
    GDF_StopwatchDestroy(___profiler_sw); \
    }

#endif
