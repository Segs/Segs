#pragma once
#include "CRUDP_Protocol.h"
#include "EventProcessor.h"
#include "Common/CRUDP_Protocol/ILink.h"
#include "PacketCodec.h"

#include <ace/ACE.h>
#include <ace/Synch.h>
#include <ace/SOCK_Dgram.h>
#include <ace/Message_Queue.h>
#include <ace/Event_Handler.h>
#include <ace/Svc_Handler.h>
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
typedef ILink                               super;
        ACE_Reactor_Notification_Strategy   m_notifier; // our queue will use this to inform the reactor of it's new elements
        ACE_HANDLE                          get_handle (void) const override {return peer_.get_handle();}
        ACE_Time_Value                      m_last_recv_activity; // last link activity time
        ACE_Time_Value                      m_last_send_activity; // last send activity on the link
public:

typedef ACE_SOCK_Dgram  stream_type;
typedef ACE_INET_Addr   addr_type;

public:
                    CRUDLink();
virtual             ~CRUDLink();

    int             open(void * = 0); //!< Called when we start to service a new connection, here we tell reactor to wake us when queue() is not empty.
    CrudP_Protocol *get_proto() {return &m_protocol;}
    int             handle_output( ACE_HANDLE = ACE_INVALID_HANDLE );
    void            received_block(BitStream &bytes);
    void            dispatch(SEGSEvent *)
                    {
                        assert(!"Should not be called");
                    }
    stream_type &   peer() {return peer_;}
    addr_type &     peer_addr() {return m_peer_addr;}
    void            set_client_data(void *d) {m_link_data=d;}

    ACE_Time_Value  client_last_seen_packets() const //! return the amount of time this client wasn't sending anything
                    {
                        return ACE_OS::gettimeofday()-m_last_recv_activity;
                    }
    ACE_Time_Value  last_sent_packets() const //! return the amount of time this client wasn't sending anything
                    {
                        return ACE_OS::gettimeofday()-m_last_send_activity;
                    }
    size_t          client_packets_waiting_for_ack() const { return m_protocol.UnackedPacketCount(); }
protected:
    SEGSEvent *     dispatch_sync( SEGSEvent * )
                    {
                        assert(!"No sync events known");
                        return 0;
                    }

    int             handle_close(ACE_HANDLE h, ACE_Reactor_Mask c)
                    {
                        reactor()->cancel_wakeup(this, ACE_Event_Handler::WRITE_MASK);
                        return EventProcessor::handle_close(h,c);
                    }
    void            event_for_packet(PacketEvent *pak_ev);
    void            packets_for_event(SEGSEvent *c_ev); // Handler posted this event to us, we will pack it into packets and post it to the link target
    void            connection_update() //! Connection updates are done only when new data is available on the link
                    {
                        m_last_recv_activity = ACE_OS::gettimeofday();
                    }
    void            connection_sent_packet() //! Connection updates are done only when new data is sent on the link
                    {
                        m_last_send_activity = ACE_OS::gettimeofday();
                    }
    EventProcessor *target() { return m_target; }
    EventProcessor *net_layer() { return m_net_layer; }
virtual CRUD_EventFactory &factory() = 0;

    CrudP_Protocol  m_protocol;
    stream_type     peer_;  /// Maintain connection with client.
    addr_type       m_peer_addr;
    void *          m_link_data;
    EventProcessor *m_net_layer;      //! All outgoing events are posted here
    EventProcessor *m_target;           //! All incoming events are posted here

};
