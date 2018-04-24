/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2017 Super Entity Game Server Team (see Authors.txt)
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

#ifdef CUSTOM_CLIENT_CODE
#define PUTDEBUG(x) bs.StoreString(x);
#else
#define PUTDEBUG(x) ;
#endif

class QString;
class BitStream : public GrowingBuffer
{
public:

explicit        BitStream(size_t size);
                BitStream(uint8_t *from,size_t bitsize);
                BitStream(const BitStream &bs);
                BitStream &operator=(const BitStream &bs);
                ~BitStream();

        void    StoreBits(uint32_t nBits, uint32_t dataBits);
        void    uStoreBits(uint32_t nBits, uint32_t dataBits);
        void    StoreFloat(float val);
        void    StorePackedBits(uint32_t nBits, uint32_t dataBits);
        void    appendBitStream(BitStream &src)
        {
            if((src.GetReadPos()&7)==0) // source is aligned ?
                StoreBitArray(src.read_ptr(),src.GetReadableBits());
            else
            {
                size_t bits_to_store=src.GetReadableBits();
                ByteAlign(false,true);
                while(bits_to_store>32)
                {
                    StoreBits(32,src.uGetBits(32));
                    bits_to_store-=32;
                }
                StoreBits(bits_to_store,src.uGetBits(bits_to_store));
            }
        }
        void    StoreBitArray(const uint8_t *array,size_t nBits);
        void    StoreString(const char *str);
        void    StoreString(const QString &str);
        int32_t GetBits(uint32_t nBits);
        int32_t uGetBits(uint32_t nBits);
        int32_t GetPackedBits(uint32_t minbits);
        void    GetBitArray(uint8_t *array, uint32_t nBits);
        void    GetString(QString &str);
        float   GetFloat();
        int64_t Get64Bits();
        size_t  GetWritableBits()   const { int64_t bitsleft = int64_t(GetAvailSize() << 3) - m_write_bit_off; return bitsleft > 0 ? bitsleft : 0; }
        size_t  GetReadableBits()   const   { return (GetReadableDataSize()<<3)+(m_write_bit_off-m_read_bit_off);}
        size_t  GetAvailSize()      const;
        bool    IsByteAligned()     const   { return m_byteAligned;}

        void    SetReadPos(uint32_t pos)    { m_read_off  = pos >> 3; m_read_bit_off  = uint8_t(pos & 0x7);}
        size_t  GetReadPos() const          { return (m_read_off<<3)  + m_read_bit_off;}
        void    SetWritePos(uint32_t pos)   { m_write_off = pos >> 3; m_write_bit_off = uint8_t(pos & 0x7);}
        size_t  GetWritePos() const         { return (m_write_off<<3)  + m_write_bit_off;}

        void    UseByteAlignedMode(bool toggle);
        void    ByteAlign(bool read_part=true,bool write_part=true);
        void    ResetReading() { m_read_bit_off=0; m_read_off=0; }
        void    ResetOffsets();

        void CompressAndStoreString(const char *str);
        void GetAndDecompressString(QString &tgt);

private:
        bool m_byteAligned;
        uint8_t m_read_bit_off;
        uint8_t m_write_bit_off;
};
