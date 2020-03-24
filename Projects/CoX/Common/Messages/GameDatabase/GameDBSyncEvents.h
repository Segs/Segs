/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Servers/InternalEvents.h"
#include "Messages/EmailService/EmailDefinitions.h"

#include <QDateTime>
namespace SEGSEvents
{

const QString EMPTY_STRING = "EMPTY"; // Client expects value "EMPTY" in several places

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
    // select by name for char id
    evGetEntityByNameRequest,
    evGetEntityByNameResponse,

    // email stuff
    evEmailCreateRequest,
    evEmailCreateResponse,
    evEmailMarkAsReadMessage,
    evEmailUpdateOnCharDeleteMessage,
    evEmailRemoveMessage,
    evGetEmailRequest,
    evGetEmailResponse,
    evGetEmailsRequest,
    evGetEmailsResponse,
    evGetEmailBySenderIdRequest,
    evGetEmailBySenderIdResponse,
    evGetEmailByRecipientIdRequest,
    evGetEmailByRecipientIdResponse,
    evFillEmailRecipientIdRequest,
    evFillEmailRecipientIdResponse,
    evFillEmailRecipientIdErrorMessage,

    evGameDbErrorMessage
};


struct CharacterUpdateData
{
    QString m_char_name;
    // Cerealized blobs
    QString m_costume_data;
    QString m_char_data;
    QString m_entity_data;
    QString m_player_data;
    uint32_t m_supergroup_id;
    uint32_t m_id;
    template <class Archive>
    void serialize( Archive & ar )
    {
        ar( m_char_name, m_costume_data );
        ar( m_char_data, m_entity_data, m_player_data );
        ar( m_supergroup_id, m_id );
    }
};
// [[ev_def:macro]]
ONE_WAY_MESSAGE(GameDBEventTypes,CharacterUpdate)

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

struct GameAccountResponseCharacterData
{
    QString m_name;
    QString m_serialized_costume_data;
    QString m_serialized_chardata;
    QString m_serialized_entity_data;
    QString m_serialized_player_data;

    uint32_t m_db_id;
    uint32_t m_account_id;
    int m_slot_idx;

    void reset()
    {
        m_name=EMPTY_STRING;
        m_serialized_entity_data.clear(); // Clearing entity data on character delete
    }

    bool isEmpty() const
    {
        return 0==m_name.compare(EMPTY_STRING, Qt::CaseInsensitive);
    }

    template <class Archive>
    void serialize( Archive & ar )
    {
        ar( m_name, m_serialized_costume_data );
        ar( m_serialized_chardata, m_serialized_entity_data );
        ar( m_serialized_player_data );
        ar( m_db_id, m_account_id );
        ar( m_slot_idx );
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
            if(c.m_name.compare(EMPTY_STRING)==0)
                return res;
            res++;
        }
        return -1;
    }
    bool valid() const { return m_game_server_acc_id!=0;}
    template <class Archive>
    void serialize( Archive & ar )
    {
        ar( m_game_server_acc_id, m_max_slots, m_characters  );
    }
};
// [[ev_def:macro]]
TWO_WAY_MESSAGE(GameDBEventTypes,GameAccount)

