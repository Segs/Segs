#include "RendererUtils.h"

#include "Model.h"
#include "RendererState.h"
#include "RenderBonedModel.h"
#include "RenderSprites.h"
#include "RenderParticles.h"
#include "Texture.h"
#include "RenderModel.h"
#include "GameState.h"
#include "ShaderProgramCache.h"
#include "utils/dll_patcher.h"

#include "GL/glew.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <vector>

extern "C"
{
__declspec(dllimport) int64_t GetCpuMhz();
__declspec(dllimport) int renderUtil_GetVersion(char *,const char *driverfile);
__declspec(dllimport) int fn_5B47F0(const char *, int, int);
__declspec(dllimport) void renderUtil_fn_4DFEF0(char *videocardname,int *pci_ven,int *pci_dev);
__declspec(dllimport) void sysutil_5AFA70(int *total_memory,int *avail_memory);
__declspec(dllimport) void ErrorfFL(const char *filename, int line);
__declspec(dllimport) void ErrorfInternal(const char *fmt, ...);
__declspec(dllimport) void renderUtil_SystemReport(SysInfo_2 *,char *);
__declspec(dllimport) int  VfPrintfWrapper(const char *fmt,...);
__declspec(dllimport) int fn_581560(const char *); // << yes/no dialog
__declspec(dllimport) void dispatch_cmd(const char *cmd);
__declspec(dllimport) void fn_57B710();
__declspec(dllimport) void fn_4B08C0();
__declspec(dllimport) bool getTTDebug();
__declspec(dllimport) void TTF_TClearAll();
__declspec(dllimport) void get_screen_size(int *,int *);

__declspec(dllimport) int GPU_FLAGS;
__declspec(dllimport) int g_using_bump_maps;
__declspec(dllimport) SysInfo_2 struct_9A09A0;
__declspec(dllimport) struct TextureBind *g_whiteTexture;
__declspec(dllimport) ParticleGlobal g_particle_engine;
__declspec(dllimport) GfxWindow g_frustumdata;
}
int enableParticleVBOs;
int enableVertShaders;
int UsingVBOs;
MaterialDefinition g_default_mat(DrawMode::COLORONLY, eBlendMode::MULTIPLY);
enum GpuVendor
{
    ATI    = 0x1002,
    NVIDIA = 0x10DE,
    INTEL  = 0x8086,
};

static int renderUtil_4E0C40(SysInfo_2 *sysinfo)
{
    char buf[1000]={0};
    segs_renderUtil_GetGfxCardVend(sysinfo);
    renderUtil_SystemReport(sysinfo, buf);
    return VfPrintfWrapper("%s", buf);
}

