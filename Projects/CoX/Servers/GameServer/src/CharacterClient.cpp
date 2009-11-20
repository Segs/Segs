/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: Client.cpp 253 2006-08-31 22:00:14Z malign $
 */
#include "CharacterClient.h"
#include "Character.h"
#include "CharacterDatabase.h"
#include "GameServer.h"
CharacterClient::~CharacterClient()
{
	reset();
}
size_t CharacterClient::getMaxSlots()
{
	return m_max_slots;
}

bool CharacterClient::getCharsFromDb()
{
	Character *act=0;
	CharacterDatabase * m_db = m_server->getDb();//m_handler->getDb();

    m_characters.resize(m_max_slots);
	if(m_characters[0]==0)
		m_characters[0] = act = new Character;
	else
		act = m_characters[0]; //reuse existing object
	act->setIndex(0);
	act->setAccountId(m_game_server_acc_id);
	m_db->fill(act);
	for(size_t i=1; i<m_characters.size(); i++)
	{
		if(m_characters[i]) //even more reuse
		{
			m_characters[i]->reset();
			continue;
		}
		m_characters[i] = new Character;
		m_characters[i]->reset();
	}
	return true;
}

Character * CharacterClient::getCharacter( size_t idx )
{
	ACE_ASSERT(idx<m_characters.size());
	return m_characters[idx];
}

void CharacterClient::reset()
{
	for(size_t i=0; i<m_characters.size(); i++)
	{
		delete m_characters[i];
		m_characters[i]=0;
	}
}

bool CharacterClient::serializeFromDb()
{
	CharacterDatabase * m_db = m_server->getDb();
	return m_db->fill(this);
}