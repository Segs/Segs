/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: authserverlist.cpp 285 2006-10-03 07:11:07Z nemerle $
 */

#include "authserverlist.h"
#include "ServerManager.h"
#include "GameServerInterface.h" // change this later to IGameServer 
#include <ace/Log_Msg.h>

bool pktAuthServerList::serializeto(GrowingBuffer &buf) 
{
	ServerManagerC *serv_manager = ServerManager::instance();
	GameServerInterface *gs;
	buf.uPut(m_op);
	buf.uPut((u8)serv_manager->GameServerCount());
	buf.uPut(unknown1); //preferred number
	for(size_t i=0; i<serv_manager->GameServerCount(); i++)
	{
		gs = serv_manager->GetGameServer(i);
		if(!gs)
		{
			ACE_TRACE((LM_WARNING,ACE_TEXT("Server manager getgameserver returned NULL!")));
			continue;
		}
		buf.Put(gs->getId());
		u32 addr= gs->getAddress().get_ip_address();
		buf.Put(ACE_SWAP_LONG(addr)); //must be network byte order
		buf.Put((u32)gs->getAddress().get_port_number());
		buf.Put(gs->getUnkn1());
		buf.Put(gs->getUnkn2());
		buf.Put(gs->getCurrentPlayers());
		buf.Put(gs->getMaxPlayers());
		buf.Put((u8)gs->Online());
	}
	return buf.getLastError()==0;
}
u16 pktAuthServerList::serializefrom(GrowingBuffer &buf)
{
	ACE_ASSERT(buf.GetReadableDataSize()>3);
	buf.Get(m_op);
	buf.Get(nServers);
	buf.Get(availableServers);

	srvlist = new serverEntry[nServers];
	ACE_ASSERT(buf.GetReadableDataSize()>=static_cast<size_t>(nServers)*16);
	// ok, we're pretty sure that we have enough data in buffer, so unchecked reading here
	for(u32 i = 0; i < nServers; i++)
	{
		buf.uGet(srvlist[i].id);
		u32 addr,port;
		buf.uGet(port);
		buf.uGet(addr);
		srvlist[i].address.set(port,addr);
		buf.uGet(srvlist[i].unk3);
		buf.uGet(srvlist[i].unk4);
		buf.uGet(srvlist[i].numPlayers);
		buf.uGet(srvlist[i].playerCapacity);
		buf.uGet(srvlist[i].server_online);
#ifdef WIN32
		_snprintf(srvlist[i].serverName, sizeof(srvlist[i].serverName), "%sWorldServer_%d", "SEGS", srvlist[i].id);
#endif
	}

	return 3+nServers*16;
}
