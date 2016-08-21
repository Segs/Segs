/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once

#include "Database.h"

#include <string>

class AccountInfo;
class AdminDatabase : public IDataBaseCallbacks
{
    Database *m_db;
    bool    GetAccount(AccountInfo & client, DbResults &query);
    IPreparedQuery *m_add_account_query;
    IPreparedQuery *m_prepared_select_account_by_id;
    IPreparedQuery *m_prepared_select_account_passw;
    IPreparedQuery *m_prepared_select_account_by_username;
public:
virtual     ~AdminDatabase();
            AdminDatabase();
    int     GetAccounts(void) const;
    // Access levels
    // 0 - lowest of low
    //
    // 0x7FFF - server gods
    bool    AddAccount(const char *username, const char *password, uint16_t access_level=1); //return true if success
    int     RemoveAccountByID(uint64_t id);
    int     RemoveAccountByName(char *username);
    int     GetBanFlag(const char *username);
    bool    ValidPassword(const char *username, const char *password);
    bool    GetAccountByName(AccountInfo &to_fill,const std::string &login);
    bool    GetAccountById(AccountInfo &to_fill,uint64_t id);
    void    setDb(Database *db) {m_db=db;}
    Database *getDb() {return m_db;}
    void    on_connected(Database *db);
};
