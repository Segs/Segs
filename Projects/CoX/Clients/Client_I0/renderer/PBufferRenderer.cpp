#include "PBufferRenderer.h"

#include "RendererState.h"
#include "Texture.h"

#include "utils/dll_patcher.h"
#include "GL/wglew.h"
#include "GL/glew.h"
#include "RendererUtils.h"
#include "GameState.h"
#include "ShaderProgramCache.h"
#include "Model.h"
#include "RenderBonedModel.h"
#include <glm/gtc/matrix_transform.hpp>

extern "C" {
    __declspec(dllimport) int texSetWhite(int unit);
    __declspec(dllimport) void handleErrorStr(const char *, ...);
    __declspec(dllimport) HICON fn_57F7A0(char *pixdata, int hotspot_x, int hotspot_y);
    __declspec(dllimport) HCURSOR hCursor;
    __declspec(dllimport) int32_t int32_7318AC;
}

namespace {
    struct OffscreenBuffer
    {
        int width;
        int height;
        GLuint FramebufferName;
        GLuint renderColorBuffer;
    };

    OffscreenBuffer struct_8D15DC = { 0,0,0,0 };
    int s_cursor_size = 32;

    void releaseOBUF(OffscreenBuffer *fb)
    {
        if (0==fb->FramebufferName)
            return;
        //Delete resources
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glDeleteRenderbuffers(1, &fb->renderColorBuffer);

        //Bind 0, which means render to back buffer, as a result, fb is unbound
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &fb->FramebufferName);
        fb->width = 0;
        fb->height = 0;
    }
    void obufInit(OffscreenBuffer *fb, int width, int height)
    {
        if (fb->FramebufferName)
            releaseOBUF(fb);

        fb->FramebufferName = 0;
        glGenFramebuffers(1, &fb->FramebufferName);
        glBindFramebuffer(GL_FRAMEBUFFER, fb->FramebufferName);

        //Create and attach a color buffer
        glGenRenderbuffers(1, &fb->renderColorBuffer);
        //We must bind color_rb before we call glRenderbufferStorage
        glBindRenderbuffer(GL_RENDERBUFFER, fb->renderColorBuffer);
        //The storage format is RGBA8
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, width, height);

        //Attach color buffer to FBO
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, fb->renderColorBuffer);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            handleErrorStr("FBO creation error: Incomplete.\n");
            return;
        }
        fb->width = width;
        fb->height = height;
        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

    }
}

