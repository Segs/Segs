/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <glm/vec3.hpp>
#include <QStringList>
#include <vector>
#include "Model.h"

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
void convertedModelToLutefisk(Urho3D::Node *tgtnode, SEGS::Model *segs_model, int opt, float draw_dist);
