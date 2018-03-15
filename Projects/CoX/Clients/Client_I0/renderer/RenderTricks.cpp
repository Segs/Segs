#include "RenderTricks.h"

#include "RendererState.h"
#include "Texture.h"
#include "Model.h"

#include "GameState.h"
#include "utils/dll_patcher.h"
#include "utils/helpers.h"
#include "GL/glew.h"

#include <stdint.h>
#include <algorithm>
#include <cmath>
#include <cassert>
#include <vector>

extern "C" {
struct HashTable;
struct Parser_StAnim;


__declspec(dllimport) TextureBind * tex_FindMapEntryTexture(const char *b);
__declspec(dllimport) void * HashTable_queryValue(HashTable *cache_tab, const char *filename);
__declspec(dllimport) int error_ReportParsing(const char *filename, const char *fmt, ...);
__declspec(dllimport) int  AddArrayEntry(void ***arr, void *entry);
__declspec(dllimport) void * dbgCalloc(int strct_size, int count, int blockType, const char *fname, int line);
__declspec(dllimport) void  tex_4E63C0(TextureBind *tex);
__declspec(dllimport) void  tex_4E6300();
__declspec(dllimport) int  tex_4E5CD0(char *name, TextureBind *value);
__declspec(dllimport) bool  file_PathIsDirectory(const char *path);
__declspec(dllimport) int VfPrintfWrapper(const char *fmt,...);
__declspec(dllimport) int  tex_4E6320();
__declspec(dllimport) void  animateSts(Parser_StAnim *anim);
__declspec(dllimport) void  fn_4DEBB0(Matrix4x3 *a2, float arg4);
__declspec(dllimport) void  matMul3x3(const Matrix3x3 *rhs, const Matrix3x3 *lhs, Matrix3x3 *dst);
__declspec(dllimport) void  fn_5B6740(float yaw_angle, Matrix3x3 *a2);
__declspec(dllimport) void wcwMgmt_EnableFog(int);

__declspec(dllimport) int GPU_FLAGS;
__declspec(dllimport) int int32_85EA64;
__declspec(dllimport) HashTable *g_texbinds_ht;
__declspec(dllimport) TextureBind **g_texbinds;
__declspec(dllimport) TextureBind *g_greyTexture;
__declspec(dllimport) TextureBind *g_blackTexture;
__declspec(dllimport) SunLight g_sun;
__declspec(dllimport) Vector4 Light0AmbienColor;
__declspec(dllimport) Vector4 Light0DiffuseColor;
__declspec(dllimport) float fogStart_;
__declspec(dllimport) float _fogEnd_;
__declspec(dllimport) Vector4 shdr_Constant1;
__declspec(dllimport) Vector4 shdr_Constant2;
__declspec(dllimport) DrawMode g_curr_draw_state;
__declspec(dllimport) eBlendMode g_curr_blend_state;
__declspec(dllimport) Vector4 struct_9A0CC0[2];
__declspec(dllimport) CameraInfo cam_info;
__declspec(dllimport) OptRel_A0 server_visible_state;
}
Matrix4x3 Unity_Matrix = {{1,0,0},{0,1,0},{0,0,1},{0,0,0}};
static GLenum g_tex_units[2] = {GL_TEXTURE0, GL_TEXTURE1};
void scrollTex(int unit_idx, Vector2 *amount)
{
    float x = g_State.view.client_loop_timer * amount->x;
    float y = g_State.view.client_loop_timer * amount->y;

    struct_9A0CC0[unit_idx].x = x;
    struct_9A0CC0[unit_idx].y = y;
    struct_9A0CC0[unit_idx].z = 0;
    glActiveTextureARB(g_tex_units[unit_idx]);
    glMatrixMode(GL_TEXTURE);
    glTranslatef(x, y, 0.0);
    glActiveTextureARB(GL_TEXTURE0);
}
void scaleTex(Vector3 *amount)
{
    glActiveTextureARB(GL_TEXTURE1);
    glMatrixMode(GL_TEXTURE);
    glScalef(amount->x, amount->y, amount->z);
    glActiveTextureARB(GL_TEXTURE0);
    glMatrixMode(GL_TEXTURE);
    glScalef(amount->x, amount->y, amount->z);
}
static void setColorFromList(int colorIdx, const ColorList &clrs)
{

    float frame = g_State.view.client_loop_timer * clrs.scale;

    float delta = frame - int32_t(frame);
    int idx = int32_t(frame) % clrs.count;
    int idx_next = int32_t(frame) % clrs.count + 1;
    if ( idx_next >= clrs.count )
        idx_next = 0;
    Vector3 lerp_color = Vector3::lerp(clrs.field_0[idx_next].to3Floats(),clrs.field_0[idx].to3Floats(),delta);
    if(colorIdx==0)
    {
        shdr_Constant1.ref3() = lerp_color;
        setUniformForProgram("constColor1",shdr_Constant1);
    }
    else
    {
        shdr_Constant2.ref3() = lerp_color;
        setUniformForProgram("constColor2",shdr_Constant2);
    }
}
void  segs_modelSetAlpha(uint8_t alpha)
{
    shdr_Constant1.w = alpha / 255.0f;
    setUniformForProgram("constColor1",shdr_Constant1);
}
int  segs_gfxNodeTricks(TrickNode *tricks, Model *model, Matrix4x3 *lhs)
{
    Matrix4x3 dst;

    if ( !tricks || tricks->_TrickFlags == 0)
        return 1;

    uint32_t flags = tricks->_TrickFlags;
    if ( flags & 0x4000 )
    {
        assert(tricks->info && tricks->info->StAnim);
        animateSts(*tricks->info->StAnim);
    }
    if ( flags & 2 )
        scrollTex(0, &tricks->ScrollST0);
    if ( flags & 0x200 )
        scrollTex(1, &tricks->ScrollST1);
    if ( flags & 0x40000000 )
        scaleTex(&tricks->tex_scale);
    if ( flags & 1 ) // Additive
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        wcwMgmt_EnableFog(0);
    }
    if ( flags & 4 && lhs )
        fn_4DEBB0(lhs, 0.0);
    if ( flags & 0x800000 && lhs )
    {
        matMul3x3((Matrix3x3 *)&cam_info.inv_viewmat, (Matrix3x3 *)lhs, (Matrix3x3 *)&dst);
        float angle = -std::atan2(dst.r3.x, dst.r3.z) - std::atan2(g_sun.shadow_direction.x, -g_sun.shadow_direction.z);
        fn_5B6740(angle, (Matrix3x3 *)lhs);
    }
    if ( flags & 8 && lhs )
    {
        *(Matrix3x3 *)&Unity_Matrix = *(Matrix3x3 *)lhs;
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
    if ( flags & 0x20 )
        glColor4ub(tricks->TintColor0.r, tricks->TintColor0.g, tricks->TintColor0.b, tricks->TintColor0.a);
    if ( flags & 0x40 )
        glDisable(GL_CULL_FACE);
    if ( flags & 0x80 )
        glDisable(GL_DEPTH_TEST);
    if ( flags & 0x800 )
        segs_wcw_statemgmt_setDepthMask(false);
    if ( flags & 0x10000000 )
    {
        if ( server_visible_state.map_time_of_day > tricks->info->NightGlow.x ||
             server_visible_state.map_time_of_day < tricks->info->NightGlow.y )
        {
            Light0AmbienColor.x = 0.25;
            Light0AmbienColor.y = 0.25;
            Light0AmbienColor.z = 0.25;
            segs_wcw_statemgmt_SetLightParam(GL_LIGHT0, GL_AMBIENT, &Light0AmbienColor);
            segs_wcw_statemgmt_SetLightParam(GL_LIGHT0, GL_DIFFUSE, &Light0DiffuseColor);
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
        if ( g_curr_blend_state == eBlendMode::ADDGLOW )
        {
            setUniformForProgram("lightsOn",{alpha_val / 255.0f, 0.0, 0.0, 0.0});
        }
        else
        {
            if ( 0==alpha_val )
                return 0;
            segs_modelSetAlpha(g_sun.lamp_alpha * shdr_Constant1.w);
        }
    }
    if ( flags & 0x20000 )
    {
        shdr_Constant1.ref3() = tricks->TintColor0.to3Floats();
        shdr_Constant2.ref3() = tricks->TintColor1.to3Floats();
        segs_setShaderConstant(0,&shdr_Constant1);
        segs_setShaderConstant(1,&shdr_Constant2);
        if ( tricks->info )
        {
            if ( tricks->info->clists[0].count )
                setColorFromList(0, tricks->info->clists[0]);
            if ( tricks->info->clists[1].count )
                setColorFromList(1, tricks->info->clists[1]);
        }
    }
    if ( flags & 0x40000 )
        ; // vertex alpha is noop in glsl shaders
    if ( flags & 0x80000 )
        wcwMgmt_EnableFog(0);
    if ( flags & 0x100000 )
    {
        glGetFloatv(GL_FOG_START, &fogStart_);
        glGetFloatv(GL_FOG_END, &_fogEnd_);
        glFogf(GL_FOG_START, tricks->info->FogDist.x);
        glFogf(GL_FOG_END, tricks->info->FogDist.y);
        glFogf(GL_FOG_START, 4000.0);
        glFogf(GL_FOG_END, 16000.0);
    }
    if ( flags & 0x1000100 )
    {
        // reflect/env tex
        std::pair<uint32_t,uint32_t> bitAndTex[2] = {
            {0x1000000,GL_TEXTURE0},
            {0x100, GL_TEXTURE1},
        };
        for(const auto tex_bit : bitAndTex)
        {
            if (!( flags & tex_bit.first ))
                continue;
            glActiveTextureARB(tex_bit.second);
            segs_enableGlClientState(ClientStates::NORMALS);
            if ( model->Model_flg1 & 0x2000 ) //OBJ_CUBEMAP ?
            {
                glEnable(GL_TEXTURE_CUBE_MAP);
                glDisable(GL_TEXTURE_2D);
                glMatrixMode(GL_TEXTURE);
                *(Matrix3x3 *)&cam_info.inv_viewmat = *(Matrix3x3 *)&dst;
                dst.TranslationPart = { 0.0f,0.0f,0.0f };
                Matrix4x4 m = dst;
                glLoadMatrixf(m.data());
                glMatrixMode(GL_MODELVIEW);
            }
            glEnable(GL_TEXTURE_GEN_S);
            glEnable(GL_TEXTURE_GEN_T);
            glEnable(GL_TEXTURE_GEN_R);
            glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
            glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
            glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
        }
        glActiveTextureARB(GL_TEXTURE0);
    }
    if ( flags & 0x2000000 )
        glAlphaFunc(GL_GREATER, tricks->info->AlphaRef);
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
        glActiveTextureARB(GL_TEXTURE1);
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glActiveTextureARB(GL_TEXTURE0);
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        struct_9A0CC0[0] = { 0,0,0,0 }; // texture scroll 0
        struct_9A0CC0[1] = { 0,0,0,0 }; // texture scroll 1
        glMatrixMode(GL_MODELVIEW);
    }
    if (trickflags & 1)
    {
        glBlendFunc(0x302u, 0x303u);
    }
    if (trickflags & 0x20)
        glColor4ub(0xFFu, 0xFFu, 0xFFu, 0xFFu);
    if (trickflags & 0x40)
        glEnable(GL_CULL_FACE);
    if ((trickflags & 0x80u) != 0)
        glEnable(GL_DEPTH_TEST);
    if (trickflags & 0x800)
        segs_wcw_statemgmt_setDepthMask(true);
    if (trickflags & 0x40000)
        ;// unhandled unused in glsl
    if (trickflags & 0x80000)
        wcwMgmt_EnableFog(1);
    if (trickflags & 0x100000)
    {
        glFogf(GL_FOG_START, fogStart_);
        glFogf(GL_FOG_END, _fogEnd_);
    }
    if (trickflags & 0x400 && g_curr_blend_state == eBlendMode::ADDGLOW)
    {
        if (model && model->num_textures > 0)
        {
            for(int i=0; i<model->num_textures; ++i)
            {
                TextureBind *v4 = model->vbo->textureP_arr[i];
                if (v4->tex_links[0]->gltexture_id == g_blackTexture->gltexture_id)
                    v4->tex_links[0] = g_greyTexture;
            }
        }
    }
    if (trickflags & 0x1000100)
    {
        int flag_bits[2] = { 0x1000000 ,0x100 };
        GLenum tex_idx[2] = { GL_TEXTURE0, GL_TEXTURE1 };
        for(int idx = 0; idx<2; ++idx)
        {
            if (trickflags & flag_bits[idx])
            {
                glActiveTextureARB(tex_idx[idx]);
                glDisable(GL_TEXTURE_GEN_S);
                glDisable(GL_TEXTURE_GEN_T);
                glDisable(GL_TEXTURE_GEN_R);
                glDisable(GL_TEXTURE_CUBE_MAP);
                glEnable(GL_TEXTURE_2D);
                glMatrixMode(GL_TEXTURE);
                glLoadIdentity();
                glMatrixMode(GL_MODELVIEW);
            }
        }
        glActiveTextureARB(GL_TEXTURE0);
    }
    if (trickflags & 0x2000000)
        glAlphaFunc(GL_GREATER, 0.0);
    if (trickflags & 0x8000000)
        segs_setTexUnitLoadBias(GL_TEXTURE0, -0.5);
    if (trickflags & 0x10000000)
    {
        segs_wcw_statemgmt_SetLightParam(GL_LIGHT0, GL_AMBIENT, &g_sun.ambient);
        segs_wcw_statemgmt_SetLightParam(GL_LIGHT0, GL_DIFFUSE, &g_sun.diffuse);
    }
}
void patch_rendertricks()
{
    PATCH_FUNC(modelSetAlpha);
    PATCH_FUNC(gfxNodeTricks);
    PATCH_FUNC(gfxNodeTricksUndo);
}
