/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: MapServerInterface.h 305 2007-01-21 08:53:16Z nemerle $
 */

#pragma once
#include <ace/INET_Addr.h>
#include <string>
#include "Base.h"
class MapServer;
// this is a proxy for calling GameServer services

class MapServerInterface
{
public:
	MapServerInterface(MapServer *mi) : m_instance(mi){};
	~MapServerInterface(void){};
	u32 ExpectClient(const ACE_INET_Addr &from,u64 id,u16 access_level);
	void AssociatePlayerWithMap(u64 player_id,int map_number);

	bool ReadConfig(const std::string &name); // later name will be used to read GameServer specific configuration
	bool Run(void);
	bool ShutDown(const std::string &reason);
	bool isLocal(){return true;} // this method returns true if this interface is a local ( same process )
	bool Online();
	const ACE_INET_Addr &getAddress();

protected:
	MapServer *m_instance;
};
