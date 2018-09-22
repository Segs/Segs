#include "RenderBonedModel.h"

#include "RendererState.h"
#include "RenderTree.h"
#include "RenderModel.h"
#include "ShaderProgramCache.h"
#include "Texture.h"
#include "Model.h"
#include "GameState.h"
#include "RenderTricks.h"
#include "ModelCache.h"

#include "utils/helpers.h"
#include "utils/dll_patcher.h"
#include "GL/glew.h"

#include <stdint.h>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <algorithm>
#include <array>
#include "RendererUtils.h"
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct GlobRenderRel;
static const char *feedback_model_name = "NO_MODEL_TO_FEEDBACK";
extern "C" {
__declspec(dllimport) bool legitBone(int);
__declspec(dllimport) void MatMult4x3(const Matrix4x3 *self, const Matrix4x3 *oth, Matrix4x3 *res);
__declspec(dllimport) void transposeMat4Copy(const Matrix4x3 *src, Matrix4x3 *dst);
__declspec(dllimport) int boneNameToIdx(const char *);

__declspec(dllimport) CameraInfo cam_info;
__declspec(dllimport) TextureBind *g_whiteTexture;
__declspec(dllimport) RdrStats struct_9E7300;
}
static BoneInfo s_fakeBoneInfos[70]; //fake bone info array


