#include "entityDebug.h"

#include "renderer/RendererState.h"
#include "renderer/RendererUtils.h"
#include "renderer/RenderTricks.h"
#include "utils/dll_patcher.h"

#include "GL/glew.h"
#include <stdint.h>
#include <cstring>
#include <cassert>

extern "C" {
    __declspec(dllimport) void wcwMgmt_EnableFog(int);
    __declspec(dllimport) OptRel_A0 server_visible_state;
    __declspec(dllimport) float g_TIMESTEP;
    __declspec(dllimport) CameraInfo cam_info;
}
void drawLine3D_2Color(Vector3 *p1, unsigned int argb_pt1, Vector3 *p2, unsigned int argb_pt2)
{
    Matrix4x4 model_view;

    Vector3 toCam = cam_info.cammat.TranslationPart - *p1;
    toCam.normalize();
    Vector3 a = toCam * 0.05f + *p1;
    toCam = cam_info.cammat.TranslationPart - *p2;
    toCam.normalize();
    Vector3 b = toCam * 0.05f + *p2;
    model_view = cam_info.viewmat;
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(model_view.data());
    segs_wcw_statemgmt_bindTexture(GL_TEXTURE_2D, 0, 0);
    segs_modelDrawState(DrawMode::COLORONLY, 0);
    segs_modelBlendState(eBlendMode::MULTIPLY, 0);
    wcwMgmt_EnableFog(0);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_LINES);
    glColor4ub((argb_pt1 & 0xFF0000) >> 16, (argb_pt1 & 0xFF00) >> 8, argb_pt1, (argb_pt1 & 0xFF000000) >> 24);
    glVertex3f(a.x, a.y, a.z);
    glColor4ub((argb_pt2 & 0xFF0000) >> 16, (argb_pt2 & 0xFF00) >> 8, argb_pt2, (argb_pt2 & 0xFF000000) >> 24);
    glVertex3f(b.x, b.y, b.z);
    glEnd();
}
void drawLine2D_2Color(int x1, int y1, int x2, int y2, uint32_t argb_pt1, uint32_t a6)
{
    segs_wcw_statemgmt_bindTexture(GL_TEXTURE_2D, 0, 0);
    glBegin(GL_LINES);
    glColor4ub((argb_pt1 & 0xFF0000u) >> 0x10, (unsigned __int16)(argb_pt1 & 0xFF00) >> 8, argb_pt1, (argb_pt1 & 0xFF000000) >> 0x18);
    glVertex3f(x1, y1, -1.0);
    glColor4ub((a6 & 0xFF0000u) >> 0x10, (unsigned __int16)(a6 & 0xFF00) >> 8, a6, (a6 & 0xFF000000) >> 0x18);
    glVertex3f(x2, y2, -1.0);
    glEnd();
}
void segs_entDebug_DrawUnculledTriangle(Vector3 *vertex1, unsigned int color1, Vector3 *vertex2, unsigned int color2, Vector3 *vertex3, unsigned int color3)
{
    Matrix4x4 m;
    m = cam_info.viewmat;
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(m.data());
    segs_wcw_statemgmt_bindTexture(GL_TEXTURE_2D, 0, 0);
    segs_setupShading(DrawMode::COLORONLY, eBlendMode::MULTIPLY);
    wcwMgmt_EnableFog(0);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glBegin(GL_TRIANGLES);
    glColor4ub((color1 & 0xFF0000) >> 0x10, (unsigned __int16)(color1 & 0xFF00) >> 8, color1, (color1 & 0xFF000000) >> 24);
    glVertex3f(vertex1->x, vertex1->y, vertex1->z);
    glColor4ub((color2 & 0xFF0000) >> 0x10, (unsigned __int16)(color2 & 0xFF00) >> 8, color2, (color2 & 0xFF000000) >> 24);
    glVertex3f(vertex2->x, vertex2->y, vertex2->z);
    glColor4ub((color3 & 0xFF0000) >> 0x10, (unsigned __int16)(color3 & 0xFF00) >> 8, color3, (color3 & 0xFF000000) >> 24);
    glVertex3f(vertex3->x, vertex3->y, vertex3->z);
    glEnd();
    glEnable(GL_CULL_FACE);
}
void patch_ent_debug()
{
    patchit("entDebug_42EE00",(void *)drawLine3D_2Color);
    patchit("entDebug_42F640", (void *)drawLine2D_2Color);
    PATCH_FUNC(entDebug_DrawUnculledTriangle);
}
