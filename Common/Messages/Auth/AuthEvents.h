/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Components/LinkLevelEvent.h"
#include "Components/Buffer.h"
#include "Components/Logging.h"
class AuthLink;

namespace SEGSEvents
{
//! all events that know how to read from
//! and write to the buffer inherit from this
class AuthLinkEvent : public Event
{
protected:
        ~AuthLinkEvent() override = default;
public:
        AuthLinkEvent(size_t evtype,EventSrc *ev_src=nullptr) : Event(evtype,ev_src)
        {}
        virtual void serializeto(GrowingBuffer &) const=0;
        virtual void serializefrom(GrowingBuffer &)=0;
};

enum AuthEventTypes
{
    evSendLeftovers=CommonTypes::ID_LAST_CommonTypes,
    evAuthProtocolVersion,
    evAuthorizationError,
    evServerSelectRequest,
    evDbError,
    evReconnectAttempt,
    evLoginRequest,
    evLoginResponse,
    evServerListResponse,
    evServerListRequest,
    evServerSelectResponse

};
// [[ev_def:type]]
class SendLeftovers final : public Event // this event is posted from AuthLink to AuthLink, it means there are leftover unsent bytes.
{
public:
        SendLeftovers() : Event(evSendLeftovers)
        {}
        EVENT_IMPL(SendLeftovers)
};
// [[ev_def:type]]
class ReconnectAttempt final : public AuthLinkEvent
{
public:
    // [[ev_def:field]]
    uint8_t m_arr[8];
    ReconnectAttempt() : AuthLinkEvent(evReconnectAttempt)
    {}
    void init(EventSrc *ev_src,const uint8_t *auth_arr) {memcpy(m_arr,auth_arr,8);m_event_source=ev_src;}
    void serializeto(GrowingBuffer &buf) const override
    {
        buf.uPut((uint8_t)3);
        buf.uPutBytes(m_arr,8);
    }
    void serializefrom(GrowingBuffer &buf) override
    {
        uint8_t op;
        buf.uGet(op);
        if(op != 3)
        {
            qWarning() << op << "in ReconnectAttempt serializefrom";
            for (auto i : m_arr)
                qWarning() << i;
        }
        buf.uGetBytes(m_arr,8);
    }
    EVENT_IMPL(ReconnectAttempt)
};
} // end of SEGSEvents namespace
#include "Auth/AuthorizationError.h"
#include "Auth/AuthorizationProtocolVersion.h"
#include "Auth/DbError.h"
#include "Auth/ServerSelectRequest.h"
#include "Auth/LoginResponse.h"
#include "Auth/LoginRequest.h"
#include "Auth/ServerListResponse.h"
#include "Auth/ServerListRequest.h"
#include "Auth/ServerSelectResponse.h"
