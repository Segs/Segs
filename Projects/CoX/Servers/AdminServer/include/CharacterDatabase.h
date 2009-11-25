/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details) 
 *
 * $Id: AdminDatabase.h 267 2006-09-18 04:46:30Z nemerle $
 */

// Inclusion guards
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
class AccountInfo;
class CharacterDatabase : public Database
{
    void    on_connected() {;} //prepare statements here
public:
	bool     CreateLinkedAccount(u64 auth_account_id,const std::string &username); // returns true on success
	int     AddCharacter(const std::string &username, const std::string &charname);
	int     RemoveCharacter(char *username, char *charname);
    bool    create(AccountInfo *);
    bool    create(u64 gid,u8 slot,Character *c);
	bool    fill( AccountInfo *); //! 
	bool    fill( Character *); //! Will call fill(CharacterCostume)
	bool    fill( CharacterCostume *); 
    int     remove_account(u64 acc_serv_id); //will remove given account, TODO add logging feature 
};
