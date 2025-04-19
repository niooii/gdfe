#pragma once

#include <gdfe/core.h>

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

#ifdef __cplusplus
}
#endif