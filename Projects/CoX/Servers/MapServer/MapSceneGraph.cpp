/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup MapServer Projects/CoX/Servers/MapServer
 * @{
 */
#include "MapSceneGraph.h"

#include "GameData/CoHMath.h"
#include "GameData/GameDataStore.h"
#include "EntityStorage.h"
#include "Logging.h"
#include "Common/GameData/Character.h"
#include "Common/Runtime/Prefab.h"
#include "Common/Runtime/SceneGraph.h"
#include "NpcGenerator.h"
#include "MapInstance.h"
#include "GameData/NpcStore.h"
#include "CritterGenerator.h"
#include "Common/GameData/map_definitions.h"
#include "Common/GameData/spawn_definitions.h"

#include "glm/mat4x4.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <QString>
#include <QSet>
#include <memory>

using namespace SEGS;

MapSceneGraph::MapSceneGraph()
{

}
MapSceneGraph::~MapSceneGraph()
{

}

bool MapSceneGraph::loadFromFile(const QString &filename)
{
    m_scene_graph.reset(loadWholeMap(filename));
    if(!m_scene_graph)
        return false;
    for(SceneNode *def : m_scene_graph->all_converted_defs)
    {
        if(def->m_properties)
        {
            m_nodes_with_properties.emplace_back(def);

        }
    }
    return true;
}

void walkSceneNode( SceneNode *self, const glm::mat4 &accumulated, std::function<bool(SceneNode *,const glm::mat4 &)> visit_func )
{
    if(!visit_func(self, accumulated))
        return;

    for(const auto & child : self->m_children)
    {
        glm::mat4 transform(child.m_matrix2);
        transform[3] = glm::vec4(child.m_translation,1);

        walkSceneNode(child.node, accumulated * transform, visit_func);
    }
}

static QString getRandCostumeFromName(const QString &n)
{
    QStringList matching_costumes;
    const NPCStorage &npc_store(getGameData().getNPCDefinitions());
    for(const Parse_NPC &npc : npc_store.m_all_npcs)
    {
        QString name(npc.m_Name);

        // skip some odd cases
        if(name.contains("blimp", Qt::CaseInsensitive))
            continue;
        if(name.contains("monorail", Qt::CaseInsensitive))
            continue;

        if(name.contains(n, Qt::CaseInsensitive))
            matching_costumes.push_back(QString(npc.m_Name));
    }

    if(matching_costumes.isEmpty())
        return "ChessKing";

    int rand_idx = rand() % matching_costumes.size();
    return matching_costumes[rand_idx];
}

static bool checkCostumeExists(const QString &n)
{
    const NPCStorage &npc_store(getGameData().getNPCDefinitions());
    for(const Parse_NPC &npc : npc_store.m_all_npcs)
    {
        if(npc.m_Name == n)
            return true;
    }

    return false;
}

QString getCostumeFromName(const QString &n)
{
    // A lot of costumes are the object name
    if(checkCostumeExists(n))
        return n;

    // Special cases will go here
    // General
    if(n.contains("CarGenerator", Qt::CaseInsensitive)) // must include underscore
        return getRandCostumeFromName("Car_");
    if(n.contains("NPCGenerator", Qt::CaseInsensitive))
        return getRandCostumeFromName("maleNPC"); // will include females
    if(n.contains("DoctorNPC", Qt::CaseInsensitive))
        return getRandCostumeFromName("maleNPC"); // will include females
    if(n.contains("BusinessNPC", Qt::CaseInsensitive))
        return getRandCostumeFromName("maleNPC"); // will include females
    if(n.contains("NPCDrones", Qt::CaseInsensitive))
        return "Police_Drone";
    if(n.contains("MonorailGenerator", Qt::CaseInsensitive))
        return "Car_Monorail";
    if(n.contains("BlimpGenerator", Qt::CaseInsensitive))
        return "Car_Blimp";

    // Outbreak
    if(n.contains("Paragon_SWAT", Qt::CaseInsensitive))
        return getRandCostumeFromName("MaleNPC_230");
    if(n.contains("Lt_MacReady", Qt::CaseInsensitive))
        return "CSE_01";
    if(n.contains("Security_Chief", Qt::CaseInsensitive))
        return getRandCostumeFromName("Security_Chief");

    // Atlas Park
    if(n.contains("FreedomCorp", Qt::CaseInsensitive))
        return getRandCostumeFromName("Freedom_Corp");

    // Most costumes are the object's name without spaces
    // and with Model_ prepended. Assume this as fallback
    QString result = n;
    result = makeReadableName(result);
    result.remove(" ");
    result.prepend("Model_");
    if(checkCostumeExists(result))
        return result;

    // As fallback, always return some costume
    return "ChessPawn";
}


struct NpcCreator
{
    MapInstance *map_instance = nullptr;
    NpcGeneratorStore *generators;
    QSet<QString> m_reported_generators;

    bool checkPersistent(SceneNode *n, const glm::mat4 &v)
    {
        assert(map_instance);
        bool has_npc = false;
        QString persistent_name;
        for (GroupProperty_Data &prop : *n->m_properties)
        {
            if(prop.propName=="PersistentNPC")
                persistent_name = prop.propValue;

            if(prop.propName.toUpper().contains("NPC"))
            {
                qCDebug(logNPCs) << prop.propName << '=' << prop.propValue;
                has_npc = true;
            }
        }

        if(has_npc && map_instance)
        {
            qCDebug(logNPCs) << "Attempting to spawn npc" << persistent_name << "at" << v[3][0] << v[3][1] << v[3][2];
            const NPCStorage & npc_store(getGameData().getNPCDefinitions());
            QString npc_costume_name = getCostumeFromName(persistent_name);
            const Parse_NPC * npc_def = npc_store.npc_by_name(&npc_costume_name);

            if(npc_def)
            {
                int idx = npc_store.npc_idx(npc_def);
                Entity *e = map_instance->m_entities.CreateNpc(getGameData(),*npc_def, idx, 0);
                e->m_char->setName(makeReadableName(persistent_name));
                forcePosition(*e,glm::vec3(v[3]));

                auto valquat = glm::quat_cast(v);
                glm::vec3 angles = glm::eulerAngles(valquat);
                angles.y += glm::pi<float>();
                forceOrientation(*e, angles);
                e->m_motion_state.m_velocity = { 0,0,0 };
            }
        }

        return true;
    }

    bool checkGenerators(SceneNode *n, const glm::mat4 &v)
    {
        if(!generators)
            return false;

        QString generator_type;
        for (GroupProperty_Data &prop : *n->m_properties)
        {
            if(prop.propName=="Generator")
                generator_type = prop.propValue;
        }

        if(generator_type.isEmpty())
            return true;

        if(!generators->m_generators.contains(generator_type))
        {
            qCDebug(logNPCs) << "Adding generator for" << generator_type;

            // Get costume by generator name, includes overrides
            NpcGenerator npcgen = {generator_type, EntType::NPC, {}, {}};

            if(generator_type.contains("Door", Qt::CaseInsensitive))
                npcgen.m_type = EntType::DOOR;
            if(generator_type.contains("Cardrdr", Qt::CaseInsensitive))
                npcgen.m_type = EntType::DOOR;
            else if(generator_type.contains("Car_", Qt::CaseInsensitive))
                npcgen.m_type = EntType::CAR;
            else if(generator_type.contains("Nemesis_Drone", Qt::CaseInsensitive))
                npcgen.m_type = EntType::CRITTER;
            else if(generator_type.contains("Door_train", Qt::CaseInsensitive))
                npcgen.m_type = EntType::MAPXFERDOOR;
            else if(generator_type.contains("MonorailGenerator", Qt::CaseInsensitive))
                npcgen.m_type = EntType::MOBILEGEOMETRY;
            else if(generator_type.contains("BlimpGenerator", Qt::CaseInsensitive))
                npcgen.m_type = EntType::MOBILEGEOMETRY;

            generators->m_generators[generator_type] = npcgen;
        }

        generators->m_generators[generator_type].m_initial_positions.push_back(v);
        return true;
    }

    bool operator()(SceneNode *n, const glm::mat4 &v)
    {
        if(!n->m_properties)
            return true;

        checkPersistent(n,v);
        checkGenerators(n,v);
        return true;
    }
};

void MapSceneGraph::spawn_npcs(MapInstance *instance)
{
    NpcCreator creator;
    creator.generators = &instance->m_npc_generators;
    creator.map_instance = instance;
    glm::mat4 initial_pos(1);

    for(auto v : m_scene_graph->refs)
        walkSceneNode(v->node, v->mat, creator);
}

struct SpawnPointLocator
{
    QMultiHash<QString, glm::mat4> *m_targets;
    SpawnPointLocator(QMultiHash<QString, glm::mat4> *targets) :
        m_targets(targets)
    {}
    bool operator()(SceneNode *n, const glm::mat4 &v)
    {
        if(!n->m_properties)
            return true;

        for (GroupProperty_Data &prop : *n->m_properties)
        {
            if(prop.propName == "SpawnLocation")
            {
                qCDebug(logPlayerSpawn) << "Spawner:" << prop.propValue << prop.propertyType;
                m_targets->insert(prop.propValue, v);
                return false;
            }
        }
        return true;
    }
};

struct EnemySpawnPointLocator
{
    QMultiHash<QString, glm::mat4> *m_targets;
    EnemySpawnPointLocator(QMultiHash<QString, glm::mat4> *targets) :
        m_targets(targets)
    {}
    bool operator()(SceneNode *n, const glm::mat4 &v)
    {
        if(!n->m_properties)
            return true;

        for (GroupProperty_Data &prop : *n->m_properties)
        {

            if(prop.propName == "EncounterPosition")
            {
                qCDebug(logPlayerSpawn) << "Encounter:" << prop.propValue << prop.propertyType;
                m_targets->insert(prop.propValue, v);
                return false;
            }
        }
        return true;
    }
};

QMultiHash<QString, glm::mat4> MapSceneGraph::getSpawnPoints() const
{
    QMultiHash<QString, glm::mat4> res;
    SpawnPointLocator locator(&res);
    for(auto v : m_scene_graph->refs)
        walkSceneNode(v->node, v->mat, locator);

    return res;
}


struct MapXferLocator
{
    QHash<QString, MapXferData> *m_targets;
    MapXferLocator(QHash<QString, MapXferData> *targets):
        m_targets(targets)
    {}
    bool operator()(SceneNode *n, const glm::mat4 &v)
    {

        if (!n->m_properties)
        {
            for (auto &child : n->m_children)
            {
                bool found_map_transfer = false;
                if (child.node->m_properties != nullptr)
                {
                    MapXferData map_transfer = MapXferData();
                    // Probably haven't processed the map swap node yet, so add it and handle later
                    for (GroupProperty_Data &prop : *child.node->m_properties)
                    {
                        if (prop.propName == "GotoSpawn")
                        {
                            map_transfer.m_target_spawn_name = prop.propValue;
                            found_map_transfer = true;
                        }
                        if (prop.propName == "GotoMap")
                        {
                            map_transfer.m_target_map_name = prop.propValue.split('.')[0];
                            // Assume that if there's a GotoMap, that it's for a map xfer.
                            // TODO: Change the transfer type detection to something less ambiguous if possible.
                            map_transfer.m_transfer_type = MapXferType::ZONE;
                            found_map_transfer = true;
                        }
                    }
                    if (found_map_transfer)
                    {
                        map_transfer.m_node_name = child.node->m_name;

                        // get position
                        glm::mat4 transform(child.m_matrix2);
                        transform[3] = glm::vec4(child.m_translation,1);
                        transform = v * transform;
                        glm::vec4 pos4 {0,0,0,1};
                        pos4 = transform * pos4;
                        glm::vec3 pos3 = glm::vec3(pos4);

                        map_transfer.m_position = pos3;
                        m_targets->insert(map_transfer.m_node_name, map_transfer);
                        return false;
                    }
                }
            }
        }

        return true;
    }
};

QHash<QString, MapXferData> MapSceneGraph::get_map_transfers() const
{
    QHash<QString, MapXferData> res;
    MapXferLocator locator(&res);
    for (auto v : m_scene_graph->refs)
    {
        walkSceneNode(v->node, v->mat, locator);
    }
    return res;
}

struct CritterSpawnLocator
{
    QHash<QString, CritterSpawnLocations> *m_spawn_def;
    CritterSpawnLocator(QHash<QString, CritterSpawnLocations> *spawn_def_hash):
        m_spawn_def(spawn_def_hash)
    {}
    bool operator()(SceneNode *n, const glm::mat4 &v)
    {
        bool found_encounter = false;
        if (n->m_properties)
        {
            CritterSpawnLocations spawnDef;

            for (GroupProperty_Data &gp: *n->m_properties)
            {
                if(gp.propName.contains("SpawnProbability"))
                {
                    spawnDef.m_spawn_probability = gp.propValue.toInt();
                }
                else if(gp.propName.contains("VillainRadius"))
                {
                    spawnDef.m_villain_radius = gp.propValue.toInt();
                }

                if(gp.propName == "EncounterSpawn" || gp.propName == "EncounterGroup")
                {
                    for(auto &child : n->m_children)
                    {
                        if(child.node->m_name.contains("EG_L", Qt::CaseInsensitive)) //Atlas & Galaxy
                        {
                            found_encounter = true;
                            glm::mat4 encounter_location(child.m_matrix2);
                            encounter_location[3] = glm::vec4(child.m_translation,1);
                            encounter_location = v * encounter_location;

                            for(auto &c : child.node->m_children) // _ES_L  EncounterSpawn
                            {
                                for(auto &s : c.node->m_children) // Encounter_
                                {
                                    CritterSpawnPoint *spawn_point = new CritterSpawnPoint();
                                    spawn_point->m_name = s.node->m_name;

                                    glm::mat4 spawn_location(s.m_matrix2);
                                    spawn_location[3] = glm::vec4(s.m_translation,1);
                                    spawn_location = v * spawn_location;
                                    glm::vec4 tpos4 {0,0,0,1};
                                    spawn_point->m_relative_position = encounter_location;

                                    if(spawn_point->m_name.contains("_V_", Qt::CaseSensitive))
                                        spawn_point->m_is_victim = true;

                                    spawnDef.m_all_spawn_points.push_back(*spawn_point);
                                }

                                spawnDef.m_node_name = child.node->m_name;
                                if(found_encounter)
                                {
                                    m_spawn_def->insert(n->m_name, spawnDef);
                                    return false;
                                }
                            }
                        }
                        else if(child.node->m_name.contains("ES_", Qt::CaseInsensitive)) // Other maps
                        {
                            for (GroupProperty_Data &prop : *child.node->m_properties)
                            {
                                if (prop.propName == "EncounterSpawn" || prop.propName == "EncounterGroup")
                                {
                                    found_encounter = true;

                                    glm::mat4 encounter_location(child.m_matrix2);
                                    encounter_location[3] = glm::vec4(child.m_translation,1);
                                    encounter_location = v * encounter_location;

                                    for(auto &c_node : child.node->m_children) // Encounter_
                                    {
                                        CritterSpawnPoint *spawn_point = new CritterSpawnPoint();
                                        spawn_point->m_name = c_node.node->m_name;

                                        glm::mat4 spawn_location(c_node.m_matrix2);
                                        spawn_location[3] = glm::vec4(c_node.m_translation,1);
                                        spawn_location = encounter_location * spawn_location;

                                        spawn_point->m_relative_position = spawn_location;

                                        if(spawn_point->m_name.contains("_V_", Qt::CaseSensitive))
                                            spawn_point->m_is_victim = true;

                                        spawnDef.m_all_spawn_points.push_back(*spawn_point);

                                    }

                                    spawnDef.m_node_name = child.node->m_name;

                                    if(found_encounter)
                                    {
                                        m_spawn_def->insert(n->m_name, spawnDef);
                                        return false;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        return true;
    }
};


void MapSceneGraph::spawn_critters(MapInstance *instance)
{
    QHash<QString, CritterSpawnLocations> res;
    CritterSpawnLocator locator(&res);
    for (auto v : m_scene_graph->refs)
    {
        walkSceneNode(v->node, v->mat, locator);
    }

    //Creates one generator per encounter.
    int count = 1;
    for (auto r: res)
    {
        CritterGenerator cg;
        cg.m_encounter_node_name = r.m_node_name;
        cg.m_generator_name = "Encounter " + QString(count);
        cg.m_critter_encounter = r;
        instance->m_critter_generators.m_generators.insert(cg.m_generator_name, cg);
        ++count;
    }
}


//! @}
