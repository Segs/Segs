#pragma once
#include "utils/helpers.h"
#include "GL/glew.h"

#include <stdint.h>
#include <vector>
struct TextureBind;
struct TrickNode;
struct SunLight
{
    Vector4 diffuse;
    Vector4 ambient;
    Vector4 ambient_for_players;
    Vector4 diffuse_for_players;
    Vector4 no_angle_light;
    Vector4 direction;
    Vector4 direction_in_viewspace; // this is precomputed at the start of each frame as  cam_info.viewmat * direction
    Vector3 shadow_direction;
    Vector3 shadow_direction_in_viewspace;
    float   gloss_scale;
    Vector3 position;
    float   lamp_alpha;
    Vector4 shadowcolor;
};
struct SmallAnimSRel
{
    int      compressed_size;
    uint32_t uncomp_size;
    char *   compressed_data;
};

struct ShaderProgram;
struct VBODataSource
{
    GLuint buffer_id;
    GLuint offset;
    bool   operator==(VBODataSource other) const { return buffer_id == other.buffer_id && offset == other.offset; }
};
struct GeometryData
{
    friend void segs_modelSetupVertexObject(struct Model *model, int useVbos);

    struct SegsGeometryData
    {
        GLuint               vao_id                 = 0;
        VBODataSource        color_buffer           = {~0U, 0};
        VBODataSource        uv_vbo                 = {~0U, 0};
        const ShaderProgram *bound_attributes       = nullptr;
        uint8_t              attributes_were_set_up : 1;
        uint8_t              color_buffer_changed : 1;
        uint8_t              vertex_buffer_changed : 1;
        uint8_t              we_own_color_buffer : 1;
        uint8_t              we_own_vertex_buffer : 1;
        uint8_t              we_own_index_buffer : 1;
        SegsGeometryData()
        {
            attributes_were_set_up = color_buffer_changed = vertex_buffer_changed = we_own_color_buffer =
                we_own_vertex_buffer = we_own_index_buffer = false;
        }

        ~SegsGeometryData()
        {
            if (we_own_color_buffer)
                glDeleteBuffers(1, &color_buffer.buffer_id);
        }
    };
    void setVertexBuffer(GLuint buf)
    {
        if (gl_vertex_buffer != buf)
        {
            segs_data->vertex_buffer_changed = true;
            gl_vertex_buffer                 = buf;
        }
    }
    void setColorBuffer(VBODataSource colorbuf)
    {
        assert(colorbuf.buffer_id!=0);
        segs_data->color_buffer_changed = !(colorbuf == segs_data->color_buffer);
        segs_data->color_buffer         = colorbuf;
    }
    void setUVVbo(GLuint bufname)
    {
        assert(bufname);
        segs_data->uv_vbo.buffer_id = bufname;
        segs_data->uv_vbo.offset    = 0;
        uv1_offset                  = nullptr;
        uv2_offset                  = nullptr;
    }
    void uploadColorsToBuffer(uint8_t *colors, size_t count)
    {
        assert(segs_data);
        if (!segs_data->we_own_color_buffer)
        {
            glGenBuffers(1, &segs_data->color_buffer.buffer_id);
            segs_data->we_own_color_buffer  = true;
            segs_data->color_buffer_changed = true;
        }
        glBindBuffer(GL_ARRAY_BUFFER, segs_data->color_buffer.buffer_id);
        segs_data->color_buffer.offset = 0;
        glBufferData(GL_ARRAY_BUFFER, count, colors, GL_DYNAMIC_DRAW);
    }
    void uploadVerticesToBuffer(float *data, size_t count, GLenum usage = GL_DYNAMIC_DRAW,const char *name = nullptr)
    {
        if (!segs_data->we_own_vertex_buffer)
        {
            glGenBuffers(1, &gl_vertex_buffer);
            if (name)
                glObjectLabel(GL_BUFFER, gl_vertex_buffer, -1, name);
            segs_data->we_own_vertex_buffer  = true;
            segs_data->vertex_buffer_changed = true;
        }
        glBindBuffer(GL_ARRAY_BUFFER, gl_vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, count * sizeof(float), data, usage);
        vertices = nullptr;
    }
    void createSizedVBO(size_t count,const char *name=nullptr)
    {
        assert(gl_vertex_buffer == 0);
        if (!segs_data->we_own_vertex_buffer)
        {
            glGenBuffers(1, &gl_vertex_buffer);
            if (name)
                glObjectLabel(GL_BUFFER, gl_vertex_buffer, -1, name);
            segs_data->we_own_vertex_buffer  = true;
            segs_data->vertex_buffer_changed = true;
        }
        glBindBuffer(GL_ARRAY_BUFFER, gl_vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, count * sizeof(float), 0, GL_DYNAMIC_DRAW);
    }
    void uploadSubBufferToVertices(float *data, size_t count, size_t offset)
    {
        glBindBuffer(GL_ARRAY_BUFFER, gl_vertex_buffer);
        glBufferSubData(GL_ARRAY_BUFFER, offset, count * sizeof(float), data);
    }
    void uploadIndicesToBuffer(const uint32_t *data, size_t count, const char *name = nullptr,
                               GLenum draw_type = GL_DYNAMIC_DRAW)
    {
        // assert(gl_index_buffer == 0);
        if (!segs_data->we_own_index_buffer)
        {
            glGenBuffers(1, &gl_index_buffer);
            if (name)
                glObjectLabel(GL_BUFFER, gl_index_buffer, -1, name);
            segs_data->we_own_index_buffer = true;
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_index_buffer);
        if (count)
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), data, draw_type);
        vertices = nullptr;
    }
    ~GeometryData()
    {
        if (segs_data)
        {
            if (segs_data->we_own_vertex_buffer)
                glDeleteBuffers(1, &gl_vertex_buffer);
            if (segs_data->we_own_index_buffer)
                glDeleteBuffers(1, &gl_index_buffer);
            glDeleteVertexArrays(1, &segs_data->vao_id);
        }
        delete segs_data;
    }
    void               setPosOffsetInVbo(intptr_t offset) { vertices = (Vector3 *)offset; }
    GLuint             gl_index_buffer     = 0;
    Vector3i *         triangles           = nullptr;
    GLuint             gl_vertex_buffer    = 0;
    Vector3 *          vertices            = nullptr;
    Vector3 *          normals_offset      = nullptr;
    Vector2 *          uv1_offset          = nullptr;
    Vector2 *          uv2_offset          = nullptr;
    Vector4 *          tangents_directions = nullptr; // packed tangent and biormal direction
    SegsGeometryData * segs_data           = nullptr;
    Vector2 *          weights             = nullptr;
    Vector2si *        boneIndices         = nullptr;
    struct ShadowInfo *shadow              = nullptr;
    TextureBind **     textureP_arr        = nullptr;
    float *            cpuside_memory      = nullptr;
    int                frame_id            = 0;
    void               createVAO()
    {
        if (!segs_data)
        {
            segs_data = new SegsGeometryData;
            glGenVertexArrays(1, &segs_data->vao_id);
        }
    }
    void prepareDraw(const ShaderProgram &drawprog);
    void draw(const ShaderProgram &drawprog, GLenum drawtype, size_t count, size_t offset);
    void drawArray(const ShaderProgram &drawprog, GLenum drawtype, size_t count, size_t offset);
};
static_assert(sizeof(GeometryData) >= 0x3C);
struct TriangleTextureBind
{
    uint16_t tex_idx;
    uint16_t tri_count;
};
enum ModelState
{
    LOADED = 4,
};
struct PackNames
{
    char **strings;
    int    count;
};
struct ModelHeader
{
    char             name[124];
    struct AnimList *parent_anim;
    float            length;
    struct Model *   models;
    int              num_subs;
};

