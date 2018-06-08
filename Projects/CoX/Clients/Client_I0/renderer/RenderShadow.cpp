#include "RenderShadow.h"

#include "Texture.h"
#include "ShaderProgramCache.h"
#include "RendererState.h"
#include "RendererUtils.h"
#include "Model.h"
#include "RenderModel.h"

#include "utils/helpers.h"
#include "utils/dll_patcher.h"
#include "GL/glew.h"
#include "RenderTricks.h"
#include "GameState.h"

#include <cmath>
#include <algorithm>
#include <cstring>

extern "C" {
    __declspec(dllimport) int texSetWhite(int unit);
    __declspec(dllimport) void matMul3x3(const Matrix3x3 *rhs, const Matrix3x3 *lhs, Matrix3x3 *dst);
    __declspec(dllimport) void  fn_5B6740(float yaw_angle, Matrix3x3 *a2);
    __declspec(dllimport) void fn_4E79C0(GLenum target, int texunit, TextureBind *texbind);
    __declspec(dllimport) void modelSetupVertexObject(Model *, int);
    __declspec(dllimport) void * dbgCalloc(int strct_size, int count, int blockType, const char *fname, int line);
    __declspec(dllimport) void MatMult4x3(const Matrix4x3 *self, const Matrix4x3 *oth, Matrix4x3 *res);
    __declspec(dllimport) void transposeMat4Copy(const Matrix4x3 *src, Matrix4x3 *dst);
    __declspec(dllimport) void matrix3x3_transpose(Matrix3x3 *);
    __declspec(dllimport) void wcwMgmt_EnableFog(int);
    __declspec(dllimport) void xyprintf(int x, int y, const char *fmt, ...);
    __declspec(dllimport) void fn_4E79C0(GLenum tex, int idx, TextureBind *bind);
    __declspec(dllimport) void animateSts(Parser_StAnim *anim);

    __declspec(dllimport) DrawMode g_curr_draw_state;
    __declspec(dllimport) eBlendMode g_curr_blend_state;
    __declspec(dllimport) RdrStats struct_9E7300;
    __declspec(dllimport) CameraInfo cam_info;
    __declspec(dllimport) Array Sub38_PGroupRel_Arr;
    __declspec(dllimport) SunLight g_sun;
    __declspec(dllimport) TextureBind *g_whiteTexture;
    __declspec(dllimport) int GPU_FLAGS;
    __declspec(dllimport) int splatShadowsDrawn;

}
struct ShadowNode
{
    Model *model;
    Matrix4x3 mat;
    char alpha;
    char shadowMask;
};
struct ShadowInfo
{
    Model *shadow_model;
    Vector3 *shadow_verts;
    int shadow_vert_count;
    Vector3 *shadow_norms;
    Vector3i *shadow_tris;
    int shadow_tri_count;
    int zero_area_tri_count;
    Vector3 *tri_norms;
    char *open_edges;
    void *shadow_boneinfo;
};
static_assert(sizeof(ShadowInfo) == 0x28, "ShadowInfo");
namespace
{
    int  initializeShadowInfo(Model *model)
    {
        int vert_count = 0;
        GeometryData *vbo = model->vbo;
        ShadowInfo *shadow = (ShadowInfo *)dbgCalloc(sizeof(ShadowInfo), 1, 1, __FILE__, __LINE__);
        vbo->shadow = shadow;
        const int strct_size = 3 * model->model_tri_count;
        shadow->shadow_verts = (Vector3 *)dbgCalloc(model->vertex_count, sizeof(Vector3), 1, __FILE__, __LINE__);
        shadow->shadow_norms = (Vector3 *)dbgCalloc(model->vertex_count, sizeof(Vector3), 1, __FILE__, __LINE__);
        shadow->shadow_tris = (Vector3i *)dbgCalloc(model->model_tri_count, 4, 1, __FILE__, __LINE__);
        shadow->tri_norms = (Vector3 *)dbgCalloc(model->model_tri_count, sizeof(Vector3), 1, __FILE__, __LINE__);
        for (int i = 0; i < strct_size; ++i)
        {
            int tri_idx = i / 3;
            int vert_idx = vbo->triangles[tri_idx][i % 3];
            Vector3 *vec1 = &vbo->vertices[vert_idx];
            int j;
            for (j = 0; j < vert_count && !isVectorDistanceLessThenEps(vec1, &shadow->shadow_verts[j], 0.000005f); ++j)
                ;
            if (j >= vert_count)
            {
                shadow->shadow_verts[vert_count] = *vec1;
                shadow->shadow_norms[vert_count++] = vbo->normals_vbo_idx[vert_idx];
            }
            shadow->shadow_tris[tri_idx][i%3] = j;
            if (i % 3 == 2)
            {
                const Vector3i tri = shadow->shadow_tris[tri_idx];
                Vector3 dv10 = shadow->shadow_verts[tri.i1] - shadow->shadow_verts[tri.i0];
                Vector3 dv21 = shadow->shadow_verts[tri.i2] - shadow->shadow_verts[tri.i1];
                shadow->tri_norms[tri_idx] = dv10.cross(dv21);
                shadow->tri_norms[tri_idx].normalize();
            }
        }
        shadow->shadow_vert_count = vert_count;
        shadow->shadow_tri_count = model->model_tri_count;
        shadow->open_edges = (char *)dbgCalloc(model->model_tri_count, 1, 1, __FILE__, __LINE__);
        memset(shadow->open_edges, 0, model->model_tri_count);
        for (int k = 0; k < strct_size; ++k)
        {
            int triangle_vertex = k % 3;
            int triangle_idx = k / 3;
            int next_triangle_vertex = (triangle_vertex + 1) % 3;
            Vector3i triangle(shadow->shadow_tris[triangle_idx]);
            int pt1_idx = triangle[triangle_vertex];
            int pt2_idx = triangle[next_triangle_vertex];
            int l;
            for (l = 0; l < strct_size; ++l)
            {
                int other_tri_idx = l / 3;
                if (other_tri_idx == triangle_idx)
                    continue;
                Vector3i other_tri(shadow->shadow_tris[other_tri_idx]);
                int other_triangle_vertex = l % 3;

                int other_next_vertex = (other_triangle_vertex + 1) % 3;
                if (pt2_idx == other_tri[other_triangle_vertex] &&
                    pt1_idx == other_tri[other_next_vertex] &&
                    isVectorDistanceLessThenEps(&shadow->tri_norms[triangle_idx], &shadow->tri_norms[other_tri_idx], 0.001f))
                    break;
            }
            if (l >= strct_size)
                shadow->open_edges[triangle_idx] |= 1 << triangle_vertex;
        }
        return 0;
    }
    void setupVolume(Vector3 *vertices, Vector3i *triangles, int *p_triangle_count, Model *model, const Matrix4x3 &a2, float distance)
    {
        Vector3 offset_t;
        Matrix4x3 res;

        Vector3 offset = g_sun.shadow_direction * distance;
        MatMult4x3(&cam_info.inv_viewmat, &a2, &res);
        matrix3x3_transpose((Matrix3x3 *)&res);
        multVertexByMatrix(&offset, (Matrix3x3 *)&res, &offset_t);

        Vector3 normalized_offset = offset_t;
        normalized_offset.normalize();
        ShadowInfo *shadow = model->vbo->shadow;
        for (int i = 0; i < shadow->shadow_vert_count; ++i)
        {
            vertices[i + shadow->shadow_vert_count] = offset_t + shadow->shadow_verts[i];
        }
        offset_t = normalized_offset * 0.1f;
        for (int i = 0; i < shadow->shadow_vert_count; ++i)
        {
            vertices[i] = offset_t + shadow->shadow_verts[i];
        }
        int created_tri_idx = 0;
        for (int i=0;i < shadow->shadow_tri_count; i++)
        {
            float side = shadow->tri_norms[i].dot(normalized_offset);
            int pt1_idx = shadow->shadow_tris[i].i0;
            int pt2_idx = shadow->shadow_tris[i].i1;
            int pt3_idx = shadow->shadow_tris[i].i2;
            // indices of extruded vertices
            int new_pt1 = pt1_idx + shadow->shadow_vert_count;
            int new_pt2 = pt2_idx + shadow->shadow_vert_count;
            int new_pt3 = pt3_idx + shadow->shadow_vert_count;
            if (side < 0.0f)
            {
                triangles[created_tri_idx++] = { pt1_idx,pt2_idx,pt3_idx };
                triangles[created_tri_idx++] = { new_pt3,new_pt2,new_pt1 };
                if (shadow->open_edges[i] & 1)
                {
                    triangles[created_tri_idx++] = { new_pt2,pt2_idx,pt1_idx };
                    triangles[created_tri_idx++] = { pt1_idx,new_pt1,new_pt2 };
                }
                if (shadow->open_edges[i] & 2)
                {
                    triangles[created_tri_idx++] = { new_pt3,pt3_idx,pt2_idx };
                    triangles[created_tri_idx++] = { pt2_idx,new_pt2,new_pt3 };
                }
                if (shadow->open_edges[i] & 4)
                {
                    triangles[created_tri_idx++] = { new_pt1,pt1_idx,pt3_idx };
                    triangles[created_tri_idx++] = { pt3_idx,new_pt3,new_pt1 };
                }
            }
            else
            {
                triangles[created_tri_idx++] = { pt3_idx,pt2_idx,pt1_idx };
                triangles[created_tri_idx++] = { new_pt1,new_pt2,new_pt3 };
                if (shadow->open_edges[i] & 1)
                {
                    triangles[created_tri_idx++] = { pt1_idx,pt2_idx,new_pt2 };
                    triangles[created_tri_idx++] = { new_pt2,new_pt1,pt1_idx };
                }
                if (shadow->open_edges[i] & 2)
                {
                    triangles[created_tri_idx++] = { pt2_idx,pt3_idx,new_pt3 };
                    triangles[created_tri_idx++] = { new_pt3,new_pt2,pt2_idx };
                }
                if (shadow->open_edges[i] & 4)
                {
                    triangles[created_tri_idx++] = { pt3_idx,pt1_idx,new_pt1 };
                    triangles[created_tri_idx++] = { new_pt1,new_pt3,pt3_idx };
                }
            }
        }
        *p_triangle_count = created_tri_idx;
    }
    void  create_stipple_pattern(uint32_t *pattern, int intensity)
    {
        // intensity to stipple pattern
        uint32_t patt[4];
        if (intensity <= 48)
        {
            patt[0] = 0x55555555;
            patt[1] = 0;
            patt[2] = 0x11111111;
            patt[3] = 0;
        }
        else if (intensity <= 64)
        {
            patt[0] = 0x55555555;
            patt[1] = 0;
            patt[2] = 0x55555555;
            patt[3] = 0;
        }
        else if (intensity <= 80)
        {
            patt[0] = 0x55555555;
            patt[1] = 0x22222222;
            patt[2] = 0x55555555;
            patt[3] = 0;
        }
        else if (intensity <= 96)
        {
            patt[0] = 0x55555555;
            patt[1] = 0x22222222;
            patt[2] = 0x55555555;
            patt[3] = 0x88888888;
        }
        else if (intensity <= 102)
        {
            patt[0] = 0x55555555;
            patt[1] = 0xAAAAAAAA;
            patt[2] = 0x55555555;
            patt[3] = 0x88888888;
        }
        else if (intensity <= 128)
        {
            patt[0] = 0x55555555;
            patt[1] = 0xAAAAAAAA;
            patt[2] = 0x55555555;
            patt[3] = 0xAAAAAAAA;
        }
        else if (intensity <= 144)
        {
            patt[0] = 0x55555555;
            patt[1] = 0xEEEEEEEE;
            patt[2] = 0x55555555;
            patt[3] = 0xAAAAAAAA;
        }
        else if (intensity <= 160)
        {
            patt[0] = 0x55555555;
            patt[1] = 0xEEEEEEEE;
            patt[2] = 0x55555555;
            patt[3] = 0xBBBBBBBB;
        }
        else if (intensity <= 176)
        {
            patt[0] = 0x55555555;
            patt[1] = 0xFFFFFFFF;
            patt[2] = 0x55555555;
            patt[3] = 0xBBBBBBBB;
        }
        else if (intensity <= 192)
        {
            patt[0] = 0x55555555;
            patt[1] = 0xFFFFFFFF;
            patt[2] = 0x55555555;
            patt[3] = 0xFFFFFFFF;
        }
        else if (intensity <= 208)
        {
            patt[0] = 0xEEEEEEEE;
            patt[1] = 0xFFFFFFFF;
            patt[2] = 0x55555555;
            patt[3] = 0xFFFFFFFF;
        }
        else if (intensity <= 224)
        {
            patt[0] = 0xEEEEEEEE;
            patt[1] = 0xFFFFFFFF;
            patt[2] = 0xBBBBBBBB;
            patt[3] = 0xFFFFFFFF;
        }
        else if (intensity <= 240)
        {
            patt[0] = 0xFFFFFFFF;
            patt[1] = 0xFFFFFFFF;
            patt[2] = 0xBBBBBBBB;
            patt[3] = 0xFFFFFFFF;
        }
        else
        {
            patt[0] = 0xFFFFFFFF;
            patt[1] = 0xFFFFFFFF;
            patt[2] = 0xFFFFFFFF;
            patt[3] = 0xFFFFFFFF;
        }
        for (int i = 0; i < 32; i += 4)
        {
            memcpy(pattern + i, patt, sizeof(uint32_t) * 4);
        }
    }

