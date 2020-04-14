/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "CRUDP_Protocol.h"
#include "Common/CRUDP_Protocol/ILink.h"

#include <ace/INET_Addr.h>
#include <ace/Reactor_Notification_Strategy.h>
#include <ace/SOCK_Dgram.h>
#include <ace/config-macros.h>
#include <stddef.h>
#include <chrono>
#include <atomic>

class EventSrc;

namespace SEGSEvents
{
    class Event;
    class Packet;
}
class CRUD_EventFactory;


class CRUDLink : public LinkBase
{

public:

using super       = LinkBase;
using stream_type = ACE_SOCK_Dgram;
using addr_type   = ACE_INET_Addr;
using time_point  = std::chrono::steady_clock::time_point;
using duration    = std::chrono::milliseconds;
using NotificatinStrategy = ACE_Reactor_Notification_Strategy;

                            CRUDLink();
                            ~CRUDLink() override;

        int                 open(void * = nullptr) override;
        int                 handle_output( ACE_HANDLE = ACE_INVALID_HANDLE ) override;
        void                received_block(BitStream &bytes);
        CrudP_Protocol *    get_proto() { return &m_protocol; }
        stream_type &       peer() { return m_peer; }
        addr_type &         peer_addr() { return m_peer_addr; }

        duration            client_last_seen_packets() const;
        duration            last_sent_packets() const;
        size_t              client_packets_waiting_for_ack() const { return m_protocol.UnackedPacketCount(); }
protected:
        int                 handle_close(ACE_HANDLE h, ACE_Reactor_Mask c) override;
        void                event_for_packet(SEGSEvents::Packet *pak_ev);
        void                packets_for_event(SEGSEvents::Event *c_ev);
        void                connection_update();
        void                connection_sent_packet();
        EventSrc *          target() { return m_target; }
        EventSrc *          net_layer() { return m_net_layer; }
virtual CRUD_EventFactory & factory() = 0;
        ACE_HANDLE          get_handle() const override {return m_peer.get_handle();}

        NotificatinStrategy m_notifier; //!< our queue will use this to inform the reactor of it's new elements

        std::atomic<long>   m_last_recv_activity; //!< last link activity time as a count of 'rep' since epoch
        std::atomic<long>   m_last_send_activity; //!< last send activity on the link
        CrudP_Protocol      m_protocol;
        stream_type         m_peer; //!< Maintain connection with client.
        addr_type           m_peer_addr;
        EventSrc *          m_net_layer; //!< All outgoing events are put here
        EventSrc *          m_target;    //!< All incoming events are put here

};
