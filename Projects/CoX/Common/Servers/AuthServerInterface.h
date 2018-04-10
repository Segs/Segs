/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once

#include <ace/INET_Addr.h>
#include <list>
#include "Server.h"
#include "ServerHandle.h"

class AuthClient;
class IMapServer;
class IAdminServer;
class IAuthServer : public Server
{
public:
virtual bool                        ReadConfig()=0;
virtual bool                        Run(void)=0;
virtual bool                        ShutDown(const QString &reason)=0;
virtual AuthClient *                GetClientByLogin(const char *)=0;
};

class AuthServerInterface : public Server
{
using tAdminHandle = ServerHandle<IAdminServer>;
using tMapHandle = ServerHandle<IMapServer>;
public:
                    AuthServerInterface(IAuthServer *server);
                    ~AuthServerInterface(void);

        bool        ReadConfig();
        bool        Run(void);
        bool        ShutDown(const QString &reason);

    AuthClient *GetClientByLogin(const char *);

protected:
    IAuthServer *m_server;
};