    void make_stipples(unsigned int(*stipple_patterns)[32])
    {
        for (int i = 0; i < 16; ++i)
            create_stipple_pattern(stipple_patterns[i], 16 * i - 1);
    }
    void drawVolume(const GLvoid *pointer, const GLvoid *indices, int count)
    {
        glVertexPointer(3, GL_FLOAT, 0, pointer);
        glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, indices);
        struct_9E7300.shadow_tri_count += count / 3;
    }
    void debugDrawShadowVolume(const GLvoid *vbuf, const GLvoid *indices, int count)
    {
        if (g_State.view.bShadowVol == 1)
        {
            glEnable(GL_CULL_FACE);
            glColor4ub(100, 255, 0, 70);
            glDisable(GL_CULL_FACE);
            drawVolume(vbuf, indices, count);
            glEnable(GL_CULL_FACE);
        }
        if (g_State.view.bShadowVol == 3)
        {
            glFrontFace(GL_CCW);
            glEnable(GL_CULL_FACE);
            glColor4ub(255, 255, 255, 150);
            glCullFace(GL_BACK);
            drawVolume(vbuf, indices, count);
            glFrontFace(GL_CW);
        }
        if (g_State.view.bShadowVol == 4)
        {
            glEnable(GL_CULL_FACE);
            glColor4ub(0, 0, 255, 100);
            glCullFace(GL_BACK);
            drawVolume(vbuf, indices, count);
        }
    }
    void stencilShadowVolume(const GLvoid *pointer, const GLvoid *indices, int count, const Matrix4x3 &mat, uint8_t alpha, unsigned int mask)
    {
        static char gl_ColorArray[100000];
        static bool s_StipplePatterns_initialized = false;
        static unsigned int stipple_patterns[16][32];
        if (!s_StipplePatterns_initialized)
        {
            s_StipplePatterns_initialized = true;
            make_stipples(stipple_patterns);
            memset(gl_ColorArray, 1, 100000u);
        }
        glColorPointer(4, GL_UNSIGNED_BYTE, 0, gl_ColorArray);
        segs_setupShading(DrawMode::SINGLETEX_NORMALS, eBlendMode::MULTIPLY);
        segs_modelDrawState(DrawMode::SINGLETEX, 1);
        segs_wcw_UnBindBufferARB();
        fn_4E79C0(GL_TEXTURE_2D, 0, g_whiteTexture);
        fn_4E79C0(GL_TEXTURE_2D, 1, g_whiteTexture);
        glMatrixMode(GL_MODELVIEW);
        Matrix4x4 modelview = mat;
        glLoadMatrixf(modelview.data());
        if (g_State.view.bShadowVol)
            debugDrawShadowVolume(pointer, indices, count*3);
        glDisable(GL_CULL_FACE);
        glEnable(GL_LIGHTING);
        segs_wcw_statemgmt_SetLightParam(GL_LIGHT0, GL_DIFFUSE, &modelview.TranslationPart);
        segs_wcw_statemgmt_SetLightParam(GL_LIGHT0, GL_POSITION, &modelview.TranslationPart);
        segs_modelDrawState(DrawMode::FILL, 0);
        segs_modelBlendState(eBlendMode::MULTIPLY, 0);
        if (alpha != 255)
        {
            glEnable(GL_POLYGON_STIPPLE);
            glPolygonStipple((const GLubyte *)stipple_patterns[alpha >> 4]);
        }
        glEnable(GL_STENCIL_TEST);
        glStencilMask(0xFFFFFFFF);
        if (mask)
            glStencilFunc(GL_NOTEQUAL, mask, mask);
        else
            glStencilFunc(GL_ALWAYS, 0, 0xFFFFFFFF);
        glColorMask(0, 0, 0, 0);
        segs_wcw_statemgmt_setDepthMask(false);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        glStencilOp(GL_KEEP, GL_INCR, GL_KEEP);
        drawVolume(pointer, indices, count * 3);
        glCullFace(GL_BACK);
        glStencilOp(GL_KEEP, GL_DECR, GL_KEEP);
        drawVolume(pointer, indices, count * 3);
        glCullFace(GL_BACK);
        glColorMask(1, 1, 1, 1);
        segs_wcw_statemgmt_setDepthMask(true);
        glDepthFunc(GL_LEQUAL);
        glDisable(GL_STENCIL_TEST);
        glDisable(GL_POLYGON_STIPPLE);
    }
    void drawModelShadowVolume(Model *model,Matrix4x3 &mat, uint8_t alpha, int shadowmask, struct GfxTree_Node *node)
    {
        static Vector3 vbuf[10000];
        static Vector3i shadow_indices[10000 / 3];
        static int tri_count;
        Matrix4x3 tmp;
        float shadow_dist;
        if (!model || g_State.view.bShadowVol == 2 || g_sun.shadowcolor.w < 0.05 || !(model->loadstate & 4) || node)
            return;

        if (model->trck_node && model->trck_node->_TrickFlags & 0x800000)
        {
            matMul3x3(&cam_info.inv_viewmat.ref3(), &mat.ref3(), (Matrix3x3 *)&tmp);
            float yaw = -std::atan2(tmp.r3.x, tmp.r3.z) - std::atan2(g_sun.shadow_direction.x, -g_sun.shadow_direction.z);
            fn_5B6740(yaw, &mat.ref3());
        }
        if (model->trck_node && model->trck_node->_TrickFlags & 0x400000)
            shadow_dist = model->trck_node->info->ShadowDist;
        else
            shadow_dist = 2*model->model_VisSphereRadius;
        assert(model && model->vbo);
        modelSetupVertexObject(model, 2);
        if (!model->vbo->gl_index_buffer)
        {
            if (!model->vbo->shadow)
                initializeShadowInfo(model);
            setupVolume(vbuf, shadow_indices, &tri_count, model, mat, shadow_dist);
            stencilShadowVolume (vbuf, shadow_indices, tri_count, mat, alpha, shadowmask);
        }
    }
    void drawShadowColor()
    {
        Vector4 v = g_sun.shadowcolor;
        segs_modelDrawState(DrawMode::COLORONLY, 1);
        segs_modelBlendState(eBlendMode::MULTIPLY, 1);
        glPushAttrib(0xFFFFFFFF);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0.0, 640.0, 0.0, 480.0, -1.0, 1.0);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glDisable(GL_LIGHTING);
        wcwMgmt_EnableFog(0);
        glActiveTextureARB(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, g_whiteTexture->gltexture_id);
        glColor4fv(&v.x);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0);
        glVertex2f(0.0, 480.0);
        glTexCoord2f(1.0, 0.0);
        glVertex2f(640.0, 480.0);
        glTexCoord2f(1.0, 1.0);
        glVertex2f(640.0, 0.0);
        glTexCoord2f(0.0, 1.0);
        glVertex2f(0.0, 0.0);
        glEnd();
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glPopAttrib();
    }
}// end of anonymous namespace

