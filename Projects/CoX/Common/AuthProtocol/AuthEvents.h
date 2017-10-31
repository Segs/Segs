#pragma once
#include "LinkLevelEvent.h"
#include "Buffer.h"
class AuthLink;

//! all events that know how to read from
//! and write to the buffer inherit from this
class AuthLinkEvent : public SEGSEvent
{
protected:
        ~AuthLinkEvent() override = default;
public:
        AuthLinkEvent(size_t evtype,EventProcessor *ev_src=nullptr) : SEGSEvent(evtype,ev_src)
        {}
        virtual void serializeto(GrowingBuffer &) const=0;
        virtual void serializefrom(GrowingBuffer &)=0;
};

enum AuthEventTypes
{
    evContinue=SEGS_EventTypes::evLAST_EVENT,
    evAuthProtocolVersion,
    evAuthorizationError,
    evServerSelectRequest,
    evDbError,
    evLogin,
    evLoginResponse,
    evServerListResponse,
    evServerListRequest,
    evServerSelectResponse

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
