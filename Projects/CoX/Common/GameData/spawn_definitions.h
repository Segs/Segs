/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include <glm/mat4x4.hpp>
#include <QtCore/QString>
#include <QtCore/QHash>
#include <vector>

struct SpawnPoint
{
    QString         m_name;
    bool            m_is_victim;
    glm::mat4       m_relative_position;
};

struct SpawnDef
{
    QString                                     m_node_name;
    std::vector<SpawnPoint>                     m_all_spawn_points;
    uint8_t                                     m_spawn_probability;
    uint8_t                                     m_villain_radius; // Aggro range?
};

struct EnemyDefinition
{
    QString     m_model;
    QString     m_name;
    QString     m_faction_name;
    bool        m_spawn_all;
};

struct SpawnNPCList
{
   QString                          m_spawn_group;
   std::vector<EnemyDefinition>     m_possible_enemies;
};

class SpawnDefinitions
{
    std::vector<SpawnNPCList>       m_spawn_npc_list;
public:

    SpawnDefinitions()              { buildList();}
    SpawnNPCList                    getSpawnGroup(const QString &spawn_group_name);
    std::vector<SpawnNPCList>       getEnemySpawnGroups();
    void                            buildList();

};
