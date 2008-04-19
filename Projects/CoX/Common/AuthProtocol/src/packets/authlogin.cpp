/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2008 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: authserverlist.cpp 285 2006-10-03 07:11:07Z nemerle $
 */

#include <ace/Log_Msg.h>
#include "authlogin.h"


void pktAuthLogin::HashPassword( char *pass, u8 *hash, u32 hashLen )
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

u16 pktAuthLogin::serializefrom( GrowingBuffer &buf )
{
	ACE_ASSERT(buf.GetReadableDataSize()>=35);
	buf.uGet(m_op);
	buf.uGetBytes(reinterpret_cast<u8 *>(username), sizeof(username));
	buf.uGetBytes(reinterpret_cast<u8 *>(password), sizeof(password));
	buf.uGet(unkval1);
	buf.uGet(unkval2);
	return 35;
}

bool pktAuthLogin::serializeto( GrowingBuffer &buf )
{
	u8 logindata[30] = {0};
	u8 hash[16] = {0};
	char szSeed[(sizeof(u32) * 3) + 1] = {0};

	ACE_ASSERT((m_seed!=0xBADC0DE)&&"Attempted send of unitialized packet!");
	//ACE_ASSERT(false&&"find a nice lib that does MD5 !");

	buf.uPut(m_op);

	HashPassword(password, hash, sizeof(hash));
	sprintf(szSeed,"%d",m_seed); //TODO: multiplatformize this!!

	return false;
	/*


	MD5 md5;
	u8 digest[MD5::DIGESTSIZE];
	md5.Update(hash, sizeof(hash));
	md5.Update((u8 *)szSeed, strlen(szSeed));
	md5.Final(digest);

	strcpy((char *)logindata, username);
	memcpy(&logindata[14], digest, MD5::DIGESTSIZE);
	// Coding happens in the *Codec class
	*/
	buf.uPutBytes((u8 *)logindata, sizeof(logindata));
	buf.uPut(unkval1);
	buf.uPut(unkval2);
	return true;
}