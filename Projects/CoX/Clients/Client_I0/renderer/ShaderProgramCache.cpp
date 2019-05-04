#include "Model.h"
#include "ShaderProgramCache.h"
#include  "RendererState.h"

#include <cstdio>
#include <vector>
#include <cstring>
#include <string>
#include "RendererUtils.h"
#include <windows.h>

ShaderProgramCache g_program_cache;

static std::string addDefines(const char *baseText, const std::vector<std::string> &defines)
{
    std::string temp;
    const char *location = strstr(baseText, "#version");
    const char *eol_location = strchr(location, '\n');
    temp.assign(baseText, eol_location + 1);
    for (const std::string &def : defines)
    {
        temp += "#define " + def + "\n";
    }
    temp.append(eol_location + 1);
    return temp;
}
static void compileShader(const char *filename, ShaderObject &object, const std::vector<std::string> &defines = {})
{
    assert(filename);
    assert(bool(object) != false);

    object.source_file = filename;

    FILE *fp = fopen(filename, "rb");
    if (!fp)
    {
        printfDebug("Error loading \"%s\" file is missing\n", filename);
        return;
    }

    fseek(fp, 0, SEEK_END);
    unsigned shader_size = ftell(fp);
    std::string string;
    string.resize(shader_size);
    if (shader_size == 0)
    {
        printfDebug("Error loading \"%s\"\n", filename);
        return;
    }
    fseek(fp, 0, SEEK_SET);
    if (fread(string.data(), 1, shader_size, fp) != shader_size)
    {
        printfDebug("Error loading \"%s\"\n", filename);
        fclose(fp);
        return;
    }
    std::string extended = addDefines(string.c_str(), defines);
    shader_size = extended.size();
    const char *parts[] = { extended.c_str() };
    const int sizes[] = { (int)shader_size };
    glShaderSource(object.id, 1, parts, sizes);
    glCompileShader(object.id);
    GLint isCompiled = 0;
    glGetObjectParameterivARB(object.id, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled != GL_FALSE)
    {
        fclose(fp);
        return;
    }
    GLint maxLength = 0;
    glGetShaderiv(object.id, GL_INFO_LOG_LENGTH, &maxLength);
    // The maxLength includes the NULL character
    std::string errorLog;
    errorLog.resize(maxLength);
    glGetShaderInfoLog(object.id, maxLength, &maxLength, &errorLog[0]);
    fprintf(stderr, "%s shader compilation failed: %s", filename, errorLog.data());
}

