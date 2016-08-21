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

#include <cstdlib>
#include <cstdio>
#include <ace/Log_Msg.h>
using namespace std;

CharacterDatabase::~CharacterDatabase()
{
    delete m_prepared_account_select;
    delete m_prepared_account_insert;
    delete m_prepared_char_insert;
    delete m_prepared_char_exists;
    delete m_prepared_char_delete;
    delete m_prepared_char_select;
    delete m_prepared_fill;
    delete m_prepared_costume_insert;
}

void CharacterDatabase::on_connected(Database *db) {
    m_db = db;
    if(db) {
        m_prepared_fill = db->prepare("SELECT * FROM costume WHERE character_id=$1 AND costume_index=$2",2);
        m_prepared_account_insert = db->prepareInsert("INSERT INTO accounts  (account_id,max_slots) VALUES ($1,2)",1);
        m_prepared_char_insert = db->prepareInsert(
                    "INSERT INTO characters  "
                    "(char_level,slot_index,account_id,char_name,archetype,origin,bodytype,current_map) "
                    "VALUES "
                    "($1,$2,$3,$4,$5,$6,$7,$8)",8);
        m_prepared_costume_insert = db->prepareInsert(
                    "INSERT INTO costume (character_id,costume_index,skin_color,parts) VALUES "
                    "($1,$2,$3,$4)",4);
        m_prepared_char_select = db->prepare("SELECT * FROM characters WHERE account_id=$1 AND slot_index=$2",2);
        m_prepared_account_select = db->prepare("SELECT * FROM accounts WHERE account_id=$1",1);
        m_prepared_char_exists = db->prepare("SELECT exists (SELECT 1 FROM characters WHERE char_name = $1 LIMIT 1)",1);
        m_prepared_char_delete = db->prepare("DELETE FROM characters WHERE account_id=$1 AND slot_index=$2",2);
    }
}


// UserToken,
bool CharacterDatabase::remove_character(AccountInfo *c,uint8_t slot_idx)
{
    DbResults results;
    PreparedArgs remove_args;
    assert(c!=0);
    remove_args.add_param(c->account_server_id());
    remove_args.add_param((uint32_t)slot_idx);
    if(!m_prepared_char_delete->execute(remove_args,results))
    {
        ACE_ERROR_RETURN((LM_ERROR,
                          ACE_TEXT("(%P|%t) CharacterDatabase::remove_character %s failed. %s.\n"),
                          m_prepared_char_delete->prepared_sql().c_str(),results.message()),false);
    }
    results.nextRow();

    return true;
}
bool CharacterDatabase::named_character_exists(const string &name)
{
    DbResults results;
    PreparedArgs exist_arg;

    exist_arg.add_param(name);
    if(!m_prepared_char_exists->execute(exist_arg,results))
    {
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) CharacterDatabase::named_character_exists query %s failed. %s.\n"),
                          m_prepared_char_exists->prepared_sql().c_str(),results.message()),false);
    }

    DbResultRow r=results.nextRow();
    assert(r.valid()); // TODO: handle case of multiple accounts with same name ?

    return r.getColBool("exists");
}
bool CharacterDatabase::fill( AccountInfo *c )
{
    DbResults results;

    assert(c&&c->account_server_id());
    PreparedArgs acc_arg;
    acc_arg.add_param(c->account_server_id());


    if(!m_prepared_account_select->execute(acc_arg,results))
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) CharacterDatabase::fill query %s failed. %s.\n"),
                          m_prepared_account_select->prepared_sql().c_str(),results.message()),false);

    DbResultRow r=results.nextRow();
    if(!r.valid()) // TODO: handle case of multiple accounts with same name ?
        return false;

    c->max_slots((uint8_t)r.getColInt16("max_slots"));
    c->game_server_id((uint64_t)r.getColInt64("id"));

    ACE_DEBUG((LM_INFO,"CharacterClient id: %i\n",c->account_server_id()));
    ACE_DEBUG((LM_INFO,"CharacterClient slots: %i\n",c->max_slots()));
    return true;
}
#define STR_OR_EMPTY(c) ((c!=0) ? c:"EMPTY")
#define STR_OR_VERY_EMPTY(c) ((c!=0) ? c:"")
bool CharacterDatabase::fill( Character *c)
{
    DbResults results;
    assert(c&&c->getAccountId());

    PreparedArgs arg_char;
    arg_char.add_param(c->getAccountId());
    arg_char.add_param((uint16_t)c->getIndex());
    if(!m_prepared_char_select->execute(arg_char,results))
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) CharacterDatabase::fill query %s failed. %s.\n"),
                          m_prepared_char_select->prepared_sql().c_str(),results.message()),false);
    DbResultRow r=results.nextRow();
    if(!r.valid())
    {
        c->reset(); // empty slot
        return true;
    }
