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
#include "MapServer/MapServer.h"
#include "MapServer/MapServerData.h"

#include <algorithm>

EntityManager::EntityManager()
{
    memset(m_map_entities,0,sizeof(Entity *)*10240);
    m_last_ent = 1;
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

void EntityManager::sendDeletes( BitStream &tgt ) const
{
    int num_to_remove=0;
    tgt.StorePackedBits(1,num_to_remove);
    for(int i=0; i<num_to_remove; i++)
    {
        tgt.StorePackedBits(1,0);//index
        tgt.StorePackedBits(1,0);//
    }
}
/**
 *  \par self_idx index of the entity that is receiving the packet, this is used to prevent marking every entity as a current player
 *
 */
void EntityManager::sendEntities( BitStream &bs,int self_idx,bool is_incremental ) const
{
    ACE_Guard<ACE_Thread_Mutex> guard_buffer(m_mutex);

    int prev_idx=-1;
    int delta;
    // sending delta between entities idxs ->
    assert(m_entlist.size()>0 && "Attempting to send empty entity list, the client will hang!");
    for(Entity *pEnt : m_entlist)
    {
        pEnt->m_create_player = (pEnt->getIdx()==self_idx);
        delta = (prev_idx==-1) ? pEnt->getIdx() : (pEnt->getIdx()-prev_idx -1);

        bs.StorePackedBits(1,delta);
        prev_idx = pEnt->getIdx();
        if(!is_incremental) {
            bool prev_state = pEnt->m_change_existence_state;
            pEnt->m_change_existence_state=true;
            serializeto(*pEnt,bs);
            pEnt->m_change_existence_state= prev_state;
        }
        else
            serializeto(*pEnt,bs);
        PUTDEBUG("end of entity");
    }
    // last entity marker
    bs.StorePackedBits(1,0); // next ent
    bs.StoreBits(1,1); // create/upte -> create
    bs.StoreBits(1,1); // empty entity. will finish the receiving loop
}
void EntityManager::InsertPlayer(Entity *ent)
{
    m_map_entities[m_last_ent++] = ent;
    ent->m_idx = m_last_ent-1;
    ent->pos = glm::vec3(128.0,16,-198); //-60.5;
    ent->qrot= glm::quat(1.0f,0.0f,0.0f,0.0f);
    m_entlist.push_back(ent);
}
Entity * EntityManager::CreatePlayer()
{
    Entity *res = new PlayerEntity;
    m_map_entities[m_last_ent++] = res;
    m_entlist.push_back(res);
    res->m_idx = m_last_ent-1;
    return res;
}

void EntityManager::removeEntityFromActiveList(Entity *ent)
{
    auto iter = std::find(m_entlist.begin(),m_entlist.end(),ent);
    assert(iter!=m_entlist.end());
    m_map_entities[ent->m_idx] = nullptr;
    m_entlist.erase(iter);
}
