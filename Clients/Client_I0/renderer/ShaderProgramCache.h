#pragma once
#include "GL/glew.h"
#include "utils/helpers.h"
#include "RendererState.h"
#include "glm/gtc/type_ptr.hpp"
#include <stdint.h>
#include <set>
#include <vector>
#include <unordered_map>
#include <array>

struct MaterialDefinition;
struct ShaderObject
{
    GLuint id = 0;
    const char *source_file=nullptr;
    explicit operator bool () const { return id != 0; }
};
/// \brief A class that stores the location/name of the uniform, and knows how to upload it to GL
template<class T>
struct ShaderUniform
{
    GLint id = -2;
    T * cpu_bound_value=nullptr;
    bool setup(uint32_t program, const char *name,T *val)
    {
        int loc = glGetUniformLocation(program, name);
        if (loc < 0)
        {
            printf("Missing uniform '%s'\n", name);
            id = -1;
        }
        else
            id = loc;
        cpu_bound_value = val;
        return !invalid();
    }
    bool invalid() const { assert(id != -2); return id == -1; }
    void upload() {
        assert(cpu_bound_value);
        upload(*cpu_bound_value);
    }
    void upload(T v);
    template<int N>
    void upload(const std::array<T,N> &v)
    {
        if (invalid())
            return;
        GLint back_id = id;
        for(const auto & entry : v)
        {
            upload(entry);
            id++;
        }
        id = back_id;
    }
    bool operator!=(T v) const
    {
        return *cpu_bound_value != v;
    }
    ShaderUniform<T> & operator=(T v)
    {
        if(*cpu_bound_value==v)
            return *this;
        *cpu_bound_value = v;
        upload();
        return *this;
    }
    void assign(T v)
    {
        *cpu_bound_value = v;
        upload();
    }
};
template<>
inline void ShaderUniform<Vector4>::upload(Vector4 v)
{
    if (invalid())
    {
        return;
    }
    glUniform4fv(id, 1, v.data());
}
template<>
inline void ShaderUniform<Matrix4x3>::upload(Matrix4x3 v)
{
    if (invalid())
    {
        return;
    }
    Matrix4x4 glmat = v;
    glUniformMatrix4fv(id, 1,false, glmat.data());
}
template<>
inline void ShaderUniform<Vector3>::upload(Vector3 v)
{
    if (invalid())
    {
        return;
    }
    glUniform3f(id, v.x, v.y, v.z);
}
template<>
inline void ShaderUniform<int>::upload(int v)
{
    if (invalid())
    {
        return;
    }
    glUniform1i(id, v);
}
template<>
inline void ShaderUniform<glm::mat4>::upload(glm::mat4 v)
{
    if (invalid())
    {
        return;
    }
    glUniformMatrix4fv(id, 1, false, glm::value_ptr(v));
}

struct ShaderProgram
{
    friend struct ShaderProgramCache;
    ShaderObject vertex_shader = {0};
    ShaderObject fragment_shader = {0};
    GLuint gl_id = ~0U;

    InstanceDrawData m_local_state;
    std::vector<uint32_t> required_units;
    std::set<const char *> required_attributes;
    std::unordered_map<const char *,uint32_t> enabled_attributes;
    int getUniformIdx(const char *name) const
    {
        return glGetUniformLocation(gl_id, name);
    }
    int attribLocation(const char *name) const
    {
        return glGetAttribLocation(gl_id, name);
    }
    ////////////////////////////////////////////
    /// vertex shader attributes block
    ////////////////////////////////////////////
    GLuint vertexPos_Location = ~0U;
    GLuint vertexNormal_Location = ~0U;
    GLuint vertexUV0_Location = ~0U;
    GLuint vertexUV1_Location = ~0U;
    GLuint vertexColor_Location = ~0U;
    GLuint boneWeights_Location = ~0U;
    GLuint boneIndices_Location = ~0U;
    GLuint tangent_Location = ~0U;
private:
    ////////////////////////////////////////////
    /// vertex shader uniform interface block
    ////////////////////////////////////////////
    struct LightParams
    {
        ShaderUniform<int> State;
        ShaderUniform<Vector4> Diffuse;
        ShaderUniform<Vector4> Ambient;
        ShaderUniform<Vector4> Specular;
        ShaderUniform<Vector4> Position; // w ==0 => directional light,
    };
    struct FogParams
    {
        ShaderUniform<int> mode;
        ShaderUniform<Vector4> color;
        ShaderUniform<Vector4> v_block;
    };
    ShaderUniform<glm::mat4> projectionMatrix;
    ShaderUniform<glm::mat4> modelViewMatrix;
    LightParams light0;
    ShaderUniform<Vector4> lightsOn;
    FogParams fog_params;
    ShaderUniform<Vector4> globalColor;
    ShaderUniform<Vector3> viewerPosition;
    // optional uniforms
    ShaderUniform<Vector4> textureScroll; // xy - scroll for Texture 0, zw scroll for Texture 1
    ShaderUniform<glm::mat4> textureMatrix0;
    ShaderUniform<glm::mat4> textureMatrix1;
    ShaderUniform<std::array<Vector4,48>> boneMatrices; // mat 3x4 - 16 matrices

    ////////////////////////////////////////////
    /// Fragment shader interface
    ////////////////////////////////////////////
    ShaderUniform<Vector4> constColor1;
    ShaderUniform<Vector4> constColor2;
    ShaderUniform<Vector4> glossFactor;

    void setupAttributes();
public:
    void disableAllEnabled() const
    {
#define CHECK_AND_DISABLE_ATTR(name)  if(~0U != name ## _Location)  glDisableVertexAttribArray(name ## _Location);
        CHECK_AND_DISABLE_ATTR(vertexPos);
        CHECK_AND_DISABLE_ATTR(vertexNormal);
        CHECK_AND_DISABLE_ATTR(vertexUV0);
        CHECK_AND_DISABLE_ATTR(vertexUV1);
        CHECK_AND_DISABLE_ATTR(vertexColor);
        CHECK_AND_DISABLE_ATTR(boneWeights);
        CHECK_AND_DISABLE_ATTR(boneIndices);
        CHECK_AND_DISABLE_ATTR(tangent);
#undef CHECK_AND_DISABLE_ATTR
    }
    void uploadUniforms(InstanceDrawData &matdef);
    void forceUploadUniforms(InstanceDrawData &instance_data);
    void setupUniforms();
    void use()
    {
        glUseProgram(gl_id);
    }
};

struct ShaderProgramCache
{
    std::unordered_map<MaterialDefinition, ShaderProgram> m_cache_data;
    ShaderProgram &getOrCreateProgram(MaterialDefinition &mat_def);
};
extern ShaderProgramCache g_program_cache;
