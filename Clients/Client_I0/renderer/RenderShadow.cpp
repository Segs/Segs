#include "RenderShadow.h"

#include "Texture.h"
#include "ShaderProgramCache.h"
#include "RendererState.h"
#include "RendererUtils.h"
#include "Model.h"
#include "ModelCache.h"
#include "RenderTricks.h"

#include "utils/helpers.h"
#include "utils/dll_patcher.h"
#include "GL/glew.h"
#include "GameState.h"

#include <cmath>
#include <algorithm>
#include <cstring>
#include <glm/gtc/matrix_transform.hpp>

extern "C" {
__declspec(dllimport) void matMul3x3(const Matrix3x3 *rhs, const Matrix3x3 *lhs, Matrix3x3 *dst);
__declspec(dllimport) void fn_5B6740(float yaw_angle, Matrix3x3 *tgt);
__declspec(dllimport) void MatMult4x3(const Matrix4x3 *self, const Matrix4x3 *oth, Matrix4x3 *res);
__declspec(dllimport) void transposeMat4Copy(const Matrix4x3 *src, Matrix4x3 *dst);
__declspec(dllimport) void matrix3x3_transpose(Matrix3x3 *);
__declspec(dllimport) void xyprintf(int x, int y, const char *fmt, ...);

__declspec(dllimport) RdrStats struct_9E7300;
__declspec(dllimport) CameraInfo cam_info;
__declspec(dllimport) TextureBind *g_whiteTexture;
}
int splatShadowsDrawn;
std::vector<ShadowNode> shadowNodes; //Sub38_PGroupRel_Arr
struct ShadowInfo
{
    Model *   shadow_model;
    Vector3 * shadow_verts;
    int       shadow_vert_count;
    Vector3 * shadow_norms;
    Vector3i *shadow_tris;
    int       shadow_tri_count;
    int       zero_area_tri_count;
    Vector3 * tri_norms;
    char *    open_edges;
    void *    shadow_boneinfo;
};
static_assert(sizeof(ShadowInfo) == 0x28, "ShadowInfo");
namespace
{
int  initializeShadowInfo(Model *model)
{
    int vert_count = 0;
    GeometryData *vbo = model->vbo;
    ShadowInfo *shadow = (ShadowInfo *)COH_CALLOC(sizeof(ShadowInfo), 1);
    vbo->shadow = shadow;
    const int strct_size = 3 * model->model_tri_count;
    shadow->shadow_verts = (Vector3 *)COH_CALLOC(model->vertex_count, sizeof(Vector3));
    shadow->shadow_norms = (Vector3 *)COH_CALLOC(model->vertex_count, sizeof(Vector3));
    shadow->shadow_tris = (Vector3i *)COH_CALLOC(model->model_tri_count, sizeof(Vector3i));
    shadow->tri_norms = (Vector3 *)COH_CALLOC(model->model_tri_count, sizeof(Vector3));
    for (int i = 0; i < strct_size; ++i)
    {
        int tri_idx = i / 3;
        int vert_idx = vbo->triangles[tri_idx][i % 3];
        Vector3 *vec1 = &(((Vector3 *)vbo->cpuside_memory)[vert_idx]);
        int j;
        for (j = 0; j < vert_count && !isVectorDistanceLessThenEps(vec1, &shadow->shadow_verts[j], 0.000005f); ++j)
            ;
        if (j >= vert_count)
        {
            shadow->shadow_verts[vert_count] = *vec1;
            shadow->shadow_norms[vert_count++] = vbo->normals_offset[vert_idx];
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
    shadow->open_edges = (char *)COH_CALLOC(model->model_tri_count, 1);
    memset(shadow->open_edges, 0, model->model_tri_count);
    for (int k = 0; k < strct_size; ++k)
    {
        uint8_t triangle_vertex = k % 3;
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
void setupVolume(Vector3 *vertices, Vector3i *triangles, int *p_triangle_count, Model *model, const Matrix4x3 &mat, float distance)
{
    Vector3 offset_t;
    Matrix4x3 res;

    Vector3 offset = g_sun.shadow_direction * distance;
    res = cam_info.inv_viewmat * mat;
    matrix3x3_transpose(&res.ref3());
    offset_t = res.ref3() * offset;

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

void drawVolume(GeometryData &geom_data, const ShaderProgram &drawprog,uint32_t count)
{
    assert(glGetError() == GL_NO_ERROR);
    geom_data.draw(drawprog, GL_TRIANGLES, count, 0);
    assert(glGetError() == GL_NO_ERROR);
    struct_9E7300.shadow_tri_count += count / 3;
}
void debugDrawShadowVolume(GeometryData &fakevbo, const MaterialDefinition &material, int index_count)
{
    MaterialDefinition mat(material);
    static const RGBA c_drawColors[] = {
        RGBA(0, 0, 0, 0),
        RGBA(100, 255, 0, 70),
        RGBA(0, 0, 0, 0),
        RGBA(255, 255, 255, 150),
        RGBA(0, 0, 255, 100),
    };
    static const RenderState::CullMode c_cullModes[] =
    {
        RenderState::CULL_UNINIT,
        RenderState::CULL_NONE,
        RenderState::CULL_UNINIT,
        RenderState::CULL_CW,
        RenderState::CULL_CCW,
    };
    assert(g_State.view.bShadowVol < 5);
    if (g_State.view.bShadowVol >= 5)
        return;
    mat.draw_data.globalColor = c_drawColors[g_State.view.bShadowVol].to4Floats();
    mat.render_state.setCullMode(c_cullModes[g_State.view.bShadowVol]);
    mat.apply();
    drawVolume(fakevbo, *mat.program, index_count);
}
void stencilShadowVolume(const GLvoid *pointer, size_t vbo_size_in_floats, const Vector3i *indices, int count,
                         const Matrix4x3 &mat, uint8_t alpha, unsigned int mask)
{
    static uint8_t gl_ColorArray[100000];
    static bool s_StipplePatterns_initialized = false;
    if (!s_StipplePatterns_initialized)
    {
        s_StipplePatterns_initialized = true;
        memset(gl_ColorArray, 1, 100000u);
    }
    MaterialDefinition stencilMat(g_default_mat);

    stencilMat.setDrawMode(DrawMode::SINGLETEX);
    stencilMat.setFragmentMode(eBlendMode::MULTIPLY);
    segs_wcw_UnBindBufferARB();
    segs_texBindTexture(GL_TEXTURE_2D, 0, g_whiteTexture);
    segs_texBindTexture(GL_TEXTURE_2D, 1, g_whiteTexture);
    stencilMat.draw_data.modelViewMatrix = mat.toGLM();

    static GeometryData fakevbo;
    fakevbo.createVAO();
    fakevbo.uploadVerticesToBuffer((float *)pointer, vbo_size_in_floats);
    fakevbo.uploadIndicesToBuffer((const uint32_t *)&indices->i0, count * 3);
    fakevbo.uploadColorsToBuffer(gl_ColorArray, 100000);


    if (g_State.view.bShadowVol)
        debugDrawShadowVolume(fakevbo, stencilMat, count * 3);
    stencilMat.render_state.setCullMode(RenderState::CULL_NONE);
    stencilMat.apply();
    MaterialDefinition stencilFlatMat(g_default_mat);
    stencilFlatMat.setDrawMode(DrawMode::FILL);
    stencilFlatMat.setFragmentMode(eBlendMode::MULTIPLY);
    stencilFlatMat.draw_data.modelViewMatrix = mat.toGLM();
    stencilFlatMat.draw_data.light0.State = 1;
    stencilFlatMat.draw_data.light0.Diffuse = Vector4(mat.TranslationPart,1); // drawing with the volume's translation color ?
    //TODO: this is wonky original fixed-function pipeline will multiply like this MV * modelview.TranslationPart
    stencilFlatMat.draw_data.light0.Position = Vector4(mat.TranslationPart, 1);// Position light at mat location
    if (alpha != 255)
    {

        stencilFlatMat.set_useStippling(true);
        stencilFlatMat.draw_data.setStippleAlpha(alpha);
    }
    else
        stencilFlatMat.set_useStippling(false);

    if (mask)
    {
        stencilFlatMat.render_state.stencil_mode = RenderState::STENCIL_NOTEQUAL;
        stencilFlatMat.render_state.stencil_ref = mask;
        stencilFlatMat.render_state.stencil_mask = mask;
    }
    else
    {
        stencilFlatMat.render_state.stencil_mode = RenderState::STENCIL_ALWAYS;
        stencilFlatMat.render_state.stencil_ref = 0;
        stencilFlatMat.render_state.stencil_mask = ~0U;
    }
    stencilFlatMat.render_state.setColorWrite(false);
    stencilFlatMat.render_state.setDepthWrite(false);
    stencilFlatMat.apply();

    stencilFlatMat.render_state.setDepthTestMode(RenderState::CMP_LESS);

    stencilFlatMat.render_state.setCullMode(RenderState::CULL_CW);
    stencilFlatMat.render_state.stencil_fail = GL_KEEP;
    stencilFlatMat.render_state.setStencilOp(GL_KEEP, GL_INCR, GL_KEEP);
    stencilFlatMat.apply();
    
    drawVolume(fakevbo,*stencilFlatMat.program,count*3);
    stencilFlatMat.render_state.setCullMode(RenderState::CULL_CCW);
    stencilFlatMat.render_state.setStencilOp(GL_KEEP, GL_DECR, GL_KEEP);
    stencilFlatMat.apply();
    drawVolume(fakevbo, *stencilFlatMat.program, count * 3);
    stencilFlatMat.render_state.setCullMode(RenderState::CULL_CCW);
    stencilFlatMat.render_state.setColorWrite(true);
    stencilFlatMat.render_state.setDepthWrite(true);
    stencilFlatMat.render_state.setDepthTestMode(RenderState::CMP_LESSEQUAL);
    stencilFlatMat.render_state.stencil_mode = RenderState::STENCIL_NONE;
    stencilFlatMat.apply();
}
void drawModelShadowVolume(Model *model,Matrix4x3 &mat, uint8_t alpha, int shadowmask, struct GfxTree_Node *node)
{
    static Vector3 vbuf[10000];
    static Vector3i shadow_indices[10000 / 3];
    static int tri_count;
    Matrix4x3 tmp;
    float shadow_dist;
    if (!model || g_State.view.bShadowVol == 2 || g_sun.shadowcolor.w < 0.05f || !(model->loadstate & 4) || node)
        return;

    if (model->trck_node && model->trck_node->_TrickFlags & 0x800000)
    {
        matMul3x3(&cam_info.inv_viewmat.ref3(), &mat.ref3(), &tmp.ref3());
        float yaw = -std::atan2(tmp.r3.x, tmp.r3.z) - std::atan2(g_sun.shadow_direction.x, -g_sun.shadow_direction.z);
        fn_5B6740(yaw, &mat.ref3());
    }
    if (model->trck_node && model->trck_node->_TrickFlags & 0x400000)
        shadow_dist = model->trck_node->info->ShadowDist;
    else
        shadow_dist = 2*model->model_VisSphereRadius;
    assert(model && model->vbo);
    segs_modelSetupVertexObject(model, 2);
    if (!model->vbo->gl_index_buffer)
    {
        if (!model->vbo->shadow)
            initializeShadowInfo(model);
        setupVolume(vbuf, shadow_indices, &tri_count, model, mat, shadow_dist);

        stencilShadowVolume (vbuf, model->vbo->shadow->shadow_vert_count*2*3, shadow_indices, tri_count, mat, alpha, shadowmask);
    }
}
void drawShadowColor(RenderState &rs)
{
    GLDebugGuard deb(__FUNCTION__);
    Vector4 v = g_sun.shadowcolor;
    MaterialDefinition shadow_color_mat(g_default_mat);
    shadow_color_mat.setDrawMode(DrawMode::COLORONLY);
    shadow_color_mat.setFragmentMode(eBlendMode::MULTIPLY);
    shadow_color_mat.render_state = rs;
    //glPushAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    shadow_color_mat.draw_data.constColor1  = g_sun.shadowcolor;
    shadow_color_mat.draw_data.projectionMatrix = glm::ortho(0.0, 640.0, 0.0, 480.0, -1.0, 1.0);
    shadow_color_mat.draw_data.modelViewMatrix = glm::mat4(1);
    shadow_color_mat.draw_data.light0.State = 0;
    shadow_color_mat.draw_data.fog_params.enabled = false;
    shadow_color_mat.draw_data.globalColor        = v;
    static GeometryData fakevbo;
    if (!fakevbo.segs_data)
    {
        fakevbo.createVAO();
        Vector3 pos[] = {
            {0, 480, 0},
            {640, 480, 0},
            {640, 0, 0},
            {0, 0, 0},
        };
        uint32_t quad_indices[] = {0, 1, 2, 0, 2, 3};
        fakevbo.uploadVerticesToBuffer((float *)pos, 12);
        fakevbo.uploadIndicesToBuffer(quad_indices, 6);
    }


    shadow_color_mat.set_colorSource(0); // global color only
    shadow_color_mat.apply();
    fakevbo.draw(*shadow_color_mat.program, GL_TRIANGLES, 6, 0);
    //glPopAttrib();
}
}// end of anonymous namespace

void  segs_rendertree_drawShadows()
{
    for(ShadowNode &shadow_node : shadowNodes)
    {
        if (shadow_node.model)
        {
            drawModelShadowVolume(shadow_node.model, shadow_node.mat, shadow_node.alpha, shadow_node.shadowMask, nullptr);
        }
    }
    shadowNodes.clear();
}
void segs_shadowFinishScene()
{
    if (g_State.view.bShadowVol == 2 || g_sun.shadowcolor.w < 0.05f)
        return;
    RenderState shadow_rs = g_render_state.getGlobal();
    RenderState recovery = g_render_state.getGlobal();
    shadow_rs.setDepthWrite(false);
    shadow_rs.setDepthTestMode(RenderState::CMP_ALWAYS);
    shadow_rs.stencil_mode = RenderState::STENCIL_NOTEQUAL;
    shadow_rs.stencil_ref = 0;
    shadow_rs.stencil_mask = ~0xC0u;
    shadow_rs.setStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    drawShadowColor(shadow_rs);
    recovery.setDepthWrite(true);
    recovery.setDepthTestMode(RenderState::CMP_LESSEQUAL);
    recovery.stencil_mode = RenderState::STENCIL_NONE;
    g_render_state.apply(recovery);
}

void segs_modelDrawShadowObject(Matrix4x3 *viewSpaceTransform, SplatSib *splat,const MaterialDefinition &base_material)
{
    RenderState reset_mat = g_render_state.getGlobal();
    GLDebugGuard debug_guard(__FUNCTION__);
    Matrix4x3 res;
    Matrix4x3 dest;

    ++splatShadowsDrawn;
    if ( g_State.view.bSimpleShadowDeb )
    {
        xyprintf(40, splatShadowsDrawn + 10, "Tris %d", splat->triangleCount);
        xyprintf(55, splatShadowsDrawn + 10, "Verts %d", splat->vertexCount);
    }
    if ( splat->invertedSplat )
        segs_modelDrawShadowObject(viewSpaceTransform, splat->invertedSplat,base_material);
    float zdist = std::min(-20.0f,viewSpaceTransform->TranslationPart.z);
    float fixup_amount = 0.0001f;
    float delta_add = 0.01f;
    float delta = fixup_amount * zdist * zdist / (zdist * fixup_amount + 1.0f) + delta_add;
    float epsilon = -2.0f * g_State.view.zFar * g_State.view.near_far_Z * delta / ((g_State.view.zFar + g_State.view.near_far_Z) * zdist * (zdist + delta));
    Matrix4x4 m = g_State.view.params_proj_mat;
    m.TranslationPart.z *=  epsilon + 1.0f;
    if ( g_State.view.bSimpleShadowDeb )
    {
        xyprintf(30, 30, "pz %f   f %f   n %f   MYCONST %f  EXTRABIT %f  PMatBefore %f", zdist, g_State.view.zFar, g_State.view.near_far_Z,
                 fixup_amount, delta_add, g_State.view.params_proj_mat.r3.z);
        xyprintf(30, 31, "delta   %f    epsilon   %f   PMatAfter %f", delta, epsilon, m.TranslationPart.z);
    }
    MaterialDefinition shadowObjectMaterial(base_material);

    shadowObjectMaterial.draw_data.projectionMatrix = m.toGLM();
    dest = Unity_Matrix;
    float offset = splatShadowsDrawn / 500.0f;
    dest.TranslationPart = splat->normal * (offset + 0.01f);
    MatMult4x3(&cam_info.viewmat, &dest, &res);
    res.TranslationPart.z += offset + 0.02f;
    shadowObjectMaterial.draw_data.modelViewMatrix = res.toGLM();
    shadowObjectMaterial.setDrawMode(DrawMode::DUALTEX);
    shadowObjectMaterial.setFragmentMode(eBlendMode::MULTIPLY);
    segs_wcw_UnBindBufferARB();
    shadowObjectMaterial.draw_data.tex_id_0 = splat->texture1;
    shadowObjectMaterial.draw_data.tex_id_1 = splat->texture2;
    segs_texBindTexture(GL_TEXTURE_2D, 0, splat->texture1);
    segs_texBindTexture(GL_TEXTURE_2D, 1, splat->texture2);
    if ( splat->flags & 1 )
    {
        shadowObjectMaterial.render_state.setBlendMode(RenderState::BLEND_ADDALPHA);
        //        if ( GPU_FLAGS & fNV_vertex_program )
        //            glFinalCombinerInputNV(GL_VARIABLE_C_NV, 0, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
    }
    shadowObjectMaterial.render_state.setCullMode(RenderState::CULL_CCW);
    if ( splat->stAnim )
        segs_animateSts(splat->stAnim,shadowObjectMaterial.draw_data);

    shadowObjectMaterial.draw_data.light0.State = 0;

    shadowObjectMaterial.render_state.setDepthWrite(false);

    shadowObjectMaterial.apply();
    GeometryData fakevbo;
    fakevbo.createVAO();
    size_t current_offset = 0;
    fakevbo.createSizedVBO(splat->vertexCount * (3+2+2));
    fakevbo.uploadSubBufferToVertices((float *)splat->vertices, splat->vertexCount * 3, current_offset);
    current_offset += splat->vertexCount * 3;
    fakevbo.uploadSubBufferToVertices((float *)splat->tex_scroll1, splat->vertexCount * 2, current_offset);
    fakevbo.uv1_offset = (Vector2 *)(current_offset*sizeof(float));
    current_offset += splat->vertexCount * 2;
    fakevbo.uploadSubBufferToVertices((float *)splat->tex_scroll2, splat->vertexCount * 2, current_offset);
    fakevbo.uv2_offset = (Vector2 *)(current_offset * sizeof(float));
    current_offset += splat->vertexCount * 2;

    fakevbo.uploadColorsToBuffer((uint8_t *)splat->colors, splat->vertexCount * 4);
    fakevbo.uploadIndicesToBuffer((uint32_t *)splat->indices, 3*splat->triangleCount);
    fakevbo.draw(*shadowObjectMaterial.program, GL_TRIANGLES, 3 * splat->triangleCount, 0);
    {
        if (splat->flags & 1)
        {
            reset_mat.setBlendMode(RenderState::BLEND_ALPHA);
        }
        g_render_state.apply(reset_mat);
    }
}
void segs_shadowStartScene()
{
    Vector3 neg_dir = - g_sun.direction.ref3();
    float dir_length = std::min(0.7f,neg_dir.length());
    //TODO: the following code makes not much sense, but the client was doing it this way :/
    neg_dir.y = clamp(neg_dir.y,-0.7f,0.7f);
    neg_dir.y = std::copysign(dir_length,neg_dir.y);
    neg_dir.normalize();
    g_sun.shadow_direction = neg_dir;
    g_sun.shadow_direction_in_viewspace = cam_info.viewmat.ref3() * g_sun.shadow_direction;
}
void patch_shadow_renderer()
{
    BREAK_FUNC(shadowFinishScene);
    BREAK_FUNC(shadowStartScene);
    BREAK_FUNC(rendertree_drawShadows);
}

