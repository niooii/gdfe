#pragma once

#include <gdfe/core.h>

typedef struct GDF_Stopwatch_T* GDF_Stopwatch;

// Stopwatch is created in "resumed" state.
GDF_Stopwatch GDF_StopwatchCreate();
// in seconds
f64 GDF_StopwatchElapsed(GDF_Stopwatch stopwatch);
f64 GDF_StopwatchReset(GDF_Stopwatch stopwatch);
void GDF_StopwatchResume(GDF_Stopwatch stopwatch);
void GDF_StopwatchDestroy(GDF_Stopwatch stopwatch);