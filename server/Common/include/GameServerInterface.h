/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: GameServerInterface.h 319 2007-01-26 17:03:18Z nemerle $
 */

// Inclusion guards
#pragma once
#ifndef GAMESERVERINTERFACE_H
#define GAMESERVERINTERFACE_H

#include <ace/INET_Addr.h>
#include <string>
#include "Base.h"
#include "Server.h"
#include "ServerHandle.h"
class GameServer;
class MapServer;
// this is a proxy for calling GameServer services

class GameServerInterface : public Server
{
public:
	GameServerInterface(GameServer *mi) : m_instance(mi){};
	~GameServerInterface(void){};
	//u32 GetClientCookie(const ACE_INET_Addr &client_addr);
	bool					ReadConfig(const std::string &name); //! later name will be used to read GameServer specific configuration
	bool					Run(void);
	bool					ShutDown(const std::string &reason);

	int						GetAccessKeyForServer(const ServerHandle<MapServer> &h_map);
	bool					MapServerReady(const ServerHandle<MapServer> &h_map);

	bool					isLocal(){return true;} //! this method returns true if this interface is a local ( same process )
	bool					Online();
	const ACE_INET_Addr &	getAddress();

	u8						getId();
	u16						getCurrentPlayers();
	u16						getMaxPlayers();
	u8						getUnkn1();
	u8						getUnkn2();
	std::string				getName();

	//////////////////////////////////////////////////////////////////////////
	// This method is used by auth server to tell us that a new authorized client will show up
	// We return an identification cookie, that will be used to verify the validity of given connection 
	//////////////////////////////////////////////////////////////////////////
	u32						ExpectClient(const ACE_INET_Addr &from,u64 id,u16 access_level);
	void					checkClientConnection(u64 id);
	bool					isClientConnected(u64 id);
	//bool					getCharacterData(u64 id,
protected:
	GameServer *m_instance;
};

#endif // GAMESERVERINTERFACE_H
