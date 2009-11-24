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

class AdminDatabase;
class CharacterDatabase;

class AccountInfo
{
    friend class CharacterDatabase;
    friend class AdminDatabase;

    std::string                 m_login;
    u8                          m_access_level;
    u64							m_acc_server_acc_id;

    u64							m_game_server_acc_id;
    std::vector<Character *>	m_characters;
    u8							m_max_slots;

    void						max_slots(u8 maxs) {m_max_slots=maxs;};
    void						game_server_id(u64 id){m_game_server_acc_id=id;}
    bool                        fill_characters_db();
    u8                          char_slot_index(Character *c);
public:
                                AccountInfo();

    u8                          access_level() const { return m_access_level; }
    void                        access_level(u8 val) { m_access_level = val; }
    const std::string &         login() const {return m_login;}
    void                        login(const std::string & v){m_login=v;}

    u64                         account_server_id() const {return m_acc_server_acc_id;}
    void                        account_server_id(const u64 &v) {m_acc_server_acc_id=v;}
    u64                         game_server_id() const {return m_game_server_acc_id;}
    u8  						max_slots() {return m_max_slots;};
    Character *                 get_character(size_t idx);
    Character *                 create_new_character(); // returns 0 if no free slots are left
    bool                        store_new_character(Character *character);
    bool                        fill_game_db(u64 game_server_idx);
    void                        reset();
};
