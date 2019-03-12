/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <memory>
#include <vector>
#include <QStringList>
#include <../Common/GameData/map_definitions.h>
#include <../Common/GameData/spawn_definitions.h>

class QString;

namespace SEGS
{
struct SceneGraph;
struct SceneNode;
} // namespace SEGS

///
/// \brief The MapSceneGraph class and functions operating on it are central point of access to the world's geometry
///
class MapSceneGraph
{
    std::unique_ptr<SEGS::SceneGraph> m_scene_graph;
    //! Contains all nodes from the scene graph that have any properties set, for faster lookups.
    //! @todo consider creating a property-name => [SceneNode,SceneNode] mapping instead ?
    std::vector<SEGS::SceneNode *> m_nodes_with_properties;
public:
    MapSceneGraph();
    ~MapSceneGraph();
    bool loadFromFile(const QString &mapname);
    QMultiHash<QString, glm::mat4> getSpawnPoints() const;
    QMultiHash<QString, glm::mat4> getEncounterSpawnPoints() const;
    QHash<QString, MapXferData> get_map_transfers() const;
    void spawn_critters(class MapInstance *instance);
    void spawn_npcs(class MapInstance *instance);
    void build_combat_navigation_graph();
    void build_pedestrian_navigation_graph();
};

QString getCostumeFromName(const QString &n);
