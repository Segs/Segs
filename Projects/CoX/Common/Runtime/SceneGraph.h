/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "GameData/scenegraph_definitions.h"
#include "Common/Runtime/AxisAlignedBox.h"

#include "glm/vec3.hpp"
#include "glm/mat3x3.hpp"
#include "glm/common.hpp"
#include "glm/gtc/constants.hpp"

#include <QString>
#include <QHash>
#include <vector>
#include <memory>

namespace SEGS
{
struct LoadingContext;
struct Model;
struct SceneNodeChildTransform
{
    struct SceneNode *node;
    glm::mat3x3 m_matrix2;
    glm::vec3 m_translation;
};
struct LightProperties
{
    glm::vec4 color;
    float range;
    int is_negative;
};
using HLightProperties = std::unique_ptr<LightProperties>;
struct SceneNode
{
    struct GeoStoreDef *    belongs_to_geoset = nullptr;
    std::vector<SceneNodeChildTransform> children;
    std::vector<GroupProperty_Data> *properties = nullptr;

    HLightProperties light;
    Model *model = nullptr;
    struct GeoStoreDef *geoset_info = nullptr; // where is this node from ?
    QString name;
    QString dir;
    AxisAlignedBoundingBox        m_bbox;
    int                           m_index_in_scenegraph=0;
    glm::vec3                     center;
    float                         radius        = 0;
    float                         vis_dist      = 0;
    float                         lod_near      = 0;
    float                         lod_far       = 0;
    float                         lod_near_fade = 0;
    float                         lod_far_fade  = 0;
    float                         lod_scale     = 0;
    float                         shadow_dist   = 0;
    bool                          lod_autogen   = false;
    bool                          in_use        = false;
    bool                          lod_fromtrick = false;
};

struct RootNode
{
    glm::mat4 mat;
    SceneNode *node = nullptr;
    uint32_t index_in_roots_array=0;
};

struct SceneGraph
{
    std::vector<SceneNode *> all_converted_defs;
    std::vector<RootNode *> refs;

    QHash<QString,SceneNode *> name_to_node;
};
struct PrefabStore;
struct LoadingContext;

bool loadSceneGraph(const QString &path, LoadingContext &ctx, PrefabStore &prefabs);
} // and of SEGS namespace
