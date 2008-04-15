/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: authrequestserverlist.h 253 2006-08-31 22:00:14Z malign $
 */

// Inclusion guards
#pragma once
#ifndef AUTHREQUESTSERVERLIST_H
#define AUTHREQUESTSERVERLIST_H

#include "AuthPacket.h"
#include "Base.h"

class pktAuthRequestServerList : public AuthPacket
{
public:
	pktAuthRequestServerList()
	{
		m_packet_type = CMSG_AUTH_REQUEST_SERVER_LIST;
		m_op = 5;
		unk1 = 1; // In the actual code, this value is a constant 1
	}
	virtual ~pktAuthRequestServerList(){}
	u16 ExpectedSize(){return 10;}
	u16 serializefrom(GrowingBuffer &buf)
	{
		ACE_ASSERT(buf.GetReadableDataSize()>=10);
		buf.uGet(m_op);
		buf.uGetBytes(unkArrayFromLoginResponse, sizeof(unkArrayFromLoginResponse));
		buf.uGet(unk1);	//	In the actual code, this value is a constant 1		
		return 10;
	}

	bool serializeto(GrowingBuffer &buf) 
	{
		buf.uPut(m_op);
		buf.uPutBytes(unkArrayFromLoginResponse, sizeof(unkArrayFromLoginResponse));
		buf.uPut(unk1);	
		return true;
	}
    u8 unkArrayFromLoginResponse[8];
	u8 unk1;
};

#endif // AUTHREQUESTSERVERLIST_H
