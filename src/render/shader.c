#include <gdfe/render/shader.h>
#include <i_render/types.h>
#include <i_subsystems.h>
#include <shaderc/shaderc.h>

/// Contains the global state related to rendering.
typedef struct gdfe_shader_state {
    shaderc_compiler_t shaderc_compiler;
    shaderc_compile_options_t shaderc_options;
} gdfe_shader_state;

static gdfe_shader_state STATE;

void gdfe_shaders_init()
{
    // initialize shader compiler for hot reloading shaders and first time compilations
    // and stuff
    STATE.shaderc_compiler = shaderc_compiler_initialize();
    STATE.shaderc_options = shaderc_compile_options_initialize();

    shaderc_compile_options_set_optimization_level(STATE.shaderc_options, shaderc_optimization_level_performance);
}

void gdfe_shaders_shutdown()
{
    shaderc_compile_options_release(STATE.shaderc_options);
    shaderc_compiler_release(STATE.shaderc_compiler);
}

GDF_Shader GDF_ShaderLoadGLSL(const char* path);
GDF_Shader GDF_ShaderLoadSPIRV(const char* path);

GDF_Shader GDF_ShaderLoadStrGLSL(const char* buf, u64 len);
GDF_Shader GDF_ShaderLoadStrSPIRV(const char* buf, u64 len);

GDF_BOOL GDF_ShaderReload(GDF_Shader shader);

void GDF_ShaderDestroy(GDF_Shader shader);