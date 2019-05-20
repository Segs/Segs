#include "RendererState.h"
#include "RendererUtils.h"
#include "Texture.h"
#include "ShaderProgramCache.h"

#include "utils/helpers.h"
#include "utils/dll_patcher.h"
#include "GL/glew.h"

#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <vector>
#include <string>

extern "C" {
__declspec(dllimport) uint32_t bound_tex[4];
__declspec(dllimport) TextureBind *g_whiteTexture;
}

struct ClientState
{
    GLenum state_enum;
    int on;
};
MaterialDefinition *MaterialDefinition::last_applied=nullptr;
RenderStateWrapper g_render_state;
FogState g_fog_state;
LightState g_light_state;
GLuint perUnitSamplers[4];

void initializeRenderer()
{
}


void segs_setTexUnitLoadBias(uint32_t texture, float param)
{
//TODO:
    return;
    /*
    float lodBias[4] = {0,0,0,0};
    uint32_t tex_idx=texture-GL_TEXTURE0;
    assert(tex_idx<4);
    if ( lodBias[tex_idx] != param )
    {
        glActiveTexture(texture);
        glSamplerParameterf(perUnitSamplers[tex_idx], GL_TEXTURE_LOD_BIAS, param);
        lodBias[tex_idx] = param;
    }
    */
}
static const GLuint idxToUnit[] = { GL_TEXTURE0, GL_TEXTURE1, GL_TEXTURE2, GL_TEXTURE3 };
void  segs_wcw_statemgmt_bindTexture(GLenum target, signed int idx, GLuint texture)
{
    if (idx <= 2 && bound_tex[idx] != texture)
    {
        glActiveTexture(idxToUnit[idx]);
        glBindTexture(target, texture);
        bound_tex[idx] = texture;
    }
}
void  segs_texSetAllToWhite()
{
    glActiveTexture(idxToUnit[0]);
    glBindTexture(GL_TEXTURE_2D, g_whiteTexture->gltexture_id);
    glActiveTexture(idxToUnit[1]);
    glBindTexture(GL_TEXTURE_2D, g_whiteTexture->gltexture_id);
    glActiveTexture(idxToUnit[2]);
    glBindTexture(GL_TEXTURE_2D, g_whiteTexture->gltexture_id);
    bound_tex[0] = g_whiteTexture->gltexture_id;
    bound_tex[1] = g_whiteTexture->gltexture_id;
    bound_tex[2] = g_whiteTexture->gltexture_id;
}
void segs_createSamplers()
{
//    glGenSamplers(4, perUnitSamplers);
//    for (GLuint i = 0; i < 4; ++i)
//        glBindSampler(i, perUnitSamplers[i]);
}

void patch_render_state()
{
    BREAK_FUNC(enableGlClientState);
    BREAK_FUNC(disableGlClientState);
    BREAK_FUNC(modelDrawState);
    BREAK_FUNC(modelBlendState);
    BREAK_FUNC(wcw_statemgmt_setDepthMask);
    BREAK_FUNC(setupLights);
    BREAK_FUNC(wcw_statemgmt_SetLightParam);
    BREAK_FUNC(setShaderConstant);

    PATCH_FUNC(wcw_statemgmt_bindTexture);
    PATCH_FUNC(texSetAllToWhite);
}

