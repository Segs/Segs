#pragma once

#include "utils/helpers.h"

#include "GL/glew.h"
#include <glm/mat4x4.hpp>

#include <stdint.h>
#include <vector>
#include <unordered_map>
#include <memory>

inline void hash_combine(std::size_t& /*seed*/) { }
template <typename T, typename... Rest>
inline void hash_combine(std::size_t& seed, const T& v, Rest... rest) {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    hash_combine(seed, rest...);
}
struct ShaderProgram;
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
struct ShaderValue
{
    union {
        Vector4 v4; // type 0
        Vector3 v3; // type 1
        int i0;     // type 2
        GLuint gl_id; // type 3
        struct TextureBind *tex; // type 4
    };
    uint8_t type;
    explicit ShaderValue() {}
    explicit ShaderValue(int v) : i0(v),type(2) {}
    explicit ShaderValue(Vector3 v) : v3(v),type(1) {}
    explicit ShaderValue(Vector4 v) : v4(v),type(0) {}
    explicit ShaderValue(TextureBind *v) : tex(v),type(3) {}
};
struct LightState
{
    Vector4 Ambient = {0,0,0,1};
    Vector4 Diffuse = { 1,1,1,1 };
    Vector4 Specular= { 1,1,1,1 };
    //TODO: all instances of this should be multiplied by current-at-the point of use modelview matrix
    Vector4 Position = { 0, 0, 1, 0 }; // last component meaning is : 0 directional light ( Position is direction) otherwise non-directional light located at xyz
    int State = 0;
};
struct FogState
{
    friend struct ShaderProgram; // allow shader program to read mode directly
    Vector4 color {0,0,0,0};
    Vector4 v_block {1,0,1,0};   // density/start/end/unused
    // for linear fog the scale is end-start
    bool enabled=false;
    void setDensity(float v) { v_block.x=v; }
    void setStart(float v) { v_block.y=v; }
    float getStart() { return v_block.y; }
    void setEnd(float v) { v_block.z=v; }
    float getEnd() { return v_block.z; }
    int getMode() const { return enabled ? mode : 0;}
    void setMode(int m) { mode = m;}
private:
    int mode=0; // 0 - no fog, 1 - linear_fog, 2 - exp fog
};
struct InstanceDrawData
{
    // Sampler block
    TextureBind *tex_id_0;
    TextureBind *tex_id_1;
    TextureBind *tex_id_2;
    ////////////////////////////////////////////
    /// vertex shader uniform interface block
    ////////////////////////////////////////////
    glm::mat4 modelViewMatrix;
    glm::mat4 projectionMatrix;
    LightState light0;
    Vector4 lightsOn;
    FogState fog_params;
    Vector4 globalColor {1,1,1,1};
    Vector3 viewerPosition;
    ////////////////////////////////////////////
    /// fragment shader uniform interface block
    ////////////////////////////////////////////

    Vector4 constColor1 { 0,0,0,1 };
    Vector4 constColor2 { 0,0,0,1 };
    Vector4 glossFactor; // x- glossFactor, y - stippling alpha, z - discard alpha
    bool colorConstsEnabled=true;
    // optional uniforms
    Vector4 textureScroll; // xy - scroll for Texture 0, zw scroll for Texture 1
    glm::mat4 textureMatrix0 = glm::mat4(1);
    glm::mat4 textureMatrix1 = glm::mat4(1);
    //    std::unique_ptr<std::array<Vector4,48>> boneMatrices; // mat 3x4 - 16 matrices
    void setStippleAlpha(uint8_t alpha) { glossFactor.y = alpha / 255.0f; }
    void setDiscardAlpha(float v) { glossFactor.z = v; }
};
struct RenderState
{
    enum CompareMode
    {
        CMP_UNINIT = -1,
        CMP_LESSEQUAL = 0,
        CMP_LESS,
        CMP_ALWAYS,
        CMP_NONE // disable the test altogether
    };
    enum CullMode
    {
        CULL_UNINIT = -1,
        CULL_CCW = 0,
        CULL_CW,
        CULL_NONE
    };
    enum GL_BlendMode
    {
        BLEND_UNINIT = -1,
        BLEND_ALPHA = 0,      //GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA
        BLEND_ADDALPHA,   //GL_SRC_ALPHA, GL_ONE
    };
    enum StencilMode
    {
        STENCIL_NONE,
        STENCIL_ALWAYS,
        STENCIL_NOTEQUAL
    };

