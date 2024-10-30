/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "DataHelpers.h"
#include "MapInstance.h"
#include "MessageHelpers.h"
#include "ScriptingEngine.h"
#include "ScriptingEnginePrivate.h"

#include "Common/GameData/spawn_definitions.h"

using namespace SEGSEvents;

void ScriptingEngine::register_SpawnerTypes()
{
    // usertype for dealing with spawner groups
    m_private->m_lua.new_usertype<SpawnerNode>("SpawnerNode",
        sol::constructors<SpawnerNode>(),
        "name", &SpawnerNode::m_name,
        "markers", &SpawnerNode::m_markers,
        "position", &SpawnerNode::m_position,
        "rotation", &SpawnerNode::m_rotation);

    // Returns count of located spawners
    m_private->m_lua["MapInstance"]["GetSpawnerCount"] = [this]()
    {
        auto sg = &mi->m_map_scenegraph->m_csNodes;
        uint32_t sCount = (uint32_t)sg->size();
        return sCount;
    };

    // Returns count of children belonging to spawner at the specified index
    m_private->m_lua["MapInstance"]["GetSpawnerChildCount"] = [this](uint32_t index)
    {
        auto sg = &mi->m_map_scenegraph->m_csNodes;
        uint32_t sChildCount = sg->at(index).m_markers.size();
        return sChildCount;
    };

    // Returns the name of the spawner at the specified index
    m_private->m_lua["MapInstance"]["GetSpawnerName"] = [this](uint32_t index)
    {
        auto sg = &mi->m_map_scenegraph->m_csNodes;
        String sName = sg->at(index).m_name;
        return sName;
    };

    // Returns the name of the child spawner at the specified indexes
    m_private->m_lua["MapInstance"]["GetSpawnerChildName"] = [this](uint32_t index, uint32_t cindex)
    {
        auto sg = &mi->m_map_scenegraph->m_csNodes;
        String sName = sg->at(index).m_markers.at(cindex).m_name;
        return sName;
    };

    // Returns the position of the spawner at the specified index
    m_private->m_lua["MapInstance"]["GetSpawnerPosition"] = [this](uint32_t index)
    {
        auto sg = &mi->m_map_scenegraph->m_csNodes;
        glm::vec3 sPos = sg->at(index).m_position;
        return sPos;
    };

    // Returns the position of the child spawner at the specified indexes
    m_private->m_lua["MapInstance"]["GetSpawnerChildPosition"] = [this](uint32_t index, uint32_t cindex)
    {
        auto sg = &mi->m_map_scenegraph->m_csNodes;
        glm::vec3 sPos = sg->at(index).m_markers.at(cindex).m_position;
        return sPos;
    };

    // Returns the rotation of the spawner at the specified index
    m_private->m_lua["MapInstance"]["GetSpawnerRotation"] = [this](uint32_t index)
    {
        auto sg = &mi->m_map_scenegraph->m_csNodes;
        glm::vec3 sRot = sg->at(index).m_rotation;
        return sRot;
    };

    // Returns the rotation of the child spawner at the specified indexex
    m_private->m_lua["MapInstance"]["GetSpawnerChildRotation"] = [this](uint32_t index, uint32_t cindex)
    {
        auto sg = &mi->m_map_scenegraph->m_csNodes;
        glm::vec3 sRot = sg->at(index).m_markers.at(cindex).m_rotation;
        return sRot;
    };

    // Returns count of located persistents
    m_private->m_lua["MapInstance"]["GetPersistentCount"] = [this]()
    {
        auto sg = &mi->m_map_scenegraph->m_persNodes;
        uint32_t pCount = sg->size();
        return pCount;
    };

    // Jumps to specified Persistent NPC and announces NPC's name
    m_private->m_lua["MapInstance"]["JumpPersist"] = [this](uint32_t index)
    {
        auto sg = &mi->m_map_scenegraph->m_persNodes;
        if(index < 1 || index > sg->size())
        {
            sendInfoMessage(static_cast<MessageChannel>(14), "Invalid persistent.", *cl);
            return;
        }
        glm::vec3 pPos = sg->at(index-1).m_position;
        forcePosition(*cl->m_ent, pPos);
        sendInfoMessage(static_cast<MessageChannel>(14), sg->at(index-1).m_name, *cl);
    };

    // Returns the name of the persistent NPC at the specified index
    m_private->m_lua["MapInstance"]["GetPersistentName"] = [this](uint32_t index)
    {
        auto sg = &mi->m_map_scenegraph->m_persNodes;
        String pName = sg->at(index).m_name;
        return pName;
    };

    // Returns the position of the persistent NPC at the specified index
    m_private->m_lua["MapInstance"]["GetPersistentPosition"] = [this](uint32_t index)
    {
        auto sg = &mi->m_map_scenegraph->m_persNodes;
        glm::vec3 pPos = sg->at(index).m_position;
        return pPos;
    };

    // Returns the rotation of the persistent NPC at the specified index
    m_private->m_lua["MapInstance"]["GetPersistentRotation"] = [this](uint32_t index)
    {
        auto sg = &mi->m_map_scenegraph->m_persNodes;
        glm::vec3 pRot = sg->at(index).m_rotation;
        return pRot;
    };

    // Returns count of located car nodes
    m_private->m_lua["MapInstance"]["GetCarCount"] = [this]()
    {
        auto sg = &mi->m_map_scenegraph->m_carNodes;
        uint32_t cCount = sg->size();
        return cCount;
    };

    // Returns the position of the car node at the specified index
    m_private->m_lua["MapInstance"]["GetCarPosition"] = [this](uint32_t index)
    {
        auto sg = &mi->m_map_scenegraph->m_carNodes;
        glm::vec3 cPos = sg->at(index).m_position;
        return cPos;
    };

    // Returns the rotation of the car node at the specified index
    m_private->m_lua["MapInstance"]["GetCarRotation"] = [this](uint32_t index)
    {
        auto sg = &mi->m_map_scenegraph->m_carNodes;
        glm::vec3 cRot = sg->at(index).m_rotation;
        return cRot;
    };

    // Returns count of located Civ/Npc nodes
    m_private->m_lua["MapInstance"]["GetCivCount"] = [this]()
    {
        auto sg = &mi->m_map_scenegraph->m_npcNodes;
        uint32_t nCount = sg->size();
        return nCount;
    };

    // Returns the position of the Civ/NPC node at the specified index
    m_private->m_lua["MapInstance"]["GetCivPosition"] = [this](uint32_t index)
    {
        auto sg = &mi->m_map_scenegraph->m_npcNodes;
        glm::vec3 cPos = sg->at(index).m_position;
        return cPos;
    };

    // Returns the rotation of the Civ/NPC node at the specified index
    m_private->m_lua["MapInstance"]["GetCivRotation"] = [this](uint32_t index)
    {
        auto sg = &mi->m_map_scenegraph->m_npcNodes;
        glm::vec3 cRot = sg->at(index).m_rotation;
        return cRot;
    };

    m_private->m_lua["MapInstance"]["AddNpc"] = [this](const char* npc_def, glm::vec3 &loc, glm::vec3 &ori, int variation, const char* npc_name)
    {
        return addNpcWithOrientation(*mi, npc_def, loc, variation, ori, npc_name);
    };

    m_private->m_lua["MapInstance"]["AddEnemy"] = [this](const char* npc_def, glm::vec3 &loc, glm::vec3 &ori, int variation, const char* npc_name, int level, const char* faction_name, int rank)
    {
        return addEnemy(*mi, npc_def, loc, variation, ori, npc_name, level, faction_name, rank);
    };


    m_private->m_lua["MapInstance"]["RemoveNpc"] = [this](int entityIdx)
    {
        Entity *e = getEntity(mi, entityIdx);
        if(e != nullptr)
        {
            mi->m_entities.removeEntityFromActiveList(e);
        }
    };
}
