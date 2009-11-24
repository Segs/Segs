#pragma once
#include <ace/ACE.h>
#include <ace/Synch.h>
#include <ace/SOCK_Stream.h>
#include <ace/Message_Queue.h>
#include <ace/Message_Block.h>
#include <ace/Event_Handler.h>
#include <ace/Svc_Handler.h>
#include <ace/Reactor_Notification_Strategy.h>

#include "LinkLevelEvent.h"
#include "AuthEvents.h"
#include "AuthPacketCodec.h"
#include "Buffer.h"
#include "EventProcessor.h"

class SEGSEvent;
class AuthClient;
enum eAuthPacketType;
// AuthLinks are created when connection is accepted.
// They serve as one of the primary event sources in the system, the other being Timers
// Whenever new bytes are received, the AuthLink tries to convert them into proper higher level Events,
// And posts them to g_target
// Each AuthLink serves as a Client's context 
class AuthLink : public EventProcessor
{
    typedef EventProcessor super;
protected:
    AuthClient *m_client;
	AuthPacketCodec m_codec;						
	GrowingBuffer m_received_bytes_storage;			//! Each link stores incoming bytes locally
	GrowingBuffer m_unsent_bytes_storage;			//! Each link stores incoming bytes locally
	ACE_Reactor_Notification_Strategy m_notifier;	// our queue will use this to inform the reactor of it's new elements
    int				m_protocol_version;

	SEGSEvent *		bytes_to_event();				//! tries to convert the available bytes into a valid AuthHandler LinkLevelEvent.
    ACE_HANDLE      get_handle (void) const {return peer_.get_handle();}
public:
    typedef ACE_SOCK_Stream stream_type;
    typedef ACE_INET_Addr addr_type;

    static EventProcessor *g_target;				//! All links post their messages to the same target

	enum eState
	{
		INITIAL,
		CONNECTED,
        AUTHORIZED,
        CLIENT_SERVSELECT,
        CLIENT_AWAITING_DISCONNECT,
		DISCONNECTED
	};

	eState		    m_state;
				    AuthLink();
				    ~AuthLink(void);

	int			    open(void * = 0); //!< Called when we start to service a new connection, here we tell reactor to wake us when queue() is not empty.
	int			    handle_input (ACE_HANDLE);							//! some bytes arrived;
	int			    handle_output(ACE_HANDLE fd = ACE_INVALID_HANDLE);	//!< Called from ACEReactor when there are events in our queue() 
	int			    handle_close(ACE_HANDLE handle,ACE_Reactor_Mask close_mask);//!< Called when this handler is removed from the ACE_Reactor.
    void            dispatch(SEGSEvent *ev);
    eAuthPacketType OpcodeToType( u8 opcode,bool direction = false ) const; // if direction is false then this is a server side link otherwise client side.
	stream_type &   peer() {return peer_;}
    addr_type &     peer_addr() {return m_peer_addr;}
    AuthClient *    client() {return m_client;}
    void            client(AuthClient *c) {m_client=c;}
    void            init_crypto(int vers,u32 seed) {set_protocol_version(vers); m_codec.SetXorKey(seed);}
protected:
    SEGSEvent *     dispatch_sync( SEGSEvent *ev )
                    {
                        ACE_ASSERT(!"No sync events known");
                        return 0;
                    }

	stream_type     peer_;  /// Maintain connection with client.
    addr_type       m_peer_addr;
	bool		    send_buffer();
	void		    encode_buffer(const AuthLinkEvent *ev,size_t start);
    void		    set_protocol_version(int vers);
};
