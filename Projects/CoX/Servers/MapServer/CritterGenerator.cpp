/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "CritterGenerator.h"
#include "DataHelpers.h"
#include "MapInstance.h"

void CritterGenerator::generate(MapInstance *map_instance)
{
    const NPCStorage &npc_store(getGameData().getNPCDefinitions());
    SpawnDefinitions *sp = new SpawnDefinitions();

    uint32_t spawn_limit = 3; // control number spawned in each group


    uint32_t count = 0;
    bool spawn_all = false;
    bool m_victim_spawned = false;

    CritterSpawnDef spawn_def = sp->getSpawnGroup(this->m_critter_encounter.m_node_name);
    CritterDefinition cd;

    for (const auto &encounter_location: this->m_critter_encounter.m_all_spawn_points)
    {
        if(spawn_def.m_possible_critters.size() > 1)
            cd = spawn_def.m_possible_critters.at(rand() % (spawn_def.m_possible_critters.size() - 1));
        else
            cd = spawn_def.m_possible_critters.at(0);

        spawn_all = cd.m_spawn_all;

        if(encounter_location.m_name.contains("encounter_e_", Qt::CaseInsensitive))
        {
            glm::vec3 pos = glm::vec3(encounter_location.m_relative_position[3]);
            auto valquat = glm::quat_cast(encounter_location.m_relative_position);
            glm::vec3 angles = glm::eulerAngles(valquat);
            angles.y += glm::pi<float>();

            //spawn enemy
            addEnemy(*map_instance, cd.m_model, pos, 1, angles, cd.m_name, 2, cd.m_faction_name, 0);
            ++count;
        }
        else if(encounter_location.m_name.contains("encounter_v_", Qt::CaseInsensitive) && !m_victim_spawned)
        {
            //spawn npc
            QString npcName;
            for(const Parse_NPC &npc : npc_store.m_all_npcs)
            {
                QString name(npc.m_Name);

                if(name.contains("maleNPC", Qt::CaseInsensitive))
                {
                    npcName = npc.m_Name;
                    break;
                }
            }

            glm::vec3 pos = glm::vec3(encounter_location.m_relative_position[3]);
            auto valquat = glm::quat_cast(encounter_location.m_relative_position);
            glm::vec3 angles = glm::eulerAngles(valquat);
            angles.y += glm::pi<float>();

            QString victim = "Victim";
            addVictim(*map_instance, npcName, pos, 1, angles, victim);

            m_victim_spawned = true;
        }
        else if(encounter_location.m_name.contains("encounter_s_", Qt::CaseInsensitive)) // Scenario? Glowie or critter attacking victim?
        {
            glm::vec3 pos = glm::vec3(encounter_location.m_relative_position[3]);
            auto valquat = glm::quat_cast(encounter_location.m_relative_position);
            glm::vec3 angles = glm::eulerAngles(valquat);
            angles.y += glm::pi<float>();

            //spawn enemy
            addEnemy(*map_instance, cd.m_model, pos, 1, angles, cd.m_name, 2, cd.m_faction_name, 0);
            ++count;
        }

        if(!cd.m_spawn_all && count == spawn_limit)
            break;
    }
}

void CritterGeneratorStore::generate(MapInstance *instance)
{
    uint32_t total_spawned = 0;
    uint32_t spawn_limit = 300;
    for(CritterGenerator &gen : m_generators)
    {
        if(total_spawned >= spawn_limit)
            break;

        gen.generate(instance);
        total_spawned += 3; // Mock total spawn limit
    }
}
