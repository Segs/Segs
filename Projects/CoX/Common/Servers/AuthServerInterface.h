/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once

#include <ace/INET_Addr.h>
#include <list>
#include "RoamingServer.h"
#include "ServerHandle.h"

class AuthClient;
class IMapServer;
class IAdminServer;
class IAuthServer : public Server
{
public:
virtual bool                        ReadConfig(const std::string &name)=0;
virtual bool                        Run(void)=0;
virtual bool                        ShutDown(const std::string &reason)=0;
virtual AuthClient *                GetClientByLogin(const char *)=0;
virtual ServerHandle<IAdminServer>  AuthenticateMapServer(const ServerHandle<IMapServer> &map,int version,const std::string &passw)=0;
};

class AuthServerInterface : public Server
{
typedef ServerHandle<IAdminServer> tAdminHandle;
typedef ServerHandle<IMapServer> tMapHandle;
public:
                    AuthServerInterface(IAuthServer *server);
                    ~AuthServerInterface(void);

        bool        ReadConfig(const std::string &name);
        bool        Run(void);
        bool        ShutDown(const std::string &reason);

    AuthClient *GetClientByLogin(const char *);

        tAdminHandle AuthenticateMapServer(const tMapHandle &map,int version,const std::string &passw);

protected:
    IAuthServer *m_server;
};
