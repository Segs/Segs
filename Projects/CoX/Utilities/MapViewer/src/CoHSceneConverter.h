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
struct ConvertedNode
{
    QString name;
    QString dir;
    ConvertedModel *model;
    float lod_near;
    float lod_far;
    float lod_near_fade;
    float lod_far_fade;
    float lod_scale;
    bool lod_fromtrick=false;
};
struct ConvertedRootNode
{
};
struct ConvertedSceneGraph
{
    int last_node_id=0; // used to create new number suffixes for generic nodes
    QHash<QString,ConvertedNode *> name_to_def;
    std::vector<ConvertedNode *> all_converted_defs;
};
bool loadSceneGraph(ConvertedSceneGraph &conv,const QString &path);
bool prepareGeoLookupArray();


