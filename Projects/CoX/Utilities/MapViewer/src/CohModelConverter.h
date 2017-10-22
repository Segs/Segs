#pragma once
#include <glm/vec3.hpp>
#include <QStringList>
#include <vector>
struct CoHNode;
struct ConvertedGeoSet;
struct GeometryModifiers;
struct ModelModifiers;
namespace Urho3D {
class StaticModel;
class Context;
class Node;
}
Urho3D::StaticModel *convertedModelToLutefisk(Urho3D::Context *ctx, Urho3D::Node *tgtnode, CoHNode *node,int opt);
