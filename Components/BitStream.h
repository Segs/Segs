/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

/************************************************************************
Class:       BitStream
Author:      Darawk,nemerle
Description: The BitStream class allows it's user to manipulate data in
                         terms of individual bits, thus allowing said data to achieve
                         much greater levels of density.  It abstracts the nuts and
                         bolts of this functionality away from it's user.
************************************************************************/

#pragma once

#include "Buffer.h"

#include <cstdint>
#include <string>
#include <vector>
class QString;
//  Constants
#define BS_BITS        3
#define BS_PACKEDBITS  4
#define BS_BITARRAY    5
#define BS_STRING      6
#define BS_F32         7

#define BITS_PER_BYTE  8
#define BITS_PER_DWORD (BITS_PER_BYTE * uint32_t(sizeof(uint32_t)))


//  Macros
#define BIT_MASK(x)      ((uint32_t)((1 << (x)) - 1))
#define BYTES_TO_BITS(x) ((x) << 3)
// x>>3 --->  x/8
#define BITS_TO_BYTES(x) (((x) + 7) >> 3)
#define BITS_LEFT(x)     (BITS_PER_BYTE - (x))
#define BYTE_ALIGN(x)    (((x) + 7) & ~7)


class BitStream : public GrowingBuffer
{
public:

        explicit BitStream(size_t size);
        BitStream(uint8_t *from,size_t bitsize);

        BitStream(const BitStream &bs);
        BitStream &operator=(const BitStream &bs);

        ~BitStream();

        void StoreBits(uint32_t nBits, uint32_t dataBits);
        void StoreBits_4_10_24_32(uint32_t dataBits)
        {
                //TODO: always full, fix this later
                StoreBits(2,3);
                StoreBits(32,dataBits);
        }
        void uStoreBits(uint32_t nBits, uint32_t dataBits);
        void StoreBitsWithDebugInfo(uint32_t nBits, uint32_t dataBits);

        void StoreFloat(float val);
        void StoreFloatWithDebugInfo(float val);

        void StorePackedBits(uint32_t nBits, uint32_t dataBits);
        void StorePackedBitsWithDebugInfo(uint32_t nBits, uint32_t dataBits);

        void StoreBitArray(const uint8_t *array,size_t nBits);
        void StoreBitArray_Unaligned(const uint8_t *src, size_t nBits);
        void StoreBitArrayWithDebugInfo(const uint8_t *array,uint32_t nBits);

        void StoreString(const char *str);
        void StoreString(const QString &str);
        void StoreStringWithDebugInfo(const char *str);

        int32_t GetBits(uint32_t nBits);
        int32_t GetBits_2_10_24_32()
        {
                static const int numbits[]={2,10,24,32};
                int type=GetBits(2);
                return GetBits(numbits[type]);
        }
        int32_t uGetBits(uint32_t nBits);
        int32_t GetBitsWithDebugInfo(uint32_t nBits);

        int32_t GetPackedBits(uint32_t minbits);
        int32_t GetPackedBitsWithDebugInfo(uint32_t minbits);

        void GetBitArray(uint8_t *array,size_t nBits);
        void GetBitArrayWithDebugInfo(uint8_t *array,size_t nBits);

        void GetString(QString &str);
        void GetStringWithDebugInfo(QString &str);

        float   GetFloat();
        float   GetFloatWithDebugInfo();
        int64_t Get64Bits();

        size_t  GetWritableBits()   const   { return (GetAvailSize()<<3)-m_write_bit_off;}
        size_t  GetReadableBits()   const   { return (GetReadableDataSize()<<3)+(m_write_bit_off-m_read_bit_off);}
        size_t  GetAvailSize()      const;
        bool    IsByteAligned()     const   { return m_byteAligned;}

        void    SetReadPos(uint32_t pos)    { m_read_off  = pos >> 3; m_read_bit_off  = (uint8_t)(pos & 0x7);}
        size_t  GetReadPos()                { return (m_read_off<<3)  + m_read_bit_off;}
        void    SetWritePos(uint32_t pos)   { m_write_off = pos >> 3; m_write_bit_off = (uint8_t)(pos & 0x7);}

        void    SetByteLength(uint32_t length);
        void    UseByteAlignedMode(bool toggle);
        void    ByteAlign(bool read_part=true,bool write_part=true);
        void    Reset();

        uint32_t GetPackedBitsLength(uint32_t nBits, uint32_t dataBits) const;
        uint32_t GetBitsLength(uint32_t nBits, uint32_t dataBits)       const;
        void CompressAndStoreString(const char *str);
        void GetAndDecompressString(std::string &tgt);

private:
        bool m_byteAligned;
        uint8_t m_read_bit_off;
        uint8_t m_write_bit_off;
};
