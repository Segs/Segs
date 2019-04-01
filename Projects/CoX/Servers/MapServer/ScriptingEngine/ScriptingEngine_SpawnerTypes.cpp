/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "DataHelpers.h"
#include "MapInstance.h"
#include "ScriptingEngine.h"

using namespace SEGSEvents;

void ScriptingEngine::register_SpawnerTypes()
{
    m_private->m_lua.new_usertype<CritterSpawnPoint>("CritterSpawnPoint",
        sol::constructors<CritterSpawnPoint>(),
        "name", sol::property(&CritterSpawnPoint::getName, &CritterSpawnPoint::setName),
        "isVictim", &CritterSpawnPoint::m_is_victim,
        "position", &CritterSpawnPoint::m_relative_position,
        "rotation", &CritterSpawnPoint::m_rotation);

    m_private->m_lua.new_usertype<CritterSpawnLocations>("critterSpawnLocations",
        sol::constructors<CritterSpawnLocations>(),
        "nodeName", sol::property(&CritterSpawnLocations::getNodeName, &CritterSpawnLocations::setNodeName),
        "allSpawnPoints", &CritterSpawnLocations::m_all_spawn_points,
        "spawnProbability", &CritterSpawnLocations::m_spawn_probability,
        "villain", &CritterSpawnLocations::m_villain_radius);

    m_private->m_lua["MapInstance"]["AddNpc"] = [this](const char* npc_def, glm::vec3 &loc, glm::vec3 &ori, int variation, const char* npc_name)
    {
        QString npc_def_name = QString::fromUtf8(npc_def);
        QString name = QString::fromUtf8(npc_name);
        addNpcWithOrientation(*mi, npc_def_name, loc, variation, ori, name);
    };

    m_private->m_lua["MapInstance"]["AddEnemy"] = [this](const char* npc_def, glm::vec3 &loc, glm::vec3 &ori, int variation, const char* npc_name, int level, const char* faction_name, int rank)
    {
        QString npc_def_name = QString::fromUtf8(npc_def);
        QString name = QString::fromUtf8(npc_name);
        QString faction = QString::fromUtf8(faction_name);
        addEnemy(*mi, npc_def_name, loc, variation, ori, name, level, faction, rank);
    };

    m_private->m_lua["MapInstance"]["RemoveNpc"] = [this](int entityIdx)
    {
        e = getEntity(mi, entityIdx);
        if(e != nullptr)
        {
            mi->m_entities.removeEntityFromActiveList(e);
        }
    };

    m_private->m_lua["MapInstance"]["GetMapEncounters"] = [this]()
    {
        return getMapEncounters(mi);
    };
}
