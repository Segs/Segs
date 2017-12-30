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
#include <set>
#include <string>
struct ClientState
{
    GLenum state_enum;
    int on;
};
ShaderProgramCache g_program_cache;
// vertex shaders
static GLuint default_vp;
static GLuint skinbump_vp;
static GLuint skin_vp;
static GLuint bump_vp;
static GLuint bump_rgb_vp;
static GLuint bump_dual_vp;
// fragment shader
static GLuint fragment_shaders[7];
std::set<std::string> enabled_states;
extern "C" {
__declspec(dllimport) int texSetWhite(int unit);
__declspec(dllimport) int enableParticleVBOs;
__declspec(dllimport) int GPU_FLAGS;
__declspec(dllimport) int UsingVBOs;
__declspec(dllimport) int g_using_bump_maps;
__declspec(dllimport) int enableVertShaders;
__declspec(dllimport) DrawMode g_curr_draw_state;
__declspec(dllimport) eBlendMode g_curr_blend_state;
__declspec(dllimport) Vector4 shdr_Constant1;
__declspec(dllimport) Vector4 shdr_Constant2;
__declspec(dllimport) int bound_tex[4];
__declspec(dllimport) TextureBind *g_whiteTexture;
}
//gl_clientstate
static ClientState gl_clientstate[10] = {
    {GL_VERTEX_ATTRIB_ARRAY1_NV, 0},
    {GL_VERTEX_ATTRIB_ARRAY5_NV, 0},
    {GL_VERTEX_ATTRIB_ARRAY6_NV, 0},
    {GL_VERTEX_ATTRIB_ARRAY7_NV, 0},
    {GL_TEXTURE_COORD_ARRAY, 0},
    {GL_TEXTURE_COORD_ARRAY, 0},
    {GL_VERTEX_ARRAY, 0},
    {GL_NORMAL_ARRAY, 0},
    {GL_COLOR_ARRAY, 0},
    {GL_VERTEX_ATTRIB_ARRAY11_NV, 0},
};

bool inAllowedUniformName(const char *name)
{
    static const char * allowed_names[12] = {
        "viewerPosition",
        "lightsOn", // vec4i
        "lightAmbient", // vec4
        "lightDiffuse", // vec4
        "lightPosition", // vec4
        "glossFactor", // vec4
        "lightDiffuse", // 12,vec4
        "textureScroll0",
        "textureScroll1",
        "boneMatrices",
        "constColor1", // 13,vec4
        "constColor2",
    };
    for (const char * n : allowed_names)
        if (0 == strcmp(n, name))
            return true;
    return false;
}
void setUniformForProgram(const char *name,Vector4 val)
{
    assert(inAllowedUniformName(name));
    auto loc = glGetUniformLocation(g_program_cache.m_current_program.gl_id,name);
    glUniform4fv(loc,1,val.data());
}
void setUniformForProgram(const char *name,Matrix4x3 val)
{
    static const char * allowed_names[12] = {
        "projection",
        "mvp", // vec4i
    };
    assert(false);
}

