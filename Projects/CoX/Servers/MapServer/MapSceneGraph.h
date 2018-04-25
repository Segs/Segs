#pragma once
#include <glm/vec3.hpp>
#include <memory>
#include <vector>

class QString;

///
/// \brief The MapSceneGraph class and functions operating on it are central point of access to the world's geometry
///
class MapSceneGraph
{
    glm::vec3 m_spawn_point;
    std::unique_ptr<struct SceneGraph> m_scene_graph;
    std::vector<struct SceneNode *> m_nodes_with_properties;
public:
    MapSceneGraph();
    ~MapSceneGraph();
    bool loadFromFile(const QString &mapname);
    void set_default_spawn_point(glm::vec3 loc) { m_spawn_point = loc; }
    glm::vec3 spawn_location() const { return m_spawn_point; }
    void spawn_npcs(class MapInstance *instance);
};
