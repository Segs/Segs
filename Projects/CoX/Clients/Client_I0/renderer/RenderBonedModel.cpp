#include "RenderBonedModel.h"

#include "RendererState.h"
#include "ShaderProgramCache.h"
#include "Texture.h"
#include "Model.h"
#include "GameState.h"
#include "RenderTricks.h"

#include "utils/helpers.h"
#include "utils/dll_patcher.h"
#include "GL/glew.h"

#include <stdint.h>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <algorithm>
#include <array>

struct GlobRenderRel;
struct ModelGlobalX;

extern "C" {
__declspec(dllimport) void modelSetupVertexObject(Model *,int);
__declspec(dllimport) bool legitBone(int);
__declspec(dllimport) void fn_4E79C0(GLenum target, int texunit, TextureBind *texbind);
__declspec(dllimport) void FreeMem(void *,int);
__declspec(dllimport) void *malloc1(int sz,int,const char *,int);
__declspec(dllimport) void wcwMgmt_EnableFog(int);
__declspec(dllimport) void MatMult4x3(const Matrix4x3 *self, const Matrix4x3 *oth, Matrix4x3 *res);
__declspec(dllimport) void transposeMat4Copy(const Matrix4x3 *a1, Matrix4x3 *a2);

__declspec(dllimport) BoneInfo struct_9EBDA0[70]; //fake bone info array
__declspec(dllimport) CameraInfo cam_info;
__declspec(dllimport) int enableParticleVBOs;
__declspec(dllimport) int GPU_FLAGS;
__declspec(dllimport) int UsingVBOs;
__declspec(dllimport) int g_using_bump_maps;
__declspec(dllimport) int enableVertShaders;
__declspec(dllimport) SunLight g_sun;
__declspec(dllimport) DrawMode g_curr_draw_state;
__declspec(dllimport) eBlendMode g_curr_blend_state;
__declspec(dllimport) Vector4 shdr_Constant1;
__declspec(dllimport) Vector4 shdr_Constant2;
__declspec(dllimport) TextureBind *int32_9EBD10[3];
__declspec(dllimport) TextureBind *g_whiteTexture;
__declspec(dllimport) RdrStats struct_9E7300;
__declspec(dllimport) Vector4 struct_9A0CC0[2];
}
static GLuint currently_bound_gl_buffer;
void segs_wcw_UnBindBufferARB()
{
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBufferARB(GL_ARRAY_BUFFER, 0);
    currently_bound_gl_buffer = ~0U;
}
int  segs_modelBindBuffer(Model *model)
{
    VBO *vbo = model->vbo;
    assert(UsingVBOs);
    if ( currently_bound_gl_buffer == vbo->gl_vertex_buffer )
        return 0;
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, vbo->gl_index_buffer);
    glBindBufferARB(GL_ARRAY_BUFFER, vbo->gl_vertex_buffer);
    currently_bound_gl_buffer = vbo->gl_vertex_buffer;
    return 1;
}
void calcNodeLights(GfxTree_Node *node,Vector4 &ambient_col,Vector4 &diffuse_col,Vector3 &light_pos)
{
    GlobRenderRel * seqGfxData = node->seqGfxData;
    if ( (g_State.view.game_mode == 1 || g_State.view.game_mode == 3) && seqGfxData->light.use & ENTLIGHT_INDOOR_LIGHT )
    {
        ambient_col = seqGfxData->light.ambient_light;
        diffuse_col = seqGfxData->light.diffuse;
        multVertexByMatrix((Vector3 *)&seqGfxData->light.direction, (Matrix3x3 *)&cam_info.viewmat, &light_pos);
    }
    else
    {
        if ( node->flg & 0x1000000 )
        {
            ambient_col = g_sun.ambient;
            diffuse_col = g_sun.diffuse;
        }
        else
        {
            ambient_col = g_sun.ambient_for_players;
            diffuse_col = g_sun.diffuse_for_players;
        }
        (Vector3 &)light_pos = (Vector3 &)g_sun.direction_in_viewspace;
    }
    if ( seqGfxData->light.use & (ENTLIGHT_CUSTOM_DIFFUSE|ENTLIGHT_CUSTOM_AMBIENT) )
    {
        ambient_col.x = ambient_col.x * seqGfxData->light.ambientScale;
        ambient_col.y = ambient_col.y * seqGfxData->light.ambientScale;
        ambient_col.z = ambient_col.z * seqGfxData->light.ambientScale;
        diffuse_col.x = diffuse_col.x * seqGfxData->light.diffuseScale;
        diffuse_col.y = diffuse_col.y * seqGfxData->light.diffuseScale;
        diffuse_col.z = diffuse_col.z * seqGfxData->light.diffuseScale;
    }

}
void  segs_UVPointer(uint32_t tex_unit, GLint size, GLenum type, GLsizei stride, GLvoid *pointer)
{
    glClientActiveTextureARB(tex_unit);
    glTexCoordPointer(size, type, stride, pointer);
}
static void setTexOrOverride(TextureBind *tex,int idx)
{
    if (tex->flags & 0x400 && int32_9EBD10[idx])
        fn_4E79C0((GLenum)tex->texture_target, idx, int32_9EBD10[idx]);
    else
        fn_4E79C0((GLenum)tex->texture_target, idx, tex);

}
void  drawLoop(Model *model)
{

    VBO *vbo = model->vbo;
    int offset = 0;
    if ( !vbo->tex_uv1 )
        return;

    for(int idx=0; idx<model->num_textures; ++idx)
    {
        GLsizei entry_count = 3 * model->texture_bind_offsets[idx].tri_count;
        TextureBind *bind = vbo->textureP_arr[idx];
        TextureBind *link1 = !bind->tex_links[1] ? g_whiteTexture : bind->tex_links[1];
        TextureBind *texture = bind->tex_links[0];

        assert( link1 );
        setTexOrOverride(bind, 0);
        setTexOrOverride(link1, 2);
        setTexOrOverride(texture, 1);
        glDrawElements(4, entry_count, GL_UNSIGNED_INT, &vbo->triangles->i0 + offset);
        offset += entry_count;
    }
}
void segs_modelDrawBonedNode(GfxTree_Node *node)
{
    Vector4 ambient_col;
    Vector4 diffuse_col;
    GLsizei count;
    TextureBind *tex=nullptr;
    TextureBind *tex2=nullptr;
    Vector4 light_pos;
    Vector4 gloss_amounts;

    Model *model = node->model;
    int tri_offset = 0;
    assert(model && model->vbo);
    modelSetupVertexObject(model, 1);
    segs_modelBindBuffer(model);
    VBO *vbo = model->vbo;
    GlobRenderRel *seqGfxData = node->seqGfxData;
    TrickNode *tricks = node->trick_node;
    BoneInfo *boneinfo = model->boneinfo;
    assert(vbo && legitBone(node->anim_id()) && seqGfxData && boneinfo);
    assert(model->loadstate & ModelState::LOADED);
    if ( !vbo->tex_uv1 )
        return;

    segs_setTexUnitLoadBias(GL_TEXTURE0, -2.0);
    segs_setTexUnitLoadBias(GL_TEXTURE1, -2.0);
    calcNodeLights(node,ambient_col,diffuse_col,light_pos.ref3());
    ambient_col.w = 1.0;
    diffuse_col.w = 1.0;
    if ( !g_using_bump_maps )
    {
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        light_pos.w = 0.0;
        segs_wcw_statemgmt_SetLightParam(GL_LIGHT0, GL_AMBIENT, &ambient_col);
        segs_wcw_statemgmt_SetLightParam(GL_LIGHT0, GL_DIFFUSE, &diffuse_col);
        segs_wcw_statemgmt_SetLightParam(GL_LIGHT0, GL_POSITION, &light_pos);
    }
    light_pos.w = 0.5;
    int tex_cnt = model->num_textures;
    if ( node->flg & 0x40000 )
    {
        assert(node->customtex[0] && node->customtex[1]);
        tex = node->customtex[0];
        tex2 = node->customtex[1];
        tex_cnt = 1;
        count = 3 * model->model_tri_count;
    }
    shdr_Constant1 = {1,1,1,(node->flg&0xFF)/255.0f};
    segs_gfxNodeTricks(tricks, model, nullptr);
    if ( node->flg & 0x200000 )
    {
        shdr_Constant1.ref3() = node->color1.to3Floats();
        shdr_Constant2.ref3() = node->color2.to3Floats();
    }
    if ( g_State.view.bWireframe && !UsingVBOs )
    {
        assert(false);
        // unhandled mode
    }
    glNormalPointer(GL_FLOAT, 0, vbo->normals_vbo_idx);
    glVertexPointer(3, GL_FLOAT, 0, vbo->vertices);
    segs_UVPointer(GL_TEXTURE0, 2, GL_FLOAT, 0, vbo->tex_uv1);
    segs_UVPointer(GL_TEXTURE1, 2, GL_FLOAT, 0, vbo->tex_uv2);
    for ( int i = 0; i < tex_cnt; ++i )
    {
        if ( !(node->flg & 0x40000) )
        {
            count = 3 * model->texture_bind_offsets[i].tri_count;
            tex = vbo->textureP_arr[i];
            tex2 = tex->tex_links[0];
        }
        fn_4E79C0(GL_TEXTURE_2D, 0, tex);
        fn_4E79C0(GL_TEXTURE_2D, 1, tex2);
        assert(enableVertShaders);
        if ( tex && tex->tex_links[1] != nullptr && (tex->tex_links[1]->flags & 0x800) && g_using_bump_maps )
        {
            segs_setupShading(DrawMode::BUMPMAP_SKINNED, eBlendMode::BUMPMAP_COLORBLEND_DUAL);
            fn_4E79C0(GL_TEXTURE_2D, 2, tex->tex_links[1]);
            gloss_amounts.w = g_sun.gloss_scale * tex->tex_links[1]->Gloss;
            if ( diffuse_col.y < 0.15f )
                gloss_amounts.w *= 0.35f;
            gloss_amounts.x = gloss_amounts.y = gloss_amounts.z = gloss_amounts.w;

            Vector4 processed_ambient;
            processed_ambient[0] = std::min(1.0f,ambient_col.x * 2);
            processed_ambient[1] = std::min(1.0f,ambient_col.y * 2);
            processed_ambient[2] = std::min(1.0f,ambient_col.z * 2);
            Vector4 processed_diffuse;
            processed_diffuse[0] = std::min(1.0f,diffuse_col.x * 4);
            processed_diffuse[1] = std::min(1.0f,diffuse_col.y * 4);
            processed_diffuse[2] = std::min(1.0f,diffuse_col.z * 4);
            ShaderProgram &current_prog(g_program_cache.m_current_program);
            current_prog.setVertexAttribPtr("binormal", 3, vbo->binormals);
            current_prog.setVertexAttribPtr("tangent", 3, vbo->tangents);
            current_prog.setUniform("lightAmbient", processed_ambient);
            current_prog.setUniform("lightDiffuse", processed_diffuse);
            current_prog.setUniform("glossFactor", gloss_amounts);
            ambient_col.w = 1.0;
        }
        else
        {
            segs_setupShading(DrawMode::HW_SKINNED, eBlendMode::COLORBLEND_DUAL);
            ambient_col.w = 0.0;
            g_program_cache.m_current_program.setUniform("lightAmbient", ambient_col);
            g_program_cache.m_current_program.setUniform("lightDiffuse", diffuse_col);
        }
        ShaderProgram &current_prog(g_program_cache.m_current_program);
        current_prog.setUniform("lightPosition", light_pos);
        if (node->flg & 0x200000)
        {
            segs_setShaderConstant(0, &shdr_Constant1);
            segs_setShaderConstant(1, &shdr_Constant2);
        }

        Vector3 *bone_trans = &node->seqGfxData->boneTranslations[node->bone_slot&0xFF];
        int boneIdx = 0;
        int boneMatricesId = current_prog.getUniformIdx("boneMatrices");
        for(int bone = 0; bone<boneinfo->numbones; ++bone)
        {
            int mat_idx = boneinfo->bone_ID[bone];
            Matrix4x3 *bone_mat = &node->seqGfxData->animMatrices[mat_idx];
            Vector3 transformed_pos = *bone_mat * *bone_trans;
            for(int idx = 0; idx<3; ++idx)
            {
                Vector4 val{ bone_mat->r1[idx],bone_mat->r2[idx],bone_mat->r3[idx],transformed_pos[idx] };
                glUniform4fv(boneMatricesId + idx + boneIdx, 1, val.data());
                //glProgramLocalParameter4fvARB(GL_VERTEX_PROGRAM_ARB, boneIdx + idx + boneMatricesId, val.data());
            }
            boneIdx += 3;
        }
        assert( vbo->weights );
        current_prog.setVertexAttribPtr("boneWeights", 2, vbo->weights);
        int indices_loc = current_prog.attribLocation("boneIndices");
        assert(indices_loc != -1);
        glEnableVertexAttribArrayARB(indices_loc);
        glVertexAttribPointerARB(indices_loc, 2, GL_SHORT, 0, 0, vbo->boneIndices);
        glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, &vbo->triangles->i0 + tri_offset);
        tri_offset += 3*model->texture_bind_offsets[i].tri_count;
    }
    ++struct_9E7300.model_drawn;
    struct_9E7300.tri_count += model->model_tri_count;
    struct_9E7300.vert_count += model->vertex_count;
    if ( model->vbo->frame_id != struct_9E7300.frame_id )
    {
        struct_9E7300.vert_unique_count += model->vertex_count;
        model->vbo->frame_id = struct_9E7300.frame_id;
    }
    glActiveTextureARB(GL_TEXTURE0);
    fn_4E79C0(GL_TEXTURE_2D, 2, g_whiteTexture);
    segs_gfxNodeTricksUndo(tricks, model);
}
int segs_modelConvertRgbBuffer(GLuint *result, int count, int own_buffer)
{
    GLuint buffer; 
    void * in_ptr = (void *)*result;
    assert( UsingVBOs );
    glGenBuffersARB(1, &buffer);
    glBindBufferARB(GL_ARRAY_BUFFER, buffer);
    glBufferDataARB(GL_ARRAY_BUFFER, 4 * count, in_ptr, GL_STATIC_DRAW);
    if ( own_buffer )
        FreeMem(in_ptr, 1);
    *result = buffer;
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBufferARB(GL_ARRAY_BUFFER, 0);
    currently_bound_gl_buffer = ~0U;
    return 1;
}
void modelDrawWireframe(RGBA *colors, Model *model)
{
    assert(model && model->vbo);
    VBO *vbo = model->vbo;
    if ( !vbo->normals_vbo_idx )
        return;
    wcwMgmt_EnableFog(0);
    segs_modelDrawState(DrawMode::COLORONLY, 1);
    segs_modelBlendState(eBlendMode::MULTIPLY, 0);
    glLineWidth(1.0);
    glColor4ub(colors->r, colors->g, colors->b, colors->a);
    modelSetupVertexObject(model, 1);
    if ( segs_modelBindBuffer(model) )
        glVertexPointer(3, GL_FLOAT, 0, vbo->vertices);
    for(int idx = 0; idx<model->model_tri_count; ++idx)
    {
        glDrawElements(GL_LINE_LOOP, 3, GL_UNSIGNED_INT, &vbo->triangles[idx]);
    }
    wcwMgmt_EnableFog(1);
}
GlobRenderRel *assignDummySeqGfxData(Matrix4x3 *mat)
{
    static GlobRenderRel s_dummy;
    memset(&s_dummy, 0, sizeof(s_dummy));
    s_dummy.animMatrices[0] = *mat;
    s_dummy.light.ambientScale = 1.0;
    s_dummy.light.diffuseScale = 1.0;
    return &s_dummy;
}
static GfxTree_Node *modelInitADummyNode(Model *model, Matrix4x3 *mat)
{
    static GfxTree_Node s_node;

    s_node.seqGfxData = assignDummySeqGfxData(mat);
    s_node.model = model;
    model->boneinfo = struct_9EBDA0;
    s_node.trick_node = model->trck_node;
    s_node.flg = (s_node.flg&(~0xFF)) | 0xFF;
    return &s_node;
}
void bumpRenderObj(Model *model, Matrix4x3 *mat, GLuint colorbuf, Vector4 *ambient, Vector4 *diffuse)
{
    Vector3 lightPos;
    Matrix4x3 viewer;
    Matrix4x3 globToViewer;

    VBO *vbo = model->vbo;

    assert(vbo && ( model->loadstate & LOADED ));
    if ( g_curr_blend_state == eBlendMode::BUMPMAP_COLORBLEND_DUAL )
    {
        GfxTree_Node *node = modelInitADummyNode(model, mat);
        model->Model_flg1 |= 0x4000u;
        segs_modelDrawBonedNode(node);
        return;
    }
    if ( !g_using_bump_maps )
        return;

    assert(model && model->vbo);
    modelSetupVertexObject(model, 1);
    segs_modelBindBuffer(model);
    transposeMat4Copy(mat, &viewer);
    MatMult4x3(&cam_info.inv_viewmat, mat, &globToViewer);
    mulVecMat4Transpose(&g_sun.position, &globToViewer, &lightPos);
    segs_modelDrawState((colorbuf == 0) ? DrawMode::BUMPMAP_NORMALS:DrawMode::BUMPMAP_RGBS, 0);
    segs_modelBlendState(eBlendMode(model->blendState), 0);
    segs_UVPointer(GL_TEXTURE0, 2, GL_FLOAT, 0, vbo->tex_uv1);
    segs_UVPointer(GL_TEXTURE1, 2, GL_FLOAT, 0, vbo->tex_uv2);
    glNormalPointer(GL_FLOAT, 0, vbo->normals_vbo_idx);
    glVertexPointer(3, GL_FLOAT, 0, vbo->vertices);
    assert(vbo->binormals && vbo->tangents);
    ShaderProgram &shdr_prog(g_program_cache.m_current_program);
    shdr_prog.setUniform("lightDiffuse",{diffuse->x, diffuse->y, diffuse->z, 1.0});
    shdr_prog.setUniform("lightAmbient",{ambient->x, ambient->y, ambient->z, 1.0});
    shdr_prog.setUniform("lightPosition",{lightPos.x, lightPos.y, lightPos.z, 1.0});
    shdr_prog.setUniform("viewerPosition",viewer.TranslationPart);
    shdr_prog.setUniform("textureScroll0",struct_9A0CC0[0]);
    shdr_prog.setUniform("textureScroll1",struct_9A0CC0[1]);
    shdr_prog.setVertexAttribPtr("binormal", 3, vbo->binormals);
    shdr_prog.setVertexAttribPtr("tangent", 3, vbo->binormals);
    if ( colorbuf )
        shdr_prog.setVertexAttribBuffer("rgbs", colorbuf, 4, GL_UNSIGNED_BYTE, true, 0);
    drawLoop(model);
}
void modelDrawTexRgb(DrawMode mode, Model *model, GLuint color_buf)
{
    VBO *vbo = model->vbo;
    segs_modelDrawState(mode, 0);
    glDisable(GL_LIGHTING);
    assert ( model && model->vbo );
    modelSetupVertexObject(model, 1);
    if ( segs_modelBindBuffer(model) )
    {
        if ( mode == DrawMode::DUALTEX )
            segs_UVPointer(GL_TEXTURE1, 2, GL_FLOAT, 0, vbo->tex_uv2);
        segs_UVPointer(GL_TEXTURE0, 2, GL_FLOAT, 0, vbo->tex_uv1);
        glVertexPointer(3, GL_FLOAT, 0, vbo->vertices);
        glNormalPointer(GL_FLOAT, 0, vbo->normals_vbo_idx);
    }
    assert(UsingVBOs);
    glBindBuffer(GL_ARRAY_BUFFER, color_buf);
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, vbo->gl_vertex_buffer);
    drawLoop(model);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void modelDrawTexNormals(DrawMode mode, Model *model)
{
    assert(model && model->vbo);

    VBO *vbo = model->vbo;
    segs_modelDrawState(mode, 0);
    bool override_light = model->Model_flg1 & 0x10;
    if (override_light)
    {
        Vector4 zero_diffuse {0,0,0,0};
        segs_wcw_statemgmt_SetLightParam(GL_LIGHT0, GL_AMBIENT, &g_sun.no_angle_light);
        segs_wcw_statemgmt_SetLightParam(GL_LIGHT0, GL_DIFFUSE, &zero_diffuse);
    }
    modelSetupVertexObject(model, 1);
    if ( segs_modelBindBuffer(model) )
    {
        if ( mode == DrawMode::DUALTEX_NORMALS && vbo->tex_uv2 )
            segs_UVPointer(GL_TEXTURE1, 2, GL_FLOAT, 0, vbo->tex_uv2);
        segs_UVPointer(GL_TEXTURE0, 2, GL_FLOAT, 0, vbo->tex_uv1);
        glVertexPointer(3, GL_FLOAT, 0, vbo->vertices);
        glNormalPointer(GL_FLOAT, 0, vbo->normals_vbo_idx);
    }
    drawLoop(model);
    if (override_light)
    {
        segs_wcw_statemgmt_SetLightParam(GL_LIGHT0, GL_AMBIENT, &g_sun.ambient);
        segs_wcw_statemgmt_SetLightParam(GL_LIGHT0, GL_DIFFUSE, &g_sun.diffuse);
    }
}

