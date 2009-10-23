/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: EntityStorage.cpp 301 2006-12-26 15:50:44Z nemerle $
 */

#include "MapHandler.h"
#include "Entity.h"
#include "GameProtocol.h"
#include "PacketCodec.h"
#include "ServerManager.h"
#include "EntityStorage.h"
//EntityManager ent_mgr;
void EntityStorage::StorePlayer( MapClient *client,Entity *player_ent )
{
	
}

void EntityManager::sendDebuggedEntities( BitStream &tgt ) const
{
	tgt.StorePackedBits(10,13); // index of debugged entity
}

void EntityManager::sendGlobalEntDebugInfo( BitStream &tgt ) const
{
	tgt.StoreBits(1,0); // nothing here for now
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
	list<Entity *>::const_iterator iter = m_entlist.begin();
	int last_idx;
	int delta;// sending delta between entities idxs -> 
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

Entity * EntityManager::CreatePlayer()
{
	Entity *res = new PlayerEntity;
	m_map_entities[m_last_ent++] = res;
	m_entlist.push_back(res);
	return res;
}