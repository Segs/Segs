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
  int compressed_size;
  uint32_t uncomp_size;
  char *compressed_data;
};

struct GeometryData
{
    GLuint gl_index_buffer;
    Vector3i *triangles;
    GLuint gl_vertex_buffer;
    Vector3 *vertices;
    Vector3 *normals_vbo_idx;
    Vector2 *tex_uv1;
    Vector2 *tex_uv2;
    Vector3 *tangents;
    Vector3 *binormals;
    Vector2 *weights;
    Vector2si *boneIndices;
    struct ShadowInfo *shadow;
    TextureBind **textureP_arr;
    float *field_34;
    int frame_id;
};
struct TriangleTextureBind
{
    uint16_t tex_idx;
    uint16_t tri_count;
};
enum ModelState
{
    LOADED=4,
};
struct PackNames
{
    char **strings;
    int count;
};
struct ModelHeader
{
    char name[124];
    struct AnimList *parent_anim;
    float length;
    struct Model *models;
    int num_subs;
};

struct AnimList
{
    AnimList *next;
    AnimList *prev;
    int header_count;
    struct ModelHeader *headers;
    char name[128];
    PackNames texnames;
    int headersize;
    int datasize;
    int loadstate;
    float lasttimeused;
    int type;
    struct XFileHandle *file;
    int tex_load_style;
    int geo_use_type;
    void *header_data;
    void *geo_data;
};
static_assert(sizeof(AnimList) == 0xC0);
struct Model {
    int Model_flg1;
    float model_VisSphereRadius;
    GeometryData *vbo;
    int num_textures;
    int16_t field_10;
    char blendState;
    char loadstate;
    struct BoneInfo *boneinfo;
    TrickNode *trck_node;
    int vertex_count;
    int model_tri_count;
    TriangleTextureBind *texture_bind_offsets;
    struct UnpackedModel_P *unpacked_1;
    Vector3 grid_P;
    float field_38;
    float field_3C;
    float field_40;
    int field_44;
    struct ModelCtri *ctris;
    int *triangle_tags;
    char *bone_name_offset;
    struct AltPivot *altpivots;
    struct ModelExtra *extra;
    Vector3 float8;
    Vector3 float11;
    Vector3 float14;
    struct AnimList *parent_anim;
    SmallAnimSRel sasr[7];
};
