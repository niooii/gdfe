#pragma once

#include <gdfe/gdfe.h>

typedef struct gdfe_state {
    i16           width;
    i16           height;
    f64           last_time;
    GDF_Stopwatch stopwatch;
    GDF_BOOL      initialized;

    GDF_AppCallbacks callbacks;
    GDF_Config       conf;
    GDF_AppState public;

    GDF_BOOL mouse_lock_toggle;
} gdfe_state;

extern gdfe_state APP_STATE;
