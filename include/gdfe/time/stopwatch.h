#pragma once

#include <gdfe/prelude.h>

typedef struct GDF_Stopwatch_T* GDF_Stopwatch;

#ifdef __cplusplus
extern "C" {
#endif

/// @return A stopwatch handle.
/// @note The stopwatch starts tracking elapsed time upon creation.
GDF_Stopwatch GDF_StopwatchCreate();

/// @param stopwatch A stopwatch handle.
/// @return The number of seconds elapsed.
f64 GDF_StopwatchElapsed(GDF_Stopwatch stopwatch);

/// @param stopwatch A stopwatch handle.
/// @return The number of seconds elapsed before the stopwatch was reset.
f64 GDF_StopwatchReset(GDF_Stopwatch stopwatch);

/// @param stopwatch A stopwatch handle.
void GDF_StopwatchDestroy(GDF_Stopwatch stopwatch);

/// Halts a thread until the stopwatch reaches a certain value. Good for running tasks
/// at a fixed rate.
/// @param stopwatch A stopwatch handle.
/// @param secs The target amount of seconds the stopwatch should reach, before resuming the thread.
/// @return 0 if the wait was successful. If \code secs\endcode is less than the stopwatch's
/// current value, then \code elapsed-secs\endcode is returned, that is, how far ahead the stopwatch
/// is from \code secs\endcode.
/// @remark This is guarenteed to wait until at least the specified time. This does not reset the
/// stopwatch, it must be manually reset when desired.
f64 GDF_StopwatchSleepUntil(GDF_Stopwatch stopwatch, f64 secs);

#ifdef __cplusplus
}
#endif