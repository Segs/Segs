/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup MapViewer Projects/CoX/Utilities/MapViewer
 * @{
 */

#include "CoHSceneConverter.h"
#include "CoHModelLoader.h"
#include "CohModelConverter.h"
#include "CohTextureConverter.h"

#include "GameData/DataStorage.h"
#include "GameData/CoHMath.h"
#include "GameData/scenegraph_serializers.h"
#include "GameData/scenegraph_definitions.h"
#include "GameData/trick_definitions.h"

#include <Lutefisk3D/Core/Context.h>
#include <Lutefisk3D/Graphics/Light.h>
#include <Lutefisk3D/Graphics/StaticModel.h>
#include <Lutefisk3D/Graphics/Texture.h>
#include <Lutefisk3D/Math/BoundingBox.h>
#include <Lutefisk3D/Math/Matrix3x4.h>
#include <Lutefisk3D/Resource/Image.h>
#include <Lutefisk3D/Resource/ResourceCache.h>
#include <Lutefisk3D/Scene/Node.h>
#include <Lutefisk3D/UI/Font.h>
#include <Lutefisk3D/UI/Text3D.h>

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QHash>
#include <vector>
#include <glm/gtc/type_ptr.hpp>

using namespace Urho3D;
extern QString basepath;
std::vector<ConvertedInstance> g_converted_instances;
std::unordered_map<SEGS::SceneNode *,int> g_node_to_converted;

namespace
{
//glm::vec3 fromUrho(Vector3 v) { return {v.x_,v.y_,v.z_};}
Vector3 toUrho(glm::vec3 v) { return {v.x,v.y,v.z};}
Urho3D::Matrix3x4 fromChildTransform(const SEGS::SceneNodeChildTransform &ct)
{
    Urho3D::Quaternion quat2 = Quaternion(-ct.m_pyr.x * 180 / M_PI, ct.m_pyr.y * 180 / M_PI, ct.m_pyr.z * 180 / M_PI);
    Urho3D::Matrix3x4 res;
    res.SetRotation(quat2.RotationMatrix());
    res.SetTranslation(toUrho(ct.m_translation));

//    Urho3D::Matrix3 val;
//    val.m00_ = ct.m_matrix2[0][0];
//    val.m01_ = ct.m_matrix2[0][1];
//    val.m02_ = ct.m_matrix2[0][2];
//    val.m10_ = ct.m_matrix2[1][0];
//    val.m11_ = ct.m_matrix2[1][1];
//    val.m12_ = ct.m_matrix2[1][2];
//    val.m20_ = ct.m_matrix2[2][0];
//    val.m21_ = ct.m_matrix2[2][1];
//    val.m22_ = ct.m_matrix2[2][2];
//    res.SetRotation(val);
//    res.SetTranslation(toUrho(ct.m_translation));
    return res;
}
struct NameList
{
    QHash<QString,QString> new_names; // map from old node name to a new name
    QString basename;
};
static NameList my_name_list;

} // namespace

extern int created_node_count;

//TODO: convert this from recursive function into iterative one.
Urho3D::Node * convertedNodeToLutefisk(SEGS::SceneNode *conv_node, Urho3D::Node *urho_parent, const Urho3D::Matrix3x4 &mat, Context *ctx, int depth, int opt)
{
    auto iter = g_node_to_converted.find(conv_node);
    Urho3D::Node * node;
    if(iter!=g_node_to_converted.end())
    {
        if(iter->second==-1)
            return nullptr;
        // we already converted this node , just clone an instance
        //NOTE: this will not handle cases where node is a child of itself
        ConvertedInstance & ci(g_converted_instances[iter->second]);
        assert(!ci.m_instances.empty());
        assert(ci.m_first_instance->GetScene());
        node = ci.m_first_instance->Clone();
        node->SetParent(urho_parent);
        node->SetTransform(mat);
        node->SetVar("CoHNode",conv_node);
        node->SetVar("CoHModel",conv_node->m_model);
        created_node_count++;
        ci.m_instances.emplace_back(node);
        return node;
    }
    node = urho_parent->CreateChild(conv_node->m_name);
    g_node_to_converted[conv_node] = g_converted_instances.size();
    g_converted_instances.emplace_back(ConvertedInstance{conv_node,Urho3D::SharedPtr<Node>(node),{}});
    g_converted_instances.back().m_instances.push_back(WeakPtr<Node>(node));

    created_node_count++;
    node->SetTransform(mat);
    if(conv_node->m_model)
    {
        //assert(def->children.empty());
        StaticModel* conv_model = convertedModelToLutefisk(ctx,node,conv_node,opt);
        if(!conv_model)
        {
            g_node_to_converted[conv_node]=-1;
            g_converted_instances.pop_back();
            return nullptr;
        }
        node->SetVar("CoHNode",conv_node);
        node->SetVar("CoHModel",conv_node->m_model);
        // some nodes contain both a model and children nodes
        //return node;
    }

    if (depth > 0)
    {
        node->SetVar("CoHNode",conv_node);
        for(SEGS::SceneNodeChildTransform &d : conv_node->m_children)
        {
            //this is used to reject models for farther lods
            if(d.node->m_model && d.node->lod_near!=0.0f)
                continue;
            Urho3D::Node *newNode = convertedNodeToLutefisk(d.node, node, fromChildTransform(d), ctx, depth - 1,opt); // recursive call
            if (newNode)
                node->AddChild(newNode);
        }
        return node;
    }
    if (depth == 0)
    { /*
        auto    boxTextNode = node->CreateChild("BoxText");
        Text3D *boxText     = boxTextNode->CreateComponent<Text3D>();
        boxText->SetText(QString("Group %1\n(%2)").arg(conv_node->name).arg(mat.Translation().ToString()));
        boxText->SetFont(cache->GetResource<Font>("Fonts/BlueHighway.sdf"), 18);
        boxText->SetColor(Color::RED);
        boxText->SetFaceCameraMode(FC_ROTATE_Y);*/
    }
    return node;
}

//! @}
