#pragma once
#include <ace/ACE.h>
#include <ace/Synch.h>
#include <ace/SOCK_Stream.h>
#include <ace/Message_Queue.h>
#include <ace/Message_Block.h>
#include <ace/Thread_Mutex.h>
#include <ace/Event_Handler.h>
#include <ace/Svc_Handler.h>
#include <ace/Reactor_Notification_Strategy.h>

#include "LinkLevelEvent.h"
#include "AuthEvents.h"
#include "AuthPacketCodec.h"
#include "Buffer.h"
#include "EventProcessor.h"
#include "AuthOpcodes.h"

class SEGSEvent;
class AuthClient;

// AuthLinks are created when connection is accepted.
// They serve as one of the primary event sources in the system, the other being Timers
// Whenever new bytes are received, the AuthLink tries to convert them into proper higher level Events,
// And posts them to g_target
// Each AuthLink serves as a Client's connection context
class AuthLink : public EventProcessor
{    
    typedef EventProcessor super;
    typedef ACE_Reactor_Notification_Strategy tNotifyStrategy;
    friend class AuthHandler; // auth handler changes our m_state variable
    enum eState
    {
        INITIAL,
        CONNECTED,
        AUTHORIZED,
        CLIENT_SERVSELECT,
        CLIENT_AWAITING_DISCONNECT,
        DISCONNECTED
    };
public:
    typedef ACE_SOCK_Stream stream_type;
    typedef ACE_INET_Addr addr_type;

static EventProcessor * g_target;				//! All links post their messages to the same target

                        AuthLink();
                        ~AuthLink(void);

        int		open(void * = 0);
        int             handle_input (ACE_HANDLE);
        int             handle_output(ACE_HANDLE fd = ACE_INVALID_HANDLE);
        int             handle_close(ACE_HANDLE handle,ACE_Reactor_Mask close_mask);
	void            dispatch(SEGSEvent *ev);
	stream_type &   peer() {return peer_;}
	addr_type &     peer_addr() {return m_peer_addr;}
	AuthClient *    client() {return m_client;}
	void            client(AuthClient *c) {m_client=c;}
	void            init_crypto(int vers,u32 seed);
protected:
	AuthClient *	m_client;
	AuthPacketCodec m_codec;
	GrowingBuffer	m_received_bytes_storage;		//! Each link stores incoming bytes locally
	GrowingBuffer	m_unsent_bytes_storage;			//! Each link stores outgoing bytes locally
        tNotifyStrategy m_notifier;	// our queue will use this to inform the reactor of it's new elements
        int             m_protocol_version;
	stream_type     peer_;  //! Maintain connection with client.
	addr_type       m_peer_addr;
        eState          m_state;
	ACE_Thread_Mutex *m_buffer_mutex;

        SEGSEvent *     dispatch_sync( SEGSEvent *ev );
        ACE_HANDLE      get_handle (void) const {return peer_.get_handle();}
        bool            send_buffer();
        void            encode_buffer(const AuthLinkEvent *ev,size_t start);
        void            set_protocol_version(int vers);
        eAuthPacketType OpcodeToType( u8 opcode,bool direction = false ) const;
        SEGSEvent *     bytes_to_event();
};
