/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: GameServerEndpoint.cpp 253 2006-08-31 22:00:14Z malign $
 */

//#include <ace/SOCK_Stream.h>
#include "ServerManager.h"
#include <ace/Message_Block.h>
#include "GameServerEndpoint.h"
#include "GamePacket.h"
static GamePacketFactory m_fact;
PacketFactory *GameServerEndpoint::getFactory() const
{
	return &m_fact;
}

void GameServerEndpoint::HeartBeat()
{

}

int GameServerEndpoint::handle_timeout( const ACE_Time_Value & tv,const void *arg )
{
	return ServerEndpoint::handle_timeout(tv,arg);
}