#pragma once
#include "GL/glew.h"
#include "utils/helpers.h"

#include <stdint.h>

struct ShaderProgram
{
    GLuint vertex_id = 0;
    GLuint shader_id = 0;
    GLuint gl_id = ~0U;
    int getUniformIdx(const char *name) const
    {
        return glGetUniformLocation(gl_id, name);
    }
    void setUniform(const char *name,Vector4 v)
    {
        int loc = getUniformIdx(name);
#ifdef _DEBUG
        if(loc==-1)
        {
            printfDebug("Missing uniform %s\n", name);
        }
#endif
        glUniform4f(loc, v.x, v.y, v.z, v.w);
    }
    void setUniform(const char *name, Vector3 v)
    {
        int loc = getUniformIdx(name);
#ifdef _DEBUG
        if (loc == -1)
        {
            printfDebug("Missing uniform %s\n", name);
        }
#endif
        glUniform3f(loc, v.x, v.y, v.z);
    }
    int attribLocation(const char *name) const
    {
        return glGetAttribLocation(gl_id, name);
    }
    void setVertexAttribPtr(const char *name,int components,void *ptr)
    {
        int loc = attribLocation(name);
#ifdef _DEBUG
        if (loc == -1)
        {
            printfDebug("Missing vertex attribute %s\n", name);
        }
#endif
        glEnableVertexAttribArrayARB(loc);
        glVertexAttribPointerARB(loc, components, GL_FLOAT, 0, 0, ptr);
    }
    void setVertexAttribBuffer(const char *name, uint32_t buffer, int components,uint32_t type, bool normalize,int offset)
    {
        uint32_t buff_id;
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, (int *)&buff_id);
        int loc = attribLocation(name);
#ifdef _DEBUG
        if (loc == -1)
        {
            printfDebug("Missing vertex attribute %s\n", name);
        }
#endif
        if (buffer != buff_id)
            glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glEnableVertexAttribArrayARB(loc);
        glVertexAttribPointerARB(loc, components, type, normalize ? 1:0, 0, (void *)offset);
        if(buffer!=buff_id)
            glBindBuffer(GL_ARRAY_BUFFER,buff_id);
    }
    void disableAttrib(const char *name)
    {
        int loc = attribLocation(name);
#ifdef _DEBUG
        if (loc == -1)
            return;
#endif
        glDisableVertexAttribArrayARB(loc);

    }
};

struct ShaderProgramCache
{
    ShaderProgram m_linked_programs[32];
    ShaderProgram m_previous_program;
    ShaderProgram m_current_program;
    void setProgram(GLuint vertex_shader, GLuint fragment_shader);
    void setVertexProgram(GLuint vertex_shader)
    {
        setProgram(vertex_shader, m_current_program.shader_id);
    }
    void setFragmentProgram(GLuint fragment_shader)
    {
        setProgram(m_current_program.vertex_id, fragment_shader);
    }

    void disableVertexShader();

    void enableVertexShader()
    {
        if (m_previous_program.vertex_id)
            setVertexProgram(m_previous_program.vertex_id);
    }
};
extern ShaderProgramCache g_program_cache;