/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <SceneGraph.h>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <memory>
#include <vector>

class QString;

struct MapSwap
{
    QString m_spawn_link_val;
    QString m_map_link_val;
    QString m_node_name;
    glm::vec3 m_position;
};

///
/// \brief The MapSceneGraph class and functions operating on it are central point of access to the world's geometry
///
class MapSceneGraph
{
    std::unique_ptr<struct SceneGraph> m_scene_graph;
    //! Contains all nodes from the scene graph that have any properties set, for faster lookups.
    //! @todo consider creating a property-name => [SceneNode,SceneNode] mapping instead ?
    std::vector<struct SceneNode *> m_nodes_with_properties;
    glm::vec3 m_spawn_point;
public:
    MapSceneGraph();
    ~MapSceneGraph();
    bool loadFromFile(const QString &mapname);
    void set_default_spawn_point(glm::vec3 loc) { m_spawn_point = loc; }
    glm::vec3 spawn_location() const { return m_spawn_point; }
    std::vector<glm::mat4> spawn_points(const QString &kind) const;
    std::vector<MapSwap> map_swaps() const;
    void spawn_npcs(class MapInstance *instance);
    void build_combat_navigation_graph();
    void build_pedestrian_navigation_graph();
};
