/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: EntityStorage.h 308 2007-01-25 05:14:43Z malign $
 */

#pragma once

#include "types.h"
#include "BitStream.h"
#include <list>

class MapClient;
class CoXMap;
class Entity;
class EntityManager
{
public:
	Entity *m_map_entities[10240];
	list <Entity *> m_entlist;
	u32 m_last_ent;
	EntityManager()
	{
		m_last_ent = 0;
	}
		void sendDebuggedEntities(BitStream &tgt) const;
		void sendGlobalEntDebugInfo(BitStream &tgt) const;
		void sendDeletes(BitStream &tgt) const;
		void sendEntities(BitStream &tgt) const;
		Entity *CreatePlayer();
};

class EntityStorage
{
public:
	void NewPlayer(MapClient *client,Entity *player_ent); // stores fresh player avatar
	void StorePlayer(MapClient *client,Entity *player_ent); // stores player avatar
	Entity *CreatePlayer(MapClient *client,int avatar_id); // retrieves client avatar from storage

	Entity * CreateInstance(CoXMap *target_world,u64 id); // will create a new instance of given entity, bound to given map
};
