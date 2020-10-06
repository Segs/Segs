/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Common/CRUDP_Protocol/CRUDP_Packet.h"
#include "Components/Logging.h"
#include "Components/EventProcessor.h"
#include "Components/LinkLevelEvent.h"

#include <memory>

namespace SEGSEvents
{

enum CRUD_EventTypes : uint32_t
{
    // skip 11 entries here, they are used by AuthEventTypes
    BEGINE_EVENTS_SKIP(CRUD_EventTypes,CommonTypes,11)
    evPacket,
    evControlEvent,
    evDisconnectRequest,
    evIdle,
    evUnknownEvent,
    evDisconnectResponse,
    evConnectRequest,
    evConnectResponse,
    END_EVENTS(CRUD_EventTypes,8)
};
// [[ev_def:type]]
class Packet : public Event
{
public:
    Packet() : Event(CRUD_EventTypes::evPacket,nullptr) {}
    Packet(EventSrc *evsrc, std::unique_ptr<CrudP_Packet> &&pkt, const ACE_INET_Addr &tgt)
        : Event(evPacket, evsrc), m_pkt(std::move(pkt)), target(tgt)
    {
    }
    std::unique_ptr<CrudP_Packet> m_pkt;
    ACE_INET_Addr  target;
    const uint8_t *bytes() const;
    size_t         size() const;
    EVENT_IMPL(Packet)
};
class CRUDLink_Event : public Event
{
public:
    CRUDLink_Event(uint32_t evtype, EventSrc *ev_src = nullptr) : Event(evtype, ev_src) //,LINK
    {
    }
    virtual void serializeto(BitStream &) const = 0;
    virtual void serializefrom(BitStream &)     = 0;
    size_t       m_seq_number;
    bool         m_reliable = true;
};

// [[ev_def:type]]
class ConnectRequest : public CRUDLink_Event
{
public:
    ConnectRequest() : CRUDLink_Event(CRUD_EventTypes::evConnectRequest) {}
    void serializeto(BitStream &tgt) const override;
    void serializefrom(BitStream &src) override;
    // [[ev_def:field]]
    uint32_t m_tickcount;
    // [[ev_def:field]]
    uint32_t m_version;
    EVENT_IMPL(ConnectRequest)
};

// [[ev_def:type]]
class ConnectResponse : public CRUDLink_Event
{
public:
    ConnectResponse() : CRUDLink_Event(CRUD_EventTypes::evConnectResponse) {}
    void serializeto(BitStream &tgt) const override;
    void serializefrom(BitStream &) override {}
    EVENT_IMPL(ConnectResponse)
};
// [[ev_def:type]]
class DisconnectRequest : public CRUDLink_Event
{
public:
    DisconnectRequest() : CRUDLink_Event(CRUD_EventTypes::evDisconnectRequest) {}
    void serializefrom(BitStream &bs) override;
    void serializeto(BitStream &bs) const override;
    EVENT_IMPL(DisconnectRequest)
};
// [[ev_def:type]]
class DisconnectResponse : public CRUDLink_Event
{
public:
    DisconnectResponse() : CRUDLink_Event(CRUD_EventTypes::evDisconnectResponse) {}
    void serializefrom(BitStream &bs) override;
    void serializeto(BitStream &bs) const override;
    EVENT_IMPL(DisconnectResponse)
};
// [[ev_def:type]]
class Idle : public CRUDLink_Event
{
public:
    Idle() : CRUDLink_Event(CRUD_EventTypes::evIdle) {}
    void serializefrom(BitStream &bs) override;
    void serializeto(BitStream &bs) const override;
    EVENT_IMPL(Idle)
};
// [[ev_def:type]]
class UnknownEvent : public CRUDLink_Event
{
public:
    UnknownEvent():CRUDLink_Event(evUnknownEvent)
    { }
    void serializeto(BitStream &) const override { }
    void serializefrom(BitStream &) override { }
    EVENT_IMPL(UnknownEvent)
};
} // end of SEGSEvents namespace
class CRUD_EventFactory
{
public:
    virtual SEGSEvents::CRUDLink_Event *EventFromStream(BitStream &bs);
};

