/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

#include "PacketCodec.h"
u32 PacketCodecNull::Checksum(const u8 *buf,size_t size)
{
	u16 v1 = 1, v2 = 0;
	u32 sum1 = v1, sum2 = v2;

	for(u32 i = 0; i < size; i++)
	{
		sum1 += buf[i];
		sum2 += sum1;

		if(sum1 >= 0xFFF1) sum1 -= 0xFFF1;
	}

	sum2 %= 0xFFF1;

	v1 = (u16)sum1;
	v2 = (u16)sum2;
	return  (((u32)ACE_HTONS(v1))<<16)|ACE_HTONS(v2) ;
	//return MAKELONG(htons(v2), htons(v1));
}
