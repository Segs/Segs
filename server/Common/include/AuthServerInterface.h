/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: AuthServerInterface.h 301 2006-12-26 15:50:44Z nemerle $
 */

// Inclusion guards
#pragma once
#ifndef AUTHSERVERINTERFACE_H
#define AUTHSERVERINTERFACE_H

#include <ace/INET_Addr.h>
#include <list>
#include "Server.h"
#include "ServerHandle.h"
class AuthClient;
class AuthServer;

class MapServer;
class AdminServer;

class AuthServerInterface : public Server
{
public:
	AuthServerInterface(void);
	~AuthServerInterface(void);

	bool ReadConfig(const std::string &name);
	bool Run(void);
	bool ShutDown(const std::string &reason);

//	void SendError(int reason);
	AuthClient *GetClientByLogin(const char *);

    ServerHandle<AdminServer> AuthenticateMapServer(const ServerHandle<MapServer> &map,int version,const std::string &passw);

protected:
	AuthServer *m_server;
};

#endif // AUTHSERVERINTERFACE_H