void RenderStateWrapper::apply(const RenderState& v)
{
    int num_changes=0;
    if(before_first_apply || v.gl_blending!=current_state.gl_blending)
    {
        num_changes++;
        glEnable(GL_BLEND);
        switch (v.gl_blending)
        {
        case RenderState::BLEND_ALPHA:
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            break;
        case RenderState::BLEND_ADDALPHA:
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            break;
        }
    }
    if (before_first_apply || v.cull_mode != current_state.cull_mode)
    {
        num_changes++;
        if (v.cull_mode == RenderState::CULL_NONE)
            glDisable(GL_CULL_FACE);
        else
        {
            glEnable(GL_CULL_FACE);
            glCullFace(v.cull_mode == RenderState::CULL_CCW ? GL_FRONT : GL_BACK);
        }
    }
    if(before_first_apply || v.depth_write != current_state.depth_write)
    {
        num_changes++;
        glDepthMask(v.depth_write ? GL_TRUE : GL_FALSE);
    }
    if (before_first_apply || v.depth_cmp != current_state.depth_cmp)
    {
        num_changes++;
        switch (v.depth_cmp)
        {
        case RenderState::CMP_LESSEQUAL:
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
            break;
        case RenderState::CMP_LESS:
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
            break;
        case RenderState::CMP_ALWAYS:
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_ALWAYS);
            break;
        case RenderState::CMP_NONE:
            glDisable(GL_DEPTH_TEST);
            glDepthFunc(GL_ALWAYS);
            break;
        }
    }
    if (before_first_apply || v.stencil_mode != current_state.stencil_mode || (v.stencil_ref != current_state.stencil_ref) || (v.stencil_mask != current_state.stencil_mask)
        || (v.stencil_fail != current_state.stencil_fail) || (v.stencil_zfail != current_state.stencil_zfail) || (v.stencil_zpass != current_state.stencil_zpass))
    {
        assert(v.stencil_fail < 0x80000000);
        assert(v.stencil_zfail < 0x80000000);
        assert(v.stencil_zpass < 0x80000000);
        num_changes++;
        switch (v.stencil_mode)
        {
        case RenderState::STENCIL_NONE:
            glDisable(GL_STENCIL_TEST);
            break;
        case RenderState::STENCIL_ALWAYS:
            glEnable(GL_STENCIL_TEST);
            glStencilFunc(GL_ALWAYS, v.stencil_ref, v.stencil_mask);
            glStencilOp(v.stencil_fail, v.stencil_zfail, v.stencil_zpass);
            break;
        case RenderState::STENCIL_NOTEQUAL:
            glEnable(GL_STENCIL_TEST);
            glStencilFunc(GL_NOTEQUAL, v.stencil_ref, v.stencil_mask);
            glStencilOp(v.stencil_fail, v.stencil_zfail, v.stencil_zpass);
            break;
        }
    }
    if (before_first_apply || v.color_write != current_state.color_write)
    {
        num_changes++;
        glColorMask(v.color_write, v.color_write, v.color_write, v.color_write);
    }
    if (before_first_apply)
        before_first_apply = false;
    if (num_changes == 0)
        return;
    current_state = v;
    g_default_mat.render_state = current_state;
}

void MaterialDefinition::setDrawMode(DrawMode vertex_mode)
{
    int new_texUnits    = 0;
    int new_colorSource = 0;
    int new_lightMode   = LIGHT_MODE_LIT;
    BumpMode new_bumpMapMode = BumpMode::NONE;
    int new_vertexMode  = 0;
    LightSpace new_light_space   = LightSpace::VIEW_SPACE;
    TexAnimMode new_tex_transform = TEX_NONE;
    switch (vertex_mode)
    {
    case DrawMode::SINGLETEX:
        new_lightMode          = LIGHT_MODE_PRE_LIT;
        new_texUnits           = 1;
        draw_data.tex_id_1 = g_whiteTexture;
        new_colorSource        = 1; // per vertex color
        new_tex_transform     = TEX_MATRIX;
        break;
    case DrawMode::DUALTEX:
        new_lightMode   = LIGHT_MODE_PRE_LIT;
        new_texUnits    = 2;
        new_colorSource = 1; // per vertex color
        new_tex_transform = TEX_MATRIX;
        break;
    case DrawMode::FILL:
        new_lightMode     = LIGHT_MODE_NONE; // behave as pre-lit
        new_colorSource = 1; // per vertex color
        new_tex_transform = TEX_MATRIX;
        break;
    case DrawMode::COLORONLY:
        new_lightMode     = LIGHT_MODE_NONE; // behave as pre-lit
        new_colorSource = 1; // per vertex color
        new_tex_transform = TEX_MATRIX;
        break;
    case DrawMode::DUALTEX_NORMALS:
        new_lightMode   = LIGHT_MODE_LIT;
        draw_data.globalColor = Vector4{1, 1, 1, 1};
        new_texUnits              = 2; // base + blend
        new_tex_transform    = TEX_MATRIX;
        break;
    case DrawMode::SINGLETEX_NORMALS:
        new_lightMode   = LIGHT_MODE_LIT;
        draw_data.tex_id_1    = g_whiteTexture;
        draw_data.globalColor = Vector4{1, 1, 1, 1};
        new_texUnits              = 1; // base
        new_tex_transform    = TEX_MATRIX;
        break;
    case DrawMode::HW_SKINNED:
        new_lightMode         = LIGHT_MODE_DIFFUSE;
        new_vertexMode            = 1;
        new_texUnits              = 2;
        draw_data.globalColor = Vector4{1, 1, 1, 1};
        break;
    case DrawMode::BUMPMAP_SKINNED:
        new_lightMode   = LIGHT_MODE_NONE;
        new_vertexMode  = 1;
        new_bumpMapMode = BumpMode::ALL;
        new_texUnits    = 3; // base + blend + normal
        break;
    case DrawMode::BUMPMAP_NORMALS:
        new_light_space   = LightSpace::MODEL_SPACE;
        new_lightMode     = LIGHT_MODE_DIFFUSE;
        new_bumpMapMode = BumpMode::SPECULAR_ONLY;
        new_texUnits    = 3; // base + blend + normal
        new_tex_transform = TEX_OFFSET;
        break;
    case DrawMode::BUMPMAP_DUALTEX:
        new_lightMode   = LIGHT_MODE_NONE;
        new_bumpMapMode = BumpMode::ALL;
        new_texUnits    = 3; // base + blend + normal
        new_tex_transform = TEX_OFFSET;
        break;
    case DrawMode::BUMPMAP_RGBS:
        new_light_space   = LightSpace::MODEL_SPACE;
        new_lightMode   = LIGHT_MODE_PRE_LIT;
        new_bumpMapMode = BumpMode::SPECULAR_ONLY;
        new_texUnits    = 3; // base + blend + normal
        new_colorSource = 1;
        new_tex_transform = TEX_OFFSET;
        break;
    }
    if (new_texUnits != texUnits)
        needs_shader_recompile = true;
    if (new_colorSource != colorSource)
        needs_shader_recompile = true;
    if (new_lightMode != lightMode)
        needs_shader_recompile = true;
    if (new_bumpMapMode != bumpMapMode)
        needs_shader_recompile = true;
    if (new_vertexMode != vertexMode)
        needs_shader_recompile = true;
    if (new_tex_transform!=texTransform)
        needs_shader_recompile = true;
    if (new_light_space!=light_space)
        needs_shader_recompile = true;

    texUnits    = new_texUnits;
    colorSource = new_colorSource;
    lightMode = new_lightMode;
    bumpMapMode = new_bumpMapMode;
    vertexMode = new_vertexMode;
    texTransform = new_tex_transform;
    light_space  = new_light_space;
}

