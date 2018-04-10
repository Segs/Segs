#pragma once

#include "LinkLevelEvent.h"
#include "Buffer.h"
#include "EventProcessor.h"
#include "AuthProtocol/AuthEvents.h"
#include "AuthProtocol/AuthPacketCodec.h"
#include "AuthProtocol/AuthOpcodes.h"

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
class AuthClient;

enum class AuthLinkType
{
    Server = 0,
    Client = 1
};

// AuthLinks are created when connection is accepted.
// They serve as one of the primary event sources in the system, the other being Timers
// Whenever new bytes are received, the AuthLink tries to convert them into proper higher level Events,
// And posts them to g_target
// Each AuthLink serves as a Client's connection context
class AuthLink final : public EventProcessor
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


static  EventProcessor *g_target;               //! All links post their messages to the same target

                        AuthLink(AuthLinkType link_type = AuthLinkType::Server);
                        ~AuthLink(void);

        int             open(void * = 0) override;
        int             handle_input (ACE_HANDLE) override;
        int             handle_output(ACE_HANDLE fd = ACE_INVALID_HANDLE) override;
        int             handle_close(ACE_HANDLE handle,ACE_Reactor_Mask close_mask) override;
        void            dispatch(SEGSEvent *ev) override;
        stream_type &   peer() {return peer_;}
        addr_type &     peer_addr() {return m_peer_addr;}
        AuthClient *    client() {return m_client;}
        void            client(AuthClient *c) {m_client=c;}
        void            init_crypto(int vers,uint32_t seed);
        ACE_HANDLE      get_handle (void) const override {return peer_.get_handle();}
protected:
        AuthClient *    m_client;
        AuthPacketCodec m_codec;
        GrowingBuffer   m_received_bytes_storage;       //!< Each link stores incoming bytes locally
        GrowingBuffer   m_unsent_bytes_storage;         //!< Each link stores outgoing bytes locally
        tNotifyStrategy m_notifier; // our queue will use this to inform the reactor of it's new elements
        int             m_protocol_version;
        stream_type     peer_;  //!< Underlying client connection object.
        addr_type       m_peer_addr;
        eState          m_state;
        ACE_Thread_Mutex *m_buffer_mutex;
        AuthLinkType    m_direction;

        bool            send_buffer();
        void            encode_buffer(const AuthLinkEvent *ev,size_t start);
        void            set_protocol_version(int vers);
        eAuthPacketType OpcodeToType(uint8_t opcode) const;
        SEGSEvent *     bytes_to_event();
};


