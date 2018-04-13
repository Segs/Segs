#pragma once

#include "Servers/InternalEvents.h"
#include "GameData/chardata_definitions.h"

#include <QDateTime>


enum GameDBEventTypes : uint32_t
{
    evCharacterUpdate= Internal_EventTypes::evLAST_EVENT,
    evCostumeUpdate,
    evRemoveCharacter,
    evGameAccountRequest,
    evGameAccountResponse,
    evGameDbError,
    evLAST_EVENT
};

#define ONE_WAY_MESSAGE(name)\
struct name ## Message final : public InternalEvent\
{\
    name ## Data m_data;\
    name ## Message(name ## Data &&d,uint64_t token) :  InternalEvent(GameDBEventTypes::ev ## name),m_data(d) {session_token(token);}\
};
/// A message without Request having additional data
#define SIMPLE_TWO_WAY_MESSAGE(name)\
struct name ## Request final : public InternalEvent\
{\
    name ## Message(uint64_t token) :  InternalEvent(GameDBEventTypes::ev ## name ## Request) {session_token(token);}\
};\
struct name ## Response final : public InternalEvent\
{\
    name ## Data m_data;\
    name ## Response(name ## Data &&d,uint64_t token) :  InternalEvent(GameDBEventTypes::ev ## name ## Response),m_data(d) {session_token(token);}\
};
/// A message with Request having additional data
#define TWO_WAY_MESSAGE(name)\
struct name ## Request final : public InternalEvent\
{\
    name ## RequestData m_data;\
    name ## Request(name ## RequestData &&d,uint64_t token) : InternalEvent(GameDBEventTypes::ev ## name ## Request),m_data(d) {session_token(token);}\
};\
struct name ## Response final : public InternalEvent\
{\
    name ## ResponseData m_data;\
    name ## Response(name ## ResponseData &&d,uint64_t token) :  InternalEvent(GameDBEventTypes::ev ## name ## Response),m_data(d) {session_token(token);}\
};

struct CharacterUpdateData
{
    QString m_char_name;
    // Cerealized blobs
    QString m_char_data;
    QString m_entitydata;
    QString m_options;
    // plain values
    uint32_t m_bodytype;
    float    m_height;
    float    m_physique;
    uint32_t m_supergroup_id;
    uint32_t m_id;
};
ONE_WAY_MESSAGE(CharacterUpdate)

struct CostumeUpdateData
{
    QString m_parts; // cereal serialized costume parts
    int m_db_id;
    int m_costume_index;
    uint32_t m_skin_color;
};
ONE_WAY_MESSAGE(CostumeUpdate)

struct RemoveCharacterData
{
    uint64_t account_id;
    int8_t slot_idx;
};
ONE_WAY_MESSAGE(RemoveCharacter)

struct GameAccountRequestData
{
    uint64_t m_auth_account_id;
    int max_character_slots;
    bool create_if_does_not_exist;
};
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

    uint32_t m_db_id;
    uint32_t m_account_id;
    uint32_t m_HitPoints;
    uint32_t m_Endurance;

    int index;
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
struct GameAccountResponseData
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
TWO_WAY_MESSAGE(GameAccount)

struct GameDbErrorData
{
    QString message;
};
ONE_WAY_MESSAGE(GameDbError)

#undef ONE_WAY_MESSAGE
#undef SIMPLE_TWO_WAY_MESSAGE
#undef TWO_WAY_MESSAGE