bool segs_setupRenderTarget(OffscreenBuffer *pbuff)
{
    if (pbuff->FramebufferName == 0)
        return false;
    glBindFramebuffer(GL_FRAMEBUFFER, pbuff->FramebufferName);
    return true;
}
void segs_PBuffer__RenderTexQuad(TextureBind *tex, int width, int height, float x, float y, float scale, uint32_t color)
{
    static GeometryData fake_vbo;
    segs_setupRenderTarget(&struct_8D15DC);
    glViewport(0, 0, struct_8D15DC.width, struct_8D15DC.height);
    auto restore = g_render_state.getGlobal();
    segs_rendererInit();
    fake_vbo.createVAO();
    MaterialDefinition mat(g_default_mat);
    mat.setDrawMode(DrawMode::SINGLETEX);
    mat.setFragmentMode(eBlendMode::MULTIPLY);
    mat.draw_data.projectionMatrix = glm::ortho<float>(0.0, struct_8D15DC.width, 0.0, struct_8D15DC.height, -1.0, 1.0);
    mat.draw_data.modelViewMatrix = glm::mat4(1);
    mat.render_state.setBlendMode(RenderState::BLEND_ALPHA);
    mat.render_state.setDepthTestMode(RenderState::CMP_NONE);
    mat.draw_data.globalColor = { 1,1,1,1 };
    mat.draw_data.light0.State = 0;
    mat.draw_data.fog_params.enabled = false;
    float x2 = (float)width * scale + x;
    float y2 = (float)height * scale + y;
    float vbo_data[] = {
         x, (struct_8D15DC.height - y),0 ,
         x2, (struct_8D15DC.height - y),0 ,
         x2, (struct_8D15DC.height - y2), 0 ,
         x, (struct_8D15DC.height - y2), 0 ,
        // UV
        0,0,
        1,0,
        1,1,
        0,1
    };
    uint32_t indices[]               = {0, 1, 2, 0, 2, 3};

    fake_vbo.uv1_offset = (Vector2 *)(sizeof(Vector3)*4); // start of uv data
    fake_vbo.uploadVerticesToBuffer(vbo_data,20);
    fake_vbo.uploadIndicesToBuffer(indices, 6);
    Vector4 converted = RGBA(color).to4Floats();
    fprintf(stderr,"global color is %x - %f,%f,%f,%f\n",color,converted.x,converted.y,converted.z,converted.w);
    mat.draw_data.globalColor = RGBA(color).to4Floats();
    mat.set_colorSource(0);
    mat.apply();

    texSetWhite(1);
    texSetWhite(0);
    segs_tex_texLoad2(tex->name1, 2, 1);
    segs_texBindTexture(GL_TEXTURE_2D, 0, tex);
    fake_vbo.draw(*mat.program, GL_TRIANGLES, 6, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    g_render_state.apply(restore);
}
void hwcursorInit()
{
    s_cursor_size = 64;
    int32_7318AC = s_cursor_size;
    obufInit(&struct_8D15DC, s_cursor_size, s_cursor_size);
}
void segs_PBuffer__Clear()
{
    segs_setupRenderTarget(&struct_8D15DC);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
static char * segs_PBuffer__FrameGrab(OffscreenBuffer *buff, char *pre_alloc, unsigned int tgt_size)
{
    char *tgt_mem;

    if (!segs_setupRenderTarget(buff))
        handleErrorStr("PBuffer::MakeCurrent() failed.\n");
    unsigned int required_size = 4 * buff->height * buff->width;
    if (required_size <= tgt_size)
        tgt_mem = pre_alloc;
    else
        tgt_mem = (char *)malloc(required_size);

    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glReadPixels(0, 0, buff->width, buff->height, GL_RGBA, GL_UNSIGNED_BYTE, tgt_mem);
    glFinish();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return tgt_mem;
}
HCURSOR segs_hwcursorSet()
{
    if (hCursor && g_State.can_set_cursor)
        return SetCursor(hCursor);
    HCURSOR result = GetCursor();
    if (result)
    {
        result = SetCursor(GetCursor());
    }
    return result;
}
static void hwcursorReload(HCURSOR *pCursor, int hostspot_x, int hotspot_y)
{
    static bool was_set = false;

    char tmp_buff[32768];
    char *pixdata;
    if (pCursor && *pCursor)
    {
        hCursor = *pCursor;
        was_set = true;
    }
    else
    {
        pixdata = segs_PBuffer__FrameGrab(&struct_8D15DC, tmp_buff, 32768);
        if (!pixdata)
            return;
        if (hCursor && !was_set)
            DestroyIcon(hCursor);
        hCursor = fn_57F7A0(pixdata, hostspot_x, hotspot_y);
        if (pixdata != tmp_buff)
            free(pixdata);
        if (pCursor)
        {
            *pCursor = hCursor;
        }
        was_set = pCursor != nullptr;
    }
    segs_hwcursorSet();
}
static int iconIsNew(TextureBind *base_spr, TextureBind *cursor_dragged_icon)
{
    static TextureBind *curr_base_spr;
    static TextureBind *curr_cursor_dragged_icon;
    bool newIcon = false;
    if (curr_base_spr != base_spr || curr_cursor_dragged_icon != cursor_dragged_icon)
    {
        newIcon = true;
        curr_base_spr = base_spr;
        curr_cursor_dragged_icon = cursor_dragged_icon;
    }
    return newIcon;
}
void segs_hwcursorReloadDefault()
{
    static int init=0;
    static TextureBind *cursor=nullptr;

    if (!init)
    {
        cursor = segs_tex_texLoad2("btest_cursor", 2, 1);
        init = 1;
    }
    iconIsNew(nullptr, nullptr);
    iconIsNew(cursor, nullptr);
    segs_PBuffer__Clear();
    segs_PBuffer__RenderTexQuad(cursor, cursor->width, cursor->height, 0.0, 0.0, 1.0, 0xFFFFFFFF);
    hwcursorReload(nullptr, 0, 0);
}
void patch_pbuffer()
{
    patchit("fn_57F170", reinterpret_cast<void*>(hwcursorInit));
    patchit("fn_57F6B0", reinterpret_cast<void*>(hwcursorReload));
    patchit("fn_57FE20", reinterpret_cast<void*>(segs_hwcursorReloadDefault));
    BREAK_FUNC(setupRenderTarget);
    PATCH_FUNC(PBuffer__RenderTexQuad);
    PATCH_FUNC(PBuffer__Clear);
    PATCH_FUNC(hwcursorSet);

}
