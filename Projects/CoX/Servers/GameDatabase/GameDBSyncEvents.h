/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#pragma once

#include "Servers/InternalEvents.h"
#include "GameData/chardata_definitions.h"

#include <QDateTime>

enum GameDBEventTypes : uint32_t
{
    evCharacterUpdate= Internal_EventTypes::evLAST_EVENT,
    //updates, no responses
    evSetClientOptions,
    evCostumeUpdate,
    evGuiUpdate,
    evOptionsUpdate,
    evKeybindsUpdate,
    evPlayerUpdate,
    //requests
    evRemoveCharacterRequest,
    evRemoveCharacterResponse,
    evGameAccountRequest,
    evGameAccountResponse,
    evWouldNameDuplicateRequest,
    evWouldNameDuplicateResponse,
    // Insert for characters
    evCreateNewCharacterRequest,
    evCreateNewCharacterResponse,
    // update by id for characters
    evSetCharacterRequest,
    evSetCharacterResponse,
    // select by id for entity data only
    evGetEntityRequest,
    evGetEntityResponse,

    evGameDbError
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
    name ## Request(name ## RequestData &&d,uint64_t token,EventProcessor *src = nullptr) :\
        InternalEvent(GameDBEventTypes::ev ## name ## Request,src),m_data(d) {session_token(token);}\
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
    QString m_player_data;
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

struct RemoveCharacterRequestData
{
    uint64_t account_id;
    int slot_idx;
};

struct RemoveCharacterResponseData
{
    int slot_idx;
};
TWO_WAY_MESSAGE(RemoveCharacter)

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
    QString m_serialized_player_data;

    uint32_t m_db_id;
    uint32_t m_account_id;

    int index;
    uint32_t m_current_costume_idx;
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
    GameAccountResponseCostumeData &current_costume() {
        if(m_current_costume_idx<m_costumes.size() )
            return m_costumes[m_current_costume_idx];
        assert(!m_costumes.empty());
        m_current_costume_idx = 0;
        return m_costumes.front();
    }
    const GameAccountResponseCostumeData &current_costume() const {
        if(m_current_costume_idx<m_costumes.size() )
            return m_costumes[m_current_costume_idx];
        assert(!m_costumes.empty());
        return m_costumes.front();
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

struct CreateNewCharacterRequestData
{
    GameAccountResponseCharacterData m_character;
    QString m_ent_data;
    uint16_t m_slot_idx;
    uint16_t m_max_allowed_slots;
    uint32_t m_client_id;
};

struct CreateNewCharacterResponseData
{
    uint32_t m_char_id;
    int slot_idx; // if -1 , no more free slots are left ?
};
TWO_WAY_MESSAGE(CreateNewCharacter)

struct GetEntityRequestData
{
    uint32_t m_char_id;
};

struct GetEntityResponseData
{
    uint32_t m_supergroup_id;
    QString m_ent_data;
};
TWO_WAY_MESSAGE(GetEntity)

struct WouldNameDuplicateRequestData
{
    QString m_name;
};

struct WouldNameDuplicateResponseData
{
    bool m_would_duplicate;
};
TWO_WAY_MESSAGE(WouldNameDuplicate)

struct GameDbErrorData
{
    QString message;
};
ONE_WAY_MESSAGE(GameDbError)

struct SetClientOptionsData
{
    uint32_t m_client_id;
    QString m_options;
    QString m_keybinds;
};
ONE_WAY_MESSAGE(SetClientOptions)

struct PlayerUpdateData
{
    uint32_t m_id;
    QString m_player_data;
};
ONE_WAY_MESSAGE(PlayerUpdate)

#undef ONE_WAY_MESSAGE
#undef SIMPLE_TWO_WAY_MESSAGE
#undef TWO_WAY_MESSAGE
