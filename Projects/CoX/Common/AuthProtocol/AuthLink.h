/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "LinkLevelEvent.h"
#include "Buffer.h"
#include "EventProcessor.h"
#include "AuthProtocol/AuthEvents.h"
#include "AuthProtocol/AuthPacketCodec.h"
#include "AuthProtocol/AuthOpcodes.h"
#include "Common/CRUDP_Protocol/ILink.h"

#include <ace/ACE.h>
#include <ace/Synch.h>
#include <ace/SOCK_Stream.h>
#include <ace/Message_Queue.h>
#include <ace/Message_Block.h>
#include <ace/Thread_Mutex.h>
#include <ace/Event_Handler.h>
#include <ace/Svc_Handler.h>
#include <ace/Reactor_Notification_Strategy.h>

namespace SEGSEvents
{
class Event;
}

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
class AuthLink final : public LinkBase
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


                        AuthLink();
                        AuthLink(EventProcessor *target, AuthLinkType link_type = AuthLinkType::Server);
                        ~AuthLink(void) override;

        int             open(void * = nullptr) override;
        int             handle_input (ACE_HANDLE) override;
        int             handle_output(ACE_HANDLE fd = ACE_INVALID_HANDLE) override;
        int             handle_close(ACE_HANDLE handle,ACE_Reactor_Mask close_mask) override;
        void            dispatch(SEGSEvents::Event *ev) override;
        stream_type &   peer() {return m_peer;}
        addr_type &     peer_addr() {return m_peer_addr;}
        void            init_crypto(int vers,uint32_t seed);
        ACE_HANDLE      get_handle () const override {return m_peer.get_handle();}
protected:
        EventProcessor *m_target; // Target handler of this link
        AuthPacketCodec m_codec;
        GrowingBuffer   m_received_bytes_storage;       //!< Each link stores incoming bytes locally
        GrowingBuffer   m_unsent_bytes_storage;         //!< Each link stores outgoing bytes locally
        tNotifyStrategy m_notifier;                     //!< Our message queue will use this to wake up the reactor on new elements
        stream_type     m_peer;  //!< Underlying client connection object.
        addr_type       m_peer_addr;
        ACE_Thread_Mutex *m_buffer_mutex;
        int             m_protocol_version;
        eState          m_state;
        AuthLinkType    m_direction;

        bool            send_buffer();
        void            encode_buffer(const SEGSEvents::AuthLinkEvent *ev,size_t start);
        void            set_protocol_version(int vers);
        eAuthPacketType OpcodeToType(uint8_t opcode) const;
        SEGSEvents::Event *bytes_to_event();
};


