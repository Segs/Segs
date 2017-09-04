/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#include "BitStream.h"

#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <cstring>
#include <cassert>
#include <algorithm>

// bitstream buffer is padded-out by 7 bytes, to allow safe 64bit reads/writes by new routines
/************************************************************************
Function:   BitStream
Description: BitStream's main constructor, initializes various internal
values and buffers
************************************************************************/
BitStream::BitStream(size_t size) : GrowingBuffer(0xFFFFFFFF,7,size)
{
    m_byteAligned    = false;
    Reset();
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
    assert(m_buf!=NULL);
    m_last_err  = bs.m_last_err;
    m_write_off = bs.m_write_off;
    m_read_off  = bs.m_read_off;
    m_max_size  = bs.m_max_size;
    if(NULL!=m_buf)
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

/************************************************************************
Function:    StoreBits/StoreBitsWithDebugInfo
Description: Stores a client-specified number of bits into the bit-
                         stream buffer.  The bits to store come from the dataBits
                         argument, starting from the least significant bit, to the
                         most significant bit
************************************************************************/
void BitStream::StoreBitsWithDebugInfo(uint32_t nBits, uint32_t dataBits)
{
    StoreBits(3, BS_BITS);
    StorePackedBits(5, nBits);
    StoreBits(dataBits, nBits);
}
/***********************grr*************************************************
Function:    StoreBits/StoreBitsWithDebugInfo
Description: Stores a client-specified number of bits into the bit-
                         stream buffer.  The bits to store come from the dataBits
                         argument, starting from the least significant bit, to the
                         most significant bit
************************************************************************/
void BitStream::StoreBits(uint32_t nBits, uint32_t dataBits)
{
    assert(nBits <= BITS_PER_DWORD);

    if(nBits>GetWritableBits())
    {
        size_t new_size = m_size+(nBits>>3);
        // growing to accommodate !
        if(resize(new_size+7)==-1)
        {
            m_last_err = 1;
            return;
        }
    }
    //  If this stream is byte-aligned, then we'll need to use a byte-aligned
    //  value for nBits
    if(IsByteAligned())
    {
        if(nBits!=32) // mask out non-needed
            dataBits &= (1<<nBits)-1;
        nBits = BYTE_ALIGN(nBits);
    }
    uStoreBits(nBits,dataBits);
}
void BitStream::uStoreBits(uint32_t nBits, uint32_t dataBits)
{
    uint64_t *tp,r;
    assert(nBits<=32);
    assert(m_write_off+7<(m_size+m_safe_area));
    tp = (uint64_t *)write_ptr();
    r = dataBits;
    uint64_t mask_ = BIT_MASK(nBits)<<m_write_bit_off; // all bits in the mask are those that'll change
    (*tp) = (r<<m_write_bit_off)|((*tp)&~mask_); // put those bits in
    write_ptr((m_write_bit_off+nBits)>>3); //advance
    m_write_bit_off = (m_write_bit_off+nBits)&0x7;
}

/************************************************************************
Function:    StorePackedBits/StorePackedBitsWithDebugInfo
Description: Stores bits in a special "packed" format.  Though i've
                         written a working implementation of it, I don't entirely
                         understand how it works

TODO: Learn more about the "packed bits" format, and write a better
          description of it, and if necessary a better implementation
************************************************************************/
void BitStream::StorePackedBitsWithDebugInfo(uint32_t nBits, uint32_t dataBits)
{
    StoreBits(3, BS_PACKEDBITS);
    StorePackedBits(5, nBits);
    StorePackedBits(nBits, dataBits);
}
/*
        Masks: 1,3,15,255
        store 1,4 -> Store(1,1) 3,Store(2,3) -> 1110000
        store 1,18 -> Store(1,1) 17,Store(2,3) 14 -> 1111110
        store 2,18 -> Store(2,3) 15,Store(4,15) 0 -> 11 1111 00000000
        store 3,18 -> Store(3,7) 11, 111 001011
*/
#define MAX_VALUE_THAT_CAN_BE_STORED(x) ((uint32_t)((1 << (x)) - 1))
#define N_ONES(n) ((uint32_t)((1 << (n)) - 1))
/**
  @fn StorePackedBits
  @fn StorePackedBitsWithDebugInfo

 Stores the given \a dataBits value in the bitstream, using the following packing algorithm:
    while value to send is larger then the maximal value that can be written using \a nBits
      reduce the value to send by maximal value that can be written using nBits
      store nBits in the stream to mark that the actual value is larger
      and increase nBits two times, making sure it doesn't grow larger then sizoef(int)*8
    after exiting the loop, send the leftover value
*/
void BitStream::StorePackedBits(uint32_t nBits, uint32_t dataBits)
{
    if(IsByteAligned())
        return StoreBits(32, dataBits);

    while((nBits < 32) && (dataBits >= MAX_VALUE_THAT_CAN_BE_STORED(nBits)))
    {
        dataBits -= MAX_VALUE_THAT_CAN_BE_STORED(nBits);
        StoreBits(nBits, N_ONES(nBits));
        nBits = std::min(nBits * 2,BITS_PER_DWORD);
    }

    StoreBits(nBits, dataBits);
}
#undef N_ONES
#undef MAX_VALUE_THAT_CAN_BE_STORED
/************************************************************************
Function:    StoreBitArray/StoreBitArrayWithDebugInfo
Description: Stores an array of bits in the bit stream buffer.  The
                         main difference between StoreBitArray and StoreBits, is
                         that StoreBitArray can accept more than 32 bits at a time
************************************************************************/
void BitStream::StoreBitArrayWithDebugInfo(const uint8_t *array,uint32_t nBits)
{
    StoreBits(3, BS_BITARRAY);
    StorePackedBits(5, nBits);
    StoreBitArray(array,nBits);
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
    ByteAlign();
    PutBytes(src,nBytes);
    m_buf[m_write_off] = 0;
}
/**
 * @brief BitStream::StoreBitArray
 * Stores an array of bits in the bit stream buffer.  The
 * main difference between StoreBitArray and StoreBits, is
 * that StoreBitArray can accept more than 32 bits at a time
 * @note The stream *end* is *not* aligned to 8 bits - mainly needed to send the correct number of readable bits
 * @param src
 * @param nBits
 */
void BitStream::StoreBitArray_Unaligned(const uint8_t *src,size_t nBits)
{
    size_t nBytes = BITS_TO_BYTES(nBits);
    assert(src);
    ByteAlign();
    PutBytes(src,nBytes);
    m_buf[m_write_off] = 0;
    if(nBits&7) // unaligned !
    {
        m_write_off--;
        m_write_bit_off = nBits&7;
    }
}


/************************************************************************
Function:    StoreString/StoreStringWithDebugInfo
Description: Stores a NULL terminated C-style string in the bit stream
                         buffer.  It includes the NULL terminator.
************************************************************************/
void BitStream::StoreStringWithDebugInfo(const char *str)
{
    StoreBits(3, BS_STRING);
    size_t str_length=0;
    if(str)
        str_length=strlen(str);
    StorePackedBits(5, (uint32_t)str_length);
    StoreString(str);
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
            m_last_err = 1;
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
int32_t BitStream::GetBitsWithDebugInfo(uint32_t nBits)
{
    if(GetBits(3) != BS_BITS)
        return 0;
    /*uint32_t datalength =*/ (void)GetPackedBits(5);
    return GetBits(nBits);
}
/************************************************************************
Function:    GetBits/GetBitsWithDebugInfo
Description: Retrieves a client-specified number of bits from the bit
                         stream
************************************************************************/
int32_t BitStream::GetBits(uint32_t nBits)
{
    int32_t tgt;
    if(nBits>GetReadableBits())
        return false;
    if(IsByteAligned())
        nBits= BYTE_ALIGN(nBits);
    tgt = uGetBits(nBits);
    return tgt;
}
int32_t BitStream::uGetBits(uint32_t nBits)
{
    unsigned long long r;
    uint64_t *tp;
    int32_t tgt;
    assert((nBits>0) && nBits<=32);
    assert(GetReadableBits()>=nBits);
    assert(m_read_off+7<(m_size+m_safe_area));
    nBits = ((nBits-1) &0x1F)+1; // make sure the nBits range is 1-32
    tp = (uint64_t *)read_ptr();
    r = *tp;//swap64(*tp);
    r>>=m_read_bit_off; // starting at the top
    tgt = int32_t(r & (~1ull)>>(64-nBits));
    read_ptr((m_read_bit_off+nBits)>>3);
    m_read_bit_off = (m_read_bit_off+nBits)&0x7;
    return tgt;
}


/************************************************************************//**
\fn  GetPackedBits
\fn  GetPackedBitsWithDebugInfo
 Retrieves an indefinite( up to 32) number of bits.
 The algorithm that determines the encoding of the value is as follows:

 set result to 0
 while there are bits available
    read minbits bits, and add them to result
    if the last read bits are all 1s, or the number of bits is equal to bit length of integer
        return the result
    otherwise
        increase the minbits two times, taking care not to allow it to exceed the bit length of integer

************************************************************************/
int32_t BitStream::GetPackedBitsWithDebugInfo(uint32_t minbits)
{
    if(GetBits(3) != BS_PACKEDBITS) return 0;
    /*uint32_t datalength =*/ (void)GetPackedBits(5);
    return GetPackedBits(minbits);
}
int32_t BitStream::GetPackedBits(uint32_t minbits)
{
    if(IsByteAligned())
        return GetBits(32);

    uint32_t accumulator = 0;
    while(GetReadableBits()>0)
    {
        uint32_t bits    = GetBits(minbits);
        uint32_t bitMask = BIT_MASK(minbits);

        if(bits < bitMask || minbits == BITS_PER_DWORD) return bits + accumulator;

        minbits     *= 2;
        accumulator += bitMask;

        if(minbits > BITS_PER_DWORD) minbits = BITS_PER_DWORD;
    }

    return -1;
}


/************************************************************************
Function:    GetBitArray/GetBitArrayWithDebugInfo
Description: Retrieves a client-specified "array" of bits.  The main
                         difference between this function, and the GetBits function
                         is that this one can potentially retrieve more than 32 bits
************************************************************************/
void BitStream::GetBitArrayWithDebugInfo(uint8_t *array,size_t nBytes)
{
    if(GetBits(3) != BS_BITARRAY) return;
    /*uint32_t datalength =*/ (void)GetPackedBits(5);
    GetBitArray(array,nBytes);
}
/************************************************************************
Function:    GetBitArray/GetBitArrayWithDebugInfo
Description: Retrieves a client-specified "array" of bits.  The main
                         difference between this function, and the GetBits function
                         is that this one can potentially retrieve more than 32 bits
************************************************************************/
void BitStream::GetBitArray(uint8_t *tgt, size_t nBits)
{
    ByteAlign(true,false);
    size_t nBytes(nBits>>3);
    GetBytes(tgt,nBytes);
}


/************************************************************************
Function:    GetString/GetStringWithDebugInfo
Description: Retrieves a null-terminated C-style string from the bit
                         stream
************************************************************************/
void BitStream::GetStringWithDebugInfo(QString &str)
{
    if(GetBits(3) != BS_STRING)
        return;
    /*uint32_t datalength =*/ (void)GetPackedBits(5);
    GetString(str);
}
/************************************************************************
Function:    GetString/GetStringWithDebugInfo
Description: Retrieves a null-terminated C-style string from the bit
                         stream
************************************************************************/
void BitStream::GetString(QString &str)
{
    if(GetReadableBits()<8)
    {
        m_last_err = 1;
        return;
    }
    str.clear();
    uint32_t bitsLeft = BITS_LEFT(m_read_bit_off);
    uint8_t chr;
    do {
        chr  = m_buf[m_read_off]  >> m_read_bit_off;
        chr |= m_buf[++m_read_off] << bitsLeft;
        if(chr)
            str += chr;

        if((chr!='\0') && GetReadableBits()<8)
        {
            m_last_err = 1;
            return;
        }
    } while(chr != '\0');
}


/************************************************************************
Function:    GetFloat/GetFloatWithDebugInfo()
Description: Retrieves a floating-point value from the bit stream.  This
                         will always be a 32-bit value.
************************************************************************/
float BitStream::GetFloatWithDebugInfo()
{
    if(GetBits(3) != BS_F32) return 0;
    /*uint32_t datalength =*/ (void)GetPackedBits(5);
    return GetFloat();
}

int64_t BitStream::Get64Bits()
{
    int64_t result=0;
    uint32_t *res_ptr=(uint32_t *)&result;
    uint8_t byte_count=GetBits(3);
    if ( byte_count > 4 )
    {
        result=GetBits(32);
        byte_count-=4;
        res_ptr+=1;
    }

    *res_ptr=GetBits(8*byte_count);
    return result;
}

size_t BitStream::GetAvailSize() const
{
    int64_t res = (int64_t)((m_size-m_write_off)-(m_write_bit_off!=0));
    return (size_t)std::max<>((int64_t)0,res);
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
        int32_t to_convert = GetBits(32);
        res = *(reinterpret_cast<float *>(&to_convert));
        assert(res==(*((float *)&to_convert)));
    }
    return res;
}
void BitStream::StoreFloatWithDebugInfo(float val)
{
    StoreBits(3,BS_F32);
    StoreBits(5,32);
    StoreBits(32,*(reinterpret_cast<uint32_t *>(&val)));
    //StoreBits(32,*((uint32_t *)&val));
}

void BitStream::StoreFloat(float val)
{
    if(IsByteAligned())
        Put(val);
    else
        StoreBits(32,*(reinterpret_cast<uint32_t *>(&val)));
    //  StoreBits(32,*((uint32_t *)&val));
}



void BitStream::Reset()
{
    GrowingBuffer::Reset();
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

/*
uint32_t BitStream::GetPackedBitsLength(uint32_t nBits, uint32_t dataBits) const
{
        if(IsByteAligned())
            return GetBitsLength(32, dataBits);

        uint32_t length = 0;
        for(nBits; nBits < 32 && dataBits >= BIT_MASK(nBits); nBits)
        {
                dataBits -= BIT_MASK(nBits);
                length   += GetBitsLength(nBits, BIT_MASK(nBits));
                nBits *= 2;
                if(nBits > 32) nBits = 32;
        }

        length += GetBitsLength(nBits, dataBits);
        return length;
}

uint32_t BitStream::GetBitsLength(uint32_t nBits, uint32_t dataBits) const
{
        //If this stream is byte-aligned, then we'll need to use a byte-aligned
        //value for nBits
        uint32_t numbits = IsByteAligned() ? BYTE_ALIGN(nBits) : nBits;
        assert(numbits <= BITS_PER_DWORD);

        uint32_t bitsAdded = 0;
        for(uint32_t bits = 0; numbits; numbits -= bits)
        {
                //If we still have more bits left to copy than are left in
                //this byte, then we only copy the number of bits left in
                //the current byte.
                bits       = numbits >= BITS_LEFT() ? GetBitsLeftInByte() : numbits;
                bitsAdded += bits;
        }

        return bitsAdded;
}*/


void BitStream::SetByteLength(uint32_t /*byteLength*/)
{
    assert(!"Not implemented!");
}
/*
BitStream::BitStream(uint8_t* arr,uint32_t bit_size)
{
        m_buf            = new uint8_t[BITS_TO_BYTES(bit_size)];
        memcpy(m_buf, arr,BITS_TO_BYTES(bit_size));
        m_allocatedBytes = BITS_TO_BYTES(bit_size);
        m_bitPos         = 0;
        m_bytePos        = 0;
        m_byteLength     = BITS_TO_BYTES(bit_size);
        m_bitLength      = bit_size;
        m_byteAligned    = false;

}*/
void BitStream::CompressAndStoreString(const char *str)
{
    size_t decompLen = strlen(str) + 1;

    uint32_t len = (decompLen * 1.0125) + 12;
    QByteArray ba = qCompress(reinterpret_cast<const uint8_t *>(str),decompLen,5);
    ba.remove(0,sizeof(uint32_t)); // qt includes uncompressed size as a first 4 bytes of QByteArray
    len = ba.size();
    StorePackedBits(1, len);        //  Store compressed len
    StorePackedBits(1, decompLen);  //  Store decompressed len
    StoreBitArray((const uint8_t *)ba.data(),len << 3);    //  Store compressed string
}
static QByteArray uncompr_zip(QByteArray &compressed_data,uint32_t size_uncom) {
    compressed_data.prepend( char((size_uncom >> 0) & 0xFF));
    compressed_data.prepend( char((size_uncom >> 8) & 0xFF));
    compressed_data.prepend( char((size_uncom >> 16) & 0xFF));
    compressed_data.prepend( char((size_uncom >> 24) & 0xFF));
    return qUncompress(compressed_data);
}
void BitStream::GetAndDecompressString(QString &tgt)
{
    uint32_t decompLen = 0;
    uint32_t len = 0;

    len         = GetPackedBits(1);     //  Store compressed len
    decompLen   = GetPackedBits(1);     //  decompressed len
    uint8_t *src = new uint8_t[len];
    GetBitArray(src,len<<3);
    QByteArray compr_data((const char *)src,len);
    tgt = uncompr_zip(compr_data,decompLen);
    delete [] src;
}
