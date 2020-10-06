/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Common/CRUDP_Protocol/CRUDP_Protocol.h"
#include "Components/EventProcessor.h"
#include "Common/GameData/map_definitions.h"
#include "Common/Messages/Map/GameCommand.h"
#include "Common/Messages/Map/MessageChannels.h"
#include "Common/GameData/Entity.h"

#include <ace/INET_Addr.h>
#include <ace/Time_Value.h>
#include <QtCore/QString>
#include <QtCore/QDateTime>
#include <glm/vec3.hpp>

namespace SEGSEvents
{
struct GameAccountResponseCharacterData;

enum Internal_EventTypes
{
    BEGINE_EVENTS_INTERNAL(Internal_EventTypes)
    evExpectClientRequest, //,0)
    evExpectClientResponse, // ,1)
    evExpectMapClientRequest, //,2)
    evExpectMapClientResponse, //,3)
    evClientConnectionRequest, //,4)
    evClientConnectionResponse, //,5)
    evReloadConfigMessage, //,6) // the server that receives this message will reload it's config file and restart with it
    evClientMapXferMessage, //,7)
    evMapSwapCollisionMessage, //,8)
    evGameServerStatusMessage = evReloadConfigMessage+4, //10)
    evMapServerStatusMessage,                     //11)
    // Message bus events
    evServiceStatusMessage = evExpectClientRequest+101,
    evClientConnectedMessage, // 102
    evClientDisconnectedMessage,  // 103
    END_EVENTS(Internal_EventTypes,105)
};

class InternalEvent : public Event
{
    uint64_t            m_session_token = 0;
public:
    using Event::Event; // forward all constructors to parent class.

    void                session_token(uint64_t token) { m_session_token = token; }
    uint64_t            session_token() const { return m_session_token; }

};

#define ONE_WAY_MESSAGE(enum_name,name)\
struct name ## Message final : public InternalEvent\
{\
    name ## Data m_data;\
    explicit name ## Message() :  InternalEvent(enum_name::ev ## name ## Message) {}\
    name ## Message(name ## Data &&d,uint64_t token,EventProcessor *ev_src=nullptr) : \
        InternalEvent(enum_name::ev ## name ## Message) , m_data(std::move(d)) {\
        session_token(token); src(ev_src);}\
    EVENT_IMPL(name ## Message)\
};

/// A message with Request having additional data
#define TWO_WAY_MESSAGE(enum_name,name)\
struct name ## Request final : public InternalEvent\
{\
    name ## RequestData m_data;\
    name ## Request(name ## RequestData &&d,uint64_t token,EventProcessor *ev_src=nullptr) : \
        InternalEvent(enum_name::ev ## name ## Request) , m_data(std::move(d)) {\
        session_token(token); src(ev_src);}\
    /* needed for serialization functions */\
    explicit name ## Request() : InternalEvent(enum_name::ev ## name ## Request) {}\
    EVENT_IMPL(name ## Request)\
};\
struct name ## Response final : public InternalEvent\
{\
    name ## ResponseData m_data;\
    name ## Response(name ## ResponseData &&d,uint64_t token) :  InternalEvent(enum_name::ev ## name ## Response),m_data(std::move(d)) {session_token(token);}\
    /* needed for serialization functions */\
    explicit name ## Response() : InternalEvent(enum_name::ev ## name ## Response) {}\
    EVENT_IMPL(name ## Response)\
};

// This tells the server that it should expect a new client connection from given address
struct ExpectClientRequestData
{
    ACE_INET_Addr m_from_addr;
    uint32_t m_client_id;
    uint8_t m_access_level;
    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_from_addr,m_client_id,m_access_level);
    }
};

// This event informs the server that given client is now expected on another server
// and passes that servers connection point, and connection cookie
struct ExpectClientResponseData
{
    uint64_t client_id;
    uint32_t cookie;
    uint32_t m_server_id; // this is the id of the server that is expecting the client
    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(client_id,cookie,m_server_id);
    }
};
//[[ev_def:macro]]
TWO_WAY_MESSAGE(Internal_EventTypes,ExpectClient)

struct ExpectMapClientRequestData
{
    uint64_t m_client_id;
    uint8_t m_access_level;
    ACE_INET_Addr m_from_addr;
    QString char_from_db_data; //! serialized character data, if this is empty Map server assumes a new character
    uint16_t m_slot_idx;
    QString m_character_name;
    QString m_map_name;
    uint16_t m_max_slots;
    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_client_id,m_access_level,m_from_addr,char_from_db_data,
           m_slot_idx,m_character_name,m_map_name,m_max_slots);
    }
};

