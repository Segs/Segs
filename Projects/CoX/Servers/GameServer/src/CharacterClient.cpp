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
size_t CharacterClient::max_slots()
{
	return m_account_info.max_slots();
}

bool CharacterClient::getCharsFromDb()
{
    
    return m_account_info.fill_game_db(0); // fill the game related info using given game server db
}

Character * CharacterClient::getCharacter( size_t idx )
{
    return m_account_info.get_character(idx);
}

void CharacterClient::reset()
{
    m_account_info.reset();
}

