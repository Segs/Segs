/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

// segs includes

#include "CharacterDatabase.h"
#include "AccountInfo.h"
#include "AdminServer.h"
#include "Character.h"
#include "Costume.h"

#include <ace/Log_Msg.h>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlDriver>
#include <QtCore/QDebug>
#include <QtSql/QSqlError>
#include <cstdlib>
#include <cstdio>
using namespace std;

CharacterDatabase::~CharacterDatabase()
{
}
static bool prepQuery(QSqlQuery &qr,const QString &txt) {
    if(!qr.prepare(txt)) {
        qDebug() << "SQL_ERROR:"<<qr.lastError();
        return false;
    }
    return true;
}
static bool doIt(QSqlQuery &qr) {
    if(!qr.exec()) {
        qDebug() << "SQL_ERROR:"<<qr.lastError();
        return false;
    }
    return true;
}
void CharacterDatabase::on_connected(QSqlDatabase *db)
{
    m_db = db;
    if(!db)
        return;

    m_prepared_fill = QSqlQuery(*db);
    m_prepared_account_insert = QSqlQuery(*db);
    m_prepared_char_insert = QSqlQuery(*db);
    m_prepared_costume_insert = QSqlQuery(*db);
    m_prepared_char_select = QSqlQuery(*db);
    m_prepared_account_select = QSqlQuery(*db);
    m_prepared_char_exists = QSqlQuery(*db);
    m_prepared_char_delete = QSqlQuery(*db);


    prepQuery(m_prepared_fill,"SELECT * FROM costume WHERE character_id=? AND costume_index=?");
    prepQuery(m_prepared_account_insert,"INSERT INTO accounts  (account_id,max_slots) VALUES (?,?)");
    prepQuery(m_prepared_char_insert,
                "INSERT INTO characters  "
                "(char_level,slot_index,account_id,char_name,archetype,origin,bodytype,current_map) "
                "VALUES "
                "(?,?,?,?,?,?,?,?)");
    prepQuery(m_prepared_costume_insert,
                "INSERT INTO costume (character_id,costume_index,skin_color,parts) VALUES "
                "(?,?,?,?)");
    prepQuery(m_prepared_char_select,"SELECT * FROM characters WHERE account_id=? AND slot_index=?");
    prepQuery(m_prepared_account_select,"SELECT * FROM accounts WHERE account_id=?");
    prepQuery(m_prepared_char_exists,"SELECT exists (SELECT 1 FROM characters WHERE char_name = $1 LIMIT 1)");
    prepQuery(m_prepared_char_delete,"DELETE FROM characters WHERE account_id=? AND slot_index=?");
}


