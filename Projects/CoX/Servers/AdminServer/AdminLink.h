#pragma once

#include "LinkLevelEvent.h"
#include "Buffer.h"
#include "EventProcessor.h"

#include <ace/ACE.h>
#include <ace/Synch.h>
#include <ace/SOCK_Stream.h>
#include <ace/Message_Queue.h>
#include <ace/Message_Block.h>
#include <ace/Thread_Mutex.h>
#include <ace/Event_Handler.h>
#include <ace/Svc_Handler.h>
#include <ace/Reactor_Notification_Strategy.h>
class SEGSEvent;

class AdminLink final : public EventProcessor
{
    typedef EventProcessor super;
    typedef ACE_Reactor_Notification_Strategy tNotifyStrategy;
    enum eState
    {
        INITIAL,
        CONNECTED,
        AUTHORIZED,
        CLIENT_AWAITING_DISCONNECT,
        DISCONNECTED
    };
public:

using   stream_type = ACE_SOCK_Stream;
using   addr_type = ACE_INET_Addr;
static  EventProcessor *g_target;               //! All links post their messages to the same target

                        AdminLink();
                        ~AdminLink() = default;

        int             open(void * p = 0) override;
        int             handle_input (ACE_HANDLE) override;
        int             handle_output(ACE_HANDLE fd = ACE_INVALID_HANDLE) override;
        int             handle_close(ACE_HANDLE handle,ACE_Reactor_Mask close_mask) override;
        void            dispatch(SEGSEvent *ev) override;

        stream_type &   peer() {return peer_;}
        addr_type &     peer_addr() {return m_peer_addr;}

private:
        SEGSEvent *     dispatch_sync( SEGSEvent *ev ) override;
        ACE_HANDLE      get_handle (void) const override {return peer_.get_handle();}

        tNotifyStrategy m_notifier; // our queue will use this to inform the reactor of it's new elements
        stream_type     peer_;  //!< Underlying client connection object.
        addr_type       m_peer_addr;
        eState          m_state;
        GrowingBuffer   m_received_bytes_storage;       //!< Each link stores incoming bytes locally
        GrowingBuffer   m_unsent_bytes_storage;         //!< Each link stores outgoing bytes locally
        ACE_Thread_Mutex m_buffer_mutex;

        bool            send_buffer();
};
