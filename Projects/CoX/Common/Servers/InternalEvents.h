#pragma once
#include "CRUDP_Protocol/CRUDP_Protocol.h"
#include "EventProcessor.h"
#include <ace/INET_Addr.h>
#include <ace/Time_Value.h>
#include <QtCore/QString>
class Character;
class Internal_EventTypes
{
public:
    BEGINE_EVENTS_INTERNAL()
    EVENT_DECL(evExpectClientRequest,0)
    EVENT_DECL(evClientExpected,1)
    EVENT_DECL(evClientConnectionQuery,2)
    EVENT_DECL(evClientConnectionResponse,3)
    END_EVENTS(4)
};
class InternalEvent : public SEGSEvent
{
    uint64_t            m_session_token = 0;
public:
    using SEGSEvent::SEGSEvent; // forward all constructors to parent class.

    void                session_token(uint64_t token) { m_session_token = token; }
    uint64_t            session_token() const { return m_session_token; }

};
// This tells the server that it should expect a new client connection from given address
class ExpectClientRequest : public InternalEvent
{
    ExpectClientRequest() = delete;
public:
    ExpectClientRequest(EventProcessor *evsrc,uint64_t client_id,uint8_t access_level,const ACE_INET_Addr &from) :
                InternalEvent(Internal_EventTypes::evExpectClientRequest,evsrc),
                m_client_id(client_id),
                m_access_level(access_level),
                m_from_addr(from)
    {
    }
    uint64_t m_client_id;
    uint8_t m_access_level;
    ACE_INET_Addr m_from_addr;
};
class ExpectMapClient : public ExpectClientRequest
{
public:
    ExpectMapClient(EventProcessor *evsrc,  uint64_t client_id, uint8_t access_level,const ACE_INET_Addr &from) :
                ExpectClientRequest(evsrc,client_id,access_level,from)
    {
        m_map_id = 0;
        m_slot_idx=0;
    }
    void setValues(uint16_t slot_idx, const QString &name, uint32_t map_id,Character *char_f=0)
    {
        m_slot_idx  =   slot_idx;
        m_character_name = name;
        m_map_id    = map_id;
        char_from_db=char_f;
    }
    Character *char_from_db;
    uint16_t m_slot_idx;
    QString m_character_name;
    uint32_t m_map_id;
};
// This event informs the server that given client is now expected on another server
// and passes that servers connection point, and connection cookie
class ClientExpected : public InternalEvent
{
public:
        ClientExpected(EventProcessor *evsrc,uint64_t cid,uint32_t c,const ACE_INET_Addr &tgt) :
                        InternalEvent(Internal_EventTypes::evClientExpected,evsrc),
                        client_id(cid),
                        cookie(c),
                        m_connection_addr(tgt)
        {}
        uint64_t client_id;
        uint32_t cookie;
        ACE_INET_Addr m_connection_addr; // this is the address that will be sent as a target connection pont to the client
};

// Called synchronously this query is used to retrieve client's connection status.
// Maybe the map/game servers should just post ClientConnection updates to AuthServer ?
class ClientConnectionQuery : public InternalEvent
{
public:
    ClientConnectionQuery(EventProcessor *evsrc,uint64_t id) : InternalEvent(Internal_EventTypes::evClientConnectionQuery,evsrc),m_id(id)
    {

    }
    uint64_t m_id;
};

class ClientConnectionResponse : public InternalEvent
{
public:
    ClientConnectionResponse(EventProcessor *evsrc,const ACE_Time_Value &lc) : InternalEvent(Internal_EventTypes::evClientConnectionResponse,evsrc),last_comm(lc)
    {
    }
    ACE_Time_Value last_comm;
};
