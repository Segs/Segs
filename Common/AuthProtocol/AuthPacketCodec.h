/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include <stdint.h>
#include <stdlib.h>

typedef struct
{
    unsigned long ek[32];
    unsigned long dk[32];
} des_ctx;
typedef enum
{
    EN0=0,
    DE1=1
} eDesCodeDecode;

class AuthPacketCodec
{
public:
    AuthPacketCodec();
    ~AuthPacketCodec(){}
    void SetXorKey(int key);
    void SetDesKey(uint64_t key);
    void Code(unsigned char *buffer,size_t size);
    void Decode(unsigned char *buffer,size_t size);
    void XorCodeBuf(unsigned char *buffer,size_t length);
    void XorDecodeBuf(unsigned char *buffer,size_t length);
    void DesCode(unsigned char *buffer,size_t size) const;
    void DesDecode(unsigned char *buffer,size_t size) const;
private:

    des_ctx m_tDesContext;
    long long xor_enc_key; //key for encryption
    long long xor_dec_key; //key for decryption
};
