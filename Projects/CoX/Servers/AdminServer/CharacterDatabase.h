/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once

// ACE Logging
#include <string>


#include <QtSql/QSqlQuery>
class IClient;
class QSqlQuery;
class CharacterClient;
class CharacterCostume;
class Character;
class AccountInfo;
class CharacterDatabase
{
        QSqlDatabase * m_db;

        QSqlQuery m_prepared_account_select;
        QSqlQuery m_prepared_account_insert;
        QSqlQuery m_prepared_char_insert;
        QSqlQuery m_prepared_char_exists;
        QSqlQuery m_prepared_char_delete;
        QSqlQuery m_prepared_char_select;
        QSqlQuery m_prepared_fill;
        QSqlQuery m_prepared_costume_insert;

public:
virtual             ~CharacterDatabase();
        bool        CreateLinkedAccount(uint64_t auth_account_id,const std::string &username); // returns true on success
        bool        create(AccountInfo *);
        bool        create(uint64_t gid,uint8_t slot,Character *c);
        bool        fill( AccountInfo *); //!x
        bool        fill( Character *); //! Will call fill(CharacterCostume)
        bool        fill( CharacterCostume *);
        int         remove_account(uint64_t acc_serv_id); //will remove given account, TODO add logging feature
        bool        remove_character(AccountInfo *,uint8_t slot_idx);
        bool        named_character_exists(const QString &name);
        void        setDb(QSqlDatabase *db) {m_db=db;}
        QSqlDatabase *getDb() {return m_db;}
        void        on_connected(QSqlDatabase *db); //prepare statements here
};
