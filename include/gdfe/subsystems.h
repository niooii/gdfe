#pragma once

#include <gdfe/core.h>
#include <gdfe/os/sysinfo.h>
#include <gdfe/os/io.h>
#include <gdfe/os/window.h>
#include <gdfe/os/socket.h>
#include <gdfe/event.h>
#include <gdfe/input.h>

enum {
    GDF_SUBSYSTEM_WINDOWING = 0b00000001,
    GDF_SUBSYSTEM_EVENTS =    0b00000010,
    GDF_SUBSYSTEM_INPUT =     0b00000100,
    GDF_SUBSYSTEM_NET =       0b00001000,
};

// womp womp
GDF_BOOL GDF_InitSubsystems(u32 flags);

GDF_BOOL GDF_ShutdownSubsystems();