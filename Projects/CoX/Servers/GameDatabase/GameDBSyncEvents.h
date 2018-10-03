/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Servers/InternalEvents.h"

#include <QDateTime>
namespace SEGSEvents
{

enum GameDBEventTypes : uint32_t
{
    BEGINE_EVENTS(GameDBEventTypes,Internal_EventTypes)
    evCharacterUpdateMessage,
    //updates, no responses
    evSetClientOptionsMessage,
    evCostumeUpdateMessage,
    evGuiUpdateMessage,
    evOptionsUpdateMessage,
    evKeybindsUpdateMessage,
    evPlayerUpdateMessage,
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
    // selecy by name for entity data
    evGetEntityByNameRequest,
    evGetEntityByNameResponse,
    // Supergroups
    evCreateNewSuperGroupRequest,
    evCreateNewSuperGroupResponse,
    evGetSuperGroupRequest,
    evGetSuperGroupResponse,
    evSuperGroupNameDuplicateRequest,
    evSuperGroupNameDuplicateResponse,
    evSuperGroupUpdateMessage,
    evRemoveSuperGroupRequest,
    evRemoveSuperGroupResponse,

    evGameDbErrorMessage
};


struct CharacterUpdateData
{
    QString m_char_name;
    // Cerealized blobs
    QString m_char_data;
    QString m_entity_data;
    QString m_player_data;
    // plain values
    uint32_t m_bodytype;
    float    m_height;
    float    m_physique;
    uint32_t m_supergroup_id;
    uint32_t m_id;
    template <class Archive>
    void serialize( Archive & ar )
    {
        ar( m_char_name, m_char_data, m_entity_data, m_player_data );
        ar( m_bodytype, m_height, m_physique );
        ar( m_supergroup_id, m_id );
    }
};
// [[ev_def:macro]]
ONE_WAY_MESSAGE(GameDBEventTypes,CharacterUpdate)

struct CostumeUpdateData
{
    QString m_parts; // cereal serialized costume parts
    int m_db_id;
    int m_costume_index;
    uint32_t m_skin_color;
    template <class Archive>
    void serialize( Archive & ar )
    {
        ar( m_parts,m_db_id,m_costume_index,m_costume_index );
    }
};
// [[ev_def:macro]]
ONE_WAY_MESSAGE(GameDBEventTypes,CostumeUpdate)

struct RemoveCharacterRequestData
{
    uint64_t account_id;
    int slot_idx;
    template <class Archive>
    void serialize( Archive & ar )
    {
        ar( account_id,slot_idx  );
    }
};

struct RemoveCharacterResponseData
{
    int slot_idx;
    template <class Archive>
    void serialize( Archive & ar )
    {
        ar( slot_idx  );
    }
};
// [[ev_def:macro]]
TWO_WAY_MESSAGE(GameDBEventTypes,RemoveCharacter)

struct GameAccountRequestData
{
    uint64_t m_auth_account_id;
    int max_character_slots;
    bool create_if_does_not_exist;
    template <class Archive>
    void serialize( Archive & ar )
    {
        ar( m_auth_account_id,max_character_slots );
        ar( create_if_does_not_exist);
    }
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

