/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

/************************************************************************
Class:		 BitStream
Author:		 Darawk,nemerle
Description: The BitStream class allows it's user to manipulate data in
                         terms of individual bits, thus allowing said data to achieve
                         much greater levels of density.  It abstracts the nuts and
                         bolts of this functionality away from it's user.
************************************************************************/
#pragma once
#include <stdint.h>
#include "Base.h"
#include "Buffer.h"

//	Constants
#define BS_BITS        3
#define BS_PACKEDBITS  4
#define BS_BITARRAY    5
#define BS_STRING      6
#define BS_F32         7

#define BITS_PER_BYTE  8
#define BITS_PER_DWORD (BITS_PER_BYTE * sizeof(u32))


//	Macros
#define BIT_MASK(x)		 ((u32)((1 << (x)) - 1))
#define BYTES_TO_BITS(x) ((x) << 3)
// x>>3 --->  x/8
#define BITS_TO_BYTES(x) (((x) + 7) >> 3)
#define BITS_LEFT(x)	 (BITS_PER_BYTE - (x))
#define BYTE_ALIGN(x)	 (((x) + 7) & ~7)


class BitStream : public GrowingBuffer
{
public:

        BitStream(size_t size);
        BitStream(u8 *from,size_t bitsize);

        BitStream(const BitStream &bs);
        BitStream &operator=(const BitStream &bs);

        ~BitStream();

        void StoreBits(u32 nBits, u32 dataBits);
        void StoreBits_4_10_24_32(u32 dataBits)
        {
                //TODO: always full, fix this later
                StoreBits(2,3);
                StoreBits(32,dataBits);
        }
        void uStoreBits(u32 nBits, u32 dataBits);
        void StoreBitsWithDebugInfo(u32 nBits, u32 dataBits);

        void StoreFloat(f32 val);
        void StoreFloatWithDebugInfo(f32 val);

        void StorePackedBits(u32 nBits, u32 dataBits);
        void StorePackedBitsWithDebugInfo(u32 nBits, u32 dataBits);

        void StoreBitArray(const u8 *array,size_t nBits);
        void StoreBitArrayWithDebugInfo(const u8 *array,u32 nBits);

        void StoreString(const char *str);
        void StoreString(const std::string &str){StoreString(str.c_str());}
        void StoreStringWithDebugInfo(const char *str);

        int32_t GetBits(u32 nBits);
        int32_t GetBits_2_10_24_32()
        {
                static const int numbits[]={2,10,24,32};
                int type=GetBits(2);
                return GetBits(numbits[type]);
        }
        int32_t uGetBits(u32 nBits);
        int32_t GetBitsWithDebugInfo(u32 nBits);

        int32_t GetPackedBits(u32 minbits);
        int32_t GetPackedBitsWithDebugInfo(u32 minbits);

        void GetBitArray(u8 *array,size_t nBits);
        void GetBitArrayWithDebugInfo(u8 *array,size_t nBits);

        void GetString(std::string &str);
        void GetStringWithDebugInfo(std::string &str);

        f32 GetFloat();
        f32 GetFloatWithDebugInfo();
        s64 Get64Bits();

        size_t GetWritableBits()    const { return (GetAvailSize()<<3)-m_write_bit_off;}
        size_t GetReadableBits()    const { return (GetReadableDataSize()<<3)+(m_write_bit_off-m_read_bit_off);}
        size_t	  GetAvailSize()    const
        {
                ssize_t res = (ssize_t)((m_size-m_write_off)-(m_write_bit_off!=0));
                return (size_t)std::max<>((ssize_t)0,res);
        }

        bool IsByteAligned()            const { return m_byteAligned;}

        void SetReadPos(u32 pos)    {   m_read_off  = pos >> 3; m_read_bit_off  = (u8)(pos & 0x7);}
        size_t GetReadPos()         {   return (m_read_off<<3)  + m_read_bit_off;}
        void SetWritePos(u32 pos)   {   m_write_off = pos >> 3; m_write_bit_off = (u8)(pos & 0x7);}

        void SetByteLength(u32 length);
        void UseByteAlignedMode(bool toggle);
        void ByteAlign(bool read_part=true,bool write_part=true);
        void Reset();

        u32 GetPackedBitsLength(u32 nBits, u32 dataBits) const;
        u32 GetBitsLength(u32 nBits, u32 dataBits)       const;
        void CompressAndStoreString(const char *str);
        void GetAndDecompressString(std::string &tgt);

private:
        bool m_byteAligned;
        u8 m_read_bit_off;
        u8 m_write_bit_off;
};
