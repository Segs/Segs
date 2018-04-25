/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#pragma once

#include <Lutefisk3D/Math/Matrix3x4.h>
#include <Lutefisk3D/Math/BoundingBox.h>
#include <Lutefisk3D/Container/Ptr.h>
#include <glm/matrix.hpp>

#include <QString>
#include <QHash>

#include <vector>

namespace Urho3D
{
    class Node;
    class Context;
};

enum
{
    CONVERT_MINIMAL,
    CONVERT_EDITOR_MARKERS,
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

struct CoHModel;

struct NodeChild
{
    Urho3D::Matrix3x4 m_matrix;
    struct CoHNode *  m_def = nullptr;
};

struct CoHNode
{
    CoHNode *                     parent = nullptr;
    std::vector<NodeChild>        children;
    QString                       name;
    QString                       dir;
    CoHModel *                    model       = nullptr;
    struct GeoStoreDef *          geoset_info = nullptr;
    Urho3D::BoundingBox           m_bbox;
    glm::vec3                     center;
    Urho3D::WeakPtr<Urho3D::Node> m_lutefisk_result;
    float                         radius        = 0;
    float                         vis_dist      = 0;
    float                         lod_near      = 0;
    float                         lod_far       = 0;
    float                         lod_near_fade = 0;
    float                         lod_far_fade  = 0;
    float                         lod_scale     = 0;
    float                         shadow_dist   = 0;
    int                           lod_autogen   = 0;
    bool                          in_use        = false;
    bool                          lod_fromtrick = false;
};

struct ConvertedRootNode
{
    Urho3D::Matrix3x4 mat;
    CoHNode *node = nullptr;
    uint32_t index_in_roots_array=0;


};

struct CoHSceneGraph
{
    int last_node_id=0; // used to create new number suffixes for generic nodes
    std::vector<CoHNode *> all_converted_defs;
    std::vector<ConvertedRootNode *> refs;
    QHash<QString,CoHNode *> name_to_node;
};

bool loadSceneGraph(CoHSceneGraph &conv,const QString &path);

Urho3D::Node * convertedNodeToLutefisk(CoHNode *def, const Urho3D::Matrix3x4 & mat, Urho3D::Context *ctx, int depth, int opt=CONVERT_MINIMAL);
