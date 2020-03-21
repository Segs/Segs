/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
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

#include <cereal/macros.hpp>

#include "Buffer.h"

#include <cstdint>
#include <utility>
//#define CUSTOM_CLIENT_CODE
#ifdef CUSTOM_CLIENT_CODE
#define PUTDEBUG(x) bs.StoreString(x)
#else
#define PUTDEBUG(x)
#endif

class QString;
class QByteArray;
class BitStream : public GrowingBuffer
{
    template <class Archive>
    friend void CEREAL_SAVE_FUNCTION_NAME( Archive & ar, const BitStream &buf );
    template <class Archive>
    friend void CEREAL_LOAD_FUNCTION_NAME( Archive & ar, BitStream &buf );

public:

explicit        BitStream(size_t size);
                BitStream(uint8_t *from,size_t bitsize);
                BitStream(const BitStream &bs);
                BitStream(BitStream &&bs) noexcept : GrowingBuffer(std::move(bs)) {
                    m_read_bit_off = bs.m_read_bit_off;
                    m_write_bit_off = bs.m_write_bit_off;
                    m_byteAligned = bs.m_byteAligned;
                }
                BitStream &operator=(const BitStream &bs);
                ~BitStream();

        void    StoreBits(uint32_t nBits, uint32_t dataBits);
        void    uStoreBits(uint32_t nBits, uint32_t dataBits);
        void    StoreFloat(float val);
        void    StorePackedBits(uint32_t nBits, uint32_t dataBits);
        void    appendBitStream(BitStream &src)
        {
            //TODO: optimize this to partial memcopy in special cases ?
            uint32_t bits_to_store =src.GetReadableBits();
            while(bits_to_store>32)
            {
                StoreBits(32,src.uGetBits(32));
                bits_to_store-=32;
            }
            StoreBits(bits_to_store,src.uGetBits(bits_to_store));
        }
        void    StoreBitArray(const uint8_t *array,size_t nBits);
        void    StoreString(const char *str);
        void    StoreString(const QByteArray &str);
        void    StoreString(const QString &str);
        int32_t GetBits(uint32_t nBits);
        int32_t uGetBits(uint32_t nBits);
        int32_t GetPackedBits(uint32_t minbits);
        void    GetBitArray(uint8_t *array, uint32_t nBits);
        void    GetString(QString &str);
        float   GetFloat();
        int64_t Get64Bits();
        uint32_t GetWritableBits()   const   { int64_t bitsleft = int64_t(GetAvailSize() << 3) - m_write_bit_off; return uint32_t(bitsleft > 0 ? bitsleft : 0); }
        uint32_t GetReadableBits() const   { return (GetReadableDataSize()<<3)+(m_write_bit_off-m_read_bit_off);}
        uint32_t GetAvailSize()      const;
        bool    IsByteAligned()     const   { return m_byteAligned;}

        void    SetReadPos(uint32_t pos)    { m_read_off  = pos >> 3; m_read_bit_off  = uint8_t(pos & 0x7);}
        uint32_t GetReadPos() const          { return (m_read_off<<3)  + m_read_bit_off;}
        void    SetWritePos(uint32_t pos)   { m_write_off = pos >> 3; m_write_bit_off = uint8_t(pos & 0x7);}
        uint32_t GetWritePos() const         { return (m_write_off<<3)  + m_write_bit_off;}

        void    UseByteAlignedMode(bool toggle);
        void    ByteAlign(bool read_part=true,bool write_part=true);
        void    ResetReading() { m_read_bit_off=0; m_read_off=0; }
        void    ResetOffsets();

        void CompressAndStoreString(const char *str);
        void GetAndDecompressString(QString &tgt);

private:
        uint8_t m_read_bit_off;
        uint8_t m_write_bit_off;
        bool m_byteAligned ;
};