void MaterialDefinition::setFragmentMode(eBlendMode pixel_mode) {
    int new_fragmentMode=(int)pixel_mode;
    bool new_useLodAlpha = false;
    int new_colorBlending = 0;
    // bump map mode is assigned from vertex mode
    switch (pixel_mode)
    {

    case eBlendMode::MULTIPLY:
        break;
    case eBlendMode::MULTIPLY_REG:
        new_useLodAlpha = true;
        break;
    case eBlendMode::COLORBLEND_DUAL:
        new_colorBlending = 1;
        break;
    case eBlendMode::ADDGLOW:
        break;
    case eBlendMode::ALPHADETAIL:
        break;
    case eBlendMode::BUMPMAP_MULTIPLY:
        new_fragmentMode = (int)eBlendMode::MULTIPLY_REG;
        new_useLodAlpha  = false;
        break;
    case eBlendMode::BUMPMAP_COLORBLEND_DUAL:
        new_fragmentMode  = (int)eBlendMode::COLORBLEND_DUAL;
        break;
    default:
        assert(false);
    }
    if (new_fragmentMode != fragmentMode)
        needs_shader_recompile = true;
    if (new_useLodAlpha != useLodAlpha)
        needs_shader_recompile = true;
    if (new_colorBlending != vertexMode)
        needs_shader_recompile = true;
    fragmentMode = new_fragmentMode;
    useLodAlpha = new_useLodAlpha;
    colorBlending = new_colorBlending;

}

MaterialDefinition::MaterialDefinition(DrawMode vertex_mode, eBlendMode pixel_mode)
{
    lightMode        = LIGHT_MODE_NONE;
    colorSource      = 0;
    vertexMode       = 0;
    texUnits         = 0;
    draw_data.light0 = g_light_state;
    render_state = g_render_state.getGlobal();

    //TODO: this should also set specular exponent to 128
    setDrawMode(vertex_mode);
    setFragmentMode(pixel_mode);
    // sanity check, only allow bumpmap blend on bumpmap vertex
    const bool vp_is_bumpmap = (vertex_mode == DrawMode::BUMPMAP_DUALTEX) ||
                         (vertex_mode == DrawMode::BUMPMAP_NORMALS) || (vertex_mode == DrawMode::BUMPMAP_RGBS) ||
                         (vertex_mode == DrawMode::BUMPMAP_SKINNED);
    const bool fp_is_bumpmap =
        (pixel_mode == eBlendMode::BUMPMAP_COLORBLEND_DUAL) || (pixel_mode == eBlendMode::BUMPMAP_MULTIPLY);
    assert(vp_is_bumpmap == fp_is_bumpmap);
}

MaterialDefinition::~MaterialDefinition()
{
    if (last_applied == this)
        last_applied = nullptr;
}

void MaterialDefinition::apply()
{
    if (needs_shader_recompile)
    {
        program = &g_program_cache.getOrCreateProgram(*this);
        needs_shader_recompile = false;
        program->forceUploadUniforms(draw_data);
        if(lightMode == LIGHT_MODE_LIT)
            assert(draw_data.light0.State==1);
    }
    else
    {
        program->use();
        program->uploadUniforms(draw_data);
    }

    g_render_state.apply(render_state);
    last_applied = this;

}

void MaterialDefinition::updateUniforms()
{
    assert(!needs_shader_recompile && program);
    program->uploadUniforms(draw_data);
}
