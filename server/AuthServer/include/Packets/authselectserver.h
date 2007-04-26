/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: authselectserver.h 253 2006-08-31 22:00:14Z malign $
 */

#pragma once
#include "AuthPacket.h"
#include "Base.h"

class pktAuthSelectServer : public AuthPacket
{
public:
	pktAuthSelectServer()
	{
		m_packet_type = CMSG_AUTH_SELECT_DBSERVER;
		m_op = 2;
	}
	virtual ~pktAuthSelectServer(){}
	u16 ExpectedSize(){return 10;}
	u16 serializefrom(GrowingBuffer &buf)
	{
		ACE_ASSERT(buf.GetReadableDataSize()>=10);
		buf.uGet(m_op);
		buf.uGetBytes(unkLoginArray, sizeof(unkLoginArray));
		buf.uGet(serverId);
		return 10;
	}
	bool serializeto(GrowingBuffer &buf) 
	{
		buf.uPut(m_op);
		buf.uPutBytes(unkLoginArray, sizeof(unkLoginArray));
		buf.uPut(serverId);
		return true;
	}

	u8 unkLoginArray[8];
	u8 serverId;
};