static void enableState(ClientStates state)
{
    ShaderProgram &current_prog(g_program_cache.m_current_program);

    assert( GPU_FLAGS & f_GL_VertexShader );
    switch ( state )
    {
        case TEX0_COORDS:
            glClientActiveTextureARB(GL_TEXTURE0);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY_EXT);
            break;
        case TEX1_COORDS:
            glClientActiveTextureARB(GL_TEXTURE1);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY_EXT);
            break;
        case POSITIONS:
            glEnableClientState(GL_VERTEX_ARRAY);
            break;
        case NORMALS:
            glEnableClientState(GL_NORMAL_ARRAY);
            break;
        case PERVERTEXCOLORS:
            enabled_states.insert("GL_COLOR_ARRAY");
            glEnableClientState(GL_COLOR_ARRAY);
            break;
        case BONE_WEIGHTS:
            enabled_states.insert("boneWeights");
            return;
        case BONE_INDICES:
            enabled_states.insert("boneIndices");
            return;
        case BINORMALS:
            enabled_states.insert("binormal");
            return;
        case TANGENTS:
            enabled_states.insert("tangent");
            return;
        case PERVERTEXCOLORS_ONLY:
            enabled_states.insert("rgbs");
            break;
    }
}
static void disableState(ClientStates state)
{
    ShaderProgram &current_prog(g_program_cache.m_current_program);
    assert( GPU_FLAGS & f_GL_VertexShader );
    switch ( state )
    {
        case BONE_WEIGHTS:
            enabled_states.erase("boneWeights");
            current_prog.disableAttrib("boneWeights");
            return;
        case BONE_INDICES:
            enabled_states.erase("boneIndices");
            current_prog.disableAttrib("boneIndices");
            return;
        case BINORMALS:
            enabled_states.erase("binormal");
            current_prog.disableAttrib("binormal");
            return;
        case TANGENTS:
            enabled_states.erase("tangent");
            current_prog.disableAttrib("tangent");
            return;
        case TEX0_COORDS:
            glClientActiveTextureARB(GL_TEXTURE0);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY_EXT);
            break;
        case TEX1_COORDS:
            glClientActiveTextureARB(GL_TEXTURE1);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY_EXT);
            break;
        case POSITIONS:
            glDisableClientState(GL_VERTEX_ARRAY);
            break;
        case NORMALS:
            glDisableClientState(GL_NORMAL_ARRAY);
            break;
        case PERVERTEXCOLORS:
            enabled_states.erase("GL_COLOR_ARRAY");
            glDisableClientState(GL_COLOR_ARRAY);
            break;
        case PERVERTEXCOLORS_ONLY:
            enabled_states.erase("rgbs");
            current_prog.disableAttrib("rgbs");
            break;
    }
}
static void segs_setupLights()
{
    glEnable(GL_LIGHTING);
    segs_wcw_statemgmt_enableColorMaterial();
    glEnable(GL_LIGHT0);
    glColor4ub(255, 255, 255, 255);
}
static void WCW_setNormalize(int normalize)
{
    if ( normalize )
        glEnable(GL_NORMALIZE);
    else
        glDisable(GL_NORMALIZE);
}

void  segs_enableGlClientState(ClientStates state)
{
    if ( !gl_clientstate[state].on )
    {
        enableState(state);
        gl_clientstate[state].on = 1;
    }
}
void segs_enableGlClientStates(std::initializer_list<ClientStates> states)
{
    for(auto state : states)
    {
        if ( gl_clientstate[state].on )
            continue;

        enableState(state);
        gl_clientstate[state].on = 1;
    }
}
void  segs_disableGlClientState(ClientStates state)
{
    if ( 1 == gl_clientstate[state].on )
    {
        disableState(state);
        gl_clientstate[state].on = 0;
    }
}
void  segs_disableGlClientStates(std::initializer_list<ClientStates> states)
{
    for(auto state : states)
    {
        if ( false == gl_clientstate[state].on )
            continue;

        disableState(state);
        gl_clientstate[state].on = 0;
    }
}