ShaderProgram &ShaderProgramCache::getOrCreateProgram(MaterialDefinition &mat_def)
{
    static char s_buf[512]={0};
    ShaderProgram *prev_program = MaterialDefinition::last_applied ? MaterialDefinition::last_applied->program : nullptr;

    //find program in cache
    auto iter = m_cache_data.find(mat_def);
    if(iter!= m_cache_data.end())
    {
        if (prev_program == &iter->second)
            return *prev_program;
        mat_def.program = &iter->second;
        glUseProgram(mat_def.program->gl_id);
        return iter->second;
    }

    std::vector<std::string> defines;
    const char *fs_source_name = "shaders/unified_shader.frag";

    snprintf(s_buf, 512, "COLORBLEND %d", mat_def.get_colorBlending());
    defines.emplace_back( s_buf );
    snprintf(s_buf, 512, "ALPHA_DISCARD_MODE %d", mat_def.get_useAlphaDiscard());
    defines.emplace_back(s_buf);
    snprintf(s_buf, 512, "USE_STIPPLING %d", mat_def.get_useStippling());
    defines.emplace_back(s_buf);
    snprintf(s_buf, 512, "LIGHT_MODE %d", mat_def.get_lightMode());
    defines.emplace_back(s_buf);
    snprintf(s_buf, 512, "BUMP_MODE %d", mat_def.get_bumpMapMode());
    defines.emplace_back(s_buf);
    snprintf(s_buf, 512, "REFLECTION_MODE_T0 %d", mat_def.get_reflectionMode() & MaterialDefinition::REFLECTION_AUTOGEN_UV0);
    defines.emplace_back(s_buf);
    snprintf(s_buf, 512, "REFLECTION_MODE_T1 %d", mat_def.get_reflectionMode() & MaterialDefinition::REFLECTION_AUTOGEN_UV1);
    defines.emplace_back(s_buf);
    snprintf(s_buf, 512, "TEXTURES %d", mat_def.get_texUnits());
    defines.emplace_back(s_buf);
    snprintf(s_buf, 512, "VERTEX_PROCESSING %d", mat_def.get_vertexMode());
    defines.emplace_back(s_buf);
    snprintf(s_buf, 512, "FRAGMENT_MODE %d", mat_def.get_fragmentMode());
    defines.emplace_back(s_buf);
    snprintf(s_buf, 512, "TC_XFORM %d", mat_def.get_texTransform());
    defines.emplace_back(s_buf);
    snprintf(s_buf, 512, "USE_CUBEMAP %d", mat_def.get_usingCubemaps());
    defines.emplace_back(s_buf);
    snprintf(s_buf, 512, "LOD_ALPHA %d", mat_def.get_useLodAlpha() ? 1 : 0);
    defines.emplace_back(s_buf);
    snprintf(s_buf, 512, "FLAT_SHADED %d", mat_def.get_shadeModel() == MaterialDefinition::FLAT);
    defines.emplace_back(s_buf);
    snprintf(s_buf, 512, "LIGHT_SPACE %d", mat_def.get_light_space());
    defines.emplace_back(s_buf);
    if (mat_def.get_useTransformFeedback())
        defines.emplace_back("TRANSFORM_FEEDBACK");

    ShaderObject frag_shader;
    frag_shader.id = glCreateShader(GL_FRAGMENT_SHADER_ARB);
    snprintf(s_buf, 256, "FS_%x", mat_def.hash_val());
    glObjectLabel(GL_SHADER, frag_shader.id, -1, s_buf);
    compileShader(fs_source_name, frag_shader,defines);

    ShaderObject vert_shader;
    vert_shader.id = glCreateShader(GL_VERTEX_SHADER_ARB);
    snprintf(s_buf, 256, "VS_%x", mat_def.hash_val());
    glObjectLabel(GL_SHADER, vert_shader.id, -1, s_buf);

    compileShader("shaders/unified_shader.vert", vert_shader, defines);

    GLuint program_id = glCreateProgram();
    char buffer[128];
    snprintf(buffer,128, "PROG_%x", mat_def.hash_val());
    glObjectLabel(GL_PROGRAM, program_id, strlen(buffer), buffer);
    ShaderProgram new_program;
    new_program.vertex_shader = vert_shader;
    new_program.fragment_shader = frag_shader;
    new_program.gl_id = program_id;

    new_program.required_attributes.insert("POS");
    if (mat_def.get_lightMode()>1)
        new_program.required_attributes.insert("NORMAL");
    if(mat_def.get_colorSource()>0)
        new_program.required_attributes.insert("PERVERTEXCOLORS");
    if(mat_def.get_texUnits()>0)
        new_program.required_attributes.insert("UV0");
    if (mat_def.get_texUnits()>1)
        new_program.required_attributes.insert("UV1");
    if(mat_def.get_vertexMode()==1)
        new_program.required_attributes.insert({ "BONEWEIGHTS","BONEINDICES"});
    if(mat_def.get_bumpMapMode()!=MaterialDefinition::BumpMode::NONE)
        new_program.required_attributes.insert("TANGENTS");
    for (int i = 0; i<mat_def.get_texUnits(); ++i)
        new_program.required_units.emplace_back(i);
    //bindLocations(m_current_program.gl_id,{0,1,2,5,6,7,11});

    glAttachShader(program_id, vert_shader.id);
    glAttachShader(program_id, frag_shader.id);
    if(mat_def.get_useTransformFeedback())
    {
        const GLchar* feedbackVaryings[] = { "outValue" };
        glTransformFeedbackVaryings(program_id, 1, feedbackVaryings, GL_INTERLEAVED_ATTRIBS);
    }
    glLinkProgram(program_id);
    GLint linked;
    glGetObjectParameterivARB(program_id, GL_OBJECT_LINK_STATUS_ARB, &linked);
    if (!linked) {
        GLint length;
        glGetObjectParameterivARB(program_id, GL_OBJECT_INFO_LOG_LENGTH_ARB, &length);
        std::string infoLog;
        infoLog.resize(length);
        glGetInfoLogARB(program_id, length, nullptr, infoLog.data());
        printfDebug("%s\n", infoLog.data());
        printf(infoLog.data());
        assert(false);
    }

    GLint baseImageLoc = glGetUniformLocation(program_id, "myTextureMap");
    GLint modMapLoc    = glGetUniformLocation(program_id, "myTextureMap2");
    GLint normalMapLoc = glGetUniformLocation(program_id, "myTextureMap3");
    glUseProgram(program_id);
    glUniform1i(baseImageLoc, 0); //Texture unit 0 is for base images.
    glUniform1i(modMapLoc, 1); //Texture unit 1 is for alpha/color maps.
    if (normalMapLoc != -1)
        glUniform1i(normalMapLoc, 2); //Texture unit 2 is for normal maps.
    m_cache_data[mat_def] = new_program;
    printf("Uniform setup for shader:\n    ");
    for(const std::string & def : defines)
    {
        printf("%s, ",def.c_str());
    }
    printf("\n");
    m_cache_data[mat_def].setupUniforms();
    m_cache_data[mat_def].setupAttributes();

    return m_cache_data[mat_def];
}

