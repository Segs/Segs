/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: authserverlist.cpp 285 2006-10-03 07:11:07Z nemerle $
 */

#include "authserverlist.h"
#include <ace/Log_Msg.h>

bool pktAuthServerList::serializeto(GrowingBuffer &buf) 
{
	buf.uPut(m_op);
	buf.uPut((u8)srvlist.size());
	buf.uPut(unknown1); //preferred number
	for(size_t i=0; i<srvlist.size(); i++)
	{
		buf.Put(srvlist[i].id);
		u32 addr= srvlist[i].address.get_ip_address();
		buf.Put(ACE_SWAP_LONG(addr)); //must be network byte order
		buf.Put((u32)srvlist[i].address.get_port_number());
		buf.Put(srvlist[i].unk1);
		buf.Put(srvlist[i].unk2);
		buf.Put(srvlist[i].numPlayers);
		buf.Put(srvlist[i].playerCapacity);
		buf.Put((u8)srvlist[i].server_online);
	}
	return buf.getLastError()==0;
}
u16 pktAuthServerList::serializefrom(GrowingBuffer &buf)
{
	ACE_ASSERT(buf.GetReadableDataSize()>3);
	buf.Get(m_op);
	buf.Get(nServers);
	buf.Get(availableServers);

	srvlist.resize(nServers);
	ACE_ASSERT(buf.GetReadableDataSize()>=static_cast<size_t>(nServers)*16);
	// ok, we're pretty sure that we have enough data in buffer, so unchecked reading here
	for(u32 i = 0; i < nServers; i++)
	{
		buf.uGet(srvlist[i].id);
		u32 addr,port;
		buf.uGet(port);
		buf.uGet(addr);
		srvlist[i].address.set(port,addr);
		buf.uGet(srvlist[i].unk1);
		buf.uGet(srvlist[i].unk2);
		buf.uGet(srvlist[i].numPlayers);
		buf.uGet(srvlist[i].playerCapacity);
		buf.uGet(srvlist[i].server_online);
#ifdef WIN32
		_snprintf(srvlist[i].serverName, sizeof(srvlist[i].serverName), "%sWorldServer_%d", "SEGS", srvlist[i].id);
#endif
	}

	return 3+nServers*16;
}
void pktAuthServerList::add_game_server(u8 id,const ACE_INET_Addr &addr,u8 unk1,u8 unk2,u16 num_players,u16 max_players, u8 is_online)
{
	serverEntry entry;
	entry.id = id;
	entry.address=addr;
	entry.unk1=unk1;
	entry.unk2=unk2;
	entry.numPlayers=num_players;
	entry.playerCapacity=max_players;
	entry.server_online=is_online;
	srvlist.push_back(entry);
}