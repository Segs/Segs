#include "entityDebug.h"

#include "renderer/RendererState.h"
#include "renderer/RendererUtils.h"
#include "renderer/RenderTricks.h"
#include "utils/dll_patcher.h"
#include "renderer/Model.h"
#include "renderer/ShaderProgramCache.h"
#include <glm/gtc/matrix_transform.hpp>

#include "GL/glew.h"
#include <stdint.h>

extern "C" {
    __declspec(dllimport) void get_screen_size(int *,int *);
    __declspec(dllimport) float g_TIMESTEP;
    __declspec(dllimport) CameraInfo cam_info;
}

static MaterialDefinition debugDrawMaterial(DrawMode::COLORONLY, eBlendMode::MULTIPLY);
static GeometryData debug_geom;
void drawLine3D_2Color(const Vector3 *p1, unsigned int argb_pt1, const Vector3 *p2, unsigned int argb_pt2)
{
    GLDebugGuard guard(__FUNCTION__);
    debug_geom.createVAO();
    Vector3 toCam = cam_info.cammat.TranslationPart - *p1;
    toCam.normalize();
    const Vector3 a = toCam * 0.05f + *p1;
    toCam = cam_info.cammat.TranslationPart - *p2;
    toCam.normalize();
    const Vector3 b = toCam * 0.05f + *p2;
    debugDrawMaterial.draw_data.modelViewMatrix = cam_info.viewmat.toGLM();
    segs_wcw_statemgmt_bindTexture(GL_TEXTURE_2D, 0, 0);
    debugDrawMaterial.draw_data.fog_params.enabled = false;

    Vector3 pos[] = {
        a,b
    };
    uint8_t colors[] = {
        uint8_t(argb_pt1 >> 16) , uint8_t(argb_pt1>> 8) , uint8_t(argb_pt1), uint8_t(argb_pt1 >> 24),
        uint8_t(argb_pt2 >> 16) , uint8_t(argb_pt2>> 8) , uint8_t(argb_pt2), uint8_t(argb_pt2 >> 24)
    };
    debugDrawMaterial.render_state.setBlendMode(RenderState::BLEND_ALPHA);
    debugDrawMaterial.apply();
    debug_geom.uploadVerticesToBuffer((float *)pos,6);
    debug_geom.uploadColorsToBuffer(colors,8);
    debug_geom.drawArray(*debugDrawMaterial.program, GL_LINES, 2, 0);
}
void drawLine3D_z_2Color(const Vector3 *p1, unsigned int argb_pt1, const Vector3 *p2, unsigned int argb_pt2)
{
    GLDebugGuard guard(__FUNCTION__);
    debug_geom.createVAO();
    const Vector3 a = *p1;
    const Vector3 b = *p2;
    debugDrawMaterial.draw_data.modelViewMatrix = cam_info.viewmat.toGLM();
    segs_wcw_statemgmt_bindTexture(GL_TEXTURE_2D, 0, 0);
    debugDrawMaterial.draw_data.fog_params.enabled = false;

    Vector3 pos[] = {
        a,b
    };
    uint8_t colors[] = {
        uint8_t(argb_pt1 >> 16) , uint8_t(argb_pt1 >> 8) , uint8_t(argb_pt1), uint8_t(argb_pt1 >> 24),
        uint8_t(argb_pt2 >> 16) , uint8_t(argb_pt2 >> 8) , uint8_t(argb_pt2), uint8_t(argb_pt2 >> 24)
    };
    debugDrawMaterial.render_state.setBlendMode(RenderState::BLEND_ALPHA);
    debugDrawMaterial.apply();
    debug_geom.uploadVerticesToBuffer((float *)pos, 6);
    debug_geom.uploadColorsToBuffer(colors, 8);
    debug_geom.drawArray(*debugDrawMaterial.program, GL_LINES, 2, 0);
}
void drawLine2D_2Color(int x1, int y1, int x2, int y2, uint32_t argb_pt1, uint32_t argb_pt2)
{
    debug_geom.createVAO();
    Vector3 a {float(x1),float(y1),-1};
    Vector3 b {float(x2),float(y2),-1};
    Vector3 pos[] = {
        a,b
    };
    uint8_t colors[] = {
        uint8_t(argb_pt1 >> 16) , uint8_t(argb_pt1>> 8) , uint8_t(argb_pt1), uint8_t(argb_pt1 >> 24),
        uint8_t(argb_pt2 >> 16) , uint8_t(argb_pt2>> 8) , uint8_t(argb_pt2), uint8_t(argb_pt2 >> 24)
    };
    debug_geom.uploadVerticesToBuffer((float *)pos,3*2);
    debug_geom.uploadColorsToBuffer(colors,8);
    debugDrawMaterial.apply();

    segs_wcw_statemgmt_bindTexture(GL_TEXTURE_2D, 0, 0);
    debug_geom.drawArray(*debugDrawMaterial.program, GL_LINES, 2, 0);
}
void segs_entDebug_DrawUnculledTriangle(Vector3 *vertex1, uint32_t color1, Vector3 *vertex2, uint32_t color2, Vector3 *vertex3, uint32_t color3)
{
    debug_geom.createVAO();
    segs_wcw_statemgmt_bindTexture(GL_TEXTURE_2D, 0, 0);
    MaterialDefinition unculled_tri(g_default_mat);
    unculled_tri.setDrawMode(DrawMode::COLORONLY);
    unculled_tri.setFragmentMode(eBlendMode::MULTIPLY);
    unculled_tri.draw_data.modelViewMatrix = cam_info.viewmat.toGLM();
    unculled_tri.render_state.setBlendMode(RenderState::BLEND_ALPHA);
    unculled_tri.render_state.setCullMode(RenderState::CULL_NONE);

    Vector3 pos[]{
        *vertex1,*vertex2,*vertex3
    };
    uint8_t colors[] =
    {
        uint8_t(color1 >> 16) , uint8_t(color1>> 8) , uint8_t(color1), uint8_t(color1 >> 24),
        uint8_t(color2 >> 16) , uint8_t(color2>> 8) , uint8_t(color2), uint8_t(color2 >> 24),
        uint8_t(color3 >> 16) , uint8_t(color3>> 8) , uint8_t(color3), uint8_t(color3 >> 24)
    };
    debug_geom.uploadVerticesToBuffer((float *)pos, 3*3);
    debug_geom.uploadColorsToBuffer(colors,12);

    unculled_tri.draw_data.fog_params.enabled = false;
    unculled_tri.apply();
    debug_geom.drawArray(*debugDrawMaterial.program, GL_TRIANGLES, 3, 0);
}
void segs_entDebug_displayEntDebugInfoTextBegin2()
{
    int screen_w;
    int screen_h;
    MaterialDefinition debug_text_mat(g_default_mat);
    debug_text_mat.setDrawMode(DrawMode::SINGLETEX);
    debug_text_mat.setFragmentMode(eBlendMode::MULTIPLY);

    get_screen_size(&screen_w, &screen_h);
    segs_texSetAllToWhite();
    debug_text_mat.draw_data.projectionMatrix = glm::ortho<float>(0.0, screen_w, 0.0, screen_h, -1.0, 100.0);
    debug_text_mat.draw_data.modelViewMatrix = glm::mat4(1);
    debug_text_mat.draw_data.light0.State = 0;
    debug_text_mat.draw_data.fog_params.enabled = false;
    debug_text_mat.render_state.setDepthTestMode(RenderState::CMP_NONE);
    debug_text_mat.render_state.setDepthWrite(true);
    debug_text_mat.draw_data.globalColor = { 1,1,1,1 };
    debug_text_mat.apply();
}
void  segs_entDebug_displayEntDebugInfoTextEnd()
{
    RenderState restore_state;
    restore_state.setDepthTestMode(RenderState::CMP_LESSEQUAL);
    g_fog_state.enabled = true;
    g_render_state.apply(restore_state);
}
void drawQuad2D_4Color(int x1, int y1, int x2, int y2, uint32_t clr2, uint32_t clr3, uint32_t clr1, uint32_t clr4)
{
    static GeometryData fakevbo;
    if (!fakevbo.segs_data)
        fakevbo.createVAO();
    if (x1 > x2)
        std::swap(x1, x2);
    if (y1 > y2)
        std::swap(y1, y2);

    MaterialDefinition colored_mat(g_default_mat);
    colored_mat.setDrawMode(DrawMode::COLORONLY);
    colored_mat.setFragmentMode(eBlendMode::MULTIPLY);

    segs_wcw_statemgmt_bindTexture(GL_TEXTURE_2D, 0, 0);
    float pos[] = {
        float(x1), float(y1),-1 ,
        float(x1), float(y2),-1 ,
        float(x2), float(y2),-1 ,
        float(x2), float(y1),-1 ,
    };
    uint8_t colors[] = {
        uint8_t(clr1>> 16), uint8_t(clr1 >> 8), uint8_t(clr1), uint8_t(clr1 >> 24),
        uint8_t(clr2>> 16), uint8_t(clr2 >> 8), uint8_t(clr2), uint8_t(clr2 >> 24),
        uint8_t(clr3>> 16), uint8_t(clr3 >> 8), uint8_t(clr3), uint8_t(clr3 >> 24),
        uint8_t(clr4>> 16), uint8_t(clr4 >> 8), uint8_t(clr4), uint8_t(clr4 >> 24)

    };
    uint32_t indices[] = {0, 1, 2, 0, 2, 3};
    fakevbo.uploadVerticesToBuffer(pos, 12);
    fakevbo.uploadColorsToBuffer(colors,16);
    fakevbo.uploadIndicesToBuffer(indices, 6);
    colored_mat.apply();
    fakevbo.draw(*colored_mat.program, GL_TRIANGLES, 6, 0);
}
void drawQuad2D_1Color(int x1, int y1, int x2, int y2, uint32_t clr)
{
    drawQuad2D_4Color(x1, y1, x2, y2, clr, clr, clr, clr);
}
void patch_ent_debug()
{
    patchit("entDebug_42EE00",(void *)drawLine3D_2Color);
    patchit("entDebug_42F640", (void *)drawLine2D_2Color);
    patchit("entDebug_42CBC0", (void *)drawQuad2D_4Color);
    patchit("entDebug_42CB90", (void *)drawQuad2D_1Color);
    PATCH_FUNC(entDebug_DrawUnculledTriangle);
    PATCH_FUNC(entDebug_displayEntDebugInfoTextBegin2);
    PATCH_FUNC(entDebug_displayEntDebugInfoTextEnd);
}