struct ExpectMapClientResponseData
{
    uint32_t      cookie;
    uint32_t      m_server_id;       // this is the id of the server that is expecting the client
    ACE_INET_Addr m_connection_addr; // this is the address that will be sent as a target connection pont to the client
    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(cookie,m_server_id,m_connection_addr);
    }
};
//[[ev_def:macro]]
TWO_WAY_MESSAGE(Internal_EventTypes,ExpectMapClient)

// For now, no data here, could be a path to a config file?
struct ReloadConfigData
{

    template<class Archive>
    void serialize(Archive &)
    {
    }
};
//[[ev_def:macro]]
ONE_WAY_MESSAGE(Internal_EventTypes,ReloadConfig)

struct GameServerStatusData
{
    ACE_INET_Addr m_addr;
    QDateTime m_last_status_update;
    uint16_t m_current_players;
    uint16_t m_max_players;
    uint8_t m_id;
    bool m_online;
    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_addr,m_last_status_update,m_current_players,m_max_players,m_id,m_online);
    }
};
// This could be put in Message bus if any other server, apart from Auth needs this info.
//[[ev_def:macro]]
ONE_WAY_MESSAGE(Internal_EventTypes,GameServerStatus)

/////////////////////////////////////////////////////////////////////////////////////////////////
/// The following messages are put on the global Message Bus, and published there for all subscribers to see
//
struct ServiceStatusData
{
    QString status_message;
    int status_value;
    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(status_message,status_value);
    }
};
//[[ev_def:macro]]
ONE_WAY_MESSAGE(Internal_EventTypes,ServiceStatus)

struct ClientConnectedData
{
    uint64_t m_session;
    uint32_t m_server_id;     // id of the server the client connected to.
    uint32_t m_sub_server_id; // only used when server_id is the map server
    uint32_t m_char_db_id;       // id of the character connecting
    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_session,m_server_id,m_sub_server_id,m_char_db_id);
    }
};
//[[ev_def:macro]]
ONE_WAY_MESSAGE(Internal_EventTypes,ClientConnected)

struct ClientDisconnectedData
{
    uint64_t m_session;
    uint32_t m_char_db_id;       // id of the character disconnected
    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_session,m_char_db_id);
    }
};
//[[ev_def:macro]]
ONE_WAY_MESSAGE(Internal_EventTypes,ClientDisconnected)

struct ClientMapXferData
{
    uint64_t m_session;
    MapXferData m_map_data;
    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_session, m_map_data);
    }
};
// [[ev_def:macro]]
ONE_WAY_MESSAGE(Internal_EventTypes,ClientMapXfer)

struct MapSwapCollisionData
{
    uint32_t m_ent_db_id;
    glm::vec3 m_pos;
    QString m_node_name;
    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_ent_db_id, m_pos, m_node_name);
    }
};
// [[ev_def:macro]]
ONE_WAY_MESSAGE(Internal_EventTypes,MapSwapCollision)

using GameCommandVector = std::vector<std::unique_ptr<GameCommandEvent>>;     // might need another vector of InternalEvents if sending to something like game_db

struct ServiceToClientData
{
    GameCommandVector m_commands;
    QString m_message;
    uint64_t m_token;
    MessageChannel m_message_channel;

    ServiceToClientData(){};

    // use this if you are sending GameCommands and find your session from token (eg EmailService)
    ServiceToClientData(uint64_t token, GameCommandVector &&commands, QString msg = {}, MessageChannel messageChannel = MessageChannel::DEBUG_INFO)
    {
        m_token = token;
        m_commands = std::move(commands);
        m_message = msg;
        m_message_channel = messageChannel;
    }

    // same as above, but with tokens
    ServiceToClientData(uint64_t token, QString msg, MessageChannel messageChannel = MessageChannel::DEBUG_INFO)
    {
        m_token = token;
        m_message = msg;
        m_message_channel = messageChannel;
    }
};

using EntityFoundAction = std::function<void(Entity* ent)>;

struct ServiceToEntityData
{
    EntityFoundAction m_entity_found_action;
    uint64_t m_token;

    ServiceToEntityData(uint64_t token, EntityFoundAction action)
    {
        m_token = token;
        m_entity_found_action = action;
    }
};

using UPtrServiceToClientData = std::unique_ptr<SEGSEvents::ServiceToClientData>;
using UPtrServiceToEntityData = std::unique_ptr<SEGSEvents::ServiceToEntityData>;

} // end of SEGSEvents namespace
