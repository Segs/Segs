/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once

#include <unordered_map>
#include <list>
#include <string>

// ACE includes
#include <ace/ACE.h>
#include <ace/INET_Addr.h>
#include <ace/Singleton.h>
#include <ace/Synch.h>
#include <ace/Acceptor.h>
#include <ace/SOCK_Acceptor.h>

// segs includes

#include "Client.h"
#include "Server.h"
#include "ServerHandle.h"
#include "AuthServerInterface.h"

class AuthLink;
typedef ACE_Acceptor<AuthLink, ACE_SOCK_ACCEPTOR> ClientAcceptor;
class IClient;
typedef std::unordered_map<std::string,AuthClient *> hmClients;
class AuthClient;
class AuthServer  : public IAuthServer
{
//boost::object_pool<AuthClient>          m_client_pool;  //!< pool used to efficiently construct new client objects.
    typedef hmClients::iterator         ihmClients; //!< helper typedef for iterators to m_clients store
    typedef hmClients::const_iterator   cihmClients; //!< helper typedef for const iterators to m_clients store
public:
    typedef enum
    {
        AUTH_OK = 0,
        AUTH_DATABASE_ERROR = 1,
        AUTH_ACCOUNT_BLOCKED,
        AUTH_WRONG_LOGINPASS = 3,
        AUTH_ALREADY_LOGGEDIN = 7,
        AUTH_UNAVAILABLE_SERVER = 8,
        AUTH_KICKED_FROM_GAME = 11,
        AUTH_SELECTED_SERVER_FULL = 15,
        AUTH_CHANGE_PASSWORD = 17,
        AUTH_ACCOUNT_TIME_EXPIRED = 18,
        AUTH_NO_PAID_TIME_REMAINS = 19,
        AUTH_UNKN_ERROR
    } eAuthError; // this is a public type so other servers can pass us valid errors

                                    AuthServer();
virtual                             ~AuthServer();

        bool                        ReadConfig(const std::string &name); // later name will be used to read GameServer specific configuration
        bool                        Run(void);
        bool                        ShutDown(const std::string &reason="No particular reason");

        ServerHandle<IAdminServer>  AuthenticateMapServer(const ServerHandle<IMapServer> &map,int version,const string &passw); // World-cluster interface
        AuthClient *                GetClientByLogin(const char *);
protected:

        ClientAcceptor *            m_acceptor;     //!< ace acceptor wrapping AuthClientService
        ACE_INET_Addr               m_location;     //!< address this server will bind at.
        bool                        m_running;      //!< true if this server is running
        hmClients                   m_clients;      //!< mapping from string:login to client's object
};
