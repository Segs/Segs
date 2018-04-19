/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *

 */

#pragma once

#include "NetStructures/Entity.h"
#include "Servers/MapServer/DataHelpers.h"

#include <ace/Thread_Mutex.h>
#include <ace/Guard_T.h>
#include <array>
#include <set>
#include <deque>

struct MapClientSession;
class MapInstance;
class Entity;
class BitStream;
class EntityStore
{
public:
    EntityStore();
    std::deque<uint32_t> m_free_entries;
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
    Entity *        CreateNpc(const Parse_NPC &tpl, int idx, int variant);
    void            removeEntityFromActiveList(Entity *ent);
    size_t          active_entities() { return m_live_entlist.size(); }
    ACE_Thread_Mutex &getEntitiesMutex() { return m_mutex; }

protected:
    mutable ACE_Thread_Mutex m_mutex; // used to prevent world state reads during updates
};
