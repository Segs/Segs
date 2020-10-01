/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
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
    void Decrypt(uint8_t * /*arr*/,size_t /*size*/){}
    void Encrypt(uint8_t * /*arr*/,size_t /*size*/){}
    void StartEncryption(uint8_t * /*other_public_key*/){}

};
