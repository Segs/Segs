/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
// SEGS includes
#include "AuthDatabase/AuthDBSyncEvents.h"
#include "EventProcessor.h"

// ACE includes
#include <ace/INET_Addr.h>
#include <ace/Synch.h>

class AuthLink;
class AuthHandler;
struct ClientAcceptor;
class AuthServer : public EventProcessor
{
public:
                                    AuthServer();
                                    ~AuthServer();

        void                        per_thread_shutdown() override;
        bool                        ReadConfigAndRestart();

protected:
        bool                        Run();
        ClientAcceptor *            m_acceptor;     //!< ace acceptor wrapping AuthClientService
        ACE_INET_Addr               m_location;     //!< address this server will bind at.
        bool                        m_running;      //!< true if this server is running
        ACE_Thread_Mutex            m_mutex;        //!< used to prevent multiple threads accessing config reload function
        std::unique_ptr<AuthHandler> m_handler;     //!< holds the AuthHandler

        // EventProcessor interface
        void dispatch(SEGSEvents::Event *ev);
};
