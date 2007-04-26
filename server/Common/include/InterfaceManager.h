#pragma once

#include <ace/Singleton.h>
#include <ace/Synch.h>
#include "ServerHandle.h"

class AuthServer;
class AuthServerInterface;
class AdminServer;
class AdminServerInterface;
class InterfaceManagerC
{
public:
	InterfaceManagerC(void);
	~InterfaceManagerC(void);
	// If the handle is a local one, InterfaceManager will only search in local server repository.
	// If the handle is a remote one, we check if h_serv.m_addr.isLocal() ^. If h_serv.m_id==-1 we return first matching server.
	AdminServerInterface *get(const ServerHandle<AdminServer> &h_serv) const;
	AuthServerInterface *get(const ServerHandle<AuthServer> &h_serv) const;
};
typedef ACE_Singleton<InterfaceManagerC,ACE_Thread_Mutex> InterfaceManager;