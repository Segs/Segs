#pragma once
#include <GL/glew.h>
#include "RendererState.h"

struct SysInfo_2
{
    int total_physical_memory;
    int AvailPhys;
    char video_card[256];
    int pci_ven;
    int pci_dev;
    float cpu_freq;
    char driver_version[256];
};
static_assert(sizeof(SysInfo_2)==0x214,"Struct size must be correct");
enum eGPU
{
    fNV_vertex_program         = 0x1,
    fGL_NV_register_combiners2 = 0x2,
    fTexture_env_combine       = 0x4,
    fATI_fragment_shader       = 0x8,
    f_GL_FragmentShader        = 0x100,
    f_GL_VertexShader          = 0x200,
};
struct GLDebugGuard
{
    GLDebugGuard(const char *fname)
    {
        if (GLEW_KHR_debug) {
            glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, fname);
        }
    }

    static void insertMessage(const char *text)
    {
        if (GLEW_KHR_debug) {
            glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_OTHER,
                0, GL_DEBUG_SEVERITY_MEDIUM, -1, text);
        }
    }
    ~GLDebugGuard()
    {
        if (GLEW_KHR_debug) {
            glPopDebugGroup();
        }
    }
};
extern void segs_renderUtil_GetGfxCardVend(SysInfo_2 *inf);
extern void segs_wcw_statemgmt_enableColorMaterial();
extern void patch_render_utils();