struct CostumeUpdateData
{
    uint32_t m_id;
    QString m_costume_data;
    template <class Archive>
    void serialize( Archive & ar )
    {
        ar( m_id, m_costume_data );
    }
};
// [[ev_def:macro]]
ONE_WAY_MESSAGE(GameDBEventTypes,CostumeUpdate)

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
        ar( m_character, m_ent_data );
        ar( m_slot_idx, m_max_allowed_slots);
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
        ar( m_char_id, slot_idx );
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
    uint32_t m_supergroup_id;
    QString m_ent_data;
    template <class Archive>
    void serialize( Archive & ar )
    {
        ar( m_supergroup_id, m_ent_data );
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
    uint32_t m_supergroup_id;
    QString m_ent_data;
    template <class Archive>
    void serialize( Archive & ar )
    {
        ar(m_supergroup_id, m_ent_data);
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

struct EmailCreateRequestData
{
    uint32_t m_sender_id;
    uint32_t m_recipient_id;
    QString m_recipient_name;
    QString m_email_data; // cerealized email

    template <class Archive>
    void serialize(Archive &ar)
    {
        ar(m_sender_id, m_recipient_id, m_recipient_name, m_email_data);
    }
};

struct EmailCreateResponseData
{
    uint32_t m_email_id;
    uint32_t m_sender_id;
    uint32_t m_recipient_id;
    QString m_recipient_name;
    QString m_cerealized_email_data; // cerealized email

    template <class Archive>
    void serialize(Archive &ar)
    {
        ar(m_email_id, m_sender_id, m_recipient_id, m_recipient_name, m_cerealized_email_data);
    }
};
// [[ev_def:macro]]
TWO_WAY_MESSAGE(GameDBEventTypes,EmailCreate)

struct EmailUpdateOnCharDeleteData
{
    uint32_t m_deleted_char_id;

    template <class Archive>
    void serialize(Archive &ar)
    {
        ar(m_deleted_char_id);
    }
};
// [[ev_def:macro]]
ONE_WAY_MESSAGE(GameDBEventTypes,EmailUpdateOnCharDelete)

struct EmailMarkAsReadData
{
    uint32_t m_email_id;
    QString m_email_data; // because m_is_read is inside the blob

    template <class Archive>
    void serialize(Archive &ar)
    {
        ar(m_email_id, m_email_data);
    }
};
// [[ev_def:macro]]
ONE_WAY_MESSAGE(GameDBEventTypes,EmailMarkAsRead)

// cannot use EmailDelete because it's used in EmailEvents already
struct EmailRemoveData
{
    uint32_t m_email_id;

    template <class Archive>
    void serialize(Archive &ar)
    {
        ar(m_email_id);
    }
};
// [[ev_def:macro]]
ONE_WAY_MESSAGE(GameDBEventTypes,EmailRemove)

struct GetEmailRequestData
{
    uint32_t m_email_id;

    template <class Archive>
    void serialize(Archive &ar)
    {
        ar(m_email_id);
    }
};

struct GetEmailResponseData
{
    uint32_t m_email_id;
    uint32_t m_sender_id;
    uint32_t m_recipient_id;
    QString m_email_data; // cerealized email

    template <class Archive>
    void serialize(Archive &ar)
    {
        ar(m_email_id, m_sender_id, m_recipient_id, m_email_data);
    }
};
// [[ev_def:macro]]
TWO_WAY_MESSAGE(GameDBEventTypes,GetEmail)

struct GetEmailsRequestData
{
    template <class Archive>
    void serialize(Archive &/*ar*/)
    {}
};

struct GetEmailsResponseData
{
    std::vector<EmailResponseData> m_email_response_datas;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_email_response_datas);
    }
};
// [[ev_def:macro]]
TWO_WAY_MESSAGE(GameDBEventTypes,GetEmails)

struct GetEmailBySenderIdRequestData
{
    uint32_t m_sender_id;

    template <class Archive>
    void serialize(Archive &ar)
    {
        ar(m_sender_id);
    }
};

struct GetEmailBySenderIdResponseData
{
    uint32_t m_email_id;
    uint32_t m_sender_id;
    uint32_t m_recipient_id;
    QString m_email_data; // cerealized email

    template <class Archive>
    void serialize(Archive &ar)
    {
        ar(m_email_id, m_sender_id, m_recipient_id, m_email_data);
    }
};
// [[ev_def:macro]]
TWO_WAY_MESSAGE(GameDBEventTypes,GetEmailBySenderId)

struct GetEmailByRecipientIdRequestData
{
    uint32_t m_recipient_id;

    template <class Archive>
    void serialize(Archive &ar)
    {
        ar(m_recipient_id);
    }
};

struct GetEmailByRecipientIdResponseData
{
    uint32_t m_email_id;
    uint32_t m_sender_id;
    uint32_t m_recipient_id;
    QString m_email_data; // cerealized email

    template <class Archive>
    void serialize(Archive &ar)
    {
        ar(m_email_id, m_sender_id, m_recipient_id, m_email_data);
    }
};
// [[ev_def:macro]]
TWO_WAY_MESSAGE(GameDBEventTypes,GetEmailByRecipientId)

struct FillEmailRecipientIdRequestData
{
    uint32_t m_sender_id;
    QString m_sender_name;
    QString m_recipient_name;
    QString m_subject;
    QString m_message;
    uint32_t m_timestamp;

    template <class Archive>
    void serialize (Archive &ar)
    {
        ar (m_sender_id, m_sender_name, m_recipient_name, m_subject, m_message, m_timestamp);
    }
};

struct FillEmailRecipientIdResponseData
{
    uint32_t m_sender_id;
    uint32_t m_recipient_id;    // the point of this is to get recipient_id from recipient_name :)
    QString m_sender_name;
    QString m_recipient_name;
    QString m_subject;
    QString m_message;
    uint32_t m_timestamp;

    template <class Archive>
    void serialize (Archive &ar)
    {
        ar (m_sender_id, m_recipient_id, m_sender_name, m_recipient_name, m_subject, m_message, m_timestamp);
    }
};
// [[ev_def:macro]]
TWO_WAY_MESSAGE(GameDBEventTypes,FillEmailRecipientId)

struct FillEmailRecipientIdErrorData
{
    uint32_t m_sender_id;
    QString m_recipient_name;

    template <class Archive>
    void serialize (Archive &ar)
    {
        ar (m_sender_id, m_recipient_name);
    }
};
// [[ev_def:macro]]
ONE_WAY_MESSAGE(GameDBEventTypes,FillEmailRecipientIdError)

} // end of SEGSEvents namespace
