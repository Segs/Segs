/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "GameData/scenegraph_definitions.h"
#include "Common/Runtime/AxisAlignedBox.h"
#include "Common/Runtime/Handle.h"

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
    glm::vec3 m_pyr;
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
    SceneNode()
    {
        is_LOD_fade_node = 0;
    }
    struct GeoStoreDef *    m_belongs_to_geoset = nullptr;
    std::vector<SceneNodeChildTransform> m_children;
    std::vector<GroupProperty_Data> *m_properties = nullptr;

    HLightProperties m_light;
    Model *m_model = nullptr;
    struct GeoStoreDef *m_geoset_info = nullptr; // where is this node from ?
    QString m_name;
    QString m_dir;
    AxisAlignedBoundingBox        m_bbox;
    int                           m_index_in_scenegraph=0;

    uint32_t                      m_fx_name_hash  = 0; //!< This is fnv1a hash of downcased fx file path.
    glm::vec3                     m_center;
    float                         radius        = 0;
    float                         vis_dist      = 0;
    float                         lod_near      = 0;
    float                         lod_far       = 0;
    float                         lod_near_fade = 0;
    float                         lod_far_fade  = 0;
    float                         lod_scale     = 0;
    float                         shadow_dist   = 0;
    HandleT<20,12,struct SoundInfo> sound_info;
    bool                          lod_fromtrick = false;
    // Start of bit flags
    uint32_t is_LOD_fade_node : 1;
    uint32_t shell : 1;
    uint32_t tray : 1;
    uint32_t region_marker : 1;
    uint32_t volume_trigger : 1;
    uint32_t water_volume : 1;
    uint32_t lava_volume : 1;
    uint32_t sewer_volume : 1;
    uint32_t door_volume : 1;
    uint32_t in_use : 2;
    uint32_t parent_fade : 1;
    uint32_t key_light : 1;
    // end of bit flags
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
SceneGraph *loadWholeMap(const QString &filename);
void loadSubgraph(const QString &filename, LoadingContext &ctx,PrefabStore &prefabs);
SceneNode * getNodeByName(const SceneGraph &graph,const QString &name);
} // and of SEGS namespace
