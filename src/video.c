#include <i_video.h>
#include <shaderc/shaderc.h>
#include <i_subsystems.h>

GDF_BOOL gdfe_video_init()
{
    RET_FALSE(gdfe_windowing_init());

    gdfe_input_init();

    return GDF_TRUE;
}

void gdfe_video_shutdown()
{
    gdfe_input_shutdown();
    gdfe_windowing_shutdown();
}

