/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: authserverlist.h 285 2006-10-03 07:11:07Z nemerle $
 */

// Inclusion guards
#pragma once
#ifndef AUTHSERVERLIST_H
#define AUTHSERVERLIST_H

#include <string>
#include "Base.h"
#include "AuthPacket.h"
#include <ace/INET_Addr.h>
#include <vector>
class IGameServer;
typedef struct _serverEntry
{
	u8 id;							//	0x00
	u8 pad1, pad2, pad3;
	ACE_INET_Addr  address;			//	0x04
	u8 unk1;						//	0x0C
	u8 unk2;						//	0x0D
	u8 server_online;				//	0x0E
	u8 pad4;
	u16 numPlayers;					//	0x10
	u16 pad6;				
	u16 playerCapacity;				//	0x14
	char serverName[256];
} serverEntry, *pServerEntry;

class pktAuthServerList : public AuthPacket
{
public:
	virtual ~pktAuthServerList(){}
	pktAuthServerList()
	{
		m_packet_type = SMSG_AUTH_SERVER_LIST;
		m_op = 4;
		unknown1 = 1;
	}
	u16 ExpectedSize() {return 0xFFFF;}
	bool serializeto(GrowingBuffer &buf); // buf size must be 3+16*MaximumNumberOfServers
	u16 serializefrom(GrowingBuffer &buf);

	void add_game_server(u8 id,const ACE_INET_Addr &addr,u8 unk1,u8 unk2,u16 num_players,u16 max_players, u8 is_online);

	u8 nServers, availableServers;
	u8 unknown1;
	std::vector<serverEntry> srvlist;
};

#endif // AUTHSERVERLIST_H
