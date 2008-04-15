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
#ifndef ADMINDATABASE_H
#define ADMINDATABASE_H

// ACE Logging
#include <string>
#include <ace/Log_Msg.h>
#include <ace/Singleton.h>

#include "types.h"
#include "Database.h"

class IClient;
class AdminDatabase : public Database
{
	int GetAccount(IClient *client,const std::string &query);
public:
	int GetAccounts(void) const;
	// Access levels 
	// 0 - lowest of low
	//
	// 0x7FFF - server gods
	int AddAccount(const char *username, const char *password, u16 access_level=1);
	int RemoveAccountByID(u64 id);
	int RemoveAccountByName(char *username);
	int GetBanFlag(const char *username);
	bool ValidPassword(const char *username, const char *password);
	int GetAccountByName(IClient *to_fill,const std::string &login);
	int GetAccountById(IClient *to_fill,u64 id);
};

#endif // ADMINDATABASE_H