    template <class Archive>
    void serialize( Archive & ar )
    {
        ar( m_serialized_data, m_character_id );
        ar( m_body_type, m_height, m_physique);
        ar( skin_color, m_slot_index );
    }
};

struct GameAccountResponseCharacterData
{
    std::vector<GameAccountResponseCostumeData> m_costumes;
    QString m_name;
    QString m_serialized_chardata;
    QString m_serialized_player_data;
    QString m_serialized_entity_data;

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
    template <class Archive>
    void serialize( Archive & ar )
    {
        ar( m_costumes,m_name );
        ar( m_serialized_chardata,m_serialized_player_data, m_serialized_entity_data);
        ar( m_db_id,m_account_id );
        ar( index,m_current_costume_idx,m_villain, m_multiple_costumes );
    }
};

struct GameAccountResponseData
{
    uint32_t m_game_server_acc_id;
    int m_max_slots;
    std::vector<GameAccountResponseCharacterData> m_characters;
    GameAccountResponseCharacterData &get_character(size_t idx)
    {
        assert(idx<m_characters.size());
        return m_characters[idx];
    }
    const GameAccountResponseCharacterData &get_character(size_t idx) const
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
    template <class Archive>
    void serialize( Archive & ar )
    {
        ar( m_game_server_acc_id,m_max_slots,m_characters  );
    }
};
// [[ev_def:macro]]
TWO_WAY_MESSAGE(GameDBEventTypes,GameAccount)

struct CreateNewCharacterRequestData
{
    GameAccountResponseCharacterData m_character;
    QString m_ent_data;
    uint16_t m_slot_idx;
    uint16_t m_max_allowed_slots;
    uint32_t m_client_id;
    template <class Archive>
    void serialize( Archive & ar )
    {
        ar( m_character,m_ent_data );
        ar( m_slot_idx,m_max_allowed_slots);
        ar( m_client_id );
    }
};

struct CreateNewCharacterResponseData
{
    uint32_t m_char_id;
    int slot_idx; // if -1 , no more free slots are left ?
    template <class Archive>
    void serialize( Archive & ar )
    {
        ar( m_char_id,slot_idx );
    }
};
// [[ev_def:macro]]
TWO_WAY_MESSAGE(GameDBEventTypes,CreateNewCharacter)

struct GetEntityRequestData
{
    uint32_t m_char_id;
    template <class Archive>
    void serialize( Archive & ar )
    {
        ar( m_char_id );
    }
};

struct GetEntityResponseData
{
    QString m_ent_data;
    template <class Archive>
    void serialize( Archive & ar )
    {
        ar( m_ent_data );
    }
};
// [[ev_def:macro]]
TWO_WAY_MESSAGE(GameDBEventTypes,GetEntity)

struct GetEntityByNameRequestData
{
    QString m_char_name;
    template <class Archive>
    void serialize( Archive & ar )
    {
        ar( m_char_name );
    }
};

struct GetEntityByNameResponseData
{
    QString m_ent_data;
    template <class Archive>
    void serialize( Archive & ar )
    {
        ar( m_ent_data);
    }
};
// [[ev_def:macro]]
TWO_WAY_MESSAGE(GameDBEventTypes,GetEntityByName)

struct WouldNameDuplicateRequestData
{
    QString m_name;
    template <class Archive>
    void serialize( Archive & ar )
    {
        ar( m_name  );
    }
};

struct WouldNameDuplicateResponseData
{
    bool m_would_duplicate;
    template <class Archive>
    void serialize( Archive & ar )
    {
        ar( m_would_duplicate  );
    }
};
// [[ev_def:macro]]
TWO_WAY_MESSAGE(GameDBEventTypes,WouldNameDuplicate)

struct GameDbErrorData
{
    QString message;
    template <class Archive>
    void serialize( Archive & ar )
    {
        ar( message  );
    }
};
// [[ev_def:macro]]
ONE_WAY_MESSAGE(GameDBEventTypes,GameDbError)

struct SetClientOptionsData
{
    uint32_t m_client_id;
    QString m_options;
    QString m_keybinds;
    template <class Archive>
    void serialize( Archive & ar )
    {
        ar( m_client_id, m_options, m_keybinds );
    }
};
// [[ev_def:macro]]
ONE_WAY_MESSAGE(GameDBEventTypes,SetClientOptions)

struct PlayerUpdateData
{
    uint32_t m_id;
    QString m_player_data;
    template <class Archive>
    void serialize( Archive & ar )
    {
        ar( m_id, m_player_data );
    }
};
// [[ev_def:macro]]
ONE_WAY_MESSAGE(GameDBEventTypes,PlayerUpdate)

// SuperGroups
struct SuperGroupUpdateData
{
    uint32_t m_sg_id;
    QString m_sg_name;
    QString m_serialized_sg_data;
    QString m_serialized_sg_members;
    template <class Archive>
    void serialize( Archive & ar )
    {
        ar( m_sg_id );
        ar( m_sg_name );
        ar( m_serialized_sg_data );
        ar( m_serialized_sg_members );
    }
};
// [[ev_def:macro]]
ONE_WAY_MESSAGE(GameDBEventTypes,SuperGroupUpdate)

struct CreateNewSuperGroupRequestData
{
    QString m_sg_name;
    QString m_serialized_sg_data;
    QString m_serialized_sg_members;
    template <class Archive>
    void serialize( Archive & ar )
    {
        ar( m_sg_name );
        ar( m_serialized_sg_data );
        ar( m_serialized_sg_members );
    }
};

struct CreateNewSuperGroupResponseData
{
    uint32_t m_sg_id;
    template <class Archive>
    void serialize( Archive & ar )
    {
        ar( m_sg_id );
    }
};
// [[ev_def:macro]]
TWO_WAY_MESSAGE(GameDBEventTypes,CreateNewSuperGroup)

struct SuperGroupNameDuplicateRequestData
{
    QString m_sg_name;
    template <class Archive>
    void serialize( Archive & ar )
    {
        ar( m_sg_name  );
    }
};

struct SuperGroupNameDuplicateResponseData
{
    bool m_supergroup_duplicate;
    template <class Archive>
    void serialize( Archive & ar )
    {
        ar( m_supergroup_duplicate  );
    }
};
// [[ev_def:macro]]
TWO_WAY_MESSAGE(GameDBEventTypes,SuperGroupNameDuplicate)

struct GetSuperGroupRequestData
{
    uint32_t m_sg_id;
    QString m_sg_name;
    bool create_if_does_not_exist;
    template <class Archive>
    void serialize( Archive & ar )
    {
        ar( m_sg_id );
        ar( m_sg_name );
        ar( create_if_does_not_exist);
    }
};

struct GetSuperGroupResponseData
{
    uint32_t m_sg_id;
    QString m_sg_name;
    QString m_serialized_sg_data;
    QString m_serialized_sg_members;
    bool isEmpty() const
    {
        return 0==m_sg_name.compare("EMPTY",Qt::CaseInsensitive);
    }
    template <class Archive>
    void serialize( Archive & ar )
    {
        ar( m_sg_id );
        ar( m_sg_name );
        ar( m_serialized_sg_data );
        ar( m_serialized_sg_members );
    }
};
// [[ev_def:macro]]
TWO_WAY_MESSAGE(GameDBEventTypes,GetSuperGroup)

struct RemoveSuperGroupRequestData
{
    uint32_t m_sg_id;
    template <class Archive>
    void serialize( Archive & ar )
    {
        ar( m_sg_id  );
    }
};

struct RemoveSuperGroupResponseData
{
    uint32_t m_sg_id;
    template <class Archive>
    void serialize( Archive & ar )
    {
        ar( m_sg_id  );
    }
};
// [[ev_def:macro]]
TWO_WAY_MESSAGE(GameDBEventTypes,RemoveSuperGroup)

} // end of SEGSEvents namespace
