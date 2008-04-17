/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: authlogin.h 253 2006-08-31 22:00:14Z malign $
 */

// Inclusion guards
#pragma once
#ifndef AUTHLOGIN_H
#define AUTHLOGIN_H

#include "AuthPacket.h"
#include <ace/Log_Msg.h>

#include <stdlib.h>

class pktAuthLogin : public AuthPacket
{
public:
	pktAuthLogin()
	{
		m_packet_type = CMSG_AUTH_LOGIN;
		m_op = 4;
		m_seed = 0xBADC0DE;
	}
	virtual ~pktAuthLogin(){}
	static void HashPassword(char *pass, u8 *hash, u32 hashLen);
	u16 ExpectedSize(){return 35;}
	u16 serializefrom(GrowingBuffer &buf);
	bool serializeto(GrowingBuffer &buf);
	

	char username[14];
	char password[14];
	u32 unkval1;
	u16 unkval2;
	u32 m_seed;
};

#endif // AUTHLOGIN_H
