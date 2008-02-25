/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: authloginresponse.h 253 2006-08-31 22:00:14Z malign $
 */

// Inclusion guards
#pragma once
#ifndef AUTHLOGINRESPONSE_H
#define AUTHLOGINRESPONSE_H

#include "AuthPacket.h"
#include "Base.h"

class pktAuthLoginResponse : public AuthPacket
{
public:
	pktAuthLoginResponse()
	{
		m_packet_type=PKT_AUTH_LOGIN_SUCCESS;
		m_op = 3;
    		memset(unkArray1,0,8);
    		memset(unkArray2,0,8);
    		unk1=unk2=unk3=1;
	}
	virtual ~pktAuthLoginResponse(){}
	u16 ExpectedSize(){return 29;}
	u16 serializefrom(GrowingBuffer &buf)
	{
		ACE_ASSERT(buf.GetReadableDataSize()>=29);
		buf.uGet(m_op);
		buf.uGetBytes(unkArray1, sizeof(unkArray1));
		buf.uGetBytes(unkArray2, sizeof(unkArray2));
		buf.uGet(unk1);
		buf.uGet(unk2);
		buf.uGet(unk3);
		return 29;
	}
	bool serializeto(GrowingBuffer &buf) 
	{
		buf.uPut(m_op);
		buf.uPutBytes(unkArray1, sizeof(unkArray1));
		buf.uPutBytes(unkArray2, sizeof(unkArray2));
		buf.uPut(unk1);
		buf.uPut(unk2);
		buf.uPut(unk3);
		return true;
	}

	u8 unkArray1[8];
	u8 unkArray2[8];
	u32 unk1, unk2, unk3;
};

#endif // AUTHLOGINRESPONSE_H
