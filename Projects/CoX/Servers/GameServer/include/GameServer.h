/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: GameServer.h 319 2007-01-26 17:03:18Z nemerle $
 */

// Inclusion guards
#pragma once
#ifndef GAMESERVER_H
#define GAMESERVER_H

#include <string>
#ifndef WIN32
#include <ext/hash_map>
#include <ext/hash_set>
// Hashing function for int64 is in ClientManager
#else // WIN32
#include <hash_map>
#include <hash_set>
using namespace stdext;
#endif // WIN32

#include <ace/ACE.h>
#include <ace/Synch.h>
#include <ace/INET_Addr.h>
#include <ace/SOCK_Dgram.h>
#include <ace/Message_Queue.h>
#include <ace/Message_Block.h>
#include <ace/Event_Handler.h>
#include <ace/Svc_Handler.h>
#include <ace/Reactor_Notification_Strategy.h>

#include "Base.h"
#include "CRUDP_Protocol.h"
#include "RoamingServer.h"
#include "ServerEndpoint.h"
#include "GameEvents.h"

class GameHandler;
class pktCS_ServerUpdate;
class CharacterDatabase;
typedef list< ServerHandle<MapServer> > lMapServerHandles;
class GameServer : public IGameServer
{
public:
	                        ~GameServer(void);
	                        GameServer(void);
	bool					ReadConfig(const std::string &configpath); // later name will be used to read GameServer specific configuration
	bool					Run(void);
	bool					ShutDown(const std::string &reason="No particular reason");
	void					Online(bool s ) {m_online=s;}
	bool					Online(void) { return m_online;}
	const ACE_INET_Addr &	getAddress() {return m_location;};
	
	// World-cluster management interface
	int						getAccessKeyForServer(const ServerHandle<IMapServer> &h_map);
	bool					isMapServerReady(const ServerHandle<IMapServer> &h_map);

	ServerHandle<IMapServer> GetMapHandle(const std::string &mapname);


	std::string				getName(void);
	u8						getId(void);
	u16						getCurrentPlayers(void);
	u16						getMaxPlayers();
	u8						getUnkn1(void);
	u8						getUnkn2(void);
	CharacterDatabase *		getDb();

	int						createLinkedAccount(u64 auth_account_id,const std::string &username); // Part of exposed db interface.

    EventProcessor *        event_target() {return (EventProcessor *)m_handler;}
protected:
	u32                     GetClientCookie(const ACE_INET_Addr &client_addr); // returns a cookie that will identify user to the gameserver
	lMapServerHandles       GetMapsHandling(const std::string &mapname);
	bool m_online;
	u8                      m_id;
	u16                     m_current_players;
	u16                     m_max_players;
	u8                      m_unk1,m_unk2;
	std::string             m_serverName;
	ACE_INET_Addr           m_location; // this value is sent to the clients
	ACE_INET_Addr           m_listen_point; // this is used as a listening endpoint
	ServerEndpoint<GameLink>	*m_endpoint;
	CharacterDatabase   *   m_database; // we might consider making database connection a singleton wrapper
    GameHandler *           m_handler;
};

#endif // GAMESERVER_H
