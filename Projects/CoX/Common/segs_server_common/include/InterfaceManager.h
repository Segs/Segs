/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

// Inclusion guards
#pragma once
#ifndef INTERFACEMANAGER_H
#define INTERFACEMANAGER_H

#include <ace/Singleton.h>
#include <ace/Synch.h>
#include "ServerHandle.h"

class IAuthServer;
class AuthServerInterface;
class IAdminServer;
class AdminServerInterface;
class InterfaceManagerC
{
public:
	InterfaceManagerC(void);
	~InterfaceManagerC(void);
	// If the handle is a local one, InterfaceManager will only search in local server repository.
	// If the handle is a remote one, we check if h_serv.m_addr.isLocal() ^. If h_serv.m_id==-1 we return first matching server.
	AdminServerInterface *get(const ServerHandle<IAdminServer> &h_serv) const;
	AuthServerInterface *get(const ServerHandle<IAuthServer> &h_serv) const;
};
typedef ACE_Singleton<InterfaceManagerC,ACE_Thread_Mutex> InterfaceManager;

#endif // INTERFACEMANAGER_H
