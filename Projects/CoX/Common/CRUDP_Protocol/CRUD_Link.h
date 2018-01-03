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

class SEGSEvent;
class PacketEvent;
class CRUD_EventFactory;


class CRUDLink : public ILink
{
protected:
using super = ILink;
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
    void            received_block(BitStream &bytes) override;
    void            dispatch(SEGSEvent *) override
                    {
                        assert(!"Should not be called");
                    }
    CrudP_Protocol *get_proto() { return &m_protocol; }
    stream_type &   peer() { return peer_; }
    addr_type &     peer_addr() { return m_peer_addr; }
    void            set_client_data(void *d) { m_link_data = d; }

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
    SEGSEvent *     dispatchSync( SEGSEvent * ) override
                    {
                        assert(!"No sync events known");
                        return nullptr;
                    }

    int             handle_close(ACE_HANDLE h, ACE_Reactor_Mask c) override;
    void            event_for_packet(PacketEvent *pak_ev);
    void            packets_for_event(SEGSEvent *c_ev);
    void            connection_update();
    void            connection_sent_packet();
    EventProcessor *target() { return m_target; }
    EventProcessor *net_layer() { return m_net_layer; }
virtual CRUD_EventFactory &factory() = 0;

    CrudP_Protocol  m_protocol;
    stream_type     peer_;  /// Maintain connection with client.
    addr_type       m_peer_addr;
    void *          m_link_data;
    EventProcessor *m_net_layer;      //! All outgoing events are put here
    EventProcessor *m_target;         //! All incoming events are put here

};
