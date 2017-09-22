#ifndef COHMODELCONVERTER_H
#define COHMODELCONVERTER_H
#include <Lutefisk3D/Graphics/StaticModel.h>

struct ConvertedNode;
struct ConvertedGeoSet;
struct GeometryModifiers;
struct DefChild;
struct ModelModifiers;
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
    DeltaPack tris;
    DeltaPack verts;
    DeltaPack norms;
    DeltaPack sts;
    DeltaPack weights;
    DeltaPack matidxs;
    DeltaPack grid;
    DeltaPack &operator[](uint8_t idx)
    {
        return (&tris)[idx];
    }
};
struct ConvertedModel
{
    QString name;
    int flags;
    float visibility_radius;
    uint32_t num_textures;
    CoHBlendMode blend_mode;
    uint32_t vertex_count;
    uint32_t model_tri_count;
    Urho3D::Vector3 scale;
    Urho3D::Vector3 m_min;
    Urho3D::Vector3 m_max;
    ConvertedGeoSet *geoset;
    PackBlock packed_data;
    std::vector<TextureBind> texture_bind_info;
    void *vbo=nullptr;
    ModelModifiers *trck_node=nullptr;
};
struct ConvertedGeoSet
{
    QString geopath;
    QString name;
    ConvertedGeoSet *parent_geoset=nullptr;
    std::vector<ConvertedModel *> subs;
    std::vector<QString> tex_names;
    std::vector<char> m_geo_data;
    uint32_t geo_data_size;
    bool data_loaded=false;
};

Urho3D::StaticModel *convertToLutefiskModel(Urho3D::Context *ctx, Urho3D::Node *tgtnode, ConvertedNode *def);
ConvertedModel * groupModelFind(const QString &a1);
bool prepareGeoLookupArray();
GeometryModifiers *findGeomModifier(const QString &modelname, const QString &trick_path);
#endif // COHMODELCONVERTER_H
