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

#include "WorldSimulation.h"

#include "Common/Servers/Database.h"
#include "DataHelpers.h"
#include "Messages/Map/GameCommandList.h"
#include "GameData/Character.h"
#include "GameData/CharacterHelpers.h"
#include <glm/gtx/vector_query.hpp>

#include "MapInstance.h"
#include "Common/Servers/InternalEvents.h"

using namespace SEGSEvents;
using namespace SEGS;

namespace  {
/******************************** MAP-SPAWN SUPPORT ROUTINES ************************************/
struct CritterSpawnLocator
{
    QHash<QString, CritterSpawnLocations> *m_spawn_def;
    CritterSpawnLocator(QHash<QString, CritterSpawnLocations> *spawn_def_hash):
        m_spawn_def(spawn_def_hash)
    {}
    bool operator()(SceneNode *n, const glm::mat4 &v)
    {
        bool found_encounter = false;
        if (!n->m_properties)
            return true;
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
        return true;
    }
};
struct NpcCreator
{
    World *map_instance = nullptr;
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

            if(!npc_def)
                return false;

            int idx = npc_store.npc_idx(npc_def);
            Entity *e = map_instance->CreateNpc(*npc_def, idx, 0);
            e->m_char->setName(makeReadableName(persistent_name));

            glm::vec3 pos = glm::vec3(v[3]);
            glm::quat valquat = glm::quat_cast(v);
            glm::vec3 angles = glm::eulerAngles(valquat);
            angles.y += glm::pi<float>();
            assert(std::abs(glm::dot(valquat,fromCoHYpr(angles)))>(1.0f-0.001f));
            map_instance->m_physics.forcePositionAndOrientation(*e,pos,angles);
            e->m_motion_state.m_velocity = { 0,0,0 };
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
struct MapXferLocator
{
    QHash<QString, MapXferData> *m_targets;
    MapXferLocator(QHash<QString, MapXferData> *targets):
        m_targets(targets)
    {}
    bool operator()(SceneNode *n, const glm::mat4 &v)
    {

        if (n->m_properties)
            return true;
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

        return true;
    }
};

} // end of anonymous namespace
World::World(EntityManager &em, const float player_fade_in, MapInstance *owner_instance)
    : ref_ent_mager(em), m_player_fade_in(player_fade_in), m_owner_instance(owner_instance)
{
}

void spawn_critters(MapSceneGraph *map_scene_graph,World *instance)
{
    QHash<QString, CritterSpawnLocations> res;
    CritterSpawnLocator locator(&res);
    map_scene_graph->visitNodesFromRoots(locator);

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
void spawn_npcs(MapSceneGraph *map_scene_graph,World *instance)
{
    NpcCreator creator;
    creator.generators = &instance->m_npc_generators;
    creator.map_instance = instance;
    map_scene_graph->visitNodesFromRoots(creator);
}

static QMultiHash<QString, glm::mat4> getSpawnPoints(MapSceneGraph *map_scene_graph)
{
    QMultiHash<QString, glm::mat4> res;
    SpawnPointLocator locator(&res);
    map_scene_graph->visitNodesFromRoots(locator);
    return res;
}

QHash<QString, MapXferData> get_map_transfers(MapSceneGraph *map_scene_graph)
{
    QHash<QString, MapXferData> res;
    MapXferLocator locator(&res);
    map_scene_graph->visitNodesFromRoots(locator);
    return res;
}

bool World::start(const QString &scenegraph_path)
{
    bool scene_graph_loaded=false;
    TIMED_LOG({
            m_map_scenegraph = new MapSceneGraph;
            scene_graph_loaded = m_map_scenegraph->loadFromFile("./data/geobin/" + scenegraph_path);
            m_all_spawners = getSpawnPoints(m_map_scenegraph);
            m_map_transfers = get_map_transfers(m_map_scenegraph);
            prepareZoneTransfers();
        }, "Loading original scene graph");

    TIMED_LOG({
        spawn_npcs(m_map_scenegraph,this);
        m_npc_generators.generate(this);
        spawn_critters(m_map_scenegraph,this);
        m_critter_generators.generate(this);
        }, "Spawning npcs");
    return scene_graph_loaded;
}

const QHash<QString, MapXferData> &World::get_map_door_transfers() const
{
    return m_map_door_transfers;
}

const QHash<QString, MapXferData> &World::get_map_zone_transfers() const
{
    return m_map_zone_transfers;
}

void World::prepareZoneTransfers()
{
    QHash<QString, MapXferData>::const_iterator i = m_map_transfers.constBegin();
    for ( ;i != m_map_transfers.constEnd(); ++i)
    {
        if (i.value().m_transfer_type == MapXferType::DOOR)
        {
            m_map_door_transfers.insert(i.key(), i.value());
        }
        else if (i.value().m_transfer_type == MapXferType::ZONE)
        {
            m_map_zone_transfers.insert(i.key(), i.value());
        }
    }
}

void World::update(const ACE_Time_Value &tick_timer)
{
    ACE_Time_Value delta;
    if(prev_tick_time==ACE_Time_Value::zero)
    {
        delta = ACE_Time_Value(0,33*1000);
    }
    else
        delta = tick_timer - prev_tick_time;
    m_time_of_day+= 4.8f*((float(delta.msec())/1000.0f)/(60.0f*60)); // 1 sec of real time is 48s of ingame time
    if(m_time_of_day>=24.0f)
        m_time_of_day-=24.0f;
    sim_frame_time = delta.msec()/1000.0f;
    accumulated_time += sim_frame_time;
    prev_tick_time = tick_timer;
    ACE_Guard<ACE_Thread_Mutex> guard_buffer(ref_ent_mager.getEntitiesMutex());

    m_physics.update(delta.usec()/1000000.0f);

    for(Entity * e : ref_ent_mager.m_live_entlist)
        updateEntity(e,delta);
}

Entity *World::CreateCritter(const Parse_NPC &tpl, int idx, int variant, int level)
{
    return ref_ent_mager.CreateCritter(tpl,idx,variant,level);
}

Entity *World::CreateGeneric(const Parse_NPC &tpl, int idx, int variant, EntType type)
{
    Entity *res = ref_ent_mager.m_store.get();
    ref_ent_mager.m_live_entlist.insert(res);

    initializeNewNpcEntity(*res,&tpl,idx,variant);
    res->m_type = type;
    res->m_world = this;
    m_physics.addEntity(res);
    return res;
}

Entity *World::CreateNpc(const Parse_NPC &tpl, int idx, int variant)
{
    return CreateGeneric(tpl,idx,variant,EntType::Invalid);
}

Entity *World::CreatePlayer()
{
    return ref_ent_mager.CreatePlayer();
}

void World::releaseEntity(Entity *e)
{
    ref_ent_mager.removeEntityFromActiveList(e);
    m_physics.removeEntity(e);
}

glm::vec3 World::closest_safe_location(glm::vec3 v) const
{
    // In the future this should get the closet NAV or NAVCMBT
    // node and return it. For now let's just pick some known
    // safe spawn locations

    Q_UNUSED(v);
    glm::vec3 loc = glm::vec3(0,0,0);

    // If no default spawners if available, spawn at 0,0,0
    if(m_all_spawners.empty())
        return loc;

    // Try NewPlayer spawners first, then hospitals, then random
    if(m_all_spawners.contains("NewPlayer"))
        loc = m_all_spawners.values("NewPlayer")[rand() % m_all_spawners.values("NewPlayer").size()][3];
    else if(m_all_spawners.contains("PlayerSpawn"))
        loc = m_all_spawners.values("PlayerSpawn")[rand() % m_all_spawners.values("PlayerSpawn").size()][3];
    else if(m_all_spawners.contains("LinkFrom_Monorail_Red"))
        loc = m_all_spawners.values("LinkFrom_Monorail_Red")[rand() % m_all_spawners.values("LinkFrom_Monorail_Red").size()][3];
    else if(m_all_spawners.contains("LinkFrom_Monorail_Blue"))
        loc = m_all_spawners.values("LinkFrom_Monorail_Blue")[rand() % m_all_spawners.values("LinkFrom_Monorail_Blue").size()][3];
    else if(m_all_spawners.contains("Citywarp01"))
        loc = m_all_spawners.values("Citywarp01")[rand() % m_all_spawners.values("Citywarp01").size()][3];
    else if(m_all_spawners.contains("Hospital_Exit"))
        loc = m_all_spawners.values("Hospital_Exit")[rand() % m_all_spawners.values("Hospital_Exit").size()][3];
    else
        loc = m_all_spawners.values()[rand() % m_all_spawners.values().size()][3];

    return loc;
}

// Teleport to a specific SpawnLocation; do nothing if the SpawnLocation is not found.
void World::setSpawnLocation(Entity &e, const QString &spawnLocation)
{
    if(m_all_spawners.empty() || !m_all_spawners.contains(spawnLocation))
        return;

    glm::mat4 v = m_all_spawners.values(spawnLocation)[rand() % m_all_spawners.values(spawnLocation).size()];

    // Position
    glm::vec3 spawn_pos = glm::vec3(v[3]);

    // Orientation
    auto valquat = glm::quat_cast(v);
    glm::vec3 spawn_pyr = toCoH_YPR(valquat);
    m_physics.forcePositionAndOrientation(e,spawn_pos,spawn_pyr);
}

void World::setPlayerSpawn(Entity &e)
{
    // Spawn player position and PYR
    glm::vec3 spawn_pos = glm::vec3(128.0f,16.0f,-198.0f);
    glm::vec3 spawn_pyr = glm::vec3(0.0f, 0.0f, 0.0f);

    if(!m_all_spawners.empty())
    {
        glm::mat4 v = glm::mat4(1.0f);

        if(m_all_spawners.contains("NewPlayer"))
            v = m_all_spawners.values("NewPlayer")[rand() % m_all_spawners.values("NewPlayer").size()];
        else if(m_all_spawners.contains("PlayerSpawn"))
            v = m_all_spawners.values("PlayerSpawn")[rand() % m_all_spawners.values("PlayerSpawn").size()];
        else if(m_all_spawners.contains("Citywarp01"))
            v = m_all_spawners.values("Citywarp01")[rand() % m_all_spawners.values("Citywarp01").size()];
        else
        {
            qWarning() << "No default spawn location found. Spawning at random spawner";
            v = m_all_spawners.values()[rand() % m_all_spawners.values().size()];
        }

        // Position
        spawn_pos = glm::vec3(v[3]);

        // Orientation
        auto valquat = glm::quat_cast(v);
        spawn_pyr = toCoH_YPR(valquat);
    }
    m_physics.forcePositionAndOrientation(e,spawn_pos,spawn_pyr);
}
float animateValue(float v,float start,float target,float length,float dT)
{
    float range=target-start;
    float current_pos = (v-start)/range;
    float accumulated_time = length*current_pos;
    accumulated_time = std::min(length,accumulated_time+dT);
    float res = start + (accumulated_time/length) * range;
    return res;
}

void World::effectsStep(Entity *e,uint32_t msec)
{
    if(e->m_is_fading)
    {
        float target=0.0f;
        float start=1.0f;
        if(e->m_fading_direction!=FadeDirection::In)
        { // must be fading out, so our target is 100% transparency.
            target = 1;
            start = 0;
        }
        e->translucency = animateValue(e->translucency,start,target,m_player_fade_in,float(msec)/50.0f);
        if(std::abs(e->translucency-target)<std::numeric_limits<float>::epsilon())
            e->m_is_fading = false;
    }
}

void World::checkPowerTimers(Entity *e, uint32_t msec)
{
    // for now we only run this on players
    if(e->m_type != EntType::PLAYER)
        return;

    // Activation Timers -- queue FIFO
    if(e->m_queued_powers.size() > 0)
    {
        QueuedPowers &qpow = e->m_queued_powers.front();
        qpow.m_activate_period -= (float(msec)/1000);

        // this must come before the activation_period check
        // to ensure that the queued power ui-effect is reset properly
        if(qpow.m_activation_state == false)
        {
            e->m_queued_powers.pop_front(); // remove first from queue
            e->m_char->m_char_data.m_has_updated_powers = true;
        }

        if(qpow.m_activate_period <= 0)
            qpow.m_activation_state = false;
    }

    // Recharging Timers -- iterate through and remove finished timers
    for(auto rpow_idx = e->m_recharging_powers.begin(); rpow_idx != e->m_recharging_powers.end(); /*rpow_idx updated inside loop*/ )
    {
        rpow_idx->m_recharge_time -= (float(msec)/1000);

        if(rpow_idx->m_recharge_time <= 0)
        {
            PowerVecIndexes power_idx(rpow_idx->m_pow_idxs);
            rpow_idx = e->m_recharging_powers.erase(rpow_idx);

            // Check if rpow is default power, if so usePower again
            if(e->m_char->m_char_data.m_trays.m_has_default_power)
            {
                if(power_idx.m_pset_vec_idx == e->m_char->m_char_data.m_trays.m_default_pset_idx
                        && power_idx.m_pow_vec_idx == e->m_char->m_char_data.m_trays.m_default_pow_idx)
                {
                    usePower(*e, power_idx.m_pset_vec_idx, power_idx.m_pow_vec_idx, getTargetIdx(*e), getTargetIdx(*e));
                }
            }

            e->m_char->m_char_data.m_has_updated_powers = true;
        }
        else
            ++rpow_idx;
    }

    // Buffs
    for(auto buff_idx = e->m_buffs.begin(); buff_idx!=e->m_buffs.end(); /*buff_idx updated inside loop*/)
    {
        buff_idx->m_activate_period -= (float(msec)/1000); // activate period is in minutes

        if(buff_idx->m_activate_period <= 0)
            buff_idx = e->m_buffs.erase(buff_idx);
        else
            ++buff_idx;
    }
}

bool World::isPlayerDead(Entity *e)
{
    if(e->m_type == EntType::PLAYER
            && getHP(*e->m_char) == 0.0f)
    {
        setStateMode(*e, ClientStates::DEAD);
        return true;
    }

    return false;
}

void World::regenHealthEnd(Entity *e, uint32_t msec)
{
    // for now on Players only
    if(e->m_type == EntType::PLAYER)
    {
        float hp = getHP(*e->m_char);
        float end = getEnd(*e->m_char);

        float regeneration = hp * (1.0f/20.0f) * float(msec)/1000/12;
        float recovery = end * (1.0f/15.0f) * float(msec)/1000/12;

        if(hp < getMaxHP(*e->m_char))
            setHP(*e->m_char, hp + regeneration);
        if(end < getMaxEnd(*e->m_char))
            setEnd(*e->m_char, end + recovery);
    }
}

void World::collisionStep(Entity *e, uint32_t /*msec*/)
{
    if (e->m_player != nullptr && !e->m_map_swap_collided)
    {
        // Range-For only uses the values, so you can't get the keys unless you use toStdMap() or iterate keys().
        // Both are less efficient than just using an iterator.
        QHash<QString, MapXferData>::const_iterator i = get_map_zone_transfers().constBegin();
        while (i != get_map_zone_transfers().constEnd())
        {
            // TODO: This needs to check against the trigger plane for transfers. This should be part of the wall objects geobin. Also need to make sure that this doesn't cause players to immediately zone after being spawned in a spawnLocation near a zoneline.
            if ((e->m_entity_data.m_pos.x >= i.value().m_position.x - 20 && e->m_entity_data.m_pos.x <= i.value().m_position.x + 20) &&
                (e->m_entity_data.m_pos.y >= i.value().m_position.y - 20 && e->m_entity_data.m_pos.y <= i.value().m_position.y + 20) &&
                (e->m_entity_data.m_pos.z >= i.value().m_position.z - 20 && e->m_entity_data.m_pos.z <= i.value().m_position.z + 20))
            {
                e->m_map_swap_collided = true;  // So we don't send repeated events for the same entity
                m_owner_instance->putq(new MapSwapCollisionMessage({e->m_db_id, e->m_entity_data.m_pos, i.key()}, 0));
                return; // don't want to keep checking for other maps for this entity
            }
            i++;

        }
    }
}

void World::updateEntity(Entity *e, const ACE_Time_Value &dT)
{
    effectsStep(e, dT.msec());
    checkPowerTimers(e, dT.msec());
    collisionStep(e, dT.msec());
    // TODO: Issue #555 needs to handle team cleanup properly
    // and we need to remove the following
    if(e->m_team != nullptr)
    {
        if(e->m_team->m_team_members.size() <= 1)
        {
            qWarning() << "Team cleanup being handled in updateEntity, but we need to move this to TeamHandler";
            e->m_has_team = false;
            e->m_team = nullptr;
        }
    }

    // check death, set clienstate if dead, and
    // if alive, recover endurance and health
    if(!isPlayerDead(e))
        regenHealthEnd(e, dT.msec());

    if(e->m_is_logging_out)
    {
        e->m_time_till_logout -= dT.msec();
        if(e->m_time_till_logout<0)
            e->m_time_till_logout=0;
    }
}

void WorldPhysics::entitiesInRange(const Sphere &range, std::vector<int> &entity_idx)
{

}

void WorldPhysics::update(float dt)
{
    //Copy input speed to velocity, taking care to use proper surface params;
    for(PhysicsState &p : m_per_entity_data)
    {
        Entity *ent = p.m_ent;
        glm::vec3 input_speed = ent->m_states.current()->m_pos_delta;
        if(input_speed!=glm::vec3(0))
        {
            SurfaceParams surf = ent->m_motion_state.m_surf_mods[p.m_mode!=PhysicsState::GROUND_MOVEMENT];
            // truncate the speed vector to max_speed.
            if(glm::length(input_speed)>surf.max_speed)
                input_speed = glm::normalize(input_speed)*surf.max_speed;
        }
        p.m_velocity = input_speed;
    }
    // put the moving entities at the front
    auto first_inactive=std::stable_partition(m_per_entity_data.begin(),m_per_entity_data.end(),[](const PhysicsState &a)->bool {
        return glm::length2(a.m_velocity)!=0.0f;
    });
    for(auto iter=m_per_entity_data.begin(); iter!=first_inactive; ++iter)
    {
        Entity *e=iter->m_ent;
        setVelocity(*e);
        //e->m_motion_state.m_velocity = za*e->m_states.current()->m_pos_delta;

        // todo: take into account time between updates
        glm::mat3 za = static_cast<glm::mat3>(iter->m_orientation); // quat to mat4x4 conversion
        //float vel_scale = e->inp_state.m_input_vel_scale/255.0f;
        const float time_scale_factor = 40.0f/1000.0f; // formerly 50.0f
        e->m_entity_data.m_pos += ((za*e->m_states.current()->m_pos_delta)*dt)/time_scale_factor;
    }

}

void WorldPhysics::forcePosition(Entity &e, glm::vec3 pos)
{
    PhysicsState *state = stateForEntity(&e);
    assert(state); // If this fails, it means we're trying to set physics data for entity without it.
    state->m_position = pos;
    e.m_entity_data.m_pos = pos;
    e.m_force_pos_and_cam = true;
}

void WorldPhysics::forceOrientation(Entity &e, glm::vec3 pyr)
{
    PhysicsState *state = stateForEntity(&e);
    assert(state); // If this fails, it means we're trying to set physics data for entity without it.
    state->m_orientation = fromCoHYpr(pyr);
    e.m_entity_data.m_orientation_pyr = pyr;
    e.m_force_pos_and_cam = true;
}

void WorldPhysics::forcePositionAndOrientation(Entity &e, glm::vec3 pos, glm::vec3 pyr)
{
    PhysicsState *state = stateForEntity(&e);
    assert(state); // If this fails, it means we're trying to set physics data for entity without it.
    state->m_position = pos;
    e.m_entity_data.m_pos = pos;
    state->m_orientation = fromCoHYpr(pyr);
    e.m_entity_data.m_orientation_pyr = pyr;
    e.m_force_pos_and_cam = true;
}

void WorldPhysics::addEntity(Entity *e)
{
    //TODO: check for adding same entity multiple times
    m_per_entity_data.emplace_back();
    m_per_entity_data.back().m_ent = e;
}

void WorldPhysics::removeEntity(Entity *e)
{
    auto iter = std::find_if(m_per_entity_data.begin(),m_per_entity_data.end(),[e](const PhysicsState &s)->bool {
        return s.m_ent==e;
    });
    if(iter!=m_per_entity_data.end())
    {
        // "swap & pop"
        // overwrite current with last
        *iter = std::move(m_per_entity_data.back());
        // remove last
        m_per_entity_data.pop_back();
    }
}

WorldPhysics::PhysicsState *WorldPhysics::stateForEntity(const Entity *e)
{
    auto iter = std::find_if(m_per_entity_data.begin(),m_per_entity_data.end(),[e](const PhysicsState &s)->bool {
        return s.m_ent==e;
    });
    if(iter==m_per_entity_data.end())
        return nullptr;
    return &(*iter);
}
// Free standing helper functions
void movePlayerToClosestSafeLocation(World &world, Entity *player)
{
    world.m_physics.forcePosition(*player,world.closest_safe_location(player->m_entity_data.m_pos));
}

void movePlayerToNamedSpawn(World &world, Entity *player, const QString &name)
{
    if(world.getSpawners().contains(name))
    {
        world.setSpawnLocation(*player, name);
    }
    else
    {
        world.setPlayerSpawn(*player);
    }
}

QString getNearestDoor(World *world, glm::vec3 location)
{
    float door_distance_check = 15.f;
    QHash<QString, MapXferData>::const_iterator i = world->get_map_door_transfers().constBegin();
    while (i != world->get_map_door_transfers().constEnd())
    {
        if (glm::distance(location, i.value().m_position) < door_distance_check)
        {
            return i.value().m_target_spawn_name;
        }
        i++;
    }
    return QString();
}

// Poll EntityManager to return Entity by Name or IDX
Entity * World::getEntity(const QString &name)
{
    EntityManager &em(ref_ent_mager);
    QString errormsg;

    // Iterate through all active entities and return entity by name
    for (Entity* pEnt : em.m_live_entlist)
    {
        if(pEnt->name() == name)
            return pEnt;
    }

    errormsg = "Entity " + name + " does not exist, or is not currently online.";
    qWarning() << errormsg;
    return nullptr;
}

Entity * World::getEntity(uint32_t idx)
{
    EntityManager &em(ref_ent_mager);
    QString errormsg;

    if(idx!=0) // Entity idx 0 is special case, so we can't return it
    {
        // Iterate through all active entities and return entity by idx
        for (Entity* pEnt : em.m_live_entlist)
        {
            if(pEnt->m_idx == idx)
                return pEnt;
        }
    }
    errormsg = "Entity " + QString::number(idx) + " does not exist, or is not currently online.";
    qWarning() << errormsg;
    return nullptr;
}
/**
 * @brief Finds the Entity in this World
 * @param db_id db id of the entity to find.
 * @return pointer to the entity or nullptr if it does not exist.
 */
Entity *World::getEntityByDBID(uint32_t db_id)
{
    EntityManager &em(ref_ent_mager);
    QString        errormsg;

    if(db_id == 0)
        return nullptr;
    // TODO: Iterate through all entities in Database and return entity by db_id
    for (Entity *pEnt : em.m_live_entlist)
    {
        if(pEnt->m_db_id == db_id)
            return pEnt;
    }
    return nullptr;
}


//! @}