void segs_setGlossMultiplier(float scale)
{
    g_sun.gloss_scale = scale;
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
    if(sysinfo->total_physical_memory<0)
        sysinfo->total_physical_memory = std::numeric_limits<int>::max();
    if(sysinfo->AvailPhys<0)
        sysinfo->AvailPhys = std::numeric_limits<int>::max();
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
        strncpy(sysinfo->driver_version, "Intel v.999",255);
        break;
    }
    case NVIDIA:
    {
        if ( !renderUtil_GetVersion(sysinfo->driver_version, "nv4_disp.dll") )
            renderUtil_GetVersion(sysinfo->driver_version, "nvdisp.drv");
        break;
    }
    default:
        strncpy(sysinfo->driver_version,"Unknown Vendor",255);
    }
}
void segs_rendererInit()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);

    RenderState basic_rs=g_render_state.getGlobal();
    basic_rs.setCullMode(RenderState::CULL_CCW);
    basic_rs.setBlendMode(RenderState::BLEND_ALPHA);
    basic_rs.setDepthTestMode(RenderState::CMP_LESSEQUAL);
    g_render_state.apply(basic_rs);
    g_default_mat.setDrawMode(DrawMode::COLORONLY);
    g_default_mat.setFragmentMode(eBlendMode::MULTIPLY);
    g_default_mat.draw_data.projectionMatrix = glm::frustum(-4.0, 4.0, -2.704225063323975, 2.704225063323975, 4.0, 4096.0);
    g_default_mat.set_shadeModel(MaterialDefinition::SMOOTH);
    g_default_mat.draw_data.globalColor = { 1,1,1,1 };
    g_default_mat.draw_data.setDiscardAlpha(0.0);
    g_default_mat.set_useAlphaDiscard(1);

    g_default_mat.draw_data.fog_params.enabled = true;
    g_default_mat.draw_data.fog_params.setMode(2);
    g_default_mat.draw_data.fog_params.setDensity(0.01f);
    g_default_mat.draw_data.light0.State = 1;
    g_default_mat.apply();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glActiveTexture(GL_TEXTURE0);
}
void segs_renderUtil_4E0CA0()
{
    glewExperimental = GL_TRUE;
    glewInit();
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

    // custom flags, using generic GL shader support
    printf("Enabling GLSL support\n");
    // since we use CORE GL profile, this is a given
    GPU_FLAGS = f_GL_FragmentShader | f_GL_VertexShader;

    g_using_bump_maps = 1;
    enableVertShaders = 1;
    UsingVBOs = 1;
    enableParticleVBOs = 0;
    if ( !g_State.view.enableVBOs )
    {
        assert(false);
        UsingVBOs = 0;
    }
    g_State.view.enableVBOs = 1;

    if ( g_State.view.unkn_4B4 )
    {
        assert(false);
        UsingVBOs = 0;
    }
    enableParticleVBOs = 1;
    assert(!g_State.view.noVBOs);
    assert(!g_State.view.noPixShaders);
    assert(!g_State.view.noVertShaders);
    assert(UsingVBOs);
    VfPrintfWrapper("RENDER SETTINGS: VBOS %d, PARTICLE VBOS %d BUMPMAPS %d, VERTSHADERS %d \n", UsingVBOs,
                    enableParticleVBOs, g_using_bump_maps, enableVertShaders);
    segs_setGlossMultiplier(1.0f);
    segs_rendererInit(); // set default GL state.
    initializeRenderer();
}
void segs_rdrRenderGame()
{
    MaterialDefinition text_mat(g_default_mat);
    auto restore = g_render_state.getGlobal();

    text_mat.setDrawMode(DrawMode::SINGLETEX);
    text_mat.setFragmentMode(eBlendMode::MULTIPLY);
    text_mat.draw_data.projectionMatrix = glm::ortho<float>(0.0, g_State.view.screen_w, 0.0, g_State.view.screen_h, -1.0, 100.0);
    segs_texSetAllToWhite();
    text_mat.draw_data.modelViewMatrix = glm::mat4(1);
    text_mat.draw_data.light0.State = 0;
    text_mat.draw_data.fog_params.enabled = false;
    text_mat.draw_data.globalColor = { 1,1,1,1 };
    text_mat.render_state.setDepthTestMode(RenderState::CMP_NONE);
    text_mat.render_state.setDepthWrite(true);
    if ( getTTDebug() || g_State.ttDebug_enabled )
    {
        text_mat.apply();
        fn_4B08C0();
    }
    else
    {
        segs_drawAllSprites(text_mat);
        TTF_TClearAll();
    }
    g_render_state.apply(restore);
}
struct FontRel
{
    float x;
    float y;
    char *string;
    RGBA color;
    float scaleX;
    float scaleY;
    struct FontStruct *font;
};
#pragma pack(push, 1)
struct FontMapEntry
{
    char u_1;
    char u_2;
    int16_t width;
    int16_t x;
    int16_t y;
    int16_t height;
};
#pragma pack(pop)
struct FontStruct
{
    FontMapEntry *letters;
    TextureBind *p_tex;
    uint8_t indices[128];
};
void segs_rdrRenderEditor(FontRel *font_array, int count)
{
    int screen_h_int;
    int screen_w_int;

    if ( 0==count )
        return;
    segs_wcw_UnBindBufferARB();
    get_screen_size(&screen_w_int, &screen_h_int);
    float screen_w = screen_w_int;
    float screen_h = screen_h_int;
    MaterialDefinition text_mat(g_default_mat);
    text_mat.setDrawMode(DrawMode::SINGLETEX);
    text_mat.setFragmentMode(eBlendMode::MULTIPLY);
    text_mat.draw_data.projectionMatrix = glm::ortho<float>(0.0, screen_w_int, 0.0, screen_h_int, -1.0, 1.0);
    text_mat.draw_data.modelViewMatrix = glm::mat4(1);
    text_mat.render_state.setDepthTestMode(RenderState::CMP_NONE);
    text_mat.draw_data.globalColor = { 1,1,1,1 };
    text_mat.draw_data.fog_params.enabled = false;
    text_mat.draw_data.light0.State = 0;
    text_mat.apply();

    segs_texSetAllToWhite();
    FontStruct *prev_fnt = (FontStruct *)-1;
    static GeometryData fakevbo;
    static bool s_initialized=false;
    if (!s_initialized)
    {
        fakevbo.createVAO();
        s_initialized = true;
    }
    for(int idx = 0;idx<count; ++idx)
    {
        FontStruct *fnt = font_array[idx].font;
        if (!fnt)
            continue;
        float x1 = font_array[idx].x;
        float y = font_array[idx].y;

        if ( x1 > 4096.0f )
            x1 = screen_w - (640.0f - (x1 - 8192.0f));
        if ( y > 4096.0f )
            y = screen_h - (480.0f - (y - 8192.0f));
        TextureBind *fnt_tex      = fnt->p_tex;
        float scale_x     = 1.0f / float(fnt_tex->width);
        float scale_y     = 1.0f / float(fnt_tex->height);
        if ( prev_fnt != fnt )
        {
            prev_fnt = fnt;
            segs_texBindTexture(GL_TEXTURE_2D, 0, fnt->p_tex);
        }
        std::vector<Vector3> bg_positions;
        std::vector<Vector2> bg_uvs;
        std::vector<uint32_t> bg_indices;
        for ( auto i = (const uint8_t *)font_array[idx].string; *i!=0; i++)
        {
            int letter_index  = fnt->indices[*i];
            FontMapEntry *letter = &fnt->letters[letter_index];
            if (letter->width == 0 )
                continue;
            float s1   = letter->x * scale_x;
            float s2 = (letter->x + letter->width) * scale_x;
            float t1   = letter->y * scale_y;
            float t2 = (letter->y + letter->height) * scale_y;
            float x2 = letter->width * font_array[idx].scaleX + x1;
            float y1   = screen_h - y;
            float y2 = screen_h - (letter->height * font_array[idx].scaleY + y);
            size_t start_idx = bg_positions.size();
            bg_positions.insert(bg_positions.end(), {
                                    { x1, y1,0},
                                    { x2, y1,0 },
                                    { x2, y2,0 },
                                    { x1, y2,0 }
                                });
            bg_uvs.insert(bg_uvs.end(), {
                              { s1, t1 },
                              { s2, t1 },
                              { s2, t2},
                              { s1, t2 },
                          });

            bg_indices.insert(bg_indices.end(),
                              {start_idx, start_idx + 1, start_idx + 2, start_idx, start_idx + 2,start_idx + 3});
            x1 = letter->width * font_array[idx].scaleX + x1;
        }
        std::vector<float> collatedBuffer;
        collatedBuffer.resize(bg_positions.size() * 3 + bg_uvs.size() * 2);
        memcpy(collatedBuffer.data(), bg_positions.data(), sizeof(Vector3)*bg_positions.size());
        memcpy(collatedBuffer.data()+ 3*bg_positions.size(), bg_uvs.data(), sizeof(Vector2)*bg_uvs.size());

        fakevbo.uploadVerticesToBuffer(collatedBuffer.data(), collatedBuffer.size());
        fakevbo.uv1_offset = (Vector2 *)(sizeof(Vector3)*bg_positions.size());
        fakevbo.uploadIndicesToBuffer(bg_indices.data(), bg_indices.size());
        segs_texBindTexture(GL_TEXTURE_2D, 0, g_whiteTexture);
        GLubyte bound_alpha = std::min<uint8_t>(0xAA, font_array[idx].color.a);
        text_mat.draw_data.globalColor = RGBA(70, 70, 70, bound_alpha).to4Floats();
        text_mat.updateUniforms();
        fakevbo.draw(*text_mat.program, GL_TRIANGLES, bg_indices.size(), 0);

        segs_texBindTexture(GL_TEXTURE_2D, 0, prev_fnt->p_tex);
        text_mat.draw_data.globalColor = font_array[idx].color.to4Floats();
        text_mat.updateUniforms();
        fakevbo.draw(*text_mat.program, GL_TRIANGLES, bg_indices.size(), 0);
    }
}
void segs_renderUtil_ClearGL()
{
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void segs_rdrPerspective(float fovy, float aspect, float zNear, float _zFar)
{
    int width;
    int height;

    if ( g_State.view.ortho )
    {
        get_screen_size(&width, &height);
        float aspect_ratio = float(width ) / float(height);
        g_default_mat.draw_data.projectionMatrix = glm::ortho(
            -double(g_State.view.orthoViewScale * aspect_ratio),
            double(g_State.view.orthoViewScale * aspect_ratio),
            -double(g_State.view.orthoViewScale),
            double(g_State.view.orthoViewScale),
            10.0,
            50000.0);
    }
    else
    {
        g_default_mat.draw_data.projectionMatrix = glm::perspective(glm::radians(fovy), aspect, zNear, _zFar);
    }
}
void segs_gfxWindowSetAng(float fovy, float aspect, float zNear, float zFar)
{

    segs_rdrPerspective(fovy, aspect, zNear, zFar);
    float v_ang = glm::radians(fovy);
    float h_ang = std::atan(aspect * std::tan(v_ang / 2.0f));
    float h_cos = std::cos(h_ang);
    float v_cos = std::cos(v_ang / 2.0f);
    g_frustumdata.hvam = std::sin(h_ang) / h_cos;
    g_frustumdata.hcos = h_cos;
    g_frustumdata.vvam = std::sin(v_ang / 2.0f) / v_cos;
    g_frustumdata.vcos = v_cos;
    g_frustumdata.znear = -zNear;
    g_frustumdata.zfar = -zFar;
}
void gfxWindowReshapeForHeadShot(int fov)
{
    GLsizei width;
    GLsizei height;

    get_screen_size(&width, &height);
    glViewport(0,0,width, height);
    float fovy = (float)height / float(fov);
    float aspect_ratio = float(width) / height;
    segs_gfxWindowSetAng(fovy, aspect_ratio, g_State.view.near_far_Z, g_State.view.zFar);
    g_State.view.params_proj_mat = g_default_mat.draw_data.projectionMatrix;
}
void segs_gfxWindowReshape()
{
    float   fovy;
    GLsizei height;
    float   aspect_ratio;
    GLsizei width;

    get_screen_size(&width, &height);
    glViewport(0, 0, width, height);
    if (g_State.g_BuildCubeMaps)
        aspect_ratio = 1.0;
    else
        aspect_ratio = (float)width / (float)height;
    fovy = g_State.view.fovY - float(rand() % 31) * g_State.view.camera_shake / 5.0f;
    segs_gfxWindowSetAng(fovy, aspect_ratio, g_State.view.near_far_Z, g_State.view.zFar);
    g_State.view.params_proj_mat = g_default_mat.draw_data.projectionMatrix;
}
int gfxSphereVisible(Vector3 *pos, float rad)
{
    int clip = 1;
    if (g_frustumdata.znear - pos->z <= -rad)
        return 0;
    if (g_frustumdata.znear - pos->z < rad)
        clip = 2;
    if (g_frustumdata.zfar - pos->z >= rad )
        return 0;
    if ((g_frustumdata.hvam * pos->z - pos->x) * g_frustumdata.hcos >= rad)
        return 0;
    if ((g_frustumdata.hvam * pos->z - pos->x) * g_frustumdata.hcos > -rad)
        clip = 2;
    if ((g_frustumdata.hvam * pos->z + pos->x) * g_frustumdata.hcos >= rad)
        return 0;
    if ((g_frustumdata.hvam * pos->z + pos->x) * g_frustumdata.hcos > -rad )
        clip = 2;
    if ((g_frustumdata.vvam * pos->z - pos->y) * g_frustumdata.vcos >= rad)
        return 0;
    if ((g_frustumdata.vvam * pos->z - pos->y) * g_frustumdata.vcos > -rad)
        clip = 2;
    if ((g_frustumdata.vvam * pos->z + pos->y) * g_frustumdata.vcos >= rad)
        return 0;
    if ((g_frustumdata.vvam * pos->z + pos->y) * g_frustumdata.vcos > -rad )
        clip = 2;
    return clip;
}
void segs_wcw_statemgmt_FreeVBO(GLenum target, GLint n, GLuint *buffer, const char *name)
{
    assert(n == 1 && UsingVBOs);
    glBindBuffer(target, *buffer);
    int bufsize;
    glGetBufferParameteriv(target, GL_BUFFER_SIZE, &bufsize);
    fn_5B47F0(name, 3, bufsize);
    glDeleteBuffers(n, buffer);
}
void *captureScreenshot(int *width, int *height)
{
    get_screen_size(width, height);
    void *result = COH_MALLOC(4 * *height * *width);
    glReadPixels(0, 0, *width, *height, GL_RGBA, GL_UNSIGNED_BYTE, result);
    return result;
}

void patch_render_utils()
{
    BREAK_FUNC(rdrPrepareToRenderParticleSystems);
    BREAK_FUNC(wcw_statemgmt_enableColorMaterial);

    PATCH_FUNC(renderUtil_ClearGL);
    PATCH_FUNC(renderUtil_GetGfxCardVend);
    PATCH_FUNC(rdrRenderGame);
    PATCH_FUNC(rdrPerspective);
    patchit("fn_48E560", (void *)gfxWindowReshapeForHeadShot);
    PATCH_FUNC(gfxWindowReshape);
    patchit("fn_4D78E0", (void *)segs_rdrRenderEditor);
    patchit("renderUtil_4DFD50", (void *)segs_rendererInit);
    patchit("renderUtil_4E06B0", reinterpret_cast<void *>(captureScreenshot));
    PATCH_FUNC(renderUtil_4E0CA0);
    PATCH_FUNC(gfxWindowSetAng);
    PATCH_FUNC(wcw_statemgmt_FreeVBO);
}
