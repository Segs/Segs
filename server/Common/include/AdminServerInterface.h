/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: AdminServerInterface.h 301 2006-12-26 15:50:44Z nemerle $
 */

#pragma once
#include <ace/INET_Addr.h>
#ifndef WIN32
#include <ext/hash_map>
using namespace __gnu_cxx;
#else
#include <hash_map>
#endif
#include <list>
#include "Server.h"
#include "ServerHandle.h"
/************************************************************************/
/* Design consideration:                                                */
/* Current implementation gets knowledge it shouldn't possess,          */
/* AdminServerInterface::GetClientByLogin returns all details known     */
/* about given client.													*/
/************************************************************************/

// this is interface for calling AdminServer services, currently it's an Interface pattern
// in future it can be a Remote Proxy

class IClient;

class MapServer;
class GameServer;

class AdminServerInterface : public Server
{
public:
	AdminServerInterface(void);
	~AdminServerInterface(void);

	bool ReadConfig(const std::string &name);
	bool Run(void);
	bool ShutDown(const std::string &reason);

    ServerHandle<GameServer> RegisterMapServer(const ServerHandle<MapServer> &map_h);
    int GetAccessKeyForServer(const ServerHandle<MapServer> &h_server);


	int GetBlockedIpList(std::list<int> &addreses); // called from auth server during user authentication, might be useful for automatical firewall rules update
	void FillClientInfo(IClient *); 
	bool Login(IClient  *client,const ACE_INET_Addr &client_addr); // Records given 'client' as logged in from 'addr'.
    int	 SaveAccount(const char *username, const char *password);  // Save account
	bool Logout(IClient  *client); // Records given 'client' as logged out in from 'addr'.
	bool ValidPassword(const IClient *client, const char *password); // If 'pass' is a valid password for client, return true
	void InvalidGameServerConnection(const ACE_INET_Addr &from);
	void RunCommand(const char *); //magical entry point to internal workings of all the servers ??
protected:
};
