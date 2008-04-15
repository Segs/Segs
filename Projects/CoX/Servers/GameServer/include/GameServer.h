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
#include "ClientManager.h"
#include "RoamingServer.h"
//#include "MapServer.h"
class Net;
class GameServerEndpoint;
class CharacterClient;
class pktCS_ServerUpdate;
class CharacterDatabase;
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
	int						GetAccessKeyForServer(const ServerHandle<IMapServer> &h_map);
	bool					MapServerReady(const ServerHandle<IMapServer> &h_map);

	ServerHandle<IMapServer> GetMapHandle(const std::string &mapname);


	std::string				getName(void);;
	u8						getId(void);
	u16						getCurrentPlayers(void);
	u16						getMaxPlayers();
	u8						getUnkn1(void);
	u8						getUnkn2(void);
	CharacterDatabase *		getDb();
	u32						ExpectClient(const ACE_INET_Addr &from,u64 id,u16 access_level);
	void					checkClientConnection(u64 id);
	CharacterClient *		ClientExpected(ACE_INET_Addr &from,pktCS_ServerUpdate *pak);
	bool					isClientConnected(u64 id); //! This function will check if given client is available in clientstore, and it's packet backlog.

protected:
	u32 GetClientCookie(const ACE_INET_Addr &client_addr); // returns a cookie that will identify user to the gameserver
	list< ServerHandle<MapServer> > GetMapsHandling(const std::string &mapname);
	bool m_online;
	hash_set<u32> waiting_for_client; // this hash_set holds all client cookies we wait for
	ClientStore<CharacterClient> m_clients;
	u8 m_id;
	u16 m_current_players;
	u16 m_max_players;
	u8 m_unk1,m_unk2;
	std::string m_serverName;
	ACE_INET_Addr m_location; // this value is sent to the clients
	ACE_INET_Addr m_listen_point; // this is used as a listening endpoint
	GameServerEndpoint	*m_endpoint;
	CharacterDatabase   *m_database; // we might consider making database connection a singleton wrapper
};

#endif // GAMESERVER_H
