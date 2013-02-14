/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

#pragma once
#include <ace/Assert.h>
#include "CRUDP_Protocol.h"
#include "LinkLevelEvent.h"
#include "Buffer.h"
#include "EventProcessor.h"
#include "CRUDP_Packet.h"
class CRUD_EventTypes
{
public:
    BEGINE_EVENTS(SEGS_EventTypes)
    EVENT_DECL(evPacket,0)
    EVENT_DECL(evControl,1)
    EVENT_DECL(evDisconnectRequest,2)
    EVENT_DECL(evIdle,3)
    EVENT_DECL(evUnknownEvent,4)
    EVENT_DECL(evDisconnectResponse,5)
    EVENT_DECL(evConnectRequest,6)
    EVENT_DECL(evConnectResponse,7)
    END_EVENTS(8)
};
class PacketEvent : public SEGSEvent
{
public:
    PacketEvent(EventProcessor *evsrc,CrudP_Packet *pkt,const ACE_INET_Addr &tgt) : SEGSEvent(CRUD_EventTypes::evPacket,evsrc),m_pkt(pkt),target(tgt)
    {

    }
    CrudP_Packet *m_pkt;
    ACE_INET_Addr target;
    const uint8_t *bytes() const {return m_pkt->GetStream()->read_ptr();}
    size_t size() const {return m_pkt->GetStream()->GetReadableDataSize();}
};
class CRUDLink_Event : public SerializableEvent<BitStream>
{
public:
    CRUDLink_Event(size_t evtype,EventProcessor *ev_src=0) : SerializableEvent<BitStream>(evtype,ev_src)//,LINK
    {}
    size_t seq_number;
};

class ConnectRequest : public CRUDLink_Event
{
public:
    ConnectRequest():CRUDLink_Event(CRUD_EventTypes::evConnectRequest)
    {

    }
    void serializeto(BitStream &tgt) const
    {
        tgt.StorePackedBits(1, 1); // opcode 1
        tgt.StorePackedBits(1, m_tickcount);
        tgt.StorePackedBits(1, m_version);
    }
    void serializefrom(BitStream &src)
    {
        m_tickcount = src.GetPackedBits(1);
        m_version = src.GetPackedBits(1);
    }
    uint32_t m_tickcount;
    uint32_t m_version;
};

class ConnectResponse : public CRUDLink_Event
{
public:
    ConnectResponse() : CRUDLink_Event(CRUD_EventTypes::evConnectResponse)
    {

    }
    void serializeto(BitStream &tgt) const
    {
        tgt.StorePackedBits(1, 0); //ctrl opcode
        tgt.StorePackedBits(1, 4); //opcode
    }
    void serializefrom(BitStream &)
    {
    }
};
class DisconnectRequest : public CRUDLink_Event
{
public:
    DisconnectRequest() : CRUDLink_Event(CRUD_EventTypes::evDisconnectRequest)
    {

    }
    void serializefrom(BitStream &bs)
    {
        bs.GetPackedBits(1);
        bs.GetPackedBits(1);
    }
    void serializeto(BitStream &bs) const
    {
        bs.StorePackedBits(1,0);
        bs.StorePackedBits(1,5);
    }
};
class DisconnectResponse : public CRUDLink_Event
{
public:
    DisconnectResponse() : CRUDLink_Event(CRUD_EventTypes::evDisconnectResponse)
    {

    }
    void serializefrom(BitStream &bs)
    {
        bs.GetPackedBits(1);
        bs.GetPackedBits(1);
    }
    void serializeto(BitStream &bs) const
    {
        bs.StorePackedBits(1,0);
        bs.StorePackedBits(1,6);
    }
};
class IdleEvent : public CRUDLink_Event
{
public:
    IdleEvent() : CRUDLink_Event(CRUD_EventTypes::evIdle)
    {

    }
    void serializefrom(BitStream &bs)
    {
        //TODO: check this
        bs.GetPackedBits(1);
        bs.GetPackedBits(1);
    }
    void serializeto(BitStream &bs) const
    {
        bs.StorePackedBits(1,0);
        bs.StorePackedBits(1,0);
    }

};

class CRUD_EventFactory
{
public:
    static CRUDLink_Event *EventFromStream(BitStream &bs)
    {
        int32_t opcode = bs.GetPackedBits(1);
        if(opcode!=0)
            return 0;
        int32_t control_opcode = bs.GetPackedBits(1);
        switch(control_opcode)
        {
        case 0: return new IdleEvent(); // CTRL_IDLE
        case 5: return new DisconnectRequest(); //CTRL_DISCONNECT_REQ
        default:
            assert(!"Unknown control packet");
            break;
        }
        return 0;
    }
};