    CompareMode depth_cmp = CMP_LESSEQUAL;
    bool depth_write = true;
    bool color_write = true;
    GL_BlendMode gl_blending = BLEND_ALPHA;
    CullMode cull_mode = CULL_CCW;
    StencilMode stencil_mode;
    GLint stencil_ref;
    GLuint stencil_mask = ~0U;
    GLenum stencil_fail=0;
    GLenum stencil_zfail=0;
    GLenum stencil_zpass=0;
    void setDepthTestMode(CompareMode mode) { depth_cmp = mode; }
    void setDepthWrite(bool enable) { depth_write = enable; }
    void setColorWrite(bool enable) { color_write = enable; }
    void setBlendMode(GL_BlendMode m) { gl_blending = m; }
    void setStencilOp(GLenum fail,GLenum zfail,GLenum zpass) { stencil_fail=fail;stencil_zfail=zfail; stencil_zpass=zpass;}
    void setCullMode(CullMode m) { cull_mode = m; }
};
struct RenderStateWrapper
{
    void apply(const RenderState &v);
    RenderState getGlobal() { return current_state; }
protected:
    bool before_first_apply = true;
    RenderState current_state;
};
extern RenderStateWrapper g_render_state;
#define STATE_ACCESSORS(type,name) \
    type get_##name() const { return name; }\
    void set_##name(type v)\
    {\
        if(v==name)\
        {\
            redundant_state_changes++;\
            return;\
        }\
        is_dirty=true;\
        name = v;\
        needs_shader_recompile=true;\
    }

struct MaterialDefinition
{
    //WARN: this might blow up if code is trying to access dead (stack-based) material
    static MaterialDefinition *last_applied;
    ShaderProgram *program=nullptr; // non owning program
    void setDrawMode(DrawMode vertex_mode);
    void setFragmentMode(eBlendMode pixel_mode);
    explicit MaterialDefinition(DrawMode vertex_mode, eBlendMode pixel_mode);
    RenderState render_state;
    InstanceDrawData draw_data;
    ~MaterialDefinition();

    enum ShadeModel
    {
        SHADE_UNINIT = -1,
        SMOOTH = 0,
        FLAT,
    };
    enum ReflectionModeFlags : uint8_t
    {
        REFLECTION_NONE = 0,
        REFLECTION_AUTOGEN_UV1 = 1,
        REFLECTION_AUTOGEN_UV0 = 2,
    };
    enum LightMode
    {
        LIGHT_MODE_NONE = 0,
        LIGHT_MODE_PRE_LIT,
        LIGHT_MODE_DIFFUSE, // per-vertex color, only use diffuse part
        LIGHT_MODE_LIT
    };
    enum class BumpMode : int
    {
        NONE = 0,
        SPECULAR_ONLY=1,
        ALL,
    };
    enum class LightSpace : int
    {
        VIEW_SPACE=0,
        MODEL_SPACE=1,
    };
    enum TexAnimMode
    {
        TEX_NONE=0,
        TEX_OFFSET,
        TEX_MATRIX,
    };
    bool operator==(const MaterialDefinition &oth) const
    {
        return texUnits == oth.texUnits &&
            colorSource==oth.colorSource &&
            vertexMode==oth.vertexMode &&
            lightMode==oth.lightMode &&
            bumpMapMode==oth.bumpMapMode &&
            reflectionMode==oth.reflectionMode &&
            usingCubemaps==oth.usingCubemaps &&
            colorBlending==oth.colorBlending &&
            fragmentMode==oth.fragmentMode
            ;
    }