static void setModelViewFromMat4x3(const Matrix4x3& mat) 
{
    glMatrixMode(GL_MODELVIEW);
    Matrix4x4 mat4_2;
    mat4_2 = mat;
    glLoadMatrixf(&mat4_2.r1.x);
}

void segs_modelDraw(Model *model, Matrix4x3 *mat, TrickNode *draw_settings, int alpha, uint8_t *rgb_entries, EntLight *light_params)
{
    Vector4 diffuse_light;
    Vector4 ambient_light;
    static char *s_rgb_buf=nullptr;
    static RGBA white{ 255,255,255,255 };

    assert ( model->vbo );
    assert ( model->loadstate & LOADED );
    if ( alpha==0 )
        return;
    setModelViewFromMat4x3(*mat);

    modelDrawWireframe(&white, model);
    return;
    TrickNode *trick = draw_settings;
    if ( !draw_settings )
    {
        draw_settings = model->trck_node;
        trick = model->trck_node;
    }
    eBlendMode blend_mode = eBlendMode(model->blendState);
    if ( g_curr_blend_state != blend_mode )
        segs_modelBlendState(blend_mode, 0);
    shdr_Constant1.x = 1.0;
    shdr_Constant1.y = 1.0;
    shdr_Constant1.z = 1.0;
    segs_modelSetAlpha(alpha);
    segs_setTexUnitLoadBias(GL_TEXTURE1, -0.5);
    segs_setTexUnitLoadBias(GL_TEXTURE0, -0.5);
    if ( !segs_gfxNodeTricks(trick, model, mat) ) 
    {
        segs_gfxNodeTricksUndo(trick, model);
        return;
    }
    if ( light_params && light_params->use & ENTLIGHT_INDOOR_LIGHT )
    {
        ambient_light.ref3() = light_params->ambient_light.ref3() * 0.5f;
        ambient_light.w = light_params->ambient_light.w;
        diffuse_light.ref3() = light_params->diffuse.ref3() * 0.5f;
        diffuse_light.w = light_params->diffuse.w;
        setModelViewFromMat4x3(cam_info.viewmat);
        segs_wcw_statemgmt_SetLightParam(GL_LIGHT0, GL_POSITION, &light_params->direction);
    }
    else
    {
        ambient_light = g_sun.ambient;
        diffuse_light = g_sun.diffuse;
    }
    if ( light_params && light_params->use & (ENTLIGHT_CUSTOM_DIFFUSE|ENTLIGHT_CUSTOM_AMBIENT) )
    {
        ambient_light.ref3() = ambient_light.ref3() * light_params->ambientScale;
        diffuse_light.ref3() = diffuse_light.ref3() * light_params->diffuseScale;
    }
    segs_wcw_statemgmt_SetLightParam(GL_LIGHT0, GL_AMBIENT, &ambient_light);
    segs_wcw_statemgmt_SetLightParam(GL_LIGHT0, GL_DIFFUSE, &diffuse_light);
    setModelViewFromMat4x3(*mat);
    GLuint colorbuf = (GLuint)rgb_entries;
    if ( model->Model_flg1 & 4 )
    {
        if ( !s_rgb_buf )
        {
            assert(UsingVBOs);
            std::array<RGBA, 5000> buf;
            buf.fill(RGBA(63, 63, 63, 63));
            s_rgb_buf = (char *)buf.data();
            segs_modelConvertRgbBuffer((GLuint *)&s_rgb_buf, buf.size(), false);
        }
        colorbuf = (GLuint)s_rgb_buf;
    }
    if ( g_State.view.bWireframe != 2 )
    {
        if ( trick && trick->_TrickFlags & 0x1000 )
        {
            modelDrawWireframe(&trick->TintColor0, model);
        }
        else if(!( model->Model_flg1 & 0x10000 ))
        {
            if ( model->Model_flg1 & 0x800 )
                bumpRenderObj(model, mat, colorbuf, &ambient_light, &diffuse_light);
            else if ( colorbuf )
                modelDrawTexRgb((model->Model_flg1 & 0x40) ? DrawMode::DUALTEX : DrawMode::SINGLETEX, model, colorbuf);
            else
                modelDrawTexNormals((model->Model_flg1 & 0x40) ? DrawMode::DUALTEX_NORMALS : DrawMode::SINGLETEX_NORMALS, model);
        }
    }
    if ( g_State.view.bWireframe )
    {
        if ( !trick || !(trick->_TrickFlags & 0x1000) )
        {
            modelDrawWireframe(&white, model);
            trick = draw_settings;
        }
    }
    if ( light_params )
    {
        setModelViewFromMat4x3(cam_info.viewmat);
        segs_wcw_statemgmt_SetLightParam(GL_LIGHT0, GL_POSITION, &g_sun.direction);
    }
    struct_9E7300.tri_count += model->model_tri_count;
    ++struct_9E7300.model_drawn;
    struct_9E7300.vert_count += model->vertex_count;
    if ( model->vbo->frame_id != struct_9E7300.frame_id )
    {
        struct_9E7300.vert_unique_count += model->vertex_count;
        model->vbo->frame_id = struct_9E7300.frame_id;
    }
    segs_gfxNodeTricksUndo(trick, model);
}
void patch_render_model()
{
    PATCH_FUNC(wcw_UnBindBufferARB);
    PATCH_FUNC(modelBindBuffer);
    PATCH_FUNC(UVPointer);
    PATCH_FUNC(modelConvertRgbBuffer);

    PATCH_FUNC(modelDraw);
}
