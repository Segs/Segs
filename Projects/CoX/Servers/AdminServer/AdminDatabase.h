/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */
#pragma once

#include "Database.h"

#include <QtSql/QSqlQuery>

class QSqlDatabase;
struct AuthAccountData;
class AdminDatabase
{
    QSqlDatabase *m_db;
    bool        GetAccount(AuthAccountData &client, QSqlQuery &query);
    QSqlQuery m_add_account_query;
    QSqlQuery m_prepared_select_account_by_id;
    QSqlQuery m_prepared_select_account_passw;
    QSqlQuery m_prepared_select_account_by_username;
public:
virtual     ~AdminDatabase();
            AdminDatabase();
    int     GetAccounts(void) const;
    // Access levels
    // 0 - lowest of low
    //
    // 0x7FFF - server gods
    bool    AddAccount(const char *username, const char *password, uint16_t access_level=1); //return true if success
    int     RemoveAccountByName(char *username);
    int     GetBanFlag(const char *username);
    bool    ValidPassword(const QString &username, const char *password);
    bool    GetAccountByName(AuthAccountData &to_fill, const QString &login);
    bool    GetAccountById(AuthAccountData &to_fill,uint64_t id);
    void    setDb(QSqlDatabase *db) {m_db=db;}
    QSqlDatabase *getDb() {return m_db;}
    void    on_connected(QSqlDatabase *db);
};
