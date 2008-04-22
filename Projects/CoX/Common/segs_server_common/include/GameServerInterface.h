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
#include "RoamingServer.h"
#include "ServerHandle.h"
class IMapServer;
// this is a proxy for calling GameServer services
class IGameServer : public RoamingServer
{
public:
virtual u32						ExpectClient(const ACE_INET_Addr &from,u64 id,u16 access_level)=0;
virtual const ACE_INET_Addr &	getAddress()=0;
virtual std::string				getName(void)=0;
virtual u8						getId(void)=0;
virtual u16						getCurrentPlayers(void)=0;
virtual u16						getMaxPlayers()=0;
virtual u8						getUnkn1(void)=0;
virtual u8						getUnkn2(void)=0;
virtual void					checkClientConnection(u64 id)=0;
virtual bool					isClientConnected(u64 id)=0;
virtual int						getAccessKeyForServer(const ServerHandle<IMapServer> &h_map)=0;
virtual bool					isMapServerReady(const ServerHandle<IMapServer> &h_map)=0;
virtual int						createLinkedAccount(u64 auth_account_id,const std::string &username)=0;

};

class GameServerInterface : public IGameServer
{
public:
	GameServerInterface(IGameServer *mi) : m_instance(mi){};
	~GameServerInterface(void){};
	//u32 GetClientCookie(const ACE_INET_Addr &client_addr);
	bool					ReadConfig(const std::string &name); //! later name will be used to read GameServer specific configuration
	bool					Run(void);
	bool					ShutDown(const std::string &reason);

	int						getAccessKeyForServer(const ServerHandle<IMapServer> &h_map);
	bool					isMapServerReady(const ServerHandle<IMapServer> &h_map);

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
	int						createLinkedAccount(u64 auth_account_id,const std::string &username);
	//bool					getCharacterData(u64 id,
protected:
	IGameServer *m_instance;
};

#endif // GAMESERVERINTERFACE_H
