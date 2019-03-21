/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup CRUDP_Protocol Projects/CoX/Common/CRUDP_Protocol
 * @{
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

//! @}
