/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "CRUDP_Protocol.h"
#include "EventProcessor.h"
#include "Common/CRUDP_Protocol/ILink.h"
#include "PacketCodec.h"

#include <ace/ACE.h>
#include <ace/Synch.h>
#include <ace/SOCK_Dgram.h>
#include <ace/Reactor_Notification_Strategy.h>
#include <ace/Thread_Mutex.h>
#include <ace/Guard_T.h>

#include <cassert>

namespace SEGSEvents
{
    class Event;
    class Packet;
}
class CRUD_EventFactory;


class CRUDLink : public LinkBase
{
protected:
using super = LinkBase;
        ACE_Reactor_Notification_Strategy   m_notifier; // our queue will use this to inform the reactor of it's new elements
        ACE_HANDLE                          get_handle (void) const override {return peer_.get_handle();}
        ACE_Time_Value                      m_last_recv_activity; // last link activity time
        ACE_Time_Value                      m_last_send_activity; // last send activity on the link
public:

using stream_type = ACE_SOCK_Dgram;
using addr_type = ACE_INET_Addr;

public:
                    CRUDLink();
                    ~CRUDLink() override;

    int             open(void * = nullptr) override;
    int             handle_output( ACE_HANDLE = ACE_INVALID_HANDLE ) override;
    void            received_block(BitStream &bytes);
    CrudP_Protocol *get_proto() { return &m_protocol; }
    stream_type &   peer() { return peer_; }
    addr_type &     peer_addr() { return m_peer_addr; }

    ACE_Time_Value  client_last_seen_packets() const //! return the amount of time this client hasn't received anything
                    {
                        return ACE_OS::gettimeofday()-m_last_recv_activity;
                    }
    ACE_Time_Value  last_sent_packets() const //! return the amount of time this client wasn't sending anything
                    {
                        return ACE_OS::gettimeofday()-m_last_send_activity;
                    }
    size_t          client_packets_waiting_for_ack() const { return m_protocol.UnackedPacketCount(); }
protected:
    int             handle_close(ACE_HANDLE h, ACE_Reactor_Mask c) override;
    void            event_for_packet(SEGSEvents::Packet *pak_ev);
    void            packets_for_event(SEGSEvents::Event *c_ev);
    void            connection_update();
    void            connection_sent_packet();
    EventSrc *      target() { return m_target; }
    EventSrc *      net_layer() { return m_net_layer; }
virtual CRUD_EventFactory &factory() = 0;

    CrudP_Protocol  m_protocol;
    stream_type     peer_;  //!< Maintain connection with client.
    addr_type       m_peer_addr;
    EventSrc *      m_net_layer;      //!< All outgoing events are put here
    EventSrc *      m_target;         //!< All incoming events are put here

};
