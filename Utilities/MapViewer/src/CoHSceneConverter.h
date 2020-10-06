/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Common/Runtime/SceneGraph.h"
#include "GameData/scenegraph_definitions.h"
#include <Lutefisk3D/Math/Matrix3x4.h>
#include <Lutefisk3D/Math/BoundingBox.h>
#include <Lutefisk3D/Container/Ptr.h>
#include <glm/matrix.hpp>

#include <QString>
#include <QHash>

#include <vector>
#include <unordered_map>

inline Urho3D::Vector3 fromGLM(glm::vec3 v)
{
    return {v.x,v.y,v.z};
}

namespace Urho3D
{
    class Node;
    class Context;
}

enum
{
    CONVERT_MINIMAL=1,
    CONVERT_EDITOR_MARKERS,
};

using CoHModel = SEGS::Model;

struct ConvertedInstance
{
    SEGS::SceneNode *source_node;
    Urho3D::SharedPtr<Urho3D::Node> m_first_instance;
    std::vector<Urho3D::WeakPtr<Urho3D::Node> > m_instances;
};
extern std::vector<ConvertedInstance> g_converted_instances;
extern std::unordered_map<SEGS::SceneNode *,int> g_node_to_converted;

Urho3D::Node * convertedNodeToLutefisk(SEGS::SceneNode *def, Urho3D::Node *urho_parent, const Urho3D::Matrix3x4 & mat, Urho3D::Context *ctx, int depth, int opt=CONVERT_MINIMAL);