static void enableTextureUnit(uint32_t unit)
{
    glActiveTextureARB(unit);
    glClientActiveTextureARB(unit);
    glEnable(GL_TEXTURE_2D);
}
static void disableTextureUnit(uint32_t unit)
{
    glActiveTextureARB(unit);
    glClientActiveTextureARB(unit);
    glDisable(GL_TEXTURE_2D);
}
void segs_setupShading(DrawMode vertex_mode, eBlendMode pixel_mode)
{
    segs_modelDrawState(vertex_mode, 0);
    segs_modelBlendState(pixel_mode, 0);
}
void  segs_modelDrawState(DrawMode mode, int force)
{
    static int drawmode_calls[20];
    static int drawmode_changes[20];
    ++drawmode_calls[int(mode)];
    if (!force && g_curr_draw_state == mode)
        return;
    ++drawmode_changes[int(mode)];
    if (g_using_bump_maps)
    {
        glActiveTextureARB(GL_TEXTURE2);
        glClientActiveTextureARB(GL_TEXTURE2);
        glDisable(GL_TEXTURE_2D);
    }
    segs_disableGlClientStates({BONE_WEIGHTS,BONE_INDICES,BINORMALS,TANGENTS,PERVERTEXCOLORS_ONLY});
    g_program_cache.setVertexProgram(default_vp);

    switch (mode)
    {
        case DrawMode::SINGLETEX:
            enableTextureUnit(GL_TEXTURE1);
            enableTextureUnit(GL_TEXTURE0);
            texSetWhite(1);
            segs_disableGlClientStates({NORMALS,TEX1_COORDS});
            segs_enableGlClientStates({POSITIONS,TEX0_COORDS,PERVERTEXCOLORS});
            glDisable(GL_LIGHTING);
            WCW_setNormalize(0);
            g_curr_draw_state = mode;
            break;
        case DrawMode::DUALTEX:
            enableTextureUnit(GL_TEXTURE1);
            enableTextureUnit(GL_TEXTURE0);
            segs_disableGlClientState(NORMALS);
            segs_enableGlClientStates({POSITIONS,TEX0_COORDS,TEX1_COORDS,PERVERTEXCOLORS});
            glDisable(GL_LIGHTING);
            WCW_setNormalize(0);
            g_curr_draw_state = mode;
            break;
        case DrawMode::COLORONLY:
            enableTextureUnit(GL_TEXTURE1);
            enableTextureUnit(GL_TEXTURE0);
            texSetWhite(1);
            texSetWhite(0);
            segs_disableGlClientStates({NORMALS,PERVERTEXCOLORS,TEX0_COORDS,TEX1_COORDS});
            glDisable(GL_LIGHTING);
            WCW_setNormalize(0);
            g_curr_draw_state = mode;
            break;
        case DrawMode::DUALTEX_NORMALS:
            enableTextureUnit(GL_TEXTURE1);
            enableTextureUnit(GL_TEXTURE0);
            segs_disableGlClientState(PERVERTEXCOLORS);
            segs_enableGlClientStates({POSITIONS,NORMALS,TEX0_COORDS,TEX1_COORDS});
            segs_setupLights();
            WCW_setNormalize(0);
            g_curr_draw_state = mode;
            break;
        case DrawMode::SINGLETEX_NORMALS:
            enableTextureUnit(GL_TEXTURE1);
            enableTextureUnit(GL_TEXTURE0);
            texSetWhite(1);
            segs_disableGlClientStates({TEX1_COORDS,PERVERTEXCOLORS});
            segs_enableGlClientStates({POSITIONS,NORMALS,TEX0_COORDS});
            segs_setupLights();
            WCW_setNormalize(1);
            g_curr_draw_state = mode;
            break;
        case DrawMode::FILL:
            disableTextureUnit(GL_TEXTURE1);
            disableTextureUnit(GL_TEXTURE0);
            segs_disableGlClientStates({NORMALS,TEX0_COORDS,TEX1_COORDS,PERVERTEXCOLORS});
            glDisable(GL_LIGHTING);
            WCW_setNormalize(0);
            g_curr_draw_state = mode;
            break;
        case DrawMode::BUMPMAP_SKINNED:
            enableTextureUnit(GL_TEXTURE2);
            enableTextureUnit(GL_TEXTURE1);
            enableTextureUnit(GL_TEXTURE0);

            g_program_cache.setVertexProgram(skinbump_vp);
            segs_enableGlClientStates({POSITIONS, BONE_WEIGHTS, NORMALS, TEX0_COORDS, TEX1_COORDS,
                                       BONE_INDICES, BINORMALS, TANGENTS});
            segs_disableGlClientState(PERVERTEXCOLORS);
            WCW_setNormalize(0);
            g_curr_draw_state = mode;
            break;
        case DrawMode::HW_SKINNED:
            if (g_using_bump_maps)
                disableTextureUnit(GL_TEXTURE2);
            enableTextureUnit(GL_TEXTURE1);
            enableTextureUnit(GL_TEXTURE0);
            g_program_cache.setVertexProgram(skin_vp);
            segs_enableGlClientStates({POSITIONS,NORMALS,TEX0_COORDS,TEX1_COORDS,BONE_WEIGHTS,BONE_INDICES});
            segs_disableGlClientState(PERVERTEXCOLORS);
            segs_setupLights();
            WCW_setNormalize(0);
            g_curr_draw_state = mode;
            break;
        case DrawMode::BUMPMAP_NORMALS:
        case DrawMode::BUMPMAP_RGBS:
            enableTextureUnit(GL_TEXTURE2);
            enableTextureUnit(GL_TEXTURE1);
            enableTextureUnit(GL_TEXTURE0);

            if (mode == DrawMode::BUMPMAP_NORMALS)
            {
                g_program_cache.setVertexProgram(bump_vp);
            }
            else
            {
                g_program_cache.setVertexProgram(bump_rgb_vp);
                segs_enableGlClientState(PERVERTEXCOLORS_ONLY);
            }
            segs_enableGlClientStates({ POSITIONS, NORMALS, TEX0_COORDS, TEX1_COORDS,
                BINORMALS, TANGENTS });
            segs_disableGlClientState(PERVERTEXCOLORS);
            WCW_setNormalize(0);
            g_curr_draw_state = mode;
            break;
        case DrawMode::BUMPMAP_DUALTEX:
            enableTextureUnit(GL_TEXTURE2);
            enableTextureUnit(GL_TEXTURE1);
            enableTextureUnit(GL_TEXTURE0);
            segs_enableGlClientStates({POSITIONS,NORMALS,TEX0_COORDS,TEX1_COORDS,BINORMALS,TANGENTS});
            segs_disableGlClientState(PERVERTEXCOLORS);
            g_program_cache.setVertexProgram(bump_dual_vp);
            WCW_setNormalize(0);
            g_curr_draw_state = mode;
            break;
    }
    setUniformForProgram("constColor1",shdr_Constant1);
    setUniformForProgram("constColor2",shdr_Constant2);

}
static void compileShader(const char *filename, GLuint program_id)
{
    assert(filename);
    assert(program_id);
    FILE *fp = fopen(filename,"rb");
    if(!fp)
    {
        printfDebug("Error loading \"%s\" file is missing\n", filename);
        return;
    }

    fseek(fp,0,SEEK_END);
    unsigned shader_size = ftell(fp);
    GLcharARB *string = (GLcharARB *)malloc(shader_size);
    if(nullptr==string)
    {
        printfDebug("Error loading \"%s\"\n", filename);
        return;
    }
    fseek(fp,0,SEEK_SET);
    if (fread(string, 1, shader_size,fp) != shader_size)
    {
        printfDebug("Error loading \"%s\"\n", filename);
        return;
    }
    glShaderSource(program_id,1,(const GLcharARB **)&string,(int *)&shader_size);

    glCompileShader(program_id);
    GLint isCompiled = 0;
    glGetObjectParameterivARB(program_id, GL_COMPILE_STATUS, &isCompiled);
    free(string);
    if(isCompiled != GL_FALSE)
        return;

    GLint maxLength = 0;
    glGetShaderiv(program_id, GL_INFO_LOG_LENGTH, &maxLength);
    // The maxLength includes the NULL character
    std::vector<GLchar> errorLog(maxLength);
    glGetShaderInfoLog(program_id, maxLength, &maxLength, &errorLog[0]);
    fprintf(stderr,"%s shader compilation failed: %s",filename,errorLog.data());
}
#define CREATE_NAMED_VP_SHADER(name)\
    name = glCreateShader(GL_VERTEX_SHADER_ARB);\
    glObjectLabel(GL_SHADER,name,strlen(#name),#name);
void initializeRenderer()
{
    static bool shaders_created=false;
    assert( GPU_FLAGS & f_GL_VertexShader );
    if (!shaders_created)
    {
        CREATE_NAMED_VP_SHADER(default_vp);
        CREATE_NAMED_VP_SHADER(skinbump_vp);
        CREATE_NAMED_VP_SHADER(skin_vp);
        CREATE_NAMED_VP_SHADER(bump_vp);
        CREATE_NAMED_VP_SHADER(bump_rgb_vp);
        CREATE_NAMED_VP_SHADER(bump_dual_vp);
        for (unsigned int & shader : fragment_shaders)
            shader = glCreateShader(GL_FRAGMENT_SHADER_ARB);
        shaders_created = true;
    }
    compileShader("shaders/default.glsl", default_vp);
    compileShader("shaders/skin_bump.glsl", skinbump_vp);
    compileShader("shaders/skin.glsl", skin_vp);
    compileShader("shaders/bump.glsl", bump_vp);
    compileShader("shaders/bump_rgb.glsl", bump_rgb_vp);
    compileShader("shaders/bump_dual.glsl", bump_dual_vp);
    const char *fragment_shader_names[] = {
        "shaders/multiply.glsl",
        "shaders/multiplyReg.glsl",
        "shaders/color_blend_dual.glsl",
        "shaders/add_glow.glsl",
        "shaders/alpha_detail.glsl",
        "shaders/bumpmap_multiply.glsl",
        "shaders/bumpmap_color_blend_dual.glsl",

    };
    int idx = 0;
    for (const char * name : fragment_shader_names)
    {
        compileShader(name, fragment_shaders[idx++]);
    }
}

void ShaderProgramCache::setProgram(GLuint vertex_shader, GLuint fragment_shader)
{
    //find program in cache
    for(ShaderProgram &p : m_linked_programs)
    {
        if(p.gl_id==~0U)
            break;
        if(p.vertex_id==vertex_shader && p.shader_id==fragment_shader)
        {
            m_current_program = p;
            glUseProgram(p.gl_id);
            return;
        }
    }
    GLuint program_id = glCreateProgram();
    fragment_shader = fragment_shader == 0 ? fragment_shaders[0] : fragment_shader;
    m_current_program = {vertex_shader,fragment_shader,program_id};

    //bindLocations(m_current_program.gl_id,{0,1,2,5,6,7,11});

    if(vertex_shader)
        glAttachShader(program_id,vertex_shader);
    glAttachShader(program_id,fragment_shader ? fragment_shader : fragment_shaders[0]);
    glLinkProgram(program_id);

    GLint baseImageLoc = glGetUniformLocation(program_id, "myTextureMap");
    GLint modMapLoc = glGetUniformLocation(program_id, "myTextureMap2");
    GLint normalMapLoc = glGetUniformLocation(program_id, "myTextureMap3");
    GLint linked;
    glGetObjectParameterivARB(program_id, GL_OBJECT_LINK_STATUS_ARB, &linked);
    if (!linked) {
        GLint length;
        glGetObjectParameterivARB(program_id, GL_OBJECT_INFO_LOG_LENGTH_ARB, &length);
        std::vector<GLcharARB> infoLog(length);
        glGetInfoLogARB(program_id, length, nullptr, infoLog.data());
        fprintf(stderr,"%s\n",infoLog.data());
        assert(false);
    }
    glUseProgram(program_id);
    glUniform1i(baseImageLoc, 0); //Texture unit 0 is for base images.
    glUniform1i(modMapLoc, 1); //Texture unit 1 is for alpha/color maps.
    if(normalMapLoc!=-1)
        glUniform1i(normalMapLoc, 2); //Texture unit 2 is for normal maps.
    // save program in cache
    for(ShaderProgram &p : m_linked_programs)
    {
        if(p.gl_id != ~0U)
            continue;
        p = m_current_program;
        return;
    }
    assert(!"Out of slots in shader program cache");
}

void ShaderProgramCache::disableVertexShader() 
{
    if (m_current_program.vertex_id)
        m_previous_program.vertex_id = m_current_program.vertex_id;
    setVertexProgram(default_vp);
}

void  segs_modelBlendState(eBlendMode num, int force)
{
    static int s_blendmode_calls[16];
    static int s_blendmode_changes[16];
    int blendmode_as_int = int(num);
    ++s_blendmode_calls[blendmode_as_int];
    if ( !force && g_curr_blend_state == num )
        return;
    ++s_blendmode_changes[blendmode_as_int];
    if (num == eBlendMode::MULTIPLY_REG)
        printfDebug("zx");
    g_program_cache.setFragmentProgram(fragment_shaders[blendmode_as_int]);
    g_curr_blend_state = num;
}
void segs_wcw_statemgmt_setDepthMask(bool flag)
{
    static bool depth_mask = true;
    if ( depth_mask != flag )
    {
        depth_mask = flag;
        glDepthMask(flag);
    }
}
void segs_wcw_statemgmt_SetLightParam(uint32_t lightidx, uint32_t pname, Vector4 *params)
{
    static Vector4 slight[4];
    assert(pname>=GL_AMBIENT && pname <=GL_POSITION);
    uint32_t  idx = pname - GL_AMBIENT;
    if ( pname == GL_POSITION || memcmp(&slight[idx], params, 0x10) )
    {
        slight[idx] = *params;
        glLightfv(GL_LIGHT0, pname, (float *)params);
    }
}
void segs_setTexUnitLoadBias(uint32_t texture, float param)
{
    float lodBias[4] = {0,0,0,0};
    uint32_t tex_idx=texture-GL_TEXTURE0;
    assert(tex_idx<4);
    if ( lodBias[tex_idx] != param )
    {
        glActiveTextureARB(texture);
        glTexEnvf(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, param);
        lodBias[tex_idx] = param;
    }
}
void  segs_setShaderConstant(GLuint idx, Vector4 *v)
{
    if(idx==0)
        g_program_cache.m_current_program.setUniform("constColor1", *v);
    else
        g_program_cache.m_current_program.setUniform("constColor2", *v);
}
static const GLuint idxToUnit[] = { GL_TEXTURE0, GL_TEXTURE1, GL_TEXTURE2, GL_TEXTURE3 };
void  segs_wcw_statemgmt_bindTexture(GLenum target, signed int idx, GLuint texture)
{
    if ((g_using_bump_maps || idx <= 1) && bound_tex[idx] != texture)
    {
        glActiveTextureARB(idxToUnit[idx]);
        glBindTexture(target, texture);
        bound_tex[idx] = texture;
    }
}
void  segs_texSetAllToWhite()
{
    glActiveTextureARB(idxToUnit[0]);
    glBindTexture(GL_TEXTURE_2D, g_whiteTexture->gltexture_id);
    glActiveTextureARB(idxToUnit[1]);
    glBindTexture(GL_TEXTURE_2D, g_whiteTexture->gltexture_id);
    if (g_using_bump_maps)
    {
        glActiveTextureARB(idxToUnit[2]);
        glBindTexture(GL_TEXTURE_2D, g_whiteTexture->gltexture_id);
        bound_tex[2] = g_whiteTexture->gltexture_id;
    }

    bound_tex[0] = g_whiteTexture->gltexture_id;
    bound_tex[1] = g_whiteTexture->gltexture_id;
}
void patch_render_state()
{
    PATCH_FUNC(enableGlClientState);
    PATCH_FUNC(disableGlClientState);
    PATCH_FUNC(setupLights);
    PATCH_FUNC(modelDrawState);
    PATCH_FUNC(modelBlendState);
    PATCH_FUNC(setShaderConstant);
    PATCH_FUNC(wcw_statemgmt_setDepthMask);
    PATCH_FUNC(wcw_statemgmt_SetLightParam);
    PATCH_FUNC(wcw_statemgmt_bindTexture);
    PATCH_FUNC(texSetAllToWhite);
}


