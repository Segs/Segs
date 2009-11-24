#pragma once
#include <ace/INET_Addr.h>
#include <ace/Time_Value.h>
#include "CRUDP_Protocol.h"
#include "EventProcessor.h"
class Internal_EventTypes
{
public:
    BEGINE_EVENTS_INTERNAL(); // internal events ids start at 1000
    EVENT_DECL(evExpectClient,0);
    EVENT_DECL(evClientExpected,1);
    EVENT_DECL(evClientConnectionQuery,2);
    EVENT_DECL(evClientConnectionResponse,3);
    END_EVENTS(4);
};

// This tells the server that it should expect a new client connection from given address
class ExpectClient : public SEGSEvent
{
public:
    ExpectClient(EventProcessor *evsrc,u64 client_id,u16 access_level,const ACE_INET_Addr &from) : 
                SEGSEvent(Internal_EventTypes::evExpectClient,evsrc),
                m_client_id(client_id),
                m_access_level(access_level),
                m_from_addr(from)
    {
    }
    u64 m_client_id;
    u16 m_access_level;
    ACE_INET_Addr m_from_addr;
};
class ExpectMapClient : public ExpectClient
{
public:
    ExpectMapClient(
            EventProcessor *evsrc,
            u64 client_id,
            u16 access_level,
            const ACE_INET_Addr &from,
            const std::string &name,
            u32 map_id) : ExpectClient(evsrc,client_id,access_level,from),m_character_name(name),m_map_id(map_id)
    {}
    std::string m_character_name;
    u32 m_map_id;
};
// This event informs the server that given client is now expected on another server
// and passes that servers connection point, and connection cookie
class ClientExpected : public SEGSEvent
{
public:
    ClientExpected(EventProcessor *evsrc,u64 cid,u32 c,const ACE_INET_Addr &tgt) : 
			SEGSEvent(Internal_EventTypes::evClientExpected,evsrc),
			client_id(cid),
			cookie(c),
			m_connection_addr(tgt)
    {}
    u64 client_id;
    u32 cookie;
	ACE_INET_Addr m_connection_addr; // this is the address that will be sent as a target connection pont to the client
};

// Called synchronously this query is used to retrieve client's connection status.
// Maybe the map/game servers should just post ClientConnection updates to AuthServer ?
class ClientConnectionQuery : public SEGSEvent
{
public:
    ClientConnectionQuery(EventProcessor *evsrc,u64 id) : SEGSEvent(Internal_EventTypes::evClientConnectionQuery,evsrc),m_id(id)
    {

    }
    u64 m_id;
};

class ClientConnectionResponse : public SEGSEvent
{
public:
    ClientConnectionResponse(EventProcessor *evsrc,const ACE_Time_Value &lc) : SEGSEvent(Internal_EventTypes::evClientConnectionResponse,evsrc),last_comm(lc)
    {
    }
    ACE_Time_Value last_comm;
};
