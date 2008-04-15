/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: MapServerEndpoint.cpp 253 2006-08-31 22:00:14Z malign $
 */

//#include <ace/SOCK_Stream.h>
#include "ServerManager.h"
#include <ace/Message_Block.h>
#include "MapServerEndpoint.h"
#include "AdminServerInterface.h"
#include "MapPacket.h"
static MapPacketFactory m_fact;
PacketFactory *MapServerEndpoint::getFactory() const
{
	return &m_fact;
}