    void setReflectionModeFlag(ReflectionModeFlags f) { reflectionMode |= f; }
    void clearReflectionModeFlag(ReflectionModeFlags f) { reflectionMode &= ~f; }
    void clearAllReflectionModeFlags() { reflectionMode = 0; }

    size_t hash_val() const
    {
        if (!is_dirty)
            return precomp_hash;
        precomp_hash = 0x42;
        hash_combine(precomp_hash, texUnits, vertexMode, lightMode, bumpMapMode, colorBlending, fragmentMode,
                     useLodAlpha, useAlphaDiscard, colorSource, reflectionMode, usingCubemaps, useStippling,
                     useTransformFeedback, int(shadeModel), int(texTransform), int(light_space));
        is_dirty = false;
        return precomp_hash;
    }
    void apply();
    void updateUniforms();
public:
    STATE_ACCESSORS(int,colorSource)
    STATE_ACCESSORS(ShadeModel,shadeModel)
    STATE_ACCESSORS(TexAnimMode,texTransform)
    STATE_ACCESSORS(bool,usingCubemaps)
    STATE_ACCESSORS(int,texUnits)
    STATE_ACCESSORS(int,vertexMode)
    STATE_ACCESSORS(int,lightMode)
    STATE_ACCESSORS(LightSpace, light_space)
    STATE_ACCESSORS(BumpMode, bumpMapMode)
    STATE_ACCESSORS(int,colorBlending)
    STATE_ACCESSORS(int,fragmentMode)
    STATE_ACCESSORS(bool,useLodAlpha)
    STATE_ACCESSORS(int,useAlphaDiscard)
    STATE_ACCESSORS(uint8_t,reflectionMode)
    STATE_ACCESSORS(bool, useStippling)
    STATE_ACCESSORS(bool, useTransformFeedback)

private:
    // Shader setup values
    int texUnits = 0;  //number of used texture units
    int vertexMode = 0; // 0 - no processing, 1 - skinned
    int lightMode=0;  // int : 0 - unlit, 1 - prelit, 2-lit, 3 - bump-map ( per-pixel specular )
    LightSpace  light_space    = LightSpace::VIEW_SPACE;
    BumpMode bumpMapMode = BumpMode::NONE;
    int colorBlending = 0; // 0 - no color blending, 1 - blend between colors based on blend texture
    int fragmentMode = 0; // basically eBlendMode converted to plain int
    bool useLodAlpha = false; //
    int useAlphaDiscard=0; // 0 - none, 1 discard if alpha <= discardAlpha, 2 discard if alpha > discardAlpha
    uint8_t reflectionMode = 0; // no reflection uv autogenerated
    bool usingCubemaps = false;
    bool useStippling = false;
    bool useTransformFeedback = false;
    ShadeModel shadeModel = SMOOTH;
    TexAnimMode texTransform=TEX_NONE;
    int colorSource = 0; // 0 - global, 1 per-vertex

    int redundant_state_changes=0;
    bool needs_shader_recompile=true;
    mutable bool is_dirty=true;
    mutable size_t precomp_hash = 0;

};

namespace std {
template<>
struct hash<MaterialDefinition>
{
    size_t operator()(const MaterialDefinition &v) const
    {
        return v.hash_val();
    }
};
}
struct RdrStats
{
    float model_total;
    int model_drawn;
    int tri_count;
    int shadow_tri_count;
    int nosort_count;
    int vert_count;
    int vert_access_count;
    int vert_unique_count;
    int unkn_20[4232];
    int frame_id;
};
static_assert (sizeof(RdrStats) == 0x4244, "sizeof(RdrStats)");
extern FogState g_fog_state;
extern LightState g_light_state;

extern GLuint perUnitSamplers[4];
extern void initializeRenderer();
extern void  segs_setTexUnitLoadBias(uint32_t texture, float param);
extern void patch_render_state();
extern void  segs_wcw_statemgmt_bindTexture(GLenum target, signed int idx, GLuint texture);
extern void  segs_texSetAllToWhite();
extern void segs_createSamplers();
