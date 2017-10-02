#pragma once

#include <Lutefisk3D/Math/Matrix3x4.h>
#include <Lutefisk3D/Math/BoundingBox.h>

#include <vector>
#include <QString>
#include <QHash>

namespace Urho3D {
class Node;
class Context;
};

enum class CoHBlendMode : uint8_t
{
    MULTIPLY                = 0,
    MULTIPLY_REG            = 1,
    COLORBLEND_DUAL         = 2,
    ADDGLOW                 = 3,
    ALPHADETAIL             = 4,
    BUMPMAP_MULTIPLY        = 5,
    BUMPMAP_COLORBLEND_DUAL = 6,
    INVALID                 = 255,
};
namespace Urho3D {
    class Texture;
}
struct ConvertedModel;
struct NodeChild
{

    Urho3D::Matrix3x4 m_matrix;
    struct ConvertedNode *m_def=nullptr;
};
struct ConvertedNode
{
    std::vector<NodeChild> children;
    QString name;
    QString dir;
    ConvertedModel *model=nullptr;
    struct GeoStoreDef *geoset_info=nullptr;
    Urho3D::BoundingBox m_bbox;
    Urho3D::Vector3 center;
    float radius=0;
    float vis_dist=0;
    float lod_near=0;
    float lod_far=0;
    float lod_near_fade=0;
    float lod_far_fade=0;
    float lod_scale=0;
    float shadow_dist=0;
    int lod_autogen = 0;
    bool in_use = false;
    bool lod_fromtrick=false;
};
struct ConvertedRootNode
{
    Urho3D::Matrix3x4 mat;
    ConvertedNode *node = nullptr;
    uint32_t index_in_roots_array=0;


};
struct ConvertedSceneGraph
{
    int last_node_id=0; // used to create new number suffixes for generic nodes
    std::vector<ConvertedNode *> all_converted_defs;
    std::vector<ConvertedRootNode *> refs;
    QHash<QString,ConvertedNode *> name_to_node;
};
bool loadSceneGraph(ConvertedSceneGraph &conv,const QString &path);
Urho3D::Node * convertedNodeToLutefisk(ConvertedNode *def, Urho3D::Matrix3x4 mat, Urho3D::Context *ctx, int depth);

