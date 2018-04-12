/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2017 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
#include "Buffer.h"
#include "CRUDP_Packet.h"
#include "CRUDP_Protocol.h"
#include "EventProcessor.h"
#include "LinkLevelEvent.h"

#include <memory>
class CRUD_EventTypes
{
public:
    BEGINE_EVENTS(SEGS_EventTypes)
    EVENT_DECL(evPacket, 0)
    EVENT_DECL(evControl, 1)
    EVENT_DECL(evDisconnectRequest, 2)
    EVENT_DECL(evIdle, 3)
    EVENT_DECL(evUnknownEvent, 4)
    EVENT_DECL(evDisconnectResponse, 5)
    EVENT_DECL(evConnectRequest, 6)
    EVENT_DECL(evConnectResponse, 7)
    END_EVENTS(8)
};
class PacketEvent : public SEGSEvent
{
public:
    PacketEvent(EventProcessor *evsrc, std::unique_ptr<CrudP_Packet> &&pkt, const ACE_INET_Addr &tgt)
        : SEGSEvent(CRUD_EventTypes::evPacket, evsrc), m_pkt(std::move(pkt)), target(tgt)
    {
    }
    std::unique_ptr<CrudP_Packet> m_pkt;
    ACE_INET_Addr  target;
    const uint8_t *bytes() const { return m_pkt->GetStream()->read_ptr(); }
    size_t         size() const { return m_pkt->GetStream()->GetReadableDataSize(); }
};
class CRUDLink_Event : public SEGSEvent
{
public:
    CRUDLink_Event(size_t evtype, EventProcessor *ev_src = nullptr) : SEGSEvent(evtype, ev_src) //,LINK
    {
    }
    virtual void serializeto(BitStream &) const = 0;
    virtual void serializefrom(BitStream &)     = 0;
    size_t       m_seq_number;
    bool         m_reliable = true;
};

class ConnectRequest : public CRUDLink_Event
{
public:
    ConnectRequest() : CRUDLink_Event(CRUD_EventTypes::evConnectRequest) {}
    void serializeto(BitStream &tgt) const override
    {
        tgt.StorePackedBits(1, 1); // opcode 1
        tgt.StorePackedBits(1, m_tickcount);
        tgt.StorePackedBits(1, m_version);
    }
    void serializefrom(BitStream &src) override
    {
        m_tickcount = src.GetPackedBits(1);
        m_version   = src.GetPackedBits(1);
    }
    uint32_t m_tickcount;
    uint32_t m_version;
};

class ConnectResponse : public CRUDLink_Event
{
public:
    ConnectResponse() : CRUDLink_Event(CRUD_EventTypes::evConnectResponse) {}
    void serializeto(BitStream &tgt) const override
    {
        tgt.StorePackedBits(1, 0); // ctrl opcode
        tgt.StorePackedBits(1, 4); // opcode
    }
    void serializefrom(BitStream &) override {}
};
class DisconnectRequest : public CRUDLink_Event
{
public:
    DisconnectRequest() : CRUDLink_Event(CRUD_EventTypes::evDisconnectRequest) {}
    void serializefrom(BitStream &bs) override
    {
        bs.GetPackedBits(1);
        bs.GetPackedBits(1);
    }
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1, 0);
        bs.StorePackedBits(1, 5);
    }
};
class DisconnectResponse : public CRUDLink_Event
{
public:
    DisconnectResponse() : CRUDLink_Event(CRUD_EventTypes::evDisconnectResponse) {}
    void serializefrom(BitStream &bs) override
    {
        bs.GetPackedBits(1);
        bs.GetPackedBits(1);
    }
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1, 0);
        bs.StorePackedBits(1, 6);
    }
};
class IdleEvent : public CRUDLink_Event
{
public:
    IdleEvent() : CRUDLink_Event(CRUD_EventTypes::evIdle) {}
    void serializefrom(BitStream &bs) override
    {
        // TODO: check this
        bs.GetPackedBits(1);
        bs.GetPackedBits(1);
    }
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1, 0);
        bs.StorePackedBits(1, 0);
    }
};

class CRUD_EventFactory
{
public:
    virtual CRUDLink_Event *EventFromStream(BitStream &bs)
    {
        int32_t opcode = bs.GetPackedBits(1);
        if (opcode != 0)
            return nullptr;
        // it seems idle commands can be shortened to only contain opcode==0.
        int32_t control_opcode = bs.GetReadableBits() == 0 ? 0 : bs.GetPackedBits(1);
        switch (control_opcode)
        {
        case 0:
            return new IdleEvent(); // CTRL_IDLE
        case 5:
            return new DisconnectRequest(); // CTRL_DISCONNECT_REQ
        default: break;
        }
        ACE_DEBUG((LM_WARNING, ACE_TEXT("Unhandled control event type %d\n"), control_opcode));
        return nullptr;
    }
};