//    else if (results.num_rows()>1)
//        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) CharacterDatabase::fill query returned wrong number of results. %s failed.\n"), query.str().c_str()),false);

    c->setLevel((uint8_t)r.getColInt16("char_level"));
    c->setName(STR_OR_EMPTY(r.getColString("char_name")));
    c->m_class_name = (STR_OR_EMPTY(r.getColString("archetype")));
    c->m_origin_name= (STR_OR_EMPTY(r.getColString("origin")));

    CharacterCostume *main_costume = new CharacterCostume;
    // appearance related.
    main_costume->m_body_type = r.getColInt32("bodytype"); // 0
    c->setMapName("Some map name"); // "V_City_00_01.txt" STR_OR_EMPTY(r.getColString("current_map"))
    c->setLastCostumeId(r.getColInt32("last_costume_id"));
    main_costume->setSlotIndex(0);
    main_costume->setCharacterId(r.getColInt32("id"));
    c->m_costumes.push_back(main_costume);
    return fill(main_costume);
}
#ifndef ARRAYSIZE
#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[1]))
#define DEFINED_ARRAYSIZE
#endif
bool CharacterDatabase::fill( CharacterCostume *c)
{
    DbResults results;
    PreparedArgs q_args;

    assert(c&&c->getCharacterId());

    q_args.add_param(c->getCharacterId());
    q_args.add_param((uint16_t)c->getSlotIndex());
    if(!m_prepared_fill->execute(q_args,results))
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) CharacterDatabase::fill query %s failed. %s.\n"),
                          m_prepared_fill->prepared_sql().c_str(),results.message()),false);

    DbResultRow r=results.nextRow();
    if(!r.valid()) // retry with the first one
    {
        PreparedArgs q2_args;
        q2_args.add_param(c->getCharacterId());
        q2_args.add_param((uint32_t)0); // get first costume

        if(!m_prepared_fill->execute(q2_args,results))
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) CharacterDatabase::fill query %s failed. %s.\n"),
                              m_prepared_fill->prepared_sql().c_str(),results.message()),false);
        r = results.nextRow();
        if(!r.valid()) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) CharacterDatabase::fill no costu,es.\n")),false);
            return false;
        }
    }
    c->a = r.getColInt32("skin_color");
    std::string serialized_parts= r.getColString("parts");
    c->serializeFromDb(serialized_parts);
    c->m_non_default_costme_p = false;
    return true;
}

bool CharacterDatabase::CreateLinkedAccount( uint64_t auth_account_id,const std::string &username )
{

    DbResults results;
    //assert(auth_account_id>0);  sqlite3 autogenerated values start from 0
    assert(username.size()>2);
    PreparedArgs acc_args;
    acc_args.add_param(auth_account_id);

    if(!m_prepared_account_insert->execute(acc_args,results))
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) CharacterDatabase::fill query %s failed. %s.\n"),
                          m_prepared_account_insert->prepared_sql().c_str(),results.message()),false);
    results.nextRow();
    return true;
}

int CharacterDatabase::remove_account( uint64_t /*acc_serv_id*/ )
{
    return 0;
}
//TODO: SQL String sanitization
bool CharacterDatabase::create( uint64_t gid,uint8_t slot,Character *c )
{
    stringstream query;
    DbResults results;
    assert(gid>0);
    assert(c);
    assert(slot<8);
    Costume *cst = c->getCurrentCostume();
    if(!cst) {
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) CharacterDatabase::create cannot insert char without costume.\n"))
                         ,false);
    }
    assert(m_prepared_char_insert);
    std::ostringstream o;
    o << gid;
    PreparedArgs insertargs;
    insertargs.add_param( c->m_level);
    insertargs.add_param( uint32_t(slot));
    insertargs.add_param( o.str());
    insertargs.add_param( c->m_name);
    insertargs.add_param( c->m_class_name);
    insertargs.add_param( c->m_origin_name);
    insertargs.add_param( c->getCurrentCostume()->m_body_type);
    insertargs.add_param( c->m_mapName);
    int64_t char_id = m_prepared_char_insert->executeInsert(insertargs,results);
    if(-1==char_id)
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) CharacterDatabase::create %s failed. %s.\n"),
                          m_prepared_char_insert->prepared_sql().c_str(),results.message()),false);

    // create costume

    std::string costume_parts;
    cst->serializeToDb(costume_parts);
    PreparedArgs costumetargs;
    costumetargs.add_param(uint64_t(char_id));
    costumetargs.add_param(uint32_t(0));
    costumetargs.add_param(uint32_t(cst->a));
    costumetargs.add_param(costume_parts);

    if(!m_prepared_costume_insert->execute(costumetargs,results))
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) CharacterDatabase::create %s failed. %s.\n"), query.str().c_str(),results.message()),false);
    results.nextRow();
    return true;
}
#ifdef DEFINED_ARRAYSIZE
#undef DEFINED_ARRAYSIZE
#undef ARRAYSIZE
#endif
