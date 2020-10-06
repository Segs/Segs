/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup Components
 * @{
 */

#include "Components/BitStream.h"

#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <cstring>
#include <cassert>
#include <algorithm>

//  Constants

#define BITS_PER_BYTE  8
#define BITS_PER_UINT32 (BITS_PER_BYTE * uint32_t(sizeof(uint32_t)))


 //  Macros
#define BIT_MASK(x)      ((uint32_t)((1ULL << (x)) - 1))
#define BITS_TO_BYTES(x) (((x) + 7) >> 3)
#define BITS_LEFT(x)     (BITS_PER_BYTE - (x))
#define BYTE_ALIGN(x)    (((x) + 7) & ~7)

// bitstream buffer is padded-out by 7 bytes, to allow safe 64bit reads/writes by new routines
/************************************************************************
Function:   BitStream
Description: BitStream's main constructor, initializes various internal
values and buffers
************************************************************************/
BitStream::BitStream(size_t size) : GrowingBuffer(0xFFFFFFFF,7,size)
{
    m_byteAligned    = false;
    ResetOffsets();
}


/************************************************************************
Function:     BitStream (copy constructor)
Description: BitStream's copy constructor.  The copy constructor creates
a deep copy of a BitStream, by copying the old buffer into
a new one.
************************************************************************/
BitStream::BitStream(const BitStream &bs) : GrowingBuffer(bs)
{

    m_byteAligned   = bs.m_byteAligned;
    m_read_bit_off  = bs.m_read_bit_off;
    m_write_bit_off = bs.m_write_bit_off;
    m_safe_area = 7;
}
/************************************************************************
Function:     BitStream
Description: BitStream's constructor, initializes various internal
                         values and buffers
************************************************************************/
BitStream::BitStream(uint8_t *arr,size_t size) : GrowingBuffer(arr,size,false)
{
    m_byteAligned   = false;
    m_read_bit_off  = 0;
    m_write_bit_off = 0;
    m_safe_area = 7;
}
BitStream &BitStream::operator =(const BitStream &bs)
{
    if(this==&bs)
        return *this;
    delete [] m_buf;
    m_safe_area = bs.m_safe_area;
    m_size      = bs.m_size;
    m_buf       = new uint8_t[m_size];
    m_last_err  = bs.m_last_err;
    m_write_off = bs.m_write_off;
    m_read_off  = bs.m_read_off;
    m_max_size  = bs.m_max_size;
    if(nullptr!=bs.m_buf)
        memcpy(m_buf,bs.m_buf,m_write_off); // copy up to write point
    m_byteAligned    = bs.m_byteAligned;
    m_read_bit_off   = bs.m_read_bit_off;
    m_write_bit_off  = bs.m_write_bit_off;
    return *this;
}
/************************************************************************
Function:    ~BitStream
Description: BitStream's destructor, de-allocates allocated memory and
cleans up anything else that BitStream has left behind.
************************************************************************/
BitStream::~BitStream()
{
}


/************************************************************************
*************************************************************************
                                                Functions to store bits
*************************************************************************
************************************************************************/

/***********************grr*************************************************
Function:    StoreBits/StoreBitsWithDebugInfo
Description: Stores a client-specified number of bits into the bit-
                         stream buffer.  The bits to store come from the dataBits
                         argument, starting from the least significant bit, to the
                         most significant bit
************************************************************************/
void BitStream::StoreBits(uint32_t nBits, uint32_t dataBits)
{
    assert(nBits <= BITS_PER_UINT32);

    if(nBits>GetWritableBits())
    {
        size_t new_size = m_size+(nBits>>3);
        // growing to accommodate !
        if(resize(new_size+7)==-1)
        {
            setLastError(1);
            return;
        }
    }
    //  If this stream is byte-aligned, then we'll need to use a byte-aligned
    //  value for nBits
    if(IsByteAligned())
    {
        if(nBits != BITS_PER_UINT32) // mask out non-needed
            dataBits &= (1<<nBits)-1;
        nBits = BYTE_ALIGN(nBits);
    }
    uStoreBits(nBits,dataBits);
}
void BitStream::uStoreBits(uint32_t nBits, uint32_t dataBits)
{
    assert(nBits <= BITS_PER_UINT32);
    assert(m_write_off+7<(m_size+m_safe_area));
    uint64_t *tp = reinterpret_cast<uint64_t *>(write_ptr());
    uint64_t r = dataBits;
    uint64_t mask_ = uint64_t(BIT_MASK(nBits))<<m_write_bit_off; // all bits in the mask are those that'll change
    (*tp) = (r<<m_write_bit_off)|((*tp)&~mask_); // put those bits in
    write_ptr((m_write_bit_off+nBits)>>3); //advance
    m_write_bit_off = (m_write_bit_off+nBits)&0x7;
}

/**
  @fn StorePackedBits
  @fn StorePackedBitsWithDebugInfo

 Stores the given \a dataBits value in the bitstream, using the following packing algorithm:
    while value to send is larger then the maximal value that can be written using \a nBits
      reduce the value to send by maximal value that can be written using nBits
      store nBits 1s in the stream to mark that the actual value is larger
      and increase nBits two times, making sure it doesn't grow larger then sizeof(int)*8
    after exiting the loop, send the leftover value
*/
void BitStream::StorePackedBits(uint32_t nBits, uint32_t dataBits)
{
    if(IsByteAligned())
        return StoreBits(BITS_PER_UINT32, dataBits);

    while((nBits < BITS_PER_UINT32) && (dataBits >= BIT_MASK(nBits)))
    {
        dataBits -= BIT_MASK(nBits);
        StoreBits(nBits, BIT_MASK(nBits));
        nBits = std::min(nBits * 2,BITS_PER_UINT32);
    }

    StoreBits(nBits, dataBits);
}
/**
 * @brief BitStream::StoreBitArray
 * Stores an array of bits in the bit stream buffer.  The
 * main difference between StoreBitArray and StoreBits, is
 * that StoreBitArray can accept more than 32 bits at a time
 * @param src
 * @param nBits
 */
void BitStream::StoreBitArray(const uint8_t *src,size_t nBits)
{
    size_t nBytes = BITS_TO_BYTES(nBits);
    assert(src);
    ByteAlign(false,true);
    PutBytes(src,nBytes);
    // nBits is not a multiple of 8, fixup
    if(nBits&7)
    {
        --m_write_off;
        m_write_bit_off = nBits&7;
    }
}


/************************************************************************
Function:    StoreString/StoreStringWithDebugInfo
Description: Stores a NULL terminated C-style string in the bit stream
                         buffer.  It includes the NULL terminator.
************************************************************************/

void BitStream::StoreString(const char *str)
{
    if(!str) // nothing to do ?
        return;

    //strlen(str) + 1, because we want to include
    //the NULL byte.
    if(IsByteAligned())
    {
        PutString(str);
        return;
    }
    size_t len = strlen(str)+1;
    uint32_t idx;
    uint8_t rshift = 8-m_write_bit_off;
    if(len>GetAvailSize())
    {
        if(resize(m_write_off+len)==-1) // space exhausted
        {
            setLastError(1);
            return;
        }
    }
    for(idx = 0; idx < len; idx++)
    {
        uint8_t upperbits   = ((uint16_t)((uint8_t *)str)[idx]) << m_write_bit_off;
        uint8_t lowerbits   = ((uint8_t *)str)[idx] >> rshift;
        uint8_t mask = (0xFF>> rshift);
        m_buf[m_write_off + idx] = (m_buf[m_write_off + idx] & mask) | upperbits;
        m_buf[m_write_off + idx + 1] = lowerbits;
    }

    m_write_off  += idx;
}

void BitStream::StoreString(const QByteArray &str)
{
    StoreString(str.constData());
}

void BitStream::StoreString(const QString &str)
{
    StoreString(qPrintable(str));
}


/************************************************************************
*************************************************************************
                                        Functions to retrieve bits
*************************************************************************
************************************************************************/


/************************************************************************
Function:    GetBits/GetBitsWithDebugInfo
Description: Retrieves a client-specified number of bits from the bit
                         stream
************************************************************************/
int32_t BitStream::GetBits(uint32_t nBits)
{
    if(nBits>GetReadableBits())
    {
        setLastError(1);
        return -1;
    }
    if(IsByteAligned())
        nBits= BYTE_ALIGN(nBits);
    int32_t tgt = uGetBits(nBits);
    return tgt;
}
int32_t BitStream::uGetBits(uint32_t nBits)
{
    assert((nBits>0) && nBits<= BITS_PER_UINT32);
    assert(GetReadableBits()>=nBits);
    assert(m_read_off+7<(m_size+m_safe_area));
    nBits = ((nBits-1) &0x1F)+1; // make sure the nBits range is 1-32
    uint64_t *tp = reinterpret_cast<uint64_t *>(read_ptr());
    uint64_t r = *tp;
    r>>=m_read_bit_off; // starting at the top
    int32_t tgt = int32_t(r & (~1ull)>>(64-nBits));
    read_ptr((m_read_bit_off+nBits)>>3);
    m_read_bit_off = (m_read_bit_off+nBits)&0x7;
    return tgt;
}


int32_t BitStream::GetPackedBits(uint32_t minbits)
{
    if(IsByteAligned())
        return GetBits(BITS_PER_UINT32);

    uint32_t accumulator = 0;
    while(GetReadableBits()>0)
    {
        uint32_t bits    = GetBits(minbits);
        uint32_t bitMask = BIT_MASK(minbits);

        if(bits < bitMask || minbits == BITS_PER_UINT32)
            return bits + accumulator;

        minbits     *= 2;
        accumulator += bitMask;

        if(minbits > BITS_PER_UINT32)
            minbits = BITS_PER_UINT32;
    }

    return -1;
}


/************************************************************************
Function:    GetBitArray/GetBitArrayWithDebugInfo
Description: Retrieves a client-specified "array" of bits.  The main
                         difference between this function, and the GetBits function
                         is that this one can potentially retrieve more than 32 bits
************************************************************************/
void BitStream::GetBitArray(uint8_t *tgt, uint32_t nBits)
{
    ByteAlign(true,false);
    GetBytes(tgt,BITS_TO_BYTES(nBits));
}

/**
\brief  Retrieves a null-terminated C-style string from the bit stream
\note will set stream error status in case of stream exhaustion
*/
void BitStream::GetString(QString &str)
{
    if(GetReadableBits()<8)
    {
        setLastError(1);
        return;
    }
    str.clear();
    uint32_t bitsLeft = BITS_LEFT(m_read_bit_off);
    uint8_t chr;
    do {
        chr  = m_buf[m_read_off]  >> m_read_bit_off;
        chr |= m_buf[++m_read_off] << bitsLeft;
        if(chr)
            str += char(chr);

        if((chr!='\0') && GetReadableBits()<8)
        {
            setLastError(1);
            return;
        }
    } while(chr != '\0');
}

/**
 * @brief Read upto 8 bytes from input stream and return them as 64bit integer
 * @return received 64 bit value
 */
int64_t BitStream::Get64Bits()
{
    int64_t result=0;
    uint32_t *res_ptr=reinterpret_cast<uint32_t *>(&result);
    int byte_count=GetBits(3);
    if( byte_count > 4 )
    {
        result=GetBits(BITS_PER_UINT32);
        byte_count-=4;
        res_ptr+=1;
    }

    *res_ptr=GetBits(8*byte_count);
    return result;
}

uint32_t BitStream::GetAvailSize() const
{
    int64_t res = int64_t(m_size)- int64_t(m_write_off)-(m_write_bit_off!=0);
    return uint32_t(std::max<int64_t>(0,res));
}
/************************************************************************
Function:    GetFloat/GetFloatWithDebugInfo()
Description: Retrieves a floating-point value from the bit stream.  This
                         will always be a 32-bit value.
************************************************************************/
float BitStream::GetFloat()
{
    float res=0.0f;
    if(IsByteAligned())
        Get(res);
    else
    {
        int32_t to_convert = GetBits(BITS_PER_UINT32);
        res = *(reinterpret_cast<float *>(&to_convert));
    }
    return res;
}

void BitStream::StoreFloat(float val)
{
    if(IsByteAligned())
        Put(val);
    else
        StoreBits(BITS_PER_UINT32,*(reinterpret_cast<uint32_t *>(&val)));
}



void BitStream::ResetOffsets()
{
    Reset();
    m_write_bit_off=m_read_bit_off=0;
}

void BitStream::UseByteAlignedMode(bool toggle)
{
    m_byteAligned = toggle;
    if(m_byteAligned) ByteAlign();
}


void BitStream::ByteAlign( bool read_part,bool write_part )
{
    //If bitPos is 0, we're already aligned
    if(write_part)
    {
        m_write_off += (m_write_bit_off>0);
        m_write_bit_off=0;
    }
    if(read_part)
    {
        m_read_off  += (m_read_bit_off>0);
        m_read_bit_off=0;
    }
}

void BitStream::CompressAndStoreString(const char *str)
{
    uint32_t decompLen = strlen(str) + 1;
    QByteArray ba = qCompress(reinterpret_cast<const uint8_t *>(str),decompLen,5);
    ba.remove(0,sizeof(uint32_t)); // qt includes uncompressed size as a first 4 bytes of QByteArray
    uint32_t len = ba.size();
    StorePackedBits(1, len);        //  Store compressed len
    StorePackedBits(1, decompLen);  //  Store decompressed len
    StoreBitArray((const uint8_t *)ba.data(),len << 3);    //  Store compressed string
}
static QByteArray uncompr_zip(QByteArray &compressed_data,uint32_t size_uncom)
{
    compressed_data.prepend( char((size_uncom >> 0) & 0xFF));
    compressed_data.prepend( char((size_uncom >> 8) & 0xFF));
    compressed_data.prepend( char((size_uncom >> 16) & 0xFF));
    compressed_data.prepend( char((size_uncom >> 24) & 0xFF));
    return qUncompress(compressed_data);
}
void BitStream::GetAndDecompressString(QString &tgt)
{
    uint32_t len = GetPackedBits(1);     //  Store compressed len
    uint32_t decompLen = GetPackedBits(1);     //  decompressed len
    uint8_t *src = new uint8_t[len]; // FixMe: GetPackedBits() returns signed values which can cause len to be high if wrapped.
    GetBitArray(src,len<<3);
    QByteArray compr_data((const char *)src,len);
    tgt = uncompr_zip(compr_data,decompLen);
    delete [] src;
}

//! @}
