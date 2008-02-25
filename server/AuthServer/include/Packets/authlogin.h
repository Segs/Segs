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
	static void HashPassword(char *pass, u8 *hash, u32 hashLen)
	{
//		u32 len = (u32)strlen(pass);
		memset(hash, 0, hashLen);
		strcpy((char *)hash, pass);

		u8 keys[16] = {0};
		*(u32 *)(&keys[0])  = (*(u32 *)hash * 0x3407F) + 0x269735;
		*(u32 *)(&keys[4])  = (*((u32 *)hash + 1) * 0x340FF) + 0x269741;
		*(u32 *)(&keys[8])  = (*((u32 *)hash + 2) * 0x340D3) + 0x269935;
		*(u32 *)(&keys[12]) = (*((u32 *)hash + 3) * 0x3433D) + 0x269ACD;

		u8 tmp = 0;
		for(u32 i = 0; i < hashLen; i++)
		{
			tmp ^= hash[i];
			tmp ^= keys[i & 0x0F];
			hash[i] = tmp;

			if(!hash[i])
				hash[i] = 0x66;
		}
	}
	u16 ExpectedSize(){return 35;}
	u16 serializefrom(GrowingBuffer &buf)
	{
		ACE_ASSERT(buf.GetReadableDataSize()>=35);
		buf.uGet(m_op);
		buf.uGetBytes(reinterpret_cast<u8 *>(username), sizeof(username));
		buf.uGetBytes(reinterpret_cast<u8 *>(password), sizeof(password));
		buf.uGet(unkval1);
		buf.uGet(unkval2);
		return 35;
	}
	bool serializeto(GrowingBuffer &buf) 
	{
		ACE_ASSERT((m_seed!=0xBADC0DE)&&"Attempted send of unitialized packet!");
                ACE_ASSERT(false&&"find a nice lib that does MD5 !");
                return false;/*
		buf.uPut(m_op);

		u8 hash[16] = {0};
    		HashPassword(password, hash, sizeof(hash));

		char szSeed[(sizeof(u32) * 3) + 1] = {0};
		sprintf(szSeed,"%d",m_seed); //TODO: multiplatformize this!!
		MD5 md5;
		u8 digest[MD5::DIGESTSIZE];
		md5.Update(hash, sizeof(hash));
		md5.Update((u8 *)szSeed, strlen(szSeed));
		md5.Final(digest);

		u8 logindata[30] = {0};
		strcpy((char *)logindata, username);
		memcpy(&logindata[14], digest, MD5::DIGESTSIZE);
        // Coding happens in the *Codec class
		buf.uPutBytes((char *)logindata, sizeof(logindata));
		buf.uPut(unkval1);
		buf.uPut(unkval2);
                return true;*/
	}
	

	char username[14];
	char password[14];
	u32 unkval1;
	u16 unkval2;
	u32 m_seed;
};

#endif // AUTHLOGIN_H