#define CREATE_NAMED_VP_SHADER(name)\
    name.id = glCreateShader(GL_VERTEX_SHADER_ARB);\
    glObjectLabel(GL_SHADER,name.id,strlen(#name),#name);

void GeometryData::prepareDraw(const ShaderProgram& drawprog)
{
    GLDebugGuard guard(__FUNCTION__);
    assert(segs_data->vao_id > 0);
    glBindVertexArray(segs_data->vao_id);
    glUseProgram(drawprog.gl_id);
    if(segs_data->color_buffer_changed && segs_data->bound_attributes == &drawprog)
    {
        if (drawprog.vertexColor_Location != ~0U)
        {
            assert(segs_data->color_buffer.buffer_id < 10000); // dumb check for pointers passed to color_buffer
            if (segs_data->color_buffer.buffer_id == ~0U)
            {
                glDisableVertexAttribArray(drawprog.vertexColor_Location);
                glVertexAttrib4f(drawprog.vertexColor_Location, 1, 1, 1, 1);
            }
            else
            {
                glBindBuffer(GL_ARRAY_BUFFER, segs_data->color_buffer.buffer_id);
                glVertexAttribPointer(drawprog.vertexColor_Location, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0,
                                      (void *)segs_data->color_buffer.offset);
                glEnableVertexAttribArray(drawprog.vertexColor_Location);
            }
        }
        segs_data->color_buffer_changed = false;
    }
    if(segs_data->vertex_buffer_changed && segs_data->bound_attributes == &drawprog)
    {
        glBindBuffer(GL_ARRAY_BUFFER, gl_vertex_buffer);

        glVertexAttribPointer(drawprog.vertexPos_Location, 3, GL_FLOAT, GL_FALSE, 0, vertices);
        glEnableVertexAttribArray(drawprog.vertexPos_Location);
        segs_data->vertex_buffer_changed = false;
    }
    if (segs_data->bound_attributes == &drawprog)
        return;

    if (segs_data->bound_attributes)
        segs_data->bound_attributes->disableAllEnabled();

    glBindBuffer(GL_ARRAY_BUFFER, gl_vertex_buffer);

    glVertexAttribPointer(drawprog.vertexPos_Location, 3, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(drawprog.vertexPos_Location);

    if (drawprog.vertexNormal_Location != ~0U)
    {
        if (normals_offset == vertices)
        {
            glDisableVertexAttribArray(drawprog.vertexColor_Location);
        }
        else
        {
            glVertexAttribPointer(drawprog.vertexNormal_Location, 3, GL_FLOAT, GL_FALSE, 0, normals_offset);
            glEnableVertexAttribArray(drawprog.vertexNormal_Location);
        }
    }
    uint32_t uv_base_offset_in_buffer = segs_data->uv_vbo.offset;
    if (segs_data->uv_vbo.buffer_id != ~0U)
        glBindBuffer(GL_ARRAY_BUFFER, segs_data->uv_vbo.buffer_id);
    if (drawprog.vertexUV0_Location != ~0U)
    {
        glVertexAttribPointer(drawprog.vertexUV0_Location, 2, GL_FLOAT, GL_FALSE, 0, uv_base_offset_in_buffer + uv1_offset);
        glEnableVertexAttribArray(drawprog.vertexUV0_Location);
    }
    if (drawprog.vertexUV1_Location != ~0U)
    {
        glVertexAttribPointer(drawprog.vertexUV1_Location, 2, GL_FLOAT, GL_FALSE, 0, uv_base_offset_in_buffer + uv1_offset);
        glEnableVertexAttribArray(drawprog.vertexUV1_Location);
    }
    if (segs_data->uv_vbo.buffer_id != ~0U)
        glBindBuffer(GL_ARRAY_BUFFER, gl_vertex_buffer);

    if (drawprog.vertexColor_Location != ~0U)
    {
        if(segs_data->color_buffer.buffer_id == ~0U)
        {
            glDisableVertexAttribArray(drawprog.vertexColor_Location);
            glVertexAttrib4f(drawprog.vertexColor_Location, 1, 1, 1, 1);
        }
        else
        {
            assert(segs_data->color_buffer.buffer_id < 10000); // dumb check for pointers passed to color_buffer
            if(segs_data->color_buffer.buffer_id!=gl_vertex_buffer)
                glBindBuffer(GL_ARRAY_BUFFER, segs_data->color_buffer.buffer_id);
            glVertexAttribPointer(drawprog.vertexColor_Location, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0,
                              (void *)segs_data->color_buffer.offset);
            glEnableVertexAttribArray(drawprog.vertexColor_Location);
            if (segs_data->color_buffer.buffer_id != gl_vertex_buffer)
                glBindBuffer(GL_ARRAY_BUFFER, gl_vertex_buffer);
        }
    }
    if (drawprog.boneWeights_Location != ~0U)
    {
        glVertexAttribPointer(drawprog.boneWeights_Location, 2, GL_FLOAT, GL_FALSE, 0, weights);
        glEnableVertexAttribArray(drawprog.boneWeights_Location);
    }
    if (drawprog.boneIndices_Location != ~0U)
    {
        glVertexAttribPointer(drawprog.boneIndices_Location, 2, GL_SHORT, GL_FALSE, 0, boneIndices);
        glEnableVertexAttribArray(drawprog.boneIndices_Location);
    }
    if (drawprog.tangent_Location != ~0U)
    {
        glVertexAttribPointer(drawprog.tangent_Location, 4, GL_FLOAT, GL_FALSE, 0, tangents_directions);
        glEnableVertexAttribArray(drawprog.tangent_Location);
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_index_buffer);

    segs_data->bound_attributes = &drawprog;
}
void debugVAOState(std::string baseMessage)
{
    baseMessage.append(" ... querying VAO state:\n");
    int vab, eabb, eabbs, mva, isOn(1), vaabb;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vab);
    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &eabb);
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &eabbs);

    baseMessage.append("  VAO: " + std::to_string(vab) + "\n");
    baseMessage.append("  IBO: " + std::to_string(eabb) + ", size=" + std::to_string(eabbs) + "\n");

    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &mva);
    for (unsigned i = 0; i < mva; ++i)
    {
        glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &isOn);
        if (isOn)
        {
            glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &vaabb);
            GLint size_vbo;
            glGetNamedBufferParameteriv(vaabb, GL_BUFFER_SIZE, &size_vbo);

            baseMessage.append("  attrib #" + std::to_string(i) + ": VBO=" + std::to_string(vaabb) + "size="+std::to_string(size_vbo)+"\n");
        }
    }
    OutputDebugString(baseMessage.c_str());
}
void GeometryData::draw(const ShaderProgram &drawprog,GLenum drawtype,size_t count,size_t offset)
{
    prepareDraw(drawprog);
    //debugVAOState("GeometryData::draw");
    glDrawElements(drawtype, count, GL_UNSIGNED_INT, (void *)(offset*4));   
    glBindVertexArray(0);

}
void GeometryData::drawArray(const ShaderProgram &drawprog, GLenum drawtype,size_t count, size_t offset)
{
    prepareDraw(drawprog);
    glDrawArrays(drawtype, offset, count);
    glBindVertexArray(0);
}

