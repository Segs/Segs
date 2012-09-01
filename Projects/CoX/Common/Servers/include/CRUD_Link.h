#pragma once
#include <ace/ACE.h>
#include <ace/Synch.h>
#include <ace/SOCK_Dgram.h>
#include <ace/Message_Queue.h>
#include <ace/Message_Block.h>
#include <ace/Event_Handler.h>
#include <ace/Svc_Handler.h>
#include <ace/Reactor_Notification_Strategy.h>
#include <ace/Thread_Mutex.h>
#include <ace/Guard_T.h>
#include "CRUDP_Protocol.h"
#include "LinkLevelEvent.h"
#include "Buffer.h"
#include "EventProcessor.h"
#include "CRUDP_Packet.h"
#include "PacketCodec.h"
class SEGSEvent;
template<class EVENT_FACTORY>
class CRUDLink : public EventProcessor
{
protected:
typedef typename EVENT_FACTORY::tClientData tClientData;
typedef EventProcessor                      super;
        ACE_Reactor_Notification_Strategy   m_notifier;	// our queue will use this to inform the reactor of it's new elements
        ACE_HANDLE                          get_handle (void) const {return peer_.get_handle();}
        ACE_Time_Value                      m_last_activity; // last link activity time

public:

typedef ACE_SOCK_Dgram  stream_type;
typedef ACE_INET_Addr   addr_type;
static  EventProcessor *g_link_target;      //! All outgoing events are posted here
static  EventProcessor *g_target;           //! All incoming events are posted here

public:
                    CRUDLink() :  m_notifier(0, 0, ACE_Event_Handler::WRITE_MASK)
                    {
                        m_notifier.event_handler(this);
                        assert(g_target);
                        m_protocol.setCodec(new PacketCodecNull);
                    }
                    ~CRUDLink(void){}

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

    tClientData *   client_data() {return m_link_data;}
    void            client_data(tClientData *d) {m_link_data=d;}

    ACE_Time_Value  inactivity_time() //! return the amount of time this client wasn't sending anything
                    {
                        return ACE_OS::gettimeofday()-m_last_activity;
                    }
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
    void            event_for_packet(SEGSEvent *pak_ev);
    void            packets_for_event(SEGSEvent *c_ev); // Handler posted this event to us, we will pack it into packets and post it to the link target
    void            connection_update() //! Connection updates are done only when new data is available on the link
                    {
                        m_last_activity = ACE_OS::gettimeofday();
                    }
    CrudP_Protocol  m_protocol;
    stream_type     peer_;  /// Maintain connection with client.
    addr_type       m_peer_addr;
    tClientData *   m_link_data;
};
