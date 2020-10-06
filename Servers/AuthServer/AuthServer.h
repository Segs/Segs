/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
// SEGS includes
#include "AuthDatabase/AuthDBSyncEvents.h"
#include "Components/EventProcessor.h"

// ACE includes
#include <ace/INET_Addr.h>
#include <ace/Synch.h>

class AuthLink;
class AuthHandler;
struct ClientAcceptor;
class AuthServer final : public EventProcessor
{
public:
                                    IMPL_ID(AuthServer)
                                    AuthServer();
                                    ~AuthServer() override;


protected:
        void                        per_thread_shutdown() override;
        bool                        per_thread_startup() override;
        bool                        Run();
        bool                        ReadConfigAndRestart();
        std::unique_ptr<ClientAcceptor> m_acceptor; //!< ace acceptor wrapping AuthClientService
        ACE_INET_Addr               m_location;     //!< address this server will bind at.
        bool                        m_running;      //!< true if this server is running
        ACE_Thread_Mutex            m_mutex;        //!< used to prevent multiple threads accessing config reload function
        std::unique_ptr<AuthHandler> m_handler;     //!< holds the AuthHandler

        // EventProcessor interface
        void                        dispatch(SEGSEvents::Event *ev) override;
        void                        serialize_from(std::istream &is) override;
        void                        serialize_to(std::ostream &is) override;
};
