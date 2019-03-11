/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "MapServer.h"
#include "GameData/Entity.h"

#include "SceneGraph.h"
#include "MapSceneGraph.h"
#include "NpcGenerator.h"
#include "CritterGenerator.h"

#include <glm/matrix.hpp>
#include <glm/vec3.hpp>
#include <ace/Time_Value.h>

#include <vector>

class EntityManager;

struct Sphere
{
    glm::vec3 center;
    float radius;
};
struct AABB
{
    glm::vec3 min;
    glm::vec3 max;
};
class WorldPhysics
{
public:
    // Physics state will likely be replaced with MotionState in the feature
    // but it will be split into smaller parts put into separate arrays
    struct PhysicsState
    {
        enum MovementMode
        {
            GROUND_MOVEMENT,
            JUMP,
            FLYING
        };
        glm::vec3 m_position= {0,0,0};
        glm::vec3 m_velocity= {0,0,0};
        MovementMode m_mode = GROUND_MOVEMENT;
        glm::quat m_orientation = {1,0,0,0};
        //TODO: replace this with HEntity (handle) ?
        Entity *m_ent=nullptr;
    };
private:    
    std::vector<PhysicsState> m_per_entity_data;
public:
    // state queries.
    void entitiesInRange(const Sphere &range,std::vector<int> &entity_idx);
    
    void update(float dt);
    void forcePosition(Entity &e, glm::vec3 pos);
    void forceOrientation(Entity &e, glm::vec3 pyr);
    void forcePositionAndOrientation(Entity &e, glm::vec3 pos,glm::vec3 pyr);
    
    void addEntity(Entity *e);
    void removeEntity(Entity *e);
    
    PhysicsState *stateForEntity(const Entity *e);
};
class World
{
    using SpawnsStorage = QMultiHash<QString, glm::mat4>;
    MapSceneGraph *         m_map_scenegraph;
    NpcGeneratorStore       m_npc_generators;
    CritterGeneratorStore   m_critter_generators;
    SpawnDefinitions        m_enemy_spawn_definitions;
    SpawnsStorage           m_all_spawners;
    // I think there's probably a better way to do this..
    // We load all transfers for the map to map_transfers, then on first access to zones or doors, we
    // then copy the relevant transfers to another hash which is then used for those specific transfers.
    // This means we only need to traverse the scenegraph once to get all transfers, but need to copy once
    // as well, rather than having to walk the scenegraph twice (once for each type).
    QHash<QString, MapXferData> m_map_door_transfers;
    QHash<QString, MapXferData> m_map_zone_transfers;
    
    
    friend void spawn_critters(MapSceneGraph *map_scene_graph,World *instance);
    friend void spawn_npcs(MapSceneGraph *map_scene_graph,World *instance);
public:
                        World(EntityManager &em, const float player_fade_in, MapInstance *owner_instance);
        bool start(const QString &scenegraph_path);                        
        void            update(const ACE_Time_Value &tick_timer);
        float           time_of_day() const { return m_time_of_day; }
        Entity *        CreateCritter(const Parse_NPC &tpl,int idx,int variant, int level);
        Entity *        CreateGeneric(const Parse_NPC &tpl,int idx,int variant,EntType type);
        Entity *        CreateNpc(const Parse_NPC &tpl,int idx,int variant);
        
        // Scene graph element access functions:
        const SpawnsStorage &getSpawners() const
                        {
                            return m_all_spawners;
                        }
        glm::vec3       closest_safe_location(glm::vec3 v) const;
        void            setSpawnLocation(Entity &e, const QString &spawnLocation);
        void            setPlayerSpawn(Entity &e);
        
        const QHash<QString, MapXferData> &get_map_door_transfers() const;
        const QHash<QString, MapXferData> &get_map_zone_transfers() const;
        
        float           sim_frame_time = 1; // in seconds
        float           accumulated_time=0;
        // for now physics state is publicly accessible.
        WorldPhysics    m_physics;
        // public for now, since MapInstance was using this directly
        QHash<QString, MapXferData> m_map_transfers;
protected:
        void            effectsStep(Entity *e, uint32_t msec);
        void            checkPowerTimers(Entity *e, uint32_t msec);
        bool            isPlayerDead(Entity *e);
        void            regenHealthEnd(Entity *e, uint32_t msec);
        void            updateEntity(Entity *e, const ACE_Time_Value &dT);
        void            collisionStep(Entity *e, uint32_t msec);
        void            prepareZoneTransfers();
        EntityManager & ref_ent_mager;
        float           m_player_fade_in;
        float           m_time_of_day = 8.0f; // hour of the day in 24h format, start at 8am
        ACE_Time_Value  prev_tick_time;
        MapInstance *   m_owner_instance;
};
void movePlayerToClosestSafeLocation(World &world, Entity *player);
void movePlayerToNamedSpawn(World &world, Entity *player,const QString &name);
QString getNearestDoor(World *world,glm::vec3 location);