/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#include "PacketCodec.h"
#include <ace/Message_Block.h>

uint32_t PacketCodecNull::Checksum(const uint8_t *buf,size_t size)
{
    uint16_t v1 = 1, v2 = 0;
    uint32_t sum1 = v1, sum2 = v2;

    for(uint32_t i = 0; i < size; i++)
    {
        sum1 += buf[i];
        sum2 += sum1;

        if(sum1 >= 0xFFF1) sum1 -= 0xFFF1;
    }

    sum2 %= 0xFFF1;

    v1 = (uint16_t)sum1;
    v2 = (uint16_t)sum2;
    return  (((uint32_t)ACE_HTONS(v1))<<16)|ACE_HTONS(v2) ;
    //return MAKELONG(htons(v2), htons(v1));
}
