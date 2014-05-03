/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

/************************************************************************
Class:       Net
Author:      Darawk
Description: The Net class abstracts normal network operations away from
the user.
************************************************************************/
#pragma once
#include <deque>
#include <cstddef>
#include <stdint.h>
#ifdef USE_REAL_CODING
#include <crypto++/cryptlib.h>
#include <crypto++/modes.h>
#include <crypto++/des.h>
#include <crypto++/md5.h>
#include <crypto++/rng.h>
#include <crypto++/osrng.h>
#include <crypto++/dh.h>
extern "C"
{
#include "blowfish.h"
}
using namespace CryptoPP;
#endif

class PacketCodecNull
{
public:
    static uint32_t  Checksum(const uint8_t *buf,size_t size);
    void Decrypt(uint8_t * /*arr*/,size_t /*size*/){};
    void Encrypt(uint8_t * /*arr*/,size_t /*size*/){};
    void StartEncryption(uint8_t * /*other_public_key*/){}

};
