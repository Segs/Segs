#ifndef COHMODELCONVERTER_H
#define COHMODELCONVERTER_H
#include <Lutefisk3D/Graphics/StaticModel.h>
#include <glm/vec3.hpp>
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
struct ConvertedModel
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
    struct VBOPointers *     vbo       = nullptr;
    ModelModifiers *         trck_node = nullptr;
};
struct ConvertedGeoSet
{
    QString                       geopath;
    QString                       name;
    ConvertedGeoSet *             parent_geoset = nullptr;
    std::vector<ConvertedModel *> subs;
    std::vector<QString>          tex_names;
    std::vector<char>             m_geo_data;
    uint32_t                      geo_data_size;
    bool                          data_loaded = false;
};

Urho3D::StaticModel *convertedModelToLutefisk(Urho3D::Context *ctx, Urho3D::Node *tgtnode, CoHNode *node,int opt);
ConvertedModel * groupModelFind(const QString &a1);
bool prepareGeoLookupArray();
GeometryModifiers *findGeomModifier(const QString &modelname, const QString &trick_path);
GeoStoreDef * groupGetFileEntryPtr(const QString &a1);
ConvertedGeoSet * geosetLoad(const QString &m);
#endif // COHMODELCONVERTER_H
