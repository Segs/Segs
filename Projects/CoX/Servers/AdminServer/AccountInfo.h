/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once

// ACE Logging
#include <QtCore/QString>
#include <string>
#include <vector>

class IClient;
class CharacterClient;
class CharacterCostume;
class Character;
class Entity;

class CharacterDatabase;

class AccountInfo
{
    friend class CharacterDatabase;

    uint64_t                    m_game_server_acc_id;
    std::vector<Character *>    m_characters;
    uint8_t                     m_max_slots;

    void                        max_slots(uint8_t maxs) {m_max_slots=maxs;}
    void                        game_server_id(uint64_t id){m_game_server_acc_id=id;}
    bool                        fill_characters_db();
    int8_t                      char_slot_index(Character *c);
public:
                                AccountInfo();

    uint64_t                    game_server_id() const {return m_game_server_acc_id;}
    uint8_t                     max_slots() {return m_max_slots;}
    Character *                 get_character(size_t idx);
    Character *                 create_new_character(); // returns 0 if no free slots are left
    bool                        store_new_character(Entity *e, Character *c);
    void                        reset();
};
