/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: MapServer.h 305 2007-01-21 08:53:16Z nemerle $
 */

// Inclusion guards
#pragma once
#ifndef MAPSERVER_H
#define MAPSERVER_H

#include <string>
#include <map>
#ifndef WIN32
#include <ext/hash_map>
#include <ext/hash_set>
using namespace __gnu_cxx;
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
#include "MapServerInterface.h"

#define MAPSERVER_VERSION 1

class Net;
class MapServerEndpoint;
class MapClient;
class SEGSMap;
class MapServer : public IMapServer
{
public:
	MapServer(void);
	virtual ~MapServer(void);

	virtual bool	Run(void);
	virtual bool	ReadConfig(const std::string &name);
	int		getMaxHandledMaps();

	bool ShutDown(const std::string &reason="No particular reason");
	void Online(bool s ) {m_online=s;}
	bool Online(void) { return m_online;}
	const ACE_INET_Addr &getAddress() {return m_location;};
	u32 ExpectClient(const ACE_INET_Addr &from,u64 id,u16 access_level);
	void AssociatePlayerWithMap(u64 player_id,int map_number); //! this method tells this server that packets of this player should go to given Map 
	// not in the interface
	MapClient * ClientExpected(ACE_INET_Addr &from,u32 cookie);
	GameServerInterface *getGameInterface(){return m_i_game;}
private:
	SEGSMap *getMapByNum(int num);
	bool startup(); // MapServerStartup sequence diagram entry point.
protected:
	u8 m_id;
	bool m_online;

	size_t m_max_maps;
	GameServerInterface *m_i_game;// GameServer access proxy object

	std::string m_serverName;

	ClientStore<MapClient> m_clients;
	std::map<int,SEGSMap *> m_handled_worlds;//! the worlds run by this server
	ACE_INET_Addr m_location; //! this value is sent to the clients
	ACE_INET_Addr m_listen_point; //! this is used as a listening endpoint
	MapServerEndpoint	*m_endpoint;
};

#endif // MAPSERVER_H
