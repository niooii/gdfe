#pragma once

#include <gdfe/time/stopwatch.h>
#include <gdfe/logging.h>

#ifdef GDFP_DISABLE

#define GDFP_START()

#define GDFP_LOG()

#define GDFP_LOG_MSG(msg)

#define GDFP_LOG_RESET()

#define GDFP_LOG_MSG_RESET(msg) 

#define GDFP_END()

#else

#define GDFP_START() { \
    GDF_Stopwatch _gdfe_profiler_sw = GDF_StopwatchCreate();

#define GDFP_LOG() \
    log_output(LOG_LEVEL_TIME, "[%lfs]", GDF_StopwatchElapsed(_gdfe_profiler_sw)); \

#define GDFP_LOG_MSG(msg) \
    log_output(LOG_LEVEL_TIME, "[%lfs] \"%s\"", GDF_StopwatchElapsed(_gdfe_profiler_sw), msg);

#define GDFP_LOG_RESET() \
    log_output(LOG_LEVEL_TIME, "[%lfs]", GDF_StopwatchElapsed(_gdfe_profiler_sw)); \
    GDF_StopwatchReset(_gdfe_profiler_sw);

#define GDFP_LOG_MSG_RESET(msg) \
    log_output(LOG_LEVEL_TIME, "[%lfs] \"%s\"", GDF_StopwatchElapsed(_gdfe_profiler_sw), msg); \
    GDF_StopwatchReset(_gdfe_profiler_sw);

#define GDFP_END() \
    GDF_StopwatchDestroy(_gdfe_profiler_sw); \
    }

#endif