void segs_wcw_UnBindBufferARB()
{
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
static int segs_modelBindBuffer(Model *model)
{
    GeometryData *vbo = model->vbo;
    assert(UsingVBOs);
    //glBindVertexArray(vbo->segs_data->vao_id);
    return 1;
}
void calcNodeLights(GfxTree_Node *node,Vector4 &ambient_col,Vector4 &diffuse_col,Vector3 &light_pos)
{

    GlobRenderRel * seqGfxData = node->seqGfxData;
    if ( (g_State.view.game_mode == 1 || g_State.view.game_mode == 3) && seqGfxData->light.use & ENTLIGHT_INDOOR_LIGHT )
    {
        ambient_col = seqGfxData->light.ambient_light;
        diffuse_col = seqGfxData->light.diffuse;
        light_pos = cam_info.viewmat * seqGfxData->light.direction.ref3();
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
        light_pos = g_sun.direction_in_viewspace.ref3();
    }
    if ( seqGfxData->light.use & (ENTLIGHT_CUSTOM_DIFFUSE|ENTLIGHT_CUSTOM_AMBIENT) )
    {
        ambient_col.ref3() *= seqGfxData->light.ambientScale;
        diffuse_col.ref3() *= seqGfxData->light.diffuseScale;
    }
}
static void setTexOrOverride(TextureBind *tex,int idx)
{
    if (tex->flags & 0x400 && g_global_texbinds[idx])
        segs_texBindTexture(tex->texture_target, idx, g_global_texbinds[idx]);
    else
        segs_texBindTexture(tex->texture_target, idx, tex);

}
void  drawLoop(Model *model,const ShaderProgram &prog)
{

    GeometryData *vbo = model->vbo;
    int offset = 0;
    if ( !vbo->uv1_offset )
        return;
    for(int idx=0; idx<model->num_textures; ++idx)
    {
        GLsizei entry_count = 3 * model->texture_bind_offsets[idx].tri_count;
        TextureBind *bind = vbo->textureP_arr[idx];
        TextureBind *link1 = !bind->tex_links[1] ? g_whiteTexture : bind->tex_links[1];
        TextureBind *texture = bind->tex_links[0];

        assert( link1 );
        setTexOrOverride(link1, 2);
        setTexOrOverride(texture, 1);
        setTexOrOverride(bind, 0);
        vbo->draw(prog, GL_TRIANGLES, entry_count, offset);
        offset += entry_count;
    }
}
void drawToFeedbackBuffer(Model *m, const MaterialDefinition &baseMat, GLuint color_buf)
{
    static std::vector<Vector4> results;
    GLDebugGuard guard(__FUNCTION__);
    GeometryData *vbo = m->vbo;
    uint32_t buff_id;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, (int *)&buff_id);

    size_t feedback_size = m->vertex_count * 4 * 4;
    MaterialDefinition feedbackMat(baseMat);
    feedbackMat.set_useTransformFeedback(true);
    feedbackMat.apply();
    vbo->setColorBuffer({ color_buf,0 });


    GLuint tbo;
    glBindVertexArray(0);
    glGenBuffers(1, &tbo);
    glBindBuffer(GL_ARRAY_BUFFER, tbo);
    glBufferData(GL_ARRAY_BUFFER, feedback_size, nullptr, GL_STATIC_READ);
    glEnable(GL_RASTERIZER_DISCARD);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tbo);
    glBeginTransformFeedback(GL_POINTS);
    glBindBuffer(GL_ARRAY_BUFFER, buff_id);
    vbo->drawArray(*feedbackMat.program, GL_POINTS, m->vertex_count, 0);
    glEndTransformFeedback();
    glFlush();
    results.resize(m->vertex_count);
    glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, feedback_size, results.data());
    glDeleteBuffers(1, &tbo);
    glDisable(GL_RASTERIZER_DISCARD);
}
static void uploadBoneMatrices(GfxTree_Node *node,ShaderProgram &toProg)
{
    Model *model = node->model;
    BoneInfo *boneinfo = model->boneinfo;

    Vector3 *bone_trans = &node->seqGfxData->boneTranslations[node->bone_slot & 0xFF];
    int boneIdx = 0;
    int boneMatricesId = toProg.getUniformIdx("boneMatrices");
    for (int bone = 0; bone<boneinfo->numbones; ++bone)
    {
        int mat_idx = boneinfo->bone_ID[bone];
        Matrix4x3 *bone_mat = &node->seqGfxData->animMatrices[mat_idx];
        Vector3 transformed_pos = *bone_mat * *bone_trans;
        for (int idx = 0; idx<3; ++idx)
        {
            Vector4 val{ bone_mat->r1[idx],bone_mat->r2[idx],bone_mat->r3[idx],transformed_pos[idx] };
            glUniform4fv(boneMatricesId + idx + boneIdx, 1, val.data());
        }
        boneIdx += 3;
    }

}
static void segs_modelDrawBonedNodeSingleTex(GfxTree_Node *node, const MaterialDefinition &init_mat)
{
    static int callcount = 0;
    Vector4 ambient_col;
    Vector4 diffuse_col;
    Vector4 light_pos;
    Vector4 gloss_amounts;
    MaterialDefinition material_def(init_mat);
    Model *model = node->model;
    callcount++;
    GLDebugGuard guard(__FUNCTION__);
    assert(node->flg & 0x40000);
    assert(node->customtex[0] && node->customtex[1]);
    assert(model && model->vbo);
    //drawDebug(node);
    segs_modelSetupVertexObject(model, 1);
    segs_modelBindBuffer(model);
    GeometryData *vbo = model->vbo;
    GlobRenderRel *seqGfxData = node->seqGfxData;
    TrickNode *tricks = node->trick_node;
    BoneInfo *boneinfo = model->boneinfo;
    assert(vbo && legitBone(node->anim_id()) && seqGfxData && boneinfo);
    assert(model->loadstate & ModelState::LOADED);
    if (!vbo->uv1_offset)
        return;

    segs_setTexUnitLoadBias(GL_TEXTURE0, -2.0);
    segs_setTexUnitLoadBias(GL_TEXTURE1, -2.0);
    calcNodeLights(node, ambient_col, diffuse_col, light_pos.ref3());
    //TODO: set light to direction mode
    ambient_col.w = 1.0;
    diffuse_col.w = 1.0;
    light_pos.w = 0.5;
    TextureBind *tex = node->customtex[0];
    TextureBind *tex2 = node->customtex[1];
    material_def.draw_data.constColor1 = { 1,1,1,node->alpha() / 255.0f };
    segs_gfxNodeTricks(tricks, model, nullptr, material_def);
    if (node->flg & 0x200000)
    {
        material_def.draw_data.constColor1.ref3() = node->color1.to3Floats();
        material_def.draw_data.constColor2.ref3() = node->color2.to3Floats();
    }
    if (g_State.view.bWireframe && !UsingVBOs)
    {
        assert(false);
        // unhandled mode
    }
    MaterialDefinition bump_blend_and_bones(material_def);
    bump_blend_and_bones.setDrawMode(DrawMode::BUMPMAP_SKINNED);
    bump_blend_and_bones.setFragmentMode(eBlendMode::BUMPMAP_COLORBLEND_DUAL);
    MaterialDefinition blend_and_bones(material_def);
    blend_and_bones.setDrawMode(DrawMode::HW_SKINNED);
    blend_and_bones.setFragmentMode(eBlendMode::COLORBLEND_DUAL);
    bool use_bumpmapped = (tex && tex->tex_links[1] != nullptr && (tex->tex_links[1]->flags & 0x800));
    MaterialDefinition &selected_material(use_bumpmapped ? bump_blend_and_bones : blend_and_bones);
    if(use_bumpmapped)
    {
        selected_material.draw_data.tex_id_2 = tex->tex_links[1];
        gloss_amounts.x = g_sun.gloss_scale * tex->tex_links[1]->Gloss;
        if (diffuse_col.y < 0.15f)
            gloss_amounts.x *= 0.35f;
        Vector4 processed_ambient{ 0,0,0,1 };
        processed_ambient[0] = std::min(1.0f, ambient_col.x * 2);
        processed_ambient[1] = std::min(1.0f, ambient_col.y * 2);
        processed_ambient[2] = std::min(1.0f, ambient_col.z * 2);
        Vector4 processed_diffuse {
            std::min(1.0f, diffuse_col.x * 4),
            std::min(1.0f, diffuse_col.y * 4),
            std::min(1.0f, diffuse_col.z * 4),
            1 };
        selected_material.draw_data.light0.Ambient = processed_ambient;
        selected_material.draw_data.light0.Diffuse = processed_diffuse;
        selected_material.draw_data.glossFactor.x = gloss_amounts.x;
        ambient_col.w = 1.0;
        selected_material.draw_data.tex_id_0 = tex;
        selected_material.draw_data.tex_id_1 = tex2;
        selected_material.draw_data.colorConstsEnabled = (node->flg & 0x200000);
        //TODO: this will set the light0 Position twice in material's apply()
        selected_material.draw_data.light0.Position = light_pos;
        segs_texBindTexture(GL_TEXTURE_2D, 2, tex->tex_links[1]);
    }
    else
    {
        ambient_col.w = 0.0;
        selected_material.draw_data.light0.Ambient = ambient_col;
        selected_material.draw_data.light0.Diffuse = diffuse_col;
        selected_material.draw_data.tex_id_0 = tex;
        selected_material.draw_data.tex_id_1 = tex2;
        selected_material.draw_data.colorConstsEnabled = (node->flg & 0x200000);
        //TODO: this will set the light0 Position twice in material's apply()
        selected_material.draw_data.light0.Position = light_pos;
    }
    selected_material.apply();
    segs_texBindTexture(GL_TEXTURE_2D, 0, tex);
    segs_texBindTexture(GL_TEXTURE_2D, 1, tex2);
    uploadBoneMatrices(node, *selected_material.program);
    assert(vbo->gl_index_buffer!=0);
    vbo->draw(*selected_material.program, GL_TRIANGLES, 3 * model->model_tri_count, 0);
    ++struct_9E7300.model_drawn;
    struct_9E7300.tri_count += model->model_tri_count;
    struct_9E7300.vert_count += model->vertex_count;
    if (model->vbo->frame_id != struct_9E7300.frame_id)
    {
        struct_9E7300.vert_unique_count += model->vertex_count;
        model->vbo->frame_id = struct_9E7300.frame_id;
    }
    glActiveTextureARB(GL_TEXTURE0);
    segs_texBindTexture(GL_TEXTURE_2D, 2, g_whiteTexture);
    segs_gfxNodeTricksUndo(tricks, model,selected_material);
}
static void segs_modelDrawBonedNodeMultiTex(GfxTree_Node *node, const MaterialDefinition &init_mat)
{
    Vector4 ambient_col;
    Vector4 diffuse_col;
    TextureBind *tex;
    Vector4 light_pos;
    Vector4 gloss_amounts;
    MaterialDefinition material_def(init_mat);

    GLDebugGuard guard(__FUNCTION__);
    //drawDebug(node);
    Model *model = node->model;
    int tri_offset = 0;
    assert(model && model->vbo);
    segs_modelSetupVertexObject(model, 1);
    segs_modelBindBuffer(model);
    GeometryData *vbo = model->vbo;
    GlobRenderRel *seqGfxData = node->seqGfxData;
    TrickNode *tricks = node->trick_node;
    BoneInfo *boneinfo = model->boneinfo;
    assert(vbo && legitBone(node->anim_id()) && seqGfxData && boneinfo);
    assert(model->loadstate & ModelState::LOADED);
    if ( !vbo->uv1_offset )
        return;

    bool all_same_material = true;
    bool prev_flag;
    for (int i = 0; i < model->num_textures; ++i)
    {
        tex = vbo->textureP_arr[i];
        bool use_bumpmapped = (tex && tex->tex_links[1] != nullptr && (tex->tex_links[1]->flags & 0x800));
        if (i == 0)
            prev_flag = use_bumpmapped;
        if (prev_flag != use_bumpmapped)
        {
            all_same_material = false;
            break;
        }
    }
    if(all_same_material)
    {
        printfDebug("Bad node flag: model with single texture type is marked as having multiple types.\n");
    }
    segs_setTexUnitLoadBias(GL_TEXTURE0, -2.0);
    segs_setTexUnitLoadBias(GL_TEXTURE1, -2.0);
    calcNodeLights(node,ambient_col,diffuse_col,light_pos.ref3());
    //TODO: set light to direction mode
    ambient_col.w = 1.0;
    diffuse_col.w = 1.0;
    light_pos.w = 0.5;
    int tex_cnt = model->num_textures;
    material_def.draw_data.constColor1 = {1,1,1,node->alpha()/255.0f};
    segs_gfxNodeTricks(tricks, model, nullptr,material_def);
    if ( node->flg & 0x200000 )
    {
        material_def.draw_data.constColor1.ref3() = node->color1.to3Floats();
        material_def.draw_data.constColor2.ref3() = node->color2.to3Floats();
    }
    if ( g_State.view.bWireframe && !UsingVBOs )
    {
        assert(false);
        // unhandled mode
    }
    MaterialDefinition bump_blend_and_bones(material_def);
    bump_blend_and_bones.setDrawMode(DrawMode::BUMPMAP_SKINNED);
    bump_blend_and_bones.setFragmentMode(eBlendMode::BUMPMAP_COLORBLEND_DUAL);
    {
        Vector4 processed_ambient{ 0,0,0,1 };
        processed_ambient[0] = std::min(1.0f, ambient_col.x * 2);
        processed_ambient[1] = std::min(1.0f, ambient_col.y * 2);
        processed_ambient[2] = std::min(1.0f, ambient_col.z * 2);
        Vector4 processed_diffuse{ 0,0,0,1 };
        processed_diffuse[0] = std::min(1.0f, diffuse_col.x * 4);
        processed_diffuse[1] = std::min(1.0f, diffuse_col.y * 4);
        processed_diffuse[2] = std::min(1.0f, diffuse_col.z * 4);
        bump_blend_and_bones.draw_data.light0.Ambient = processed_ambient;
        bump_blend_and_bones.draw_data.light0.Diffuse = processed_diffuse;
        bump_blend_and_bones.draw_data.colorConstsEnabled = (node->flg & 0x200000);
        //TODO: this will set the light0 Position twice in material's apply()
        bump_blend_and_bones.draw_data.light0.Position = light_pos;
        bump_blend_and_bones.apply();
        uploadBoneMatrices(node, *bump_blend_and_bones.program);

    }
    MaterialDefinition blend_and_bones(material_def);
    blend_and_bones.setDrawMode(DrawMode::HW_SKINNED);
    blend_and_bones.setFragmentMode(eBlendMode::COLORBLEND_DUAL);
    {
        ambient_col.w = 0.0;
        blend_and_bones.draw_data.light0.Ambient = ambient_col;
        blend_and_bones.draw_data.light0.Diffuse = diffuse_col;
        blend_and_bones.draw_data.colorConstsEnabled = (node->flg & 0x200000);
        //TODO: this will set the light0 Position twice in material's apply()
        blend_and_bones.draw_data.light0.Position = light_pos;
        blend_and_bones.apply();
        uploadBoneMatrices(node, *blend_and_bones.program);

    }
    // pre setup for common draw params for both materials.

    for ( int i = 0; i < tex_cnt; ++i )
    {
        GLsizei count = 3 * model->texture_bind_offsets[i].tri_count;
        tex = vbo->textureP_arr[i];
        TextureBind *tex2 = tex->tex_links[0];
        segs_texBindTexture(GL_TEXTURE_2D, 0, tex);
        segs_texBindTexture(GL_TEXTURE_2D, 1, tex2);
        assert(enableVertShaders);
        bool use_bumpmapped = (tex && tex->tex_links[1] != nullptr && (tex->tex_links[1]->flags & 0x800));
        MaterialDefinition &selected_material(use_bumpmapped ? bump_blend_and_bones : blend_and_bones);
        if (use_bumpmapped)
        {
            segs_texBindTexture(GL_TEXTURE_2D, 2, tex->tex_links[1]);
            selected_material.draw_data.tex_id_2 = tex->tex_links[1];
            gloss_amounts.x = g_sun.gloss_scale * tex->tex_links[1]->Gloss;
            if ( diffuse_col.y < 0.15f )
                gloss_amounts.x *= 0.35f;
            selected_material.draw_data.glossFactor.x = gloss_amounts.x;

        }
        selected_material.draw_data.tex_id_0 = tex;
        selected_material.draw_data.tex_id_1 = tex2;

        selected_material.apply();

        assert( vbo->weights );
        vbo->draw(*selected_material.program, GL_TRIANGLES, count, tri_offset);
        //glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, (int *)vbo->triangles + tri_offset);
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
    segs_texBindTexture(GL_TEXTURE_2D, 2, g_whiteTexture);
    segs_gfxNodeTricksUndo(tricks, model,material_def);
}
void segs_modelDrawBonedNode(GfxTree_Node *node, const MaterialDefinition &init_mat)
{
    const std::string part_name = std::string(__FUNCTION__) + "_" + node->model->parent_anim->headers->name;
    GLDebugGuard      debug_guard(part_name.c_str());
    if (node->flg & 0x40000)
    {
        segs_modelDrawBonedNodeSingleTex(node, init_mat);
    }
    else
    {
        segs_modelDrawBonedNodeMultiTex(node, init_mat);
    }
}

GLuint segs_modelConvertRgbBuffer(void *data, int count, int own_buffer)
{
    GLDebugGuard guard(__FUNCTION__);
    GLuint buffer;
    void * in_ptr = data;
    assert( UsingVBOs );
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, 4 * count, in_ptr, GL_STATIC_DRAW);
    if ( own_buffer )
        COH_FREE(in_ptr);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glObjectLabel(GL_BUFFER, buffer, -1, "RGB_VBO");
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return buffer;
}
void modelDrawWireframe(RGBA *colors, Model *model)
{
    assert(model && model->vbo);
    GeometryData *vbo = model->vbo;
    if ( !vbo->normals_offset )
        return;
    MaterialDefinition wireframe_mat(g_default_mat);
    wireframe_mat.setDrawMode(DrawMode::COLORONLY);
    wireframe_mat.setFragmentMode(eBlendMode::MULTIPLY);
    wireframe_mat.draw_data.fog_params.enabled = false;
    glLineWidth(1.0);
    wireframe_mat.draw_data.globalColor = { colors->r / 255.0f, colors->g / 255.0f, colors->b / 255.0f, colors->a/255.0f };
    segs_modelSetupVertexObject(model, 1);
    segs_modelBindBuffer(model);
    wireframe_mat.apply();
    for(int idx = 0; idx<model->model_tri_count; ++idx)
    {
        model->vbo->draw(*wireframe_mat.program, GL_LINE_LOOP, 3, 3 * idx);
    }
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
    model->boneinfo = s_fakeBoneInfos;
    s_node.trick_node = model->trck_node;
    s_node.setAlpha(255);
    return &s_node;
}
void bumpRenderObj(Model *model, Matrix4x3 *mat, GLuint colorbuf, Vector4 *ambient, Vector4 *diffuse,const MaterialDefinition &base_mat)
{
    Vector3 lightPos;
    Matrix4x3 viewer;
    Matrix4x3 worldToViewer;

    GeometryData *vbo = model->vbo;

    assert(vbo && ( model->loadstate & LOADED ));
    assert(model && model->vbo);
    const std::string part_name = std::string(__FUNCTION__) + "_" + model->parent_anim->headers->name;
    GLDebugGuard      debug_guard(part_name.c_str());

    segs_modelSetupVertexObject(model, 1);
    segs_modelBindBuffer(model);
    transposeMat4Copy(mat, &viewer);
    worldToViewer = cam_info.inv_viewmat * *mat;
    bool prelit = colorbuf != 0;
    MaterialDefinition bump_material(base_mat);
    bump_material.setDrawMode(prelit ? DrawMode::BUMPMAP_RGBS : DrawMode::BUMPMAP_NORMALS);
    bump_material.setFragmentMode(eBlendMode(model->blendState));
    mulVecMat4Transpose(&g_sun.position, &worldToViewer, &lightPos);
    //TODO: this
    bump_material.draw_data.light0.Diffuse = { diffuse->x, diffuse->y, diffuse->z, 1.0 };
    bump_material.draw_data.light0.Ambient = { ambient->x, ambient->y, ambient->z, 1.0 };
    bump_material.draw_data.light0.Position= { lightPos.x, lightPos.y, lightPos.z, 1.0 };
    bump_material.draw_data.viewerPosition = viewer.TranslationPart;
    // pack texture scrolling info into 1 uniform
    bump_material.draw_data.textureScroll = g_tex_scrolls;
    bump_material.apply();
    if(nullptr!=strstr(model->bone_name_offset, feedback_model_name))
        drawToFeedbackBuffer(model, bump_material, colorbuf);
    vbo->setColorBuffer({ colorbuf ? colorbuf : ~0U,0 });
    drawLoop(model,*bump_material.program);
}
static void modelDrawTexRgb(Model *model, GLuint color_buf,MaterialDefinition &mat)
{
    assert ( model && model->vbo );
    const std::string part_name = std::string(__FUNCTION__) + "_" + model->parent_anim->headers->name;
    GLDebugGuard      debug_guard(part_name.c_str());
    segs_modelSetupVertexObject(model, 1);
    segs_modelBindBuffer(model);

    mat.draw_data.light0.State = false;
    mat.updateUniforms();
    if (nullptr != strstr(model->bone_name_offset, feedback_model_name))
        drawToFeedbackBuffer(model, mat, 0);
    model->vbo->setColorBuffer({ color_buf,0 });
    assert(UsingVBOs);

    drawLoop(model,*mat.program);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void modelDrawTexNormals(Model *model, MaterialDefinition &material)
{

    assert(model && model->vbo);
    const std::string part_name = std::string(__FUNCTION__) + "_" + model->parent_anim->headers->name;
    GLDebugGuard      debug_guard(part_name.c_str());

    bool override_light = model->Model_flg1 & OBJ_NOLIGHTANGLE;
    if (override_light)
    {
        material.draw_data.light0.Ambient = g_sun.no_angle_light;
        material.draw_data.light0.Diffuse = {0,0,0,0};
        material.updateUniforms(); // light state changed ?
    }
    segs_modelSetupVertexObject(model, 1);
    segs_modelBindBuffer(model);
    if (nullptr != strstr(model->bone_name_offset, feedback_model_name))
        drawToFeedbackBuffer(model, material, 0);
    material.apply();

    drawLoop(model,*material.program);
    if (override_light)
    {
        material.updateUniforms(); // light state changed ?
    }

}
void segs_modelDraw(Model *model, Matrix4x3 *mat, TrickNode *draw_settings, int alpha, GLuint rgb_entries, EntLight *light_params,const MaterialDefinition &initial_mat)
{
    static GLuint s_rgb_buf=0;
    static RGBA white{ 255,255,255,255 };
    auto restore = g_render_state.getGlobal();
    Vector4 diffuse_light;
    Vector4 ambient_light;

    assert ( model->vbo );
    assert ( model->loadstate & LOADED );
    if ( alpha==0 )
        return;
//    if(strcmp(model->parent_anim->headers->name,"vents_etc")==0)
//    {
//        printf("doing vents\n");
//    }
    const std::string part_name = std::string(__FUNCTION__) +"_"+model->parent_anim->headers->name;
    GLDebugGuard debug_guard(part_name.c_str());
    bool is_dual_tex = (model->Model_flg1 & OBJ_DUALTEXTURE);

    TrickNode *trick = draw_settings;
    if (!draw_settings)
    {
        draw_settings = model->trck_node;
        trick = model->trck_node;
    }
    eBlendMode blend_mode = eBlendMode(model->blendState);
    MaterialDefinition material_definition(initial_mat);
    material_definition.draw_data.constColor1 = {1,1,1,alpha / 255.0f};
    material_definition.setDrawMode(is_dual_tex ? DrawMode::DUALTEX : DrawMode::SINGLETEX);
    material_definition.setFragmentMode(blend_mode);

    segs_setTexUnitLoadBias(GL_TEXTURE1, -0.5);
    segs_setTexUnitLoadBias(GL_TEXTURE0, -0.5);
    if (!segs_gfxNodeTricks(trick, model, mat,material_definition))
    {
        debug_guard.insertMessage("Discarded by day/night alpha");
        segs_gfxNodeTricksUndo(trick, model,material_definition);
        return;
    }
    if (light_params && light_params->use & ENTLIGHT_INDOOR_LIGHT)
    {
        ambient_light.ref3() = light_params->ambient_light.ref3() * 0.5f;
        ambient_light.w = light_params->ambient_light.w;
        diffuse_light.ref3() = light_params->diffuse.ref3() * 0.5f;
        diffuse_light.w = light_params->diffuse.w;
        material_definition.draw_data.light0.Position = cam_info.viewmat * light_params->direction.ref3();
    }
    else
    {
        ambient_light = g_sun.ambient;
        diffuse_light = g_sun.diffuse;
    }
    if (light_params && light_params->use & (ENTLIGHT_CUSTOM_DIFFUSE | ENTLIGHT_CUSTOM_AMBIENT))
    {
        ambient_light.ref3() = ambient_light.ref3() * light_params->ambientScale;
        diffuse_light.ref3() = diffuse_light.ref3() * light_params->diffuseScale;
    }
    material_definition.draw_data.light0.Ambient = ambient_light;
    material_definition.draw_data.light0.Diffuse = diffuse_light;
    material_definition.draw_data.modelViewMatrix = mat->toGLM();
    GLuint colorbuf = rgb_entries;
    if (model->Model_flg1 & OBJ_FULLBRIGHT)
    {
        if ( !s_rgb_buf )
        {
            assert(UsingVBOs);
            std::array<RGBA, 5000> buf;
            buf.fill(RGBA(63, 63, 63, 255));
            s_rgb_buf = segs_modelConvertRgbBuffer(buf.data(), buf.size(), false);
        }
        colorbuf = s_rgb_buf;
    }
    if ( g_State.view.bWireframe != 2 )
    {
        if (trick && trick->_TrickFlags & TF_Wireframe)
        {
            modelDrawWireframe(&trick->TintColor0, model);
        } 
        else if (!(model->Model_flg1 & OBJ_HIDE))
        {
            if (model->Model_flg1 & OBJ_BUMPMAP)
            {
                if (blend_mode != eBlendMode::BUMPMAP_COLORBLEND_DUAL)
                    bumpRenderObj(model, mat, colorbuf, &ambient_light, &diffuse_light,material_definition);
                else {
                    GfxTree_Node *node = modelInitADummyNode(model, mat);
                    model->Model_flg1 |= OBJ_DRAW_AS_ENT;
                    segs_modelDrawBonedNode(node, material_definition);
                }
            }
            else if (colorbuf)
            {
                debug_guard.insertMessage("model with colors");
                material_definition.apply();
                modelDrawTexRgb(model, colorbuf, material_definition);
            }
            else
            {
                debug_guard.insertMessage("model with normals");
                material_definition.setDrawMode(is_dual_tex ? DrawMode::DUALTEX_NORMALS : DrawMode::SINGLETEX_NORMALS);

                material_definition.apply();
                modelDrawTexNormals(model, material_definition);
            }
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
    struct_9E7300.tri_count += model->model_tri_count;
    ++struct_9E7300.model_drawn;
    struct_9E7300.vert_count += model->vertex_count;
    if ( model->vbo->frame_id != struct_9E7300.frame_id )
    {
        struct_9E7300.vert_unique_count += model->vertex_count;
        model->vbo->frame_id = struct_9E7300.frame_id;
    }
    segs_gfxNodeTricksUndo(trick, model,material_definition);

    g_render_state.apply(restore);
}
BoneInfo * assignDummyBoneInfo(char *boneset_name) //4D5610
{
    int bone_idx;
    char buf[100];

    strcpy(buf,boneset_name+4);
    bone_idx = boneNameToIdx(buf);
    if ( bone_idx < 0 )
        bone_idx = 0;
    BoneInfo &result(s_fakeBoneInfos[bone_idx]);
    result.bone_ID[0] = bone_idx;
    result.weights = nullptr;
    result.matidxs = nullptr;
    result.numbones = 1;
    return &result;
}
void patch_render_model()
{
    BREAK_FUNC(wcw_UnBindBufferARB); // the only usages left should be those in cloth rendering
    BREAK_FUNC(modelBindBuffer);
    BREAK_FUNC(UVPointer);
    BREAK_FUNC(modelConvertRgbBuffer);

    PATCH_FUNC(modelDraw);
}

