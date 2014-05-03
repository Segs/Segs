/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#include <sstream>
#include <ace/OS_NS_time.h>
#include <ace/Log_Msg.h>

// segs includes
#include "AdminServer.h"
#include "CharacterDatabase.h"
#include "AccountInfo.h"
#include "Character.h"
#include "Costume.h"


bool AccountInfo::fill_characters_db()
{
    Character *act=0;
    m_characters.resize(m_max_slots);
    if(m_characters[0]==0)
        m_characters[0] = act = new Character;
    else
        act = m_characters[0]; //reuse existing object
    act->setIndex(0);
    act->setAccountId(m_game_server_acc_id);
    CharacterDatabase *char_db = AdminServer::instance()->character_db();
    if(!char_db->fill(act))
        return false;
    for(size_t i=1; i<m_characters.size(); i++)
    {
        if(m_characters[i]) //even more reuse
        {
            m_characters[i]->reset();
        }
        else
            m_characters[i] = new Character;
        m_characters[i]->setIndex(i);
        m_characters[i]->setAccountId(m_game_server_acc_id);
        if(!AdminServer::instance()->character_db()->fill(m_characters[i]))
            return false;
    }
    return true;
}

Character * AccountInfo::get_character(size_t idx)
{
    assert(idx<m_characters.size());
    return m_characters[idx];
}

void AccountInfo::reset()
{
    for(size_t i=0; i<m_characters.size(); i++)
    {
        delete m_characters[i];
        m_characters[i]=0;
    }
}

AccountInfo::AccountInfo() : m_access_level(0),m_acc_server_acc_id(0),m_game_server_acc_id(0),m_max_slots(0)
{

}

bool AccountInfo::fill_game_db( uint64_t )
{
    if(!AdminServer::instance()->character_db()->fill(this)) // read basic facts
        return false;
    return fill_characters_db();
}

Character * AccountInfo::create_new_character()
{
    for(size_t i=0; i<m_characters.size(); i++)
    {
        if(m_characters[i]->getName().compare("EMPTY")==0)
            return m_characters[i]; // free slot
    }
    return 0;
}
uint8_t AccountInfo::char_slot_index(Character *c)
{
    for(size_t i=0; i<m_characters.size(); i++)
    {
        if(m_characters[i]==c)
            return uint8_t(i);
    }
    return ~1;
}
bool AccountInfo::store_new_character(Character *character)
{
    uint8_t slot_idx=char_slot_index(character);
    if(slot_idx==0xFF)
        return false;
    CharacterDatabase *cdb = AdminServer::instance()->character_db();
    DbTransactionGuard grd(*cdb->getDb());
    if(false==cdb->create(m_game_server_acc_id,slot_idx,character))
        return false;
    grd.commit();
    return true;
}

bool AccountInfo::remove_character( Character *character )
{
    CharacterDatabase *cdb = AdminServer::instance()->character_db();
    uint8_t slot_idx=char_slot_index(character);
    if(slot_idx==0xFF)
        return false;
    cdb->remove_character(this,slot_idx);
    return true;
}