// UserToken,
bool CharacterDatabase::remove_character(AccountInfo *c,uint8_t slot_idx)
{
    assert(c!=0);
    m_prepared_char_delete.bindValue(0,c->account_server_id());
    m_prepared_char_delete.bindValue(1,(uint32_t)slot_idx);
    if(!doIt(m_prepared_char_delete))
        return false;
    return true;
}
bool CharacterDatabase::named_character_exists(const QString &name)
{
    m_prepared_char_exists.bindValue(0,name);
    if(!doIt(m_prepared_char_exists))
        return false;

    if(!m_prepared_char_exists.next())
        return false;
    // TODO: handle case of multiple accounts with same name ?

    return m_prepared_char_exists.value("exists").toBool();
}
bool CharacterDatabase::fill( AccountInfo *c )
{
    assert(c&&c->account_server_id());

    m_prepared_account_select.bindValue(0,c->account_server_id());
    if(!doIt(m_prepared_account_select))
        return false;


    if(!m_prepared_account_select.next())
        return false;
    // TODO: handle case of multiple accounts with same name ?

    c->max_slots(m_prepared_account_select.value("max_slots").toUInt());
    c->game_server_id(m_prepared_account_select.value("id").toULongLong());

    ACE_DEBUG((LM_INFO,"CharacterClient id: %i\n",c->account_server_id()));
    ACE_DEBUG((LM_INFO,"CharacterClient slots: %i\n",c->max_slots()));
    return true;
}
#define STR_OR_EMPTY(c) ((!c.isEmpty()) ? c:"EMPTY")
#define STR_OR_VERY_EMPTY(c) ((c!=0) ? c:"")
bool CharacterDatabase::fill( Character *c)
{
    assert(c&&c->getAccountId());

    m_prepared_char_select.bindValue(0,c->getAccountId());
    m_prepared_char_select.bindValue(1,(uint16_t)c->getIndex());
    if(!doIt(m_prepared_char_select))
        return false;

    if(!m_prepared_char_select.next())
    {
        c->reset(); // empty slot
        return true;
    }
//    else if (results.num_rows()>1)
//        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) CharacterDatabase::fill query returned wrong number of results. %s failed.\n"), query.str().c_str()),false);

    c->setLevel((uint8_t)m_prepared_char_select.value("char_level").toUInt());
    c->setName(STR_OR_EMPTY(m_prepared_char_select.value("char_name").toString()));
    c->m_class_name = (STR_OR_EMPTY(m_prepared_char_select.value("archetype").toString()));
    c->m_origin_name= (STR_OR_EMPTY(m_prepared_char_select.value("origin").toString()));

    CharacterCostume *main_costume = new CharacterCostume;
    // appearance related.
    main_costume->m_body_type = m_prepared_char_select.value("bodytype").toUInt(); // 0
    c->setMapName("Some map name"); // "V_City_00_01.txt" STR_OR_EMPTY(r.getColString("current_map"))
    c->setLastCostumeId(m_prepared_char_select.value("last_costume_id").toUInt());
    main_costume->setSlotIndex(0);
    main_costume->setCharacterId(m_prepared_char_select.value("id").toULongLong());
    c->m_costumes.push_back(main_costume);
    return fill(main_costume);
}
#ifndef ARRAYSIZE
#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[1]))
#define DEFINED_ARRAYSIZE
#endif
bool CharacterDatabase::fill( CharacterCostume *c)
{
    assert(c&&c->getCharacterId());

    m_prepared_fill.bindValue(0,c->getCharacterId());
    m_prepared_fill.bindValue(1,(uint16_t)c->getSlotIndex());
    if(!doIt(m_prepared_fill))
        return false;

    if(!m_prepared_fill.next()) // retry with the first one
    {
        m_prepared_fill.bindValue(0,c->getCharacterId());
        m_prepared_fill.bindValue(1,0); // get first costume

        if(!doIt(m_prepared_fill))
            return false;
        if(!m_prepared_fill.next()) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) CharacterDatabase::fill no costumes.\n")),false);
            return false;
        }
    }
    c->a = m_prepared_fill.value("skin_color").toUInt();
    QString serialized_parts= m_prepared_fill.value("parts").toString();
    c->serializeFromDb(serialized_parts);
    c->m_non_default_costme_p = false;
    return true;
}

bool CharacterDatabase::CreateLinkedAccount( uint64_t auth_account_id,const std::string &username )
{
    //assert(auth_account_id>0);  sqlite3 autogenerated values start from 0
    assert(username.size()>2);
    m_prepared_account_insert.bindValue(0,auth_account_id);
    m_prepared_account_insert.bindValue(0,2); // max_slots

    if(!doIt(m_prepared_account_insert))
        return false;
    return true;
}

int CharacterDatabase::remove_account( uint64_t /*acc_serv_id*/ )
{
    return 0;
}
//TODO: SQL String sanitization
bool CharacterDatabase::create( uint64_t gid,uint8_t slot,Character *c )
{
    assert(m_db->driver()->hasFeature(QSqlDriver::LastInsertId));
    assert(gid>0);
    assert(c);
    assert(slot<8);
    Costume *cst = c->getCurrentCostume();
    if(!cst) {
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) CharacterDatabase::create cannot insert char without costume.\n"))
                         ,false);
    }
    m_prepared_char_insert.bindValue(0, c->m_level);
    m_prepared_char_insert.bindValue(1, uint32_t(slot));
    m_prepared_char_insert.bindValue(2, gid);
    m_prepared_char_insert.bindValue(3, c->m_name);
    m_prepared_char_insert.bindValue(4, c->m_class_name);
    m_prepared_char_insert.bindValue(5, c->m_origin_name);
    m_prepared_char_insert.bindValue(6, c->getCurrentCostume()->m_body_type);
    m_prepared_char_insert.bindValue(7, c->m_mapName);
    if(!doIt(m_prepared_char_insert))
        return false;
    int64_t char_id = m_prepared_char_insert.lastInsertId().toLongLong();

    // create costume

    QString costume_parts;
    cst->serializeToDb(costume_parts);
    m_prepared_costume_insert.bindValue(0,uint64_t(char_id));
    m_prepared_costume_insert.bindValue(1,uint32_t(0));
    m_prepared_costume_insert.bindValue(2,uint32_t(cst->a));
    m_prepared_costume_insert.bindValue(3,costume_parts);

    if(!doIt(m_prepared_costume_insert))
        return false;
    return true;
}
#ifdef DEFINED_ARRAYSIZE
#undef DEFINED_ARRAYSIZE
#undef ARRAYSIZE
#endif
