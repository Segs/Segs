#pragma once
#include "CohTextureConverter.h"
#include <glm/vec3.hpp>
#include <QStringList>
#include <memory>
#include <vector>
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
struct Vector3i
{
    uint32_t xi,yi,zi;
    uint32_t operator[](int idx) const { return ((const uint32_t *)this)[idx];}
};
struct VBOPointers
{
    std::vector<glm::vec3> pos;
    std::vector<glm::vec3> norm;
    std::vector<glm::vec2> uv1;
    std::vector<glm::vec2> uv2;
    std::vector<Vector3i> triangles;
    std::vector<TextureWrapper> assigned_textures;
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
    ModelModifiers *         trck_node = nullptr;
};
std::unique_ptr<VBOPointers> getVBO(ConvertedModel &m);
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
void initLoadedModel(std::function<TextureWrapper(const QString &)> funcloader, ConvertedModel *model,
                     const std::vector<TextureWrapper> &textures);

ConvertedModel * groupModelFind(const QString &a1);
bool prepareGeoLookupArray();
GeometryModifiers *findGeomModifier(const QString &modelname, const QString &trick_path);
GeoStoreDef * groupGetFileEntryPtr(const QString &a1);
ConvertedGeoSet * geosetLoad(const QString &m);
float *combineBuffers(VBOPointers &meshdata,ConvertedModel *mdl);
void geosetLoadData(class QFile &fp, ConvertedGeoSet *geoset);
