/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#include "PacketCodec.h"

#include <QtCore/QtEndian>
uint32_t PacketCodecNull::Checksum(const uint8_t *buf,size_t size)
{
    uint16_t v1 = 1, v2 = 0;
    uint32_t sum1 = 1, sum2 = 0;

    for(uint32_t i = 0; i < size; i++)
    {
        sum1 += buf[i];
        sum2 += sum1;

        if(sum1 >= 0xFFF1)
            sum1 -= 0xFFF1;
    }

    sum2 %= 0xFFF1;

    v1 = (uint16_t)sum1;
    v2 = (uint16_t)sum2;
    return  (((uint32_t)qToBigEndian(v1))<<16)|qToBigEndian (v2) ;
    //return MAKELONG(htons(v2), htons(v1));
}
