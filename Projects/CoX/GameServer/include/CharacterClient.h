/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: Client.h 253 2006-08-31 22:00:14Z malign $
 */

// Inclusion guards
#pragma once
#ifndef CHARACTERCLIENT_H
#define CHARACTERCLIENT_H

#include <string>
#include <ace/OS_NS_time.h>
#include "ServerManager.h"
#include "Client.h"
#include <vector>
// skeleton class used during authentication
class GameServerInterface;
class Character;
class CharacterHandler;
class CharacterClient : public IClient
{
	friend class CharacterDatabase;
		std::vector<Character *>	m_characters;
		CharacterHandler *			m_handler;
		u8							m_max_slots;
		u64							m_game_server_acc_id;
		void						setMaxSlots(u8 maxs) {m_max_slots=maxs;};
		void						setGameServerAccountId(u64 id){m_game_server_acc_id=id;}
public:
					CharacterClient(){}
virtual				~CharacterClient();
		void		setHandler(CharacterHandler * val) { m_handler = val; }
		Character *	getCharacter(size_t idx);
		bool		serializeFromDb();
		bool		getCharsFromDb();
		size_t		getMaxSlots();
		void		reset();
};

#endif // CHARACTERCLIENT_H
