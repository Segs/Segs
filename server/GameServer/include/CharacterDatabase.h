/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details) 
 *
 * $Id: AdminDatabase.h 267 2006-09-18 04:46:30Z nemerle $
 */
#pragma once
// ACE Logging
#include <string>
#include <ace/Log_Msg.h>
#include <ace/Singleton.h>

#include "types.h"
#include "Database.h"
class IClient;
class CharacterClient;
class CharacterCostume;
class Character;
class CharacterDatabase : public Database
{
public:
	int AddCharacter(const std::string &username, const std::string &charname);
	int RemoveCharacter(char *username, char *charname);
	bool fill( CharacterClient *); //! 
	bool fill( Character *); //! Will call fill(CharacterCostume)
	bool fill( CharacterCostume *); 
	u8	maxSlots(u64 account_id); //! returns maximum number of slots available to this account
};
