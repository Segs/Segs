#include "RenderTricks.h"

#include "RendererState.h"
#include "Texture.h"
#include "Model.h"
#include "ShaderProgramCache.h"
#include "GameState.h"
#include "utils/dll_patcher.h"
#include "utils/helpers.h"

#include "GL/glew.h"

#include <glm/mat4x4.hpp>
#include <glm/mat4x3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdint.h>
#include <algorithm>
#include <cmath>
#include <cassert>
#include <vector>

extern "C" {
__declspec(dllimport) void  fn_4DEBB0(Matrix4x3 *, float );
__declspec(dllimport) void  matMul3x3(const Matrix3x3 *rhs, const Matrix3x3 *lhs, Matrix3x3 *dst);
__declspec(dllimport) void  fn_5B6740(float yaw_angle, Matrix3x3 *a2);
__declspec(dllimport) int animateStsTexture(Matrix4x3 *res, float time, AnimTrack_Entry *bt, float stScale, int frameSnap);

__declspec(dllimport) TextureBind *g_greyTexture;
__declspec(dllimport) TextureBind *g_blackTexture;
__declspec(dllimport) CameraInfo cam_info;
__declspec(dllimport) OptRel_A0 server_visible_state;
}
struct Parser_StAnim
{
    AnimTrack_Entry *btrackTex0;
    AnimTrack_Entry *btrackTex1;
    int src_var;
    float speed_scale;
    float st_scale;
    int flags;
};
static_assert(sizeof(Parser_StAnim) == 0x18);
Vector4 g_tex_scrolls;

