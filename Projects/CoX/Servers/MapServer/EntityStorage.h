/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "GameData/Entity.h"
#include "GameData/EntityHelpers.h"
#include "HandleBasedStorage.h"

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
    friend class World;
    struct EntityIdxCompare
    {
        bool operator()(const Entity *a,const Entity *b) const
        {
            return getIdx(*a) < getIdx(*b);
        }
    };
    using lEntity = std::set<Entity *,EntityIdxCompare>;
    Entity *        CreateCritter(const Parse_NPC &tpl, int idx, int variant, int level);
    Entity *        CreateGeneric(const Parse_NPC &tpl, int idx, int variant, EntType type);
    Entity *        CreatePlayer();
    void            removeEntityFromActiveList(Entity *ent);
public:
    EntityStore     m_store;
    lEntity         m_live_entlist;
                    EntityManager();
    void            sendDebuggedEntities(BitStream &tgt) const;
    void            sendGlobalEntDebugInfo(BitStream &tgt) const;
    void            sendDeletes(BitStream &tgt, MapClientSession &client) const;
    void            sendEntities(BitStream &tgt, MapClientSession &target, bool is_incremental) const;
    size_t          active_entities() { return m_live_entlist.size(); }
    ACE_Thread_Mutex &getEntitiesMutex() { return m_mutex; }

protected:
    mutable ACE_Thread_Mutex m_mutex; // used to prevent world state reads during updates
};
