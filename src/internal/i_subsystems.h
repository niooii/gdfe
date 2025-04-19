#pragma once

#include <gdfe/core.h>
#include <gdfe/os/video.h>

/* Private header for subsystem initialization, deinitialization and updating */

void gdfe_input_init();
void gdfe_input_shutdown();
void gdfe_input_update(GDF_Window active, f64 delta_time);

GDF_BOOL gdfe_logging_init();
void gdfe_logging_shutdown();

GDF_BOOL gdfe_events_init();
void gdfe_events_shutdown();

/* OS-specific subsystems */

GDF_BOOL gdfe_windowing_init();
void gdfe_windowing_shutdown();

GDF_BOOL gdfe_mem_init();
void gdfe_mem_shutdown();

void gdfe_io_init();
void gdfe_io_shutdown();

void gdfe_misc_init();
void gdfe_misc_shutdown();

GDF_BOOL gdfe_sock_init();
void gdfe_sock_shutdown();