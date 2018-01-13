#pragma once
#include "CohTextureConverter.h"
#include <glm/vec3.hpp>
#include <QStringList>
#include <memory>
#include <vector>
namespace Urho3D
{
    class StaticModel;
}
struct CoHNode;
struct ConvertedGeoSet;
struct GeometryModifiers;
struct ModelModifiers;

struct GeoStoreDef
{
    QString geopath;        //!< a path to a .geo file
    QStringList entries;    //!< the names of models contained in a geoset
    bool loaded;
};
struct VBOPointers
{
    std::vector<glm::vec3> pos;
    std::vector<glm::vec3> norm;
    std::vector<glm::vec2> uv1;
    std::vector<glm::vec2> uv2;
    std::vector<glm::ivec3> triangles;
    std::vector<TextureWrapper> assigned_textures;
    bool needs_tangents=false;
};

enum class CoHBlendMode : uint8_t;
#pragma pack(push, 1)
struct DeltaPack
{
    int compressed_size=0;
    uint32_t uncomp_size=0;
    int buffer_offset=0;
    uint8_t *compressed_data=nullptr;
};
struct TextureBind
{
    uint16_t tex_idx;
    uint16_t tri_count;
};
#pragma pack(pop)
struct PackBlock
{
    DeltaPack  tris;
    DeltaPack  verts;
    DeltaPack  norms;
    DeltaPack  sts;
    DeltaPack  weights;
    DeltaPack  matidxs;
    DeltaPack  grid;
    DeltaPack &operator[](uint8_t idx) { return (&tris)[idx]; }
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
struct CoHModel
{
    QString                  name;
    int                      flags;
    float                    visibility_radius;
    uint32_t                 num_textures;
    CoHBlendMode             blend_mode;
    uint32_t                 vertex_count;
    uint32_t                 model_tri_count;
    glm::vec3                scale;
    glm::vec3                m_min;
    glm::vec3                m_max;
    ptrdiff_t                boneinfo_offset = 0;
    ConvertedGeoSet *        geoset;
    PackBlock                packed_data;
    std::vector<TextureBind> texture_bind_info;
    ModelModifiers *         trck_node = nullptr;
    Urho3D::StaticModel *    converted_model = nullptr;
};
std::unique_ptr<VBOPointers> getVBO(CoHModel &m);
struct ConvertedGeoSet
{
    QString                       geopath;
    QString                       name;
    ConvertedGeoSet *             parent_geoset = nullptr;
    std::vector<CoHModel *> subs;
    std::vector<QString>          tex_names;
    std::vector<char>             m_geo_data;
    uint32_t                      geo_data_size;
    bool                          data_loaded = false;
};
void initLoadedModel(std::function<TextureWrapper(const QString &)> funcloader, CoHModel *model,
                     const std::vector<TextureWrapper> &textures);

CoHModel * groupModelFind(const QString &a1);
bool prepareGeoLookupArray();
GeometryModifiers *findGeomModifier(const QString &modelname, const QString &trick_path);
GeoStoreDef * groupGetFileEntryPtr(const QString &a1);
ConvertedGeoSet * geosetLoad(const QString &m);
float *combineBuffers(VBOPointers &meshdata,CoHModel *mdl);
void geosetLoadData(class QFile &fp, ConvertedGeoSet *geoset);
