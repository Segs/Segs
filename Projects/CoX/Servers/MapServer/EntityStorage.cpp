/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *

 */

#include "Entity.h"
#include "EntityStorage.h"
//EntityManager ent_mgr;

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

void EntityManager::sendEntities( BitStream &tgt ) const
{
        Entity *pEnt = NULL;
        std::list<Entity *>::const_iterator iter = m_entlist.begin();
        int last_idx;
        int delta;// sending delta between entities idxs ->
        assert(m_entlist.size()>0 && "Attempting to send empty entity list, the client will hang!");
        if(iter!=m_entlist.end())
        {
                pEnt = *iter;
                tgt.StorePackedBits(1,pEnt->getIdx());
                last_idx = pEnt->getIdx();
                pEnt->serializeto(tgt);
                iter++;
        }
        while(iter!=m_entlist.end())
        {
                //assert(!"Only one for now");
                pEnt = *iter;
                delta = pEnt->getIdx()-last_idx -1;
                tgt.StorePackedBits(1,delta);
                last_idx = pEnt->getIdx();
                pEnt->serializeto(tgt);
                iter++;
        }
        // last entity marker
        tgt.StorePackedBits(1,0); // next ent
        tgt.StoreBits(1,1); // create/update -> create
        tgt.StoreBits(1,1); // empty entity. will finish the receiving loop
}
void EntityManager::InsertPlayer(Entity *ent)
{
    m_map_entities[m_last_ent++] = ent;
    ent->m_idx = m_last_ent-1;
    ent->pos = osg::Vec3(128.0,16,-198); //-60.5;
    ent->qrot= osg::Quat(1.0f,0.0f,0.0f,1.0f);
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
