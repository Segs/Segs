/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
  */

#include "EntityStorage.h"
#include "Entity.h"
#include "EntityUpdateCodec.h"
#include "MapClient.h"
#include "MapServer/MapServer.h"
#include "MapServer/MapServerData.h"

#include <QtCore/QDebug>
#include <algorithm>


EntityStore::EntityStore()
{
    int32_t idx=0;
    for(Entity &e : m_map_entities)
    {
        e.m_idx = idx++;
    }
    // starting from 1 to prevent returning special entity idx 0 to anyone
    for(int i=1; i<m_map_entities.size(); ++i)
        m_free_entries.emplace_back(i);
}

Entity *EntityStore::get()
{
    assert(!m_free_entries.empty());
    int idx = m_free_entries.front();
    m_free_entries.pop_front();
    m_map_entities[idx].m_destroyed = false;
    return &m_map_entities[idx];
}

void EntityStore::release(Entity *src)
{
    assert(src>m_map_entities.data() && src<m_map_entities.data()+m_map_entities.size());
    src->m_destroyed = true;
    m_free_entries.push_back(src->m_idx);
}

EntityManager::EntityManager()
{
}

void EntityManager::sendDebuggedEntities( BitStream &tgt ) const
{
    tgt.StorePackedBits(10,0); // index of debugged entity
    return ;
    //Send debug info

}

void EntityManager::sendGlobalEntDebugInfo( BitStream &tgt ) const
{
    tgt.StoreBits(1,0); // nothing here for now
    // first while loop here
    tgt.StoreBits(1,0);
    // second while loop here
    tgt.StoreBits(1,0);
    // if here
    // players in zone and such
    tgt.StoreBits(1,0);
    // third while loop here
}

void EntityManager::sendDeletes( BitStream &tgt,MapClient *client ) const
{
    std::vector<int> entities_to_remove;
    // find the entities this client believes exist, but they are no longer amongst us.
    for(const std::pair<int,ClientEntityStateBelief> &entry : client->m_worldstate_belief)
    {
        if(entry.second.m_entity==nullptr)
            continue;
        if(m_live_entlist.end()==m_live_entlist.find((Entity *)entry.second.m_entity))
            entities_to_remove.push_back(entry.first);
    }
    tgt.StorePackedBits(1,entities_to_remove.size());
    for(int idx : entities_to_remove)
    {
        tgt.StorePackedBits(12,idx);//index
        tgt.StorePackedBits(12,idx);//
        client->m_worldstate_belief.erase(idx);
    }
}
/**
 *  \par self_idx index of the entity that is receiving the packet, this is used to prevent marking every entity as a current player
 *
 */
void EntityManager::sendEntities(BitStream& bs, MapClient *target, bool is_incremental) const
{
    ACE_Guard<ACE_Thread_Mutex> guard_buffer(m_mutex);
    int self_idx = getIdx(*target->char_entity());
    int prev_idx = -1;
    int delta;
    if(m_live_entlist.empty())
    {
        qDebug() << "Trying to send an empty entity list, probably leftover packets to last disconnected client";
        return;
    }

    lEntity to_send = m_live_entlist;
    lEntity client_belief_set;

    for (Entity* pEnt : m_live_entlist)
    {
        bool client_believes_this_entity_exists=client_belief_set.find(pEnt)!=client_belief_set.end();
        if(!client_believes_this_entity_exists && pEnt->m_destroyed)
            continue;
        pEnt->m_create_player = (getIdx(*pEnt) == self_idx);
        delta = (prev_idx == -1) ? getIdx(*pEnt) : (getIdx(*pEnt) - prev_idx - 1);

        bs.StorePackedBits(1, delta);
        prev_idx = getIdx(*pEnt);
        ClientEntityStateBelief &belief(target->m_worldstate_belief[pEnt->m_idx]);
        if(!target->m_in_map)
            belief.m_entity = nullptr; // force full creates until client is actualy in map
        serializeto(*pEnt,belief, bs);
        PUTDEBUG("end of entity");
    }

    // last entity marker
    bs.StorePackedBits(1, 0); // next ent
    bs.StoreBits(1, 1); // create/upte -> create
    bs.StoreBits(1, 1); // empty entity. will finish the receiving loop
}
void EntityManager::InsertPlayer(Entity *ent)
{
    ent->pos = glm::vec3(128.0,16,-198); //-60.5;
    ent->qrot= glm::quat(1.0f,0.0f,0.0f,0.0f);
    m_live_entlist.insert(ent);
}
Entity * EntityManager::CreatePlayer()
{
    Entity *res = m_store.get();
    m_live_entlist.insert(res);
    initializeNewPlayerEntity(*res);
    return res;
}

void EntityManager::removeEntityFromActiveList(Entity *ent)
{
    ent->m_client = nullptr;
    m_live_entlist.erase(ent);
    m_store.release(ent);
}
