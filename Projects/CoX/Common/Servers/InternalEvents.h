#pragma once
#include "CRUDP_Protocol/CRUDP_Protocol.h"
#include "EventProcessor.h"
#include <ace/INET_Addr.h>
#include <ace/Time_Value.h>
#include <QtCore/QString>
#include <QtCore/QDateTime>
class GameAccountResponseCharacterData;
class Internal_EventTypes
{
public:
    BEGINE_EVENTS_INTERNAL()
    EVENT_DECL(evExpectClientRequest,0)
    EVENT_DECL(evExpectClientResponse,1)
    EVENT_DECL(evExpectMapClientRequest,2)
    EVENT_DECL(evExpectMapClientResponse,3)
    EVENT_DECL(evClientConnectionRequest,4)
    EVENT_DECL(evClientConnectionResponse,5)
    EVENT_DECL(evReloadConfig,6) // the server that receives this message will reload it's config file and restart with it

    EVENT_DECL(evGameServerStatus,10)
    EVENT_DECL(evMapServerStatus,11)
    // Message bus events
    EVENT_DECL(evServiceStatus,101)
    EVENT_DECL(evClientConnected,102)
    EVENT_DECL(evClientDisconnected,103)
    END_EVENTS(105)
};
class InternalEvent : public SEGSEvent
{
    uint64_t            m_session_token = 0;
public:
    using SEGSEvent::SEGSEvent; // forward all constructors to parent class.

    void                session_token(uint64_t token) { m_session_token = token; }
    uint64_t            session_token() const { return m_session_token; }

};
#define ONE_WAY_MESSAGE(name)\
struct name ## Message final : public InternalEvent\
{\
    name ## Data m_data;\
    explicit name ## Message() :  InternalEvent(Internal_EventTypes::ev ## name) {}\
    name ## Message(name ## Data &&d) :  InternalEvent(Internal_EventTypes::ev ## name),m_data(d) {}\
};
/// A message without Request having additional data
#define SIMPLE_TWO_WAY_MESSAGE(name)\
struct name ## Request final : public InternalEvent\
{\
    name ## Message(int token) :  InternalEvent(Internal_EventTypes::ev ## name ## Request) {session_token(token);}\
};\
struct name ## Response final : public InternalEvent\
{\
    name ## Data m_data;\
    name ## Response(name ## Data &&d,uint64_t token) :  InternalEvent(Internal_EventTypes::ev ## name ## Response),m_data(d) {session_token(token);}\
};
/// A message with Request having additional data
#define TWO_WAY_MESSAGE(name)\
struct name ## Request final : public InternalEvent\
{\
    name ## RequestData m_data;\
    name ## Request(name ## RequestData &&d,uint64_t token) : InternalEvent(Internal_EventTypes::ev ## name ## Request),m_data(d) {session_token(token);}\
};\
struct name ## Response final : public InternalEvent\
{\
    name ## ResponseData m_data;\
    name ## Response(name ## ResponseData &&d,uint64_t token) :  InternalEvent(Internal_EventTypes::ev ## name ## Response),m_data(d) {session_token(token);}\
};
// This tells the server that it should expect a new client connection from given address
struct ExpectClientRequestData
{
    uint64_t m_client_id;
    ACE_INET_Addr m_from_addr;
};
// This event informs the server that given client is now expected on another server
// and passes that servers connection point, and connection cookie
struct ExpectClientResponseData
{
    uint64_t client_id;
    uint32_t cookie;
    uint32_t m_server_id; // this is the id of the server that is expecting the client
};
TWO_WAY_MESSAGE(ExpectClient)
struct ExpectMapClientRequestData
{
    uint64_t m_client_id;
    uint8_t m_access_level;
    ACE_INET_Addr m_from_addr;
    GameAccountResponseCharacterData *char_from_db;
    uint16_t m_slot_idx;
    QString m_character_name;
    uint32_t m_map_id;
};
struct ExpectMapClientResponseData
{
    uint64_t      client_id;
    uint32_t      cookie;
    uint32_t      m_server_id;       // this is the id of the server that is expecting the client
    ACE_INET_Addr m_connection_addr; // this is the address that will be sent as a target connection pont to the client
};
TWO_WAY_MESSAGE(ExpectMapClient)

// For now, no data here, could be a path to a config file?
struct ReloadConfigData
{

};
ONE_WAY_MESSAGE(ReloadConfig)

struct GameServerStatusData
{
    ACE_INET_Addr m_addr;
    QDateTime m_last_status_update;
    uint16_t m_current_players;
    uint16_t m_max_players;
    uint8_t m_id;
    bool m_online;
};
// This could be put in Message bus if any other server, apart from Auth needs this info.
ONE_WAY_MESSAGE(GameServerStatus)

/////////////////////////////////////////////////////////////////////////////////////////////////
/// The following messages are put on the global Message Bus, and published there for all subscribers to see
//
struct ServiceStatusData
{
    QString status_message;
    int status_value;
};
ONE_WAY_MESSAGE(ServiceStatus)
struct ClientConnectedData
{
    uint64_t m_session;
    uint32_t m_server_id;     // id of the server the client connected to.
    uint32_t m_sub_server_id; // only used when server_id is the map server
};
ONE_WAY_MESSAGE(ClientConnected)
struct ClientDisconnectedData
{
    uint64_t m_session;
};
ONE_WAY_MESSAGE(ClientDisconnected)

#undef ONE_WAY_MESSAGE
#undef SIMPLE_TWO_WAY_MESSAGE
#undef TWO_WAY_MESSAGE
