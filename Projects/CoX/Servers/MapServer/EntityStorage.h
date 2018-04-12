/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *

 */

#pragma once

#include "Entity.h"
#include "Servers/MapServer/DataHelpers.h"

#include <ace/Thread_Mutex.h>
#include <ace/Guard_T.h>
#include <array>
#include <set>
#include <deque>

class MapClientSession;
class MapInstance;
class Entity;
class BitStream;
class EntityStore
{
public:
    EntityStore();
    std::deque<int32_t> m_free_entries;
    std::array<Entity,10240> m_map_entities;
    Entity *get();
    void release(Entity *src);
};
class EntityManager
{
    struct EntityIdxCompare {
        bool operator()(const Entity *a,const Entity *b) const {
            return getIdx(*a) < getIdx(*b);
        }
    };
    using lEntity = std::set<Entity *,EntityIdxCompare>;
public:
    EntityStore     m_store;
    lEntity         m_live_entlist;
                    EntityManager();
    void            sendDebuggedEntities(BitStream &tgt) const;
    void            sendGlobalEntDebugInfo(BitStream &tgt) const;
    void            sendDeletes(BitStream &tgt, MapClientSession *client) const;
    void            sendEntities(BitStream &tgt, MapClientSession *target, bool is_incremental) const;
    void            InsertPlayer(Entity *);
    Entity *        CreatePlayer();
    void            removeEntityFromActiveList(Entity *ent);
    size_t          active_entities() { return m_live_entlist.size(); }
    ACE_Thread_Mutex &getEntitiesMutex() { return m_mutex; }

protected:
    mutable ACE_Thread_Mutex m_mutex; // used to prevent world state reads during updates
};

class EntityStorage
{
public:
//  void NewPlayer(MapClient *client,Entity *player_ent); // stores fresh player avatar
//  void StorePlayer(MapClient *client,Entity *player_ent); // stores player avatar
        Entity *    CreatePlayer(MapClientSession *client,int avatar_id); // retrieves client avatar from storage
        Entity *    CreateInstance(MapInstance *target_world,uint64_t id); // will create a new instance of given entity, bound to given map
};
