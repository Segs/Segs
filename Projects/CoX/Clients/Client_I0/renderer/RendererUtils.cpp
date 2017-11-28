#include "RendererUtils.h"

#include "GameState.h"
#include "utils/dll_patcher.h"

#include "GL/glew.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <cstring>

extern "C"
{
__declspec(dllimport) int64_t GetCpuMhz();
__declspec(dllimport) int c_assert(const char *,const char *,const char *file,int);
__declspec(dllimport) int renderUtil_GetVersion(char *,const char *driverfile);
__declspec(dllimport) void renderUtil_fn_4DFEF0(char *videocardname,int *pci_ven,int *pci_dev);
__declspec(dllimport) void sysutil_5AFA70(int *total_memory,int *avail_memory);
__declspec(dllimport) void ErrorfFL(const char *filename, int line);
__declspec(dllimport) void ErrorfInternal(const char *fmt, ...);
__declspec(dllimport) void renderUtil_SystemReport(SysInfo_2 *,char *);
__declspec(dllimport) int VfPrintfWrapper(const char *fmt,...);
__declspec(dllimport) int fn_581560(const char *); // << yes/no dialog
__declspec(dllimport) void dispatch_cmd(const char *cmd);
__declspec(dllimport) void setGlossMultiplier(float);
__declspec(dllimport) void wcwMgmt_EnableFog(int);
__declspec(dllimport) void renderUtil_4DFD50();
__declspec(dllimport) void genShaderBindingsATI();
__declspec(dllimport) void buildShader2Ati(); //ColorBlendDual
__declspec(dllimport) void buildShader3Ati(); // add glow
__declspec(dllimport) void buildShader4Ati(); // alpha detail
__declspec(dllimport) void buildShader1Ati(); // multiply
__declspec(dllimport) void buildShader5Ati(); // bump multiply
__declspec(dllimport) void buildShader6Ati(); // bump color
__declspec(dllimport) void fn_4DC4B0(); //model blend state init
__declspec(dllimport) void bump_4CEB20(); //bump init
__declspec(dllimport) void renderUtil_4E0BC0();
__declspec(dllimport) void fn_57B710();
__declspec(dllimport) int enableParticleVBOs;
__declspec(dllimport) int GPU_FLAGS;
__declspec(dllimport) int UsingVBOs;
__declspec(dllimport) int g_using_bump_maps;
__declspec(dllimport) int enableVertShaders;
__declspec(dllimport) SysInfo_2 struct_9A09A0;
}
enum GpuVendor
{
    ATI    = 0x1002,
    NVIDIA = 0x10DE,
    INTEL  = 0x8086,
};

static int renderUtil_4E0C40(SysInfo_2 *sysinfo)
{
    char buf[1000];
    segs_renderUtil_GetGfxCardVend(sysinfo);
    renderUtil_SystemReport(sysinfo, buf);
    return VfPrintfWrapper("%s", buf);
}

static bool checkExt(const char *extname,int &missing)
{
    if ( !glewIsSupported(extname) ) // glewIsSupported
    {
        ErrorfFL(".\\render\\renderUtil.c", 190);
        ErrorfInternal("You card or driver doesn't support %s",extname);
        missing = 1;
        return false;
    }
    return true;
}
static void reportOldDrivers(const char *brand)
{
    sprintf(g_State.driver_notice, "Important: Your computer appears to be using older drivers for your video card.");
    sprintf(g_State.driver_notice_2, "City of Heroes may run poorly without updated drivers. Go to %s to get the latest drivers.", brand);
    sprintf(g_State.driver_notice_3, "In some cases, such as laptops, you may need to get the latest drivers from your computer manufacturer's site.");
}


void segs_renderUtil_GetGfxCardVend(SysInfo_2 *sysinfo)
{
    // fill in current gpu's information
    renderUtil_fn_4DFEF0(sysinfo->video_card, &sysinfo->pci_ven, &sysinfo->pci_dev);
    // fill in memory information
    sysutil_5AFA70(&sysinfo->total_physical_memory, &sysinfo->AvailPhys);
    // fill in cpu
    sysinfo->cpu_freq = GetCpuMhz();
    // TODO: extract card from gl info
    switch(sysinfo->pci_ven)
    {
    case ATI:
    {
        renderUtil_GetVersion(sysinfo->driver_version, "atioglxx.dll");
        break;
    }
    case INTEL:
    {
        //TODO: implement me
        strcpy(sysinfo->driver_version, "Intel v.999");
        break;
    }
    case NVIDIA:
    {
        if ( !renderUtil_GetVersion(sysinfo->driver_version, "nv4_disp.dll") )
            renderUtil_GetVersion(sysinfo->driver_version, "nvdisp.drv");
        break;
    }
    default:
        strcpy(sysinfo->driver_version, "Unknown Vendor");
    }
}
static void segs_wcw_statemgmt_enableColorMaterial()
{
    glEnable(GL_COLOR_MATERIAL);
}
static void rendererInit()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-4.0, 4.0, -2.704225063323975, 2.704225063323975, 4.0, 4096.0);
    glMatrixMode(GL_MODELVIEW);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.0);
    wcwMgmt_EnableFog(1);
    glFogf(GL_FOG_DENSITY, 0.01f);
    glFogi(GL_FOG_MODE, GL_EXP);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glActiveTextureARB(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glColor4f(1.0, 1.0, 1.0, 1.0);
    glEnable(GL_LIGHT0);
    segs_wcw_statemgmt_enableColorMaterial();
}
void segs_renderUtil_4E0CA0()
{
    glewInit();
    int extension_missing=0;
    segs_renderUtil_GetGfxCardVend(&struct_9A09A0);
    renderUtil_4E0C40(&struct_9A09A0);
    if ( g_State.create_bins )
    {
        UsingVBOs = 0;
        enableVertShaders = 0;
        g_State.view.noVertShaders = 1;
        g_State.view.noVBOs = 1;
        return;
    }
    GPU_FLAGS = 0;

    checkExt("GL_EXT_compiled_vertex_array",extension_missing);
    checkExt("GL_ARB_multitexture",extension_missing);
    checkExt("GL_ARB_texture_compression",extension_missing);
    if( glewIsSupported("GL_ARB_fragment_program") && glewIsSupported("GL_ARB_vertex_program") )
    {
        // custom flags, using generic GL shader support
        GPU_FLAGS = f_GL_FragmentShader | f_GL_VertexShader;
    }
    else if ( glewIsSupported("GL_NV_register_combiners") )
    {
        int dummy_missing=0;
        if ( checkExt("GL_NV_vertex_program",dummy_missing) )
        {
            GPU_FLAGS = fNV_vertex_program;
            if ( glewIsSupported("GL_NV_register_combiners2") )
                GPU_FLAGS |= fGL_NV_register_combiners2;
        }
    }
    else
    {
        if ( glewIsSupported("GL_EXT_texture_env_combine") )
            GPU_FLAGS = fTexture_env_combine;
        if ( glewIsSupported("GL_EXT_vertex_shader") && glewIsSupported("GL_ATI_fragment_shader") )
            GPU_FLAGS |= fATI_fragment_shader;
    }

    if ( GPU_FLAGS == 0 || extension_missing )
    {
        char Text[1024];
        Text[0] = 0;
        sprintf(Text, "Detected video card or driver is currently unsupported \n %s \n", struct_9A09A0.video_card);
        strcat(Text, "The game may run poorly or not at all with you current configuration.\n");
        strcat(Text, "Current system requirements are NVidia GeForce 2 or better or ATI Radeon 8500 or better.\n");
        strcat(Text, "(If your card does meet the minimum requirements, you might not have the latest drivers.\n");
        strcat(Text, "Go to www.nvidia.com or www.ati.com to update your driver. In some cases, such as laptops,\n");
        strcat(Text, "you may need to get the latest drivers from you computer manufacturer.)\n");
        strcat(Text, "Do you want to continue?");
        if ( !fn_581560(Text) )
            dispatch_cmd("quit");
        fn_57B710();
    }
    int driver_version = 0;
    const char *version_str = strrchr(struct_9A09A0.driver_version, '.');
    if ( version_str )
        driver_version = atoi(version_str + 1);
    if ( GPU_FLAGS & fATI_fragment_shader && ( driver_version < 0x10B1 ) )
    {
        g_State.unkn_1FD8 = 1;
        reportOldDrivers("www.ati.com");
    }
    else if ( (GPU_FLAGS & fNV_vertex_program) && driver_version <= 0x1460 )
    {
        g_State.unkn_1FD8 = 1;
        reportOldDrivers("www.nvidia.com");
    }
    else if ( g_State.unkn_1FD8 == 1 )
    {
        reportOldDrivers("");
    }

    g_using_bump_maps = 1;
    enableVertShaders = 1;
    UsingVBOs = 1;
    if ( GPU_FLAGS == 0 )
    {
        UsingVBOs = 0;
        g_using_bump_maps = 0;
        enableVertShaders = 0;
    }
    if ( !(GPU_FLAGS & (fATI_fragment_shader|fGL_NV_register_combiners2)) )
    {
        UsingVBOs = 0;
        g_using_bump_maps = 0;
        enableVertShaders = 0;
    }
    enableParticleVBOs = 0;
    if ( !g_State.view.enableVBOs )
    {
        UsingVBOs = 0;
    }
    else if ( !(GPU_FLAGS & fNV_vertex_program) )
    {
        g_State.view.enableVBOs = 0;
        VfPrintfWrapper("!!!!!!!!!!!!!!!!! %s ", struct_9A09A0.driver_version);
        VfPrintfWrapper(" %d ", driver_version);
        if ( driver_version >= 0x10B1 )
        {
            g_State.view.enableVBOs = 1;
        }
    }

    if ( g_State.view.unkn_4B4 )
    {
        UsingVBOs = 0;
    }
    if ( g_State.view.enableVBOs_particles )
        enableParticleVBOs = 1;
    if ( g_State.view.disableVBOs_particles )
        enableParticleVBOs = 0;
    if ( g_State.view.noVBOs )
    {
        UsingVBOs = 0;
    }
    if ( g_State.view.noPixShaders )
        g_using_bump_maps = 0;
    if ( g_State.view.noVertShaders )
    {
        enableVertShaders = 0;
        UsingVBOs = 0;
        g_using_bump_maps = 0;
    }
    if ( !(GPU_FLAGS & (fATI_fragment_shader|fNV_vertex_program)) )
    {
        UsingVBOs = 0;
        enableParticleVBOs = 0;
    }
    else if ( !UsingVBOs )
    {
        enableParticleVBOs = 0;
    }
    VfPrintfWrapper("RENDER SETTINGS: VBOS %d, PARTICLE VBOS %d BUMPMAPS %d, VERTSHADERS %d \n", UsingVBOs,
                    enableParticleVBOs, g_using_bump_maps, enableVertShaders);

    setGlossMultiplier(1.0f);
    rendererInit(); // set default GL state.
    if ( GPU_FLAGS & fATI_fragment_shader )
    {
        genShaderBindingsATI();
        buildShader2Ati();
        buildShader3Ati();
        buildShader4Ati();
        buildShader1Ati();
        buildShader5Ati();
        buildShader6Ati();
    }
    fn_4DC4B0();         // prepate nvidia gl lists with register combiners
    //TODO: we need to convert the code here to use generic pixel shaders, would have to patch all functions using blending modes
    bump_4CEB20();       // create bump map vertex shaders
    renderUtil_4E0BC0(); // create skinning vertex shaders
}
void patch_render_utils()
{
    PATCH_FUNC(renderUtil_GetGfxCardVend);
    patchit("renderUtil_4DFD50",(void *)rendererInit);
    PATCH_FUNC(renderUtil_4E0CA0);
    PATCH_FUNC(wcw_statemgmt_enableColorMaterial);
}
