/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Common/CRUDP_Protocol/ILink.h"

#include <ace/SOCK_Stream.h>
#include <ace/Thread_Mutex.h>
#include <ace/Reactor_Notification_Strategy.h>

#include <memory>

enum class AuthLinkType
{
    Server = 0,
    Client = 1
};
struct AuthLinkState;
// AuthLinks are created when connection is accepted.
// They serve as one of the primary event sources in the system, the other being Timers
// Whenever new bytes are received, the AuthLink tries to convert them into proper higher level Events,
// And posts them to g_target
// Each AuthLink serves as a Client's connection context
class AuthLink final : public LinkBase
{
    using super = LinkBase;
    using tNotifyStrategy = ACE_Reactor_Notification_Strategy;
    friend class AuthHandler; // auth handler changes our m_state variable
public:
    enum eLinkStage
    {
        INITIAL,
        CONNECTED,
        AUTHORIZED,
        CLIENT_SERVSELECT,
        CLIENT_AWAITING_DISCONNECT,
        DISCONNECTED
    };

    using stream_type = ACE_SOCK_Stream;
    using addr_type = ACE_INET_Addr;

                        AuthLink();
                        AuthLink(EventProcessor *target, AuthLinkType link_type = AuthLinkType::Server);
                        ~AuthLink() override;

        int             open(void * = nullptr) override;
        int             handle_input (ACE_HANDLE) override;
        int             handle_output(ACE_HANDLE fd = ACE_INVALID_HANDLE) override;
        int             handle_close(ACE_HANDLE handle,ACE_Reactor_Mask close_mask) override;
        stream_type &   peer() {return m_peer;}
        addr_type &     peer_addr() {return m_peer_addr;}
        void            init_crypto(int vers,uint32_t seed);
        ACE_HANDLE      get_handle () const override {return m_peer.get_handle();}
        eLinkStage      get_link_stage() const;
        void            set_link_stage(AuthLink::eLinkStage stage);
protected:

        EventProcessor *m_target; // Target handler of this link
        tNotifyStrategy m_notifier;                     //!< Our message queue will use this to wake up the reactor on new elements
        stream_type     m_peer;  //!< Underlying client connection object.
        addr_type       m_peer_addr;
        ACE_Thread_Mutex m_buffer_mutex;
        std::unique_ptr<AuthLinkState> m_state;
};


