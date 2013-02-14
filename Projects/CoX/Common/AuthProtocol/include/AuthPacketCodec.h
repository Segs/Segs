/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once

#include "types.h"
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
    int Code(unsigned char *buffer,size_t size);
    int Decode(unsigned char *buffer,size_t size);
    void XorCodeBuf(unsigned char *buffer,size_t length);
    void XorDecodeBuf(unsigned char *buffer,size_t length);
    void DesCode(unsigned char *buffer,size_t size) const;
    void DesDecode(unsigned char *buffer,size_t size) const;
private:

    des_ctx m_tDesContext;
    long long xor_enc_key; //key for encryption
    long long xor_dec_key; //key for decryption
};
