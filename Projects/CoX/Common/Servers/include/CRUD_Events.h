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
    BEGINE_EVENTS(SEGSEvent);
    EVENT_DECL(evPacket,0);
    EVENT_DECL(evControl,1);
    EVENT_DECL(evDisconnectRequest,2);
    EVENT_DECL(evIdle,3);
    EVENT_DECL(evUnknownEvent,4);
    EVENT_DECL(evDisconnectResponse,5);
    EVENT_DECL(evConnectRequest,6);
    EVENT_DECL(evConnectResponse,7);
    END_EVENTS(8);
};
class PacketEvent : public SEGSEvent
{
public:
    PacketEvent(EventProcessor *evsrc,CrudP_Packet *pkt,const ACE_INET_Addr &tgt) : SEGSEvent(CRUD_EventTypes::evPacket,evsrc),m_pkt(pkt),target(tgt)
    {

    }
    CrudP_Packet *m_pkt;
    ACE_INET_Addr target;
    const u8 *bytes() const {return m_pkt->GetStream()->read_ptr();}
    size_t size() const {return m_pkt->GetStream()->GetReadableDataSize();}
};
template<class LINK>
class CRUDLink_Event : public LinkLevelEvent<BitStream,LINK>
{
public:
    CRUDLink_Event(size_t evtype,EventProcessor *ev_src=0) : LinkLevelEvent<BitStream,LINK>(evtype,ev_src)
    {}
    size_t seq_number;
};

template<class LINK_EVENT>
class ConnectRequest : public LINK_EVENT
{
public:
    ConnectRequest():LINK_EVENT(CRUD_EventTypes::evConnectRequest)
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
    u32 m_tickcount;
    u32 m_version;
};

template<class LINK_EVENT>
class ConnectResponse : public LINK_EVENT
{
public:
    ConnectResponse() : LINK_EVENT(CRUD_EventTypes::evConnectResponse)
    {

    }
    void serializeto(BitStream &tgt) const
    {
        tgt.StorePackedBits(1, 0); //ctrl opcode
        tgt.StorePackedBits(1, 4); //opcode
    }
    void serializefrom(BitStream &src)
    {
    }
};
template<class LINK_EVENT>
class DisconnectRequest : public LINK_EVENT
{
public:
    DisconnectRequest() : LINK_EVENT(CRUD_EventTypes::evDisconnectRequest)
    {

    }
    void serializefrom(BitStream &bs)
    {
    }
    void serializeto(BitStream &bs) const
    {
        bs.StorePackedBits(1,0);
        bs.StorePackedBits(1,5);
    }
};
template<class LINK_EVENT>
class DisconnectResponse : public LINK_EVENT
{
public:
    DisconnectResponse() : LINK_EVENT(CRUD_EventTypes::evDisconnectResponse)
    {

    }
    void serializefrom(BitStream &bs)
    {
    }
    void serializeto(BitStream &bs) const
    {
        bs.StorePackedBits(1,0);
        bs.StorePackedBits(1,6);
    }
};
template<class LINK_EVENT>
class IdleEvent : public LINK_EVENT
{
public:
    IdleEvent() : LINK_EVENT(CRUD_EventTypes::evIdle)
    {

    }
    void serializefrom(BitStream &bs)
    {

    }
    void serializeto(BitStream &bs) const
    {
        bs.StorePackedBits(1,0);
        bs.StorePackedBits(1,0);
    }

};
template<class EVENT_CLASS>
class CRUD_EventFactory
{
public:
    typedef EVENT_CLASS event_type;
    static EVENT_CLASS *EventFromStream(BitStream &bs)
    {
        u8 opcode = bs.GetPackedBits(1);
        EVENT_CLASS *ev=0;
        if(opcode!=0)
            return ev;
        u8 control_opcode = bs.GetPackedBits(1);
        switch(control_opcode)
        {
        case 0: // CTRL_IDLE
            ev = new IdleEvent<EVENT_CLASS>();
            // link control event
            break;
        case 5: //CTRL_DISCONNECT_REQ
            ev = new DisconnectRequest<EVENT_CLASS>();
            break;
        default:
            ACE_ASSERT(!"Unknown control packet");
            break;
        }
        return ev;
    }
};
