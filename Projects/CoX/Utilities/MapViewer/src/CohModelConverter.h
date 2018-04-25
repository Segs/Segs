/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

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
