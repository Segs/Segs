#pragma once
#include <ace/SOCK_Stream.h>
#include "LinkLevelEvent.h"
#include "Buffer.h"
class AuthLink;

//! all events that know how to read from
//  and write to the buffer inherit from this
typedef LinkLevelEvent<GrowingBuffer,AuthLink> AuthLinkEvent;
enum AuthEventTypes
{
    evContinue=SEGSEvent::evLAST_EVENT,
    evAuthProtocolVersion,
    evAuthorizationError,
    evServerSelectRequest,
    evDbError,
    evLogin,
    evLoginResponse,
    evServerListResponse,
    evServerListRequest,
    evServerSelectResponse,

};
class ContinueEvent : public SEGSEvent // this event is posted from AuthLink to AuthLink, it means there are leftover unsent bytes.
{
public:
	ContinueEvent() : SEGSEvent(evContinue)
	{}
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