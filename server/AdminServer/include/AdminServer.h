/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: AdminServer.h 315 2007-01-25 14:17:50Z nemerle $
 */

// Inclusion guards
#pragma once
#ifndef ADMINSERVER_H
#define ADMINSERVER_H

#include <list>
#include <string>

// ACE includes
#include <ace/OS.h>
#include <ace/INET_Addr.h>
#include <ace/Singleton.h>
#include <ace/Synch.h>

// segs includes
#include "Server.h"
#include "ServerHandle.h"

class AdminDatabase;
class GameServer;
class MapServer;
class IClient;
//! The AdminServer class handles administrative functions such as account saving, account banning, etcetera.
class AdminServer : public Server
{
public:

    // Constructor/Destructor
	AdminServer(void);
	~AdminServer(void);

    // Client handling related interface
	int                         GetBlockedIpList(std::list<int> &) // Called from auth server during user authentication, might be useful for automatical firewall rules update
	                            {return 0;}

	bool                        Logout(const IClient *client) const; // Client logout
	bool                        Login(const IClient *client,const ACE_INET_Addr &client_addr); // Records given client as logged in.
	bool                        ValidPassword(const IClient *client, const char *password); // Verifies entered password matches stored password

	void                        FillClientInfo(IClient *client);// Refresh client object from database
	int                         SaveAccount(const char *username, const char *password); // Save user account credentials to storage
	int                         RemoveAccount(IClient *client); // Removes account from database via id #

	//bool AccountBlocked(const char *login) const; // Check if account is blocked.
	int                         AddIPBan(const ACE_INET_Addr &client_addr); // Add client's IP to the banlist.
	void                        InvalidGameServerConnection(const ACE_INET_Addr &){}

	bool                        ReadConfig(const std::string &name); // later name will be used to read GameServer specific configuration
	bool                        Run(void);
	bool                        ShutDown(const std::string &reason="No particular reason");
    // Internal World-cluster interface
    ServerHandle<GameServer>    RegisterMapServer(const ServerHandle<MapServer> &map_h );
    int                         GetAccessKeyForServer(const ServerHandle<MapServer> &h_server );

protected:
	bool m_running;
	std::list<ACE_INET_Addr> m_ban_list;
	AdminDatabase *m_db;
};
typedef ACE_Singleton<AdminServer,ACE_Thread_Mutex> IAdminServer; // AdminServer Interface

#endif // ADMINSERVER_H
