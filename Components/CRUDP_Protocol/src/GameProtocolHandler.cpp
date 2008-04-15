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

LinkCommandHandler::~LinkCommandHandler()
{
    delete m_proto;
}

bool LinkCommandHandler::ReceiveControlPacket(const ControlPacket *pak)
{
	return false;
}
