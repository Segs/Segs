/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <glm/vec3.hpp>
#include <QStringList>
#include <vector>

namespace SEGS {
struct SceneNode;
struct GeoSet;

}
struct GeometryModifiers;
struct ModelModifiers;
namespace Urho3D {
class StaticModel;
class Context;
class Node;
}
Urho3D::StaticModel *convertedModelToLutefisk(Urho3D::Context *ctx, Urho3D::Node *tgtnode, SEGS::SceneNode *node,int opt);