void  segs_rendertree_drawShadows()
{
    return;
    ShadowNode *nodes = (ShadowNode *)Sub38_PGroupRel_Arr.contents;
    for(int idx=0; idx<Sub38_PGroupRel_Arr.capacity; ++idx)
    {
        ShadowNode &shadow_node(nodes[idx]);
        if (shadow_node.model)
        {
            drawModelShadowVolume(shadow_node.model, shadow_node.mat, shadow_node.alpha, shadow_node.shadowMask, nullptr);
        }
    }
    Sub38_PGroupRel_Arr.capacity = 0;
}
void segs_shadowFinishScene()
{
    if (g_State.view.bShadowVol == 2 || g_sun.shadowcolor.w < 0.05)
        return;
    glEnable(GL_STENCIL_TEST);
    segs_wcw_statemgmt_setDepthMask(false);
    glDepthFunc(GL_ALWAYS);
    glStencilFunc(GL_NOTEQUAL, 0, ~0xC0u);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    drawShadowColor();
    segs_wcw_statemgmt_setDepthMask(true);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_STENCIL_TEST);
}
void segs_modelDrawShadowObject(Matrix4x3 *viewSpaceTransform, SplatSib *splat)
{
    Matrix4x3 res;
    Matrix4x3 dest;
    float delta;
    float epsilon;
    float zdist;

    ++splatShadowsDrawn;
    if ( g_State.view.bSimpleShadowDeb )
    {
        xyprintf(40, splatShadowsDrawn + 10, "Tris %d", splat->triangleCount);
        xyprintf(55, splatShadowsDrawn + 10, "Verts %d", splat->vertexCount);
    }
    if ( splat->invertedSplat )
        segs_modelDrawShadowObject(viewSpaceTransform, splat->invertedSplat);
    zdist = std::min(-20.0f,viewSpaceTransform->TranslationPart.z);
    float fixup_amount = 0.0001f;
    float delta_add = 0.01f;
    delta = fixup_amount * zdist * zdist / (zdist * fixup_amount + 1.0f) + delta_add;
    epsilon = -2.0f * g_State.view.zFar * g_State.view.near_far_Z * delta / ((g_State.view.zFar + g_State.view.near_far_Z) * zdist * (zdist + delta));
    Matrix4x4 m = g_State.view.params_proj_mat;
    m.TranslationPart.z *=  (epsilon + 1.0f);
    if ( g_State.view.bSimpleShadowDeb )
    {
        xyprintf(30, 30, "pz %f   f %f   n %f   MYCONST %f  EXTRABIT %f  PMatBefore %f", zdist, g_State.view.zFar, g_State.view.near_far_Z,
                           fixup_amount, delta_add, g_State.view.params_proj_mat.r3.z);
        xyprintf(30, 31, "delta   %f    epsilon   %f   PMatAfter %f", delta, epsilon, m.TranslationPart.z);
    }
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadMatrixf(m.data());
    dest = Unity_Matrix;
    float offset = splatShadowsDrawn / 500.0f;
    dest.TranslationPart = splat->normal * (offset + 0.01f);
    MatMult4x3(&cam_info.viewmat, &dest, &res);
    res.TranslationPart.z += offset + 0.02f;
    Matrix4x4 res_gl = res;
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(res_gl.data());
    segs_modelBlendState(eBlendMode::MULTIPLY, 0);
    segs_modelDrawState(DrawMode::DUALTEX, 0);
    segs_wcw_UnBindBufferARB();
    fn_4E79C0(GL_TEXTURE_2D, 0, splat->texture1);
    fn_4E79C0(GL_TEXTURE_2D, 1, splat->texture2);
    if ( splat->flags & 1 )
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        if ( GPU_FLAGS & fNV_vertex_program )
            glFinalCombinerInputNV(GL_VARIABLE_C_NV, 0, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
    }
    if ( splat->stAnim )
        animateSts(splat->stAnim);
    glMatrixMode(GL_MODELVIEW);
    segs_wcw_statemgmt_setDepthMask(0);
    glDisable(GL_LIGHTING);
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, splat->colors);
    segs_UVPointer(GL_TEXTURE0, 2, GL_FLOAT, 0, splat->tex_scroll1);
    segs_UVPointer(GL_TEXTURE1, 2, GL_FLOAT, 0, splat->tex_scroll2);
    glVertexPointer(3, GL_FLOAT, 0, splat->vertices);
    glCullFace(GL_BACK);
    glDrawElements(GL_TRIANGLES, 3 * splat->triangleCount, GL_UNSIGNED_INT, splat->indices);
    if ( splat->flags & 1 )
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        if ( GPU_FLAGS & 1 )
        {
            g_curr_draw_state = DrawMode::INVALID;
            g_curr_blend_state = eBlendMode::INVALID;
        }
    }
    glMatrixMode(GL_TEXTURE);
    glClientActiveTextureARB(GL_TEXTURE0);
    glActiveTextureARB(GL_TEXTURE0);
    glLoadIdentity();
    glClientActiveTextureARB(GL_TEXTURE1);
    glActiveTextureARB(GL_TEXTURE1);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    segs_wcw_statemgmt_setDepthMask(true);
    glEnable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}
void patch_shadow_renderer()
{
    PATCH_FUNC(shadowFinishScene);
    PATCH_FUNC(rendertree_drawShadows);
}
