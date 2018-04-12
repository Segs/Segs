/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
// segs includes

#include "Common/Servers/Server.h"
#include "AuthDatabase/AccountData.h"

// QT includes
#include <QtCore/QHash>
#include <QtCore/QString>

// ACE includes
#include <ace/ACE.h>
#include <ace/INET_Addr.h>
#include <ace/Singleton.h>
#include <ace/Synch.h>
#include <ace/Acceptor.h>
#include <ace/SOCK_Acceptor.h>

#include <unordered_map>

class AuthLink;
class AuthClient;

typedef ACE_Acceptor<AuthLink, ACE_SOCK_ACCEPTOR> ClientAcceptor;
class IClient;
typedef QHash<QString,AuthClient *> hmClients;
class AuthClient;
class AuthServer final : public Server
{
public:
                                    AuthServer();
                                    ~AuthServer() override;

        bool                        ReadConfig() override;
        bool                        Run(void) override;
        bool                        ShutDown(const QString &reason="No particular reason") override;

        bool                        GetClientByLogin(const char *,AuthAccountData &toFill);
protected:
        ClientAcceptor *            m_acceptor;     //!< ace acceptor wrapping AuthClientService
        ACE_INET_Addr               m_location;     //!< address this server will bind at.
        bool                        m_running;      //!< true if this server is running
};
