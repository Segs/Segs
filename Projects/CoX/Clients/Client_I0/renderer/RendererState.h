#pragma once

#include "utils/helpers.h"

#include "GL/glew.h"

#include <stdint.h>
#include <initializer_list>

enum ClientStates : int
{
    BONE_WEIGHTS         = 0,
    BONE_INDICES         = 1,
    BINORMALS            = 2,
    TANGENTS             = 3,
    TEX0_COORDS          = 4,
    TEX1_COORDS          = 5,
    POSITIONS            = 6,
    NORMALS              = 7,
    PERVERTEXCOLORS      = 8,
    PERVERTEXCOLORS_ONLY = 9,
};
enum class DrawMode : int
{
    SINGLETEX = 1,
    DUALTEX = 2,
    COLORONLY = 3,
    DUALTEX_NORMALS = 4, //TODO: rename to _LIT
    SINGLETEX_NORMALS = 5,
    FILL = 6,
    BUMPMAP_SKINNED = 7,
    HW_SKINNED = 8,
    BUMPMAP_NORMALS = 9,
    BUMPMAP_DUALTEX = 10,
    BUMPMAP_RGBS = 11,
    INVALID = ~0 // all bits set, should be properly sized to underlying type
};
enum class eBlendMode : int
{
    MULTIPLY=0,
    MULTIPLY_REG=1,
    COLORBLEND_DUAL=2,
    ADDGLOW=3,
    ALPHADETAIL=4,
    BUMPMAP_MULTIPLY=5,
    BUMPMAP_COLORBLEND_DUAL=6,
    INVALID=~0 // all bits set, should be properly sized to underlying type
};
struct RdrStats
{
    float field_0;
    int model_drawn;
    int tri_count;
    int shadow_tri_count;
    int field_10;
    int vert_count;
    int field_18;
    int vert_unique_count;
    int field_20[4232];
    int frame_id;
};
static_assert (sizeof(RdrStats) == 0x4244, "sizeof(RdrStats)");

extern void initializeRenderer();
extern void segs_enableGlClientState(ClientStates state);
extern void  segs_wcw_statemgmt_setDepthMask(bool flag);
extern void segs_wcw_statemgmt_SetLightParam(uint32_t lightidx, uint32_t pname, Vector4 *params);
extern void setUniformForProgram(const char *name,Vector4 val);
extern void  segs_setTexUnitLoadBias(uint32_t texture, float param);
extern void patch_render_state();
extern void  segs_modelDrawState(DrawMode mode, int a2);
extern void  segs_modelBlendState(eBlendMode num, int force);
extern void  segs_setShaderConstant(GLuint idx, Vector4 *v);
extern void segs_setupShading(DrawMode vertex_mode, eBlendMode pixel_mode);
extern void  segs_wcw_statemgmt_bindTexture(GLenum target, signed int idx, GLuint texture);
extern void  segs_texSetAllToWhite();
extern void  segs_disableGlClientStates(std::initializer_list<ClientStates> states);
