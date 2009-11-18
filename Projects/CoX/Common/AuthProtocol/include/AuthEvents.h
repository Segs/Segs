#pragma once
#include <ace/SOCK_Stream.h>
#include "LinkLevelEvent.h"
#include "Buffer.h"
class AuthLink;

//! all events that know how to read from
//  and write to the buffer inherit from this
typedef LinkLevelEvent<GrowingBuffer,AuthLink> AuthLinkEvent;

class ContinueEvent : public SEGSEvent // this event is posted from AuthLink to AuthLink, it means there are leftover unsent bytes.
{
public:
	ContinueEvent() : SEGSEvent(evContinue)
	{}
};
class ConnectEvent : public SEGSEvent
{
public:
	ConnectEvent(EventProcessor *ev_src) : SEGSEvent(evAuthConnect,ev_src)
	{
		ACE_ASSERT(ev_src);
	}
	ACE_INET_Addr src_addr;
};
class DisconnectEvent : public SEGSEvent
{
public:
    DisconnectEvent(EventProcessor *ev_src) : SEGSEvent(evAuthDisconnect,ev_src)
    {
        ACE_ASSERT(ev_src);
    }
};

#include "Events/AuthorizationError.h"
#include "Events/AuthorizationProtocolVersion.h"
#include "Events/DbError.h"
#include "Events/ServerSelectRequest.h"
#include "Events/LoginResponse.h"
#include "Events/LoginRequest.h"
#include "Events/ServerListResponse.h"
#include "Events/ServerListRequest.h"
#include "Events/ServerSelectResponse.h"