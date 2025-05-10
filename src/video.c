#include <gdfe/video.h>
#include <i_video.h>
#include <shaderc/shaderc.h>
#include <i_subsystems.h>

/// Contains the global state related to rendering.
typedef struct gdfe_render_state {
    shaderc_compiler_t shaderc_compiler;
    shaderc_compile_options_t shaderc_options;
} gdfe_render_state;

static gdfe_render_state STATE;

GDF_BOOL gdfe_video_init()
{
    // initialize shader compiler for hot reloading shaders and first time compilations
    // and stuff
    STATE.shaderc_compiler = shaderc_compiler_initialize();
    STATE.shaderc_options = shaderc_compile_options_initialize();

    shaderc_compile_options_set_optimization_level(STATE.shaderc_options, shaderc_optimization_level_performance);

    RET_FALSE(gdfe_windowing_init());

    gdfe_input_init();

    return GDF_TRUE;
}

void gdfe_video_shutdown()
{
    shaderc_compile_options_release(STATE.shaderc_options);
    shaderc_compiler_release(STATE.shaderc_compiler);
}

GDF_BOOL GDF_ShaderReload(GDF_Shader shader)
{
    #ifndef GDF_RELEASE



    #endif
}