static void scrollTex(int unit_idx, Vector2 *amount,glm::mat4 &tgt)
{
    float x = g_State.view.client_loop_timer * amount->x;
    float y = g_State.view.client_loop_timer * amount->y;

    if(unit_idx==0) {
        g_tex_scrolls.x = x;
        g_tex_scrolls.y = y;
    }
    else
    {
        g_tex_scrolls.z = x;
        g_tex_scrolls.w = y;
    }
    tgt = glm::translate(tgt,glm::vec3 {x,y,0});
}
void segs_scaleTex(Vector3 *amount)
{
    glActiveTexture(GL_TEXTURE1);
    glMatrixMode(GL_TEXTURE);
    glScalef(amount->x, amount->y, amount->z);
    glActiveTexture(GL_TEXTURE0);
    glMatrixMode(GL_TEXTURE);
    glScalef(amount->x, amount->y, amount->z);
}
static void setColorFromList(int colorIdx, const ColorList &clrs,InstanceDrawData &data)
{

    float frame = g_State.view.client_loop_timer * clrs.scale;

    float delta = frame - int32_t(frame);
    int idx = int32_t(frame) % clrs.count;
    int idx_next = int32_t(frame) % clrs.count + 1;
    if ( idx_next >= clrs.count )
        idx_next = 0;
    Vector3 lerp_color = Vector3::lerp(clrs.rgba_colors[idx_next].to3Floats(),clrs.rgba_colors[idx].to3Floats(),delta);
    if(colorIdx==0)
    {
        data.constColor1.ref3() = lerp_color;
    }
    else
    {
        data.constColor2.ref3() = lerp_color;
    }
}
void segs_animateSts(Parser_StAnim *arg0,InstanceDrawData &instance_data)
{
    Matrix4x3 tex_xform;
    float time = g_State.view.client_loop_timer * arg0->speed_scale;
    int frame_snap = !(arg0->flags & 1);
    bool are_same = (arg0->btrackTex0==nullptr) == (arg0->btrackTex1==nullptr);
    assert(are_same);
    if (arg0->btrackTex0)
    {
        //assert(false); // this should set program variables instead.
        animateStsTexture(&tex_xform, time, arg0->btrackTex0, arg0->st_scale, frame_snap);
        glm::mat4x3 helper;
        for(int r=0; r<4; r++)
        {
            for(int c=0; c<3; c++)
                helper[r][c] = tex_xform[r][c];
        }
        instance_data.textureMatrix0 = glm::mat4(helper);
        //assert(false); // this should set program variables instead.
        animateStsTexture(&tex_xform, time, arg0->btrackTex1,arg0->st_scale, frame_snap);
        for(int r=0; r<4; r++)
        {
            for(int c=0; c<3; c++)
                helper[r][c] = tex_xform[r][c];
        }
        instance_data.textureMatrix1 = glm::mat4(helper);
    }
}
int  segs_gfxNodeTricks(TrickNode *tricks, Model *model, Matrix4x3 *lhs,MaterialDefinition &def)
{
    Matrix4x3 dst;

    if ( !tricks || tricks->_TrickFlags == 0)
        return 1;

    uint32_t flags = tricks->_TrickFlags;
    if ( flags & 0x4000 )
    {
        assert(tricks->info && tricks->info->StAnim);
        segs_animateSts(*tricks->info->StAnim,def.draw_data);
    }
    if ( flags & 2 ) {
        scrollTex(0, &tricks->ScrollST0,def.draw_data.textureMatrix0);
    }
    if ( flags & 0x200 )
        scrollTex(1, &tricks->ScrollST1,def.draw_data.textureMatrix1);
    if ( flags & 0x40000000 )
    {
        segs_scaleTex(&tricks->tex_scale);
    }
    if ( flags & 1 ) // Additive
    {
        def.render_state.setBlendMode(RenderState::BLEND_ADDALPHA);
        def.draw_data.fog_params.enabled = false;
    }
    if ( flags & 4 && lhs )
        fn_4DEBB0(lhs, 0.0);
    if ( flags & 0x800000 && lhs )
    {
        matMul3x3(&cam_info.inv_viewmat.ref3(), &lhs->ref3(), &dst.ref3());
        float angle = -std::atan2(dst.r3.x, dst.r3.z) - std::atan2(g_sun.shadow_direction.x, -g_sun.shadow_direction.z);
        fn_5B6740(angle, (Matrix3x3 *)lhs);
    }
    if ( flags & 8 && lhs )
    {
        lhs->ref3() = Unity_Matrix.ref3();
        float z_translated = -lhs->TranslationPart.z - 1.2f;
        lhs->r1.x = -1.0;
        if(z_translated>0)
        {
            float fix_amount = std::min(z_translated,3.0f);
            float scale = (fix_amount + lhs->TranslationPart.z) / lhs->TranslationPart.z;
            lhs->TranslationPart.x *= scale;
            lhs->TranslationPart.y *= scale;
            lhs->TranslationPart.z *= scale;
        }
    }
    if (flags & 0x20)
    {
        def.draw_data.globalColor = tricks->TintColor0.to4Floats();
    }
    if (flags & 0x40)
    {
        def.render_state.setCullMode(RenderState::CULL_NONE);
    }
    if (flags & 0x80)
    {
        def.render_state.setDepthTestMode(RenderState::CMP_NONE);
    }
    if (flags & 0x800)
    {
        def.render_state.setDepthWrite(false);
    }
    if ( flags & 0x10000000 )
    {
        if ( server_visible_state.map_time_of_day > tricks->info->NightGlow.x ||
             server_visible_state.map_time_of_day < tricks->info->NightGlow.y )
        {
            def.draw_data.light0.Ambient = { 0.25,0.25,0.25,1.0 };
            def.draw_data.light0.Diffuse = { 0,0,0,1 };
        }
    }
    if ( flags & 0x400 )
    {
        if ( model && g_curr_blend_state == eBlendMode::ADDGLOW && model->num_textures > 0 )
        {
            for(int i=0; i<model->num_textures; ++i)
            {
                if(model->vbo->textureP_arr[i]->gltexture_id==g_greyTexture->gltexture_id)
                    model->vbo->textureP_arr[i]->tex_links[0] = g_blackTexture;
            }
        }
        int alpha_val = g_sun.lamp_alpha;
        if ( flags & 0x10 )
        {
            alpha_val = (alpha_val * tricks->TintColor0.a) >> 8;
        }
        // lightsOn is only used by ADDGLOW shader
        def.draw_data.lightsOn = {alpha_val / 255.0f, 0.0, 0.0, 0.0};
        if ( g_curr_blend_state != eBlendMode::ADDGLOW )
        {
            if ( 0==alpha_val )
                return 0;
            def.draw_data.constColor1.w *= g_sun.lamp_alpha;
        }
    }
    if ( flags & 0x20000 )
    {
        def.draw_data.constColor1.ref3() = tricks->TintColor0.to3Floats();
        def.draw_data.constColor2.ref3() = tricks->TintColor1.to3Floats();
        if ( tricks->info )
        {
            if ( tricks->info->clists[0].count )
                setColorFromList(0, tricks->info->clists[0], def.draw_data);
            if ( tricks->info->clists[1].count )
                setColorFromList(1, tricks->info->clists[1], def.draw_data);
        }
    }
    if ( flags & 0x40000 )
    {
        ; // vertex alpha is noop in glsl shaders
    }
    if ( flags & 0x80000 )
        def.draw_data.fog_params.enabled = false;
    if ( flags & 0x100000 )
    {
        def.draw_data.fog_params.setStart(tricks->info->FogDist.x);
        def.draw_data.fog_params.setEnd(tricks->info->FogDist.y);
        def.draw_data.fog_params.setStart(4000.0);
        def.draw_data.fog_params.setEnd(16000.0);
    }
    if ( flags & 0x1000100 )
    {

        // reflect/env tex
        std::tuple<uint32_t,uint32_t,uint8_t> bitAndTex[2] = {
            {0x1000000,GL_TEXTURE0,MaterialDefinition::REFLECTION_AUTOGEN_UV0},
            {0x100, GL_TEXTURE1,MaterialDefinition::REFLECTION_AUTOGEN_UV1 },
        };
        def.clearAllReflectionModeFlags();
        for(const auto &tex_bit : bitAndTex)
        {
            if (!( flags & std::get<0>(tex_bit) ))
                continue;
            def.setReflectionModeFlag(MaterialDefinition::ReflectionModeFlags(std::get<2>(tex_bit)));
            glActiveTexture(std::get<1>(tex_bit));
            if ( model->Model_flg1 & 0x2000 ) //OBJ_CUBEMAP ?
            {
                def.set_usingCubemaps(true);
                glm::mat3 inv_viewmat (cam_info.inv_viewmat.r1.x, cam_info.inv_viewmat.r1.y, cam_info.inv_viewmat.r1.z,
                                        cam_info.inv_viewmat.r2.x, cam_info.inv_viewmat.r2.y, cam_info.inv_viewmat.r2.z,
                                        cam_info.inv_viewmat.r3.x, cam_info.inv_viewmat.r3.y, cam_info.inv_viewmat.r3.z);
                def.draw_data.textureMatrix0 = glm::mat4(inv_viewmat);

            }
            /*
             glEnable(GL_TEXTURE_GEN_S);
             glEnable(GL_TEXTURE_GEN_T);
            glEnable(GL_TEXTURE_GEN_R);
            glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
            glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
            glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
             */
        }
    }
    if (flags & 0x2000000)
    {
        def.set_useAlphaDiscard(1);
        def.draw_data.setDiscardAlpha(tricks->info->AlphaRef);
    }
    if ( flags & 0x8000000 )
        segs_setTexUnitLoadBias(GL_TEXTURE0, tricks->info->ObjTexBias);
    return 1;
}
void segs_gfxNodeTricksUndo(TrickNode *trick, Model *model)
{
    if (!trick)
        return;
    uint32_t trickflags = trick->_TrickFlags;
    if (!(trickflags & 0x1B1C4FE3))
        return;

    if (trickflags & 0x40004202)
    {
        g_tex_scrolls = {0,0,0,0};
    }
    if (trickflags & 1)
    {
        //glBlendFunc(0x302u, 0x303u);
    }
    if (trickflags & 0x20)
    {
        //g_material_state.globalColor = { 1,1,1,1 };
    }
//    if (trickflags & 0x40)
//        glEnable(GL_CULL_FACE);
//    if ((trickflags & 0x80u) != 0)
//        glEnable(GL_DEPTH_TEST);
    if (trickflags & 0x800)
    {
     //   segs_wcw_statemgmt_setDepthMask(true);
    }
    if (trickflags & 0x40000)
    {
        ;// unhandled unused in glsl
    }
    if (trickflags & 0x400 && g_curr_blend_state == eBlendMode::ADDGLOW)
    {
        if (model && model->num_textures > 0)
        {
            for(int i=0; i<model->num_textures; ++i)
            {
                TextureBind *bind = model->vbo->textureP_arr[i];
                if (bind->tex_links[0]->gltexture_id == g_blackTexture->gltexture_id)
                    bind->tex_links[0] = g_greyTexture;
            }
        }
    }
//    if (trickflags & 0x2000000)
//        glAlphaFunc(GL_GREATER, 0.0);
    if (trickflags & 0x8000000)
        segs_setTexUnitLoadBias(GL_TEXTURE0, -0.5);
}
TrickNode *segs_getOrCreateTrickNode(Model *model)
{
    if ( !model->trck_node )
        model->trck_node = (TrickNode *)COH_CALLOC(sizeof(TrickNode), 1);
    return model->trck_node;
}
void patch_rendertricks()
{
    patchit("Model__GetOrCreateTrickNode",(void *)segs_getOrCreateTrickNode);
    BREAK_FUNC(modelSetAlpha);
    BREAK_FUNC(gfxNodeTricks);
    BREAK_FUNC(gfxNodeTricksUndo);
}