struct AnimList
{
    AnimList *          next;
    AnimList *          prev;
    int                 header_count;
    struct ModelHeader *headers;
    char                name[128];
    PackNames           texnames;
    int                 headersize;
    int                 datasize;
    int                 loadstate;
    float               lasttimeused;
    int                 type;
    struct XFileHandle *file;
    int                 tex_load_style;
    int                 geo_use_type;
    void *              header_data;
    void *              geo_data;
};
static_assert(sizeof(AnimList) == 0xC0);
struct PolyGridEntry
{
    Vector3 pos;
    float   size;
    float   size_inv;
    int     tag;
    int     bitcount;
};
enum ModelFlags : uint32_t
{
    OBJ_ALPHASORT      = 0x1,
    OBJ_FULLBRIGHT     = 0x4,
    OBJ_NOLIGHTANGLE   = 0x10,
    OBJ_DUALTEXTURE    = 0x40,
    OBJ_LOD            = 0x80,
    OBJ_TREE           = 0x100,
    OBJ_DUALTEX_NORMAL = 0x200,
    OBJ_FORCEOPAQUE    = 0x400,
    OBJ_BUMPMAP        = 0x800,
    OBJ_WORLDFX        = 0x1000,
    OBJ_CUBEMAP        = 0x2000,
    OBJ_DRAW_AS_ENT    = 0x4000,
    OBJ_STATICFX       = 0x8000,
    OBJ_HIDE           = 0x10000,
};
struct Model
{
    uint32_t                Model_flg1;
    float                   model_VisSphereRadius;
    GeometryData *          vbo;
    int                     num_textures;
    int16_t                 u_10;
    char                    blendState;
    char                    loadstate;
    struct BoneInfo *       boneinfo;
    TrickNode *             trck_node;
    int                     vertex_count;
    int                     model_tri_count;
    TriangleTextureBind *   texture_bind_offsets;
    struct UnpackedModel_P *unpacked_1;
    PolyGridEntry           gird;
    struct ModelCtri *      ctris;
    int *                   triangle_tags;
    char *                  bone_name_offset;
    struct AltPivot *       altpivots;
    struct ModelExtra *     extra;
    Vector3                 float8;
    Vector3                 float11;
    Vector3                 float14;
    struct AnimList *       parent_anim;
    SmallAnimSRel           sasr[7];
};
extern SunLight g_sun;