void ShaderProgram::setupAttributes()
{
    GLDebugGuard guard(__FUNCTION__);
#define LOOKUP_ATTRIBUTE(name) name ## _Location = GLuint(attribLocation(#name));
    LOOKUP_ATTRIBUTE(vertexPos);
    LOOKUP_ATTRIBUTE(vertexNormal);
    LOOKUP_ATTRIBUTE(vertexUV0);
    LOOKUP_ATTRIBUTE(vertexUV1);
    LOOKUP_ATTRIBUTE(vertexColor);
    LOOKUP_ATTRIBUTE(boneWeights);
    LOOKUP_ATTRIBUTE(boneIndices);
    LOOKUP_ATTRIBUTE(tangent);
#undef LOOKUP_ATTRIBUTE
}
void ShaderProgram::uploadUniforms(InstanceDrawData &instance_data)
{
    projectionMatrix = instance_data.projectionMatrix;
    modelViewMatrix = instance_data.modelViewMatrix;
    constColor1 = instance_data.constColor1;
    assert(instance_data.constColor2.w == 1);
    constColor2 = instance_data.constColor2;
    globalColor = instance_data.globalColor;

    fog_params.mode = instance_data.fog_params.getMode();
    if(instance_data.fog_params.enabled)
    {
        fog_params.color = instance_data.fog_params.color;
        fog_params.v_block  = instance_data.fog_params.v_block;
    }
    light0.State = instance_data.light0.State;
    light0.Ambient = instance_data.light0.Ambient;
    light0.Diffuse = instance_data.light0.Diffuse;
    light0.Position = instance_data.light0.Position;
    textureScroll   = instance_data.textureScroll;
    textureMatrix0 = instance_data.textureMatrix0;
    textureMatrix1  = instance_data.textureMatrix1;
}
void ShaderProgram::forceUploadUniforms(InstanceDrawData &instance_data)
{
    projectionMatrix.assign(instance_data.projectionMatrix);
    modelViewMatrix.assign(instance_data.modelViewMatrix);
    constColor1.assign(instance_data.constColor1);
    assert(instance_data.constColor2.w == 1);
    constColor2.assign(instance_data.constColor2);
    globalColor.assign(instance_data.globalColor);
    fog_params.mode.assign(instance_data.fog_params.getMode());
    if (instance_data.fog_params.enabled)
    {
        fog_params.color.assign(instance_data.fog_params.color);
        fog_params.v_block.assign(instance_data.fog_params.v_block);
    }
    light0.State.assign(instance_data.light0.State);
    light0.Ambient.assign(instance_data.light0.Ambient);
    light0.Diffuse.assign(instance_data.light0.Diffuse);
    light0.Position.assign(instance_data.light0.Position);
    textureScroll.assign(instance_data.textureScroll);
    textureMatrix0.assign(instance_data.textureMatrix0);
    textureMatrix1.assign(instance_data.textureMatrix1);
}
void ShaderProgram::setupUniforms()
{
#define SETUP_UNIFORM(uniform_name) uniform_name .setup(gl_id, #uniform_name,&m_local_state. uniform_name)
    SETUP_UNIFORM(modelViewMatrix);
    SETUP_UNIFORM(projectionMatrix);
    SETUP_UNIFORM(light0.State);
    SETUP_UNIFORM(light0.Diffuse);
    SETUP_UNIFORM(light0.Specular);
    SETUP_UNIFORM(light0.Ambient);
    SETUP_UNIFORM(light0.Position);
    SETUP_UNIFORM(lightsOn);
    SETUP_UNIFORM(fog_params.mode);
    SETUP_UNIFORM(fog_params.color);
    SETUP_UNIFORM(fog_params.v_block);
    SETUP_UNIFORM(viewerPosition);
    SETUP_UNIFORM(globalColor);
    SETUP_UNIFORM(textureScroll);
    SETUP_UNIFORM(textureMatrix0);
    SETUP_UNIFORM(textureMatrix1);
    //    boneMatrices.setup(gl_id, "boneMatrices",m_local_state.boneMatrices.get());

    SETUP_UNIFORM(constColor1);
    SETUP_UNIFORM(constColor2);
    SETUP_UNIFORM(glossFactor);
#undef SETUP_UNIFORM

}
