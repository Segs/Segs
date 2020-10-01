/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include <glm/mat4x4.hpp>
#include <QtCore/QString>
#include <QtCore/QHash>
#include <vector>

// Move SpawnerNode to MapSceneGraph and remove spawn_definitions.cpp/.h?
// CritterGenerator.cpp/.h now also obselete?
class SpawnerNode
{
    public:
    QString                     m_name;         // Nodes name
    std::vector<SpawnerNode>    m_markers;      // Children nodes, typically spawn markers
    glm::vec3                   m_position;     // definite world position
    glm::vec3                   m_rotation;     // definite rotation
};

// Everything below this point obselete?
class CritterSpawnPoint
{
public:
    QString         m_name;
    bool            m_is_victim;
    glm::vec3       m_relative_position;
    glm::vec3       m_rotation;

    std::string getName() const { return m_name.toStdString();}
    void setName(const char *n) { m_name = n; }
};

class CritterSpawnLocations
{
public:
    QString                                     m_node_name;
    std::vector<CritterSpawnPoint>              m_all_spawn_points;
    uint8_t                                     m_spawn_probability;
    uint8_t                                     m_villain_radius; // Aggro range?

    std::string getNodeName() const { return m_node_name.toStdString();}
    void setNodeName(const char *n) { m_node_name = n; }
};

struct CritterDefinition
{
    QString     m_model;
    QString     m_name;
    QString     m_faction_name;
    bool        m_spawn_all;
};

struct CritterSpawnDef
{
   QString                              m_spawn_group;
   std::vector<CritterDefinition>       m_possible_critters;
};

class SpawnDefinitions
{
    std::vector<CritterSpawnDef>        m_critter_spawn_list;
public:

    SpawnDefinitions()                  { buildList();}
    CritterSpawnDef                     getSpawnGroup(const QString &spawn_group_name);
    std::vector<CritterSpawnDef>        getCritterSpawnDefinitions();
    void                                buildList();

};
