/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: authselectserverresponse.h 253 2006-08-31 22:00:14Z malign $
 */

#pragma once
#include "AuthPacket.h"
#include "Base.h"

// PlayOk Packet
class pktAuthSelectServerResponse : public AuthPacket
{
public:
	pktAuthSelectServerResponse()
	{
		m_packet_type = PKT_SELECT_SERVER_RESPONSE;
		m_op = 7;
		m_unk2=0;
	}
	virtual ~pktAuthSelectServerResponse(){}
	u16 ExpectedSize()
	{
		return 10;
	}
	u16 serializefrom(GrowingBuffer &buf)
	{
		ACE_ASSERT(buf.GetReadableDataSize()>=10);
		buf.uGet(m_op);
		buf.uGet(db_server_cookie);
		buf.uGet(cookie);
		buf.uGet(m_unk2);
		return 10;
	}
	bool serializeto(GrowingBuffer &buf) 
	{
		buf.uPut(m_op);
		buf.uPut(db_server_cookie);
		buf.uPut(cookie);
		buf.uPut(m_unk2);
		return true;
	}

    u32 db_server_cookie;
	u32 cookie;
	u8 m_unk2;
};
