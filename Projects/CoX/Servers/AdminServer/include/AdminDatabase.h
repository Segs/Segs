/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details) 
 *
 * $Id$
 */

#pragma once

// ACE Logging
#include <string>
#include <ace/Log_Msg.h>
#include <ace/Singleton.h>

#include "types.h"
#include "Database.h"

class AccountInfo;
class AdminDatabase : public Database
{
    bool    GetAccount(AccountInfo & client,const std::string &query);
    PreparedQuery *m_add_account_query;
public:
            AdminDatabase();
	int     GetAccounts(void) const;
	// Access levels 
	// 0 - lowest of low
	//
	// 0x7FFF - server gods
	bool    AddAccount(const char *username, const char *password, u16 access_level=1); //return true if success 
	int     RemoveAccountByID(u64 id);
	int     RemoveAccountByName(char *username);
	int     GetBanFlag(const char *username);
	bool    ValidPassword(const char *username, const char *password);
    bool    GetAccountByName(AccountInfo &to_fill,const std::string &login);
    bool    GetAccountById(AccountInfo &to_fill,u64 id);
    void    on_connected();
};    
