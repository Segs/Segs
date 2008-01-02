/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: GameProtocolHandler.cpp 319 2007-01-26 17:03:18Z nemerle $
 */

#include "GameProtocolHandler.h"
#include "PacketBase.h"
#include "GameProtocol.h"
#include "Client.h"
#include "opcodes/ControlCodes.h"
#include "opcodes/Opcodes.h"

LinkCommandHandler::~LinkCommandHandler()
{
    delete m_proto;
//    delete m_client;
}

bool LinkCommandHandler::ReceiveControlPacket(const ControlPacket *pak)
{
	GamePacket *res = NULL;
	if(pak->getType()!=COMM_CONTROLCOMMAND)
	{
		return false;
	}

	switch(pak->m_comm_opcode)
	{
	case CTRL_IDLE:
		{
			res = new pktIdle;
			break;
		}
	case CTRL_KEY_REQUEST:
	case CTRL_KEY_REPLY:
		{
			ACE_ASSERT(!"Why are you asking for this, eh ?");
			break;
		}
	}
	if(res)
	{
		m_proto->SendPacket(res);
		res=NULL;
		return true;
	}
	return false;
}
