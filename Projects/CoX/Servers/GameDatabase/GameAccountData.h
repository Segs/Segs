#pragma once

#include <QString>
#include <vector>
#include <cassert>

struct GameAccountResponseCostumeData
{
    QString m_serialized_data;
    uint64_t m_character_id; //! Character to whom this costume belongs
    uint32_t m_body_type;
    float m_height;
    float m_physique;
    uint32_t skin_color;
    uint8_t  m_slot_index;

};
struct GameAccountResponseCharacterData
{
    std::vector<GameAccountResponseCostumeData> m_costumes;
    QString m_name;
    QString m_serialized_chardata;
    QString m_serialized_options;
    QString m_serialized_gui;
    QString m_serialized_keybinds;
    int index;
    uint32_t m_db_id;
    uint32_t m_account_id;
    uint32_t m_HitPoints;
    uint32_t m_Endurance;
    int m_current_costume_idx;
    bool m_villain;
    bool m_multiple_costumes;
    void reset()
    {
        m_name="EMPTY";
        m_villain=false;
        m_multiple_costumes=false;
        m_current_costume_idx=0;
    }
    bool isEmpty() const
    {
        return 0==m_name.compare("EMPTY",Qt::CaseInsensitive);
    }
};
struct GameAccountData
{
    uint64_t m_game_server_acc_id;
    int m_max_slots;
    std::vector<GameAccountResponseCharacterData> m_characters;


    GameAccountResponseCharacterData &get_character(size_t idx)
    {
        assert(idx<m_characters.size());
        return m_characters[idx];
    }
    int8_t next_free_slot_idx() const
    {
        int8_t res = 0;
        for(const auto & c : m_characters)
        {
            if(c.m_name.compare("EMPTY")==0)
                return res;
            res++;
        }
        return -1;
    }
    bool valid() const { return m_game_server_acc_id!=0;}
};
