#include <gdfe/subsystems.h>

static u32 _flags;

GDF_BOOL GDF_InitSubsystems(u32 flags)
{
    _flags = flags;
    GDF_InitIO();
    if (!GDF_InitSysinfo())
        return GDF_FALSE;
    if ((flags & GDF_SUBSYSTEM_WINDOWING) == GDF_SUBSYSTEM_WINDOWING)
    {
        if (!GDF_InitWindowing())
            return GDF_FALSE;
    }
    if ((flags & GDF_SUBSYSTEM_EVENTS) == GDF_SUBSYSTEM_EVENTS)
    {
        if (!GDF_InitEvents())
            return GDF_FALSE;
    }
    if ((flags & GDF_SUBSYSTEM_INPUT) == GDF_SUBSYSTEM_INPUT)
    {
        // if the events subsystem wasn't initialized return GDF_FALSE w an error
        if ((flags & GDF_SUBSYSTEM_EVENTS) != GDF_SUBSYSTEM_EVENTS)
        {
            LOG_ERR("Input subsystem depends on events subsystem.");
            return GDF_FALSE;
        }
        GDF_InitInput();
    }
    if ((flags & GDF_SUBSYSTEM_NET) == GDF_SUBSYSTEM_NET)
    {
        if (!GDF_InitSockets())
            return GDF_FALSE;
    }
    return GDF_TRUE;
}

GDF_BOOL GDF_ShutdownSubsystems()
{
    if (_flags & GDF_SUBSYSTEM_EVENTS)
    {
        GDF_ShutdownEvents();
    }
    if (_flags & GDF_SUBSYSTEM_WINDOWING)
    {
        GDF_ShutdownWindowing();
    }
    if (_flags & GDF_SUBSYSTEM_INPUT)
    {
        GDF_ShutdownInput();
    }
    GDF_ShutdownLogging();

    return GDF_TRUE;
}