/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup CRUDP_Protocol Projects/CoX/Common/CRUDP_Protocol
 * @{
 */

#include "CRUD_Link.h"

#include "CRUD_Events.h"
#include "PacketCodec.h"
#include "Components/BitStream.h"

#include <ace/Event_Handler.h>
#include <ace/Svc_Handler.h>

#include <QDebug>
#include <set>

using namespace SEGSEvents;
using namespace std::chrono;
// CRUD link receives messages from ServerEndpoint,
// these are basically CRUDP_Packets preprocessed by CRUDP_Protocol

// ServerEndpoint gets new input -> Bytes -> CRUDP_Protocol -> pushq(PacketEvent)
// ACE_Reactor knows to wake CRUDLink up, whenever there are new events
std::set<CRUDLink *> all_links;
CRUDLink::CRUDLink() :  m_notifier(nullptr, nullptr, ACE_Event_Handler::WRITE_MASK),m_net_layer(nullptr),m_target(nullptr)
{
    m_notifier.event_handler(this);
    m_protocol.setCodec(new PacketCodecNull);
    all_links.insert(this);
}

CRUDLink::~CRUDLink()
{
    all_links.erase(this);
}

///
/// \brief CRUDLink::event_for_packet - convert incoming packet into higher level events and push them to our target()
/// \param pak_ev - received packet
///
void CRUDLink::event_for_packet(Packet * pak_ev)
{
    CrudP_Packet *pak=pak_ev->m_pkt.get();
    // switch this to while, maybe many events are coming from single packet ?
    CRUDLink_Event *res = factory().EventFromStream(*pak->GetStream());
    if(!res)
    {
        ACE_ERROR((LM_ERROR, ACE_TEXT ("%p EventFromStream returned NULL\n")));
        return;
    }
    res->serializefrom(*pak->GetStream());
    res->src(this);
    res->m_seq_number = pak->GetSequenceNumber();
    target()->putq(res);
    if(pak->GetStream()->GetReadableBits()>1)
    {
        qDebug() << res->info() << "left" << pak->GetStream()->GetReadableBits() <<"bits";
    }
}

///
/// \brief CRUDLink::packets_for_event - convert event to 1-n packets and push them to our net_layer()
/// \param ev - an event that we've received from our downstream.
void CRUDLink::packets_for_event(Event *ev)
{
    lCrudP_Packet   packets_to_send;
    CRUDLink_Event *c_ev = static_cast<CRUDLink_Event *>(ev);
    CrudP_Packet *  res  = new CrudP_Packet;

    c_ev->serializeto(*res->GetStream()); // serialize packet into res packet
    res->SetReliabilty(c_ev->m_reliable);
    // create one or more properly formated CrudP_Packets in the protocol object
    // qDebug() << "Adding packets for"<<c_ev->info();
    m_protocol.SendPacket(res);
    if(!m_protocol.batchSend(packets_to_send))
    {
        // link is unresponsive, tell our target object
        target()->putq(new Disconnect(this));
        return;
    }
    // wrap all packets as PacketEvents and put them on link queue
    for (std::unique_ptr<CrudP_Packet> &pkt : packets_to_send)
    {
        net_layer()->putq(new Packet(this, std::move(pkt), peer_addr()));
    }
    packets_to_send.clear();
    connection_sent_packet(); // data was sent, update
}

//! Connection updates are done only when new data is available on the link
void CRUDLink::connection_update()
{
    m_last_recv_activity = steady_clock::now().time_since_epoch().count();

}

//! Connection updates are done only when new data is sent on the link
void CRUDLink::connection_sent_packet()
{
    m_last_send_activity = steady_clock::now().time_since_epoch().count();
}

//! Called when we start to service a new connection, here we tell reactor to wake us
//! when queue() is not empty.
int CRUDLink::open (void *p)
{
    if(super::open (p) == -1)
        return -1;
    m_notifier.reactor(reactor());  // notify reactor with write event,
    msg_queue()->notification_strategy (&m_notifier);   // whenever there is a new event on msg_queue()
    connection_update(); // record first activity time.
    connection_sent_packet();
    return 0;
}

int CRUDLink::handle_output( ACE_HANDLE )
{
    Event *ev;
    ACE_Time_Value nowait (ACE_OS::gettimeofday ());
    while (-1 != getq(ev, &nowait))
    {
        switch(ev->type())
        {
            case evFinish:
                ev->release();
                return -1;
            case evConnect:
                m_peer_addr=static_cast<Connect *>(ev)->src_addr;
                connection_update();
                break;
            case evDisconnect:
                putq(Finish::s_instance->shallow_copy()); // close the link
                break;
            case evPacket: // CRUDP_Protocol has posted a pre-parsed packet to us
                event_for_packet(static_cast<Packet *>(ev));
                break;
            default:
                packets_for_event(static_cast<CRUDLink_Event *>(ev));
                break;
        }
        ev->release();
        //TODO: check how getq works when nowait is before now() ??
        //nowait = ACE_OS::gettimeofday();
        //TODO: consider breaking out of this loop after processing N messages ?
    }
    // Now if our message queue is empty, we will wait unitl m_notifier awakens us.
    if(msg_queue()->is_empty ()) // we don't want to be woken up
        reactor()->cancel_wakeup(this, ACE_Event_Handler::WRITE_MASK);
    else // unless there is something to send still
        reactor()->schedule_wakeup(this, ACE_Event_Handler::WRITE_MASK);
    return 0;
}
///
/// \brief CRUDLink::received_block - convert incoming bytes to PacketEvent's
/// \param bytes - raw data to convert into packets.
///
void CRUDLink::received_block( BitStream &bytes )
{
    size_t recv_count=0; // count of proper packets
    // Fill the protocol with 'raw' bit stream
    m_protocol.ReceivedBlock(bytes);
    // now try to get actual packets
    CrudP_Packet *pkt = m_protocol.RecvPacket();
    while(pkt)
    {
        std::unique_ptr<CrudP_Packet> own_it(pkt);
        putq(new Packet(net_layer(),std::move(own_it),peer_addr()));
        ++recv_count;
        pkt=m_protocol.RecvPacket();
    }
    if(recv_count>0)
        connection_update(); //update the last time we've seen packets on this link
}
//! return the amount of time, in milliseconds, this client hasn't received anything
CRUDLink::duration CRUDLink::client_last_seen_packets() const
{
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch() - duration(m_last_recv_activity));
}

//! return the amount of time this client wasn't sending anything
CRUDLink::duration CRUDLink::last_sent_packets() const
{
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch() - duration(m_last_send_activity));
}

int CRUDLink::handle_close(ACE_HANDLE h, ACE_Reactor_Mask c)
{
    reactor()->cancel_wakeup(this, ACE_Event_Handler::WRITE_MASK);
    return super::handle_close(h,c);
}

//! @}
