/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

/************************************************************************
Class:	     Net
Author:		 Darawk
Description: The Net class abstracts normal network operations away from
the user.
************************************************************************/
#pragma once
#include <deque>
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
#include "Base.h"

class PacketCodecNull 
{
public:
	static u32  Checksum(const u8 *buf,size_t size);
	void Decrypt(u8 * /*arr*/,size_t /*size*/){};
	void Encrypt(u8 * /*arr*/,size_t /*size*/){};
	void StartEncryption(u8 * /*other_public_key*/){};

};
