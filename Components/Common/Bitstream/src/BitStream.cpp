/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: BitStream.cpp 253 2006-08-31 22:00:14Z malign $
 */

#include "BitStream.h"
#include <ace/Log_Msg.h>
#include <zlib.h>
// bitstream buffer is padded-out by 7 bytes, to allow safe 64bit reads/writes by new routines
/************************************************************************
Function:	 BitStream
Description: BitStream's main constructor, initializes various internal
values and buffers
************************************************************************/
BitStream::BitStream(u32 size) : GrowingBuffer(0xFFFFFFFF,7,size)
{ 
	m_byteAligned    = false;
	Reset();
};


/************************************************************************
Function:	 BitStream (copy constructor)
Description: BitStream's copy constructor.  The copy constructor creates
a deep copy of a BitStream, by copying the old buffer into
a new one.
************************************************************************/
BitStream::BitStream(const BitStream &bs) : GrowingBuffer(bs)
{

	m_byteAligned	 = bs.m_byteAligned;
	m_read_bit_off   = bs.m_read_bit_off;
	m_write_bit_off  = bs.m_write_bit_off;
	m_safe_area = 7;
}
BitStream::BitStream(u8 *arr,u32 size) : GrowingBuffer(arr,size,false)
{
	m_byteAligned	= false;
	m_read_bit_off  = 0;
	m_write_bit_off = 0;
	m_safe_area = 7;
}
BitStream &BitStream::operator =(const BitStream &bs)
{
	if(this!=&bs)
	{
		delete		[] m_buf;
		m_safe_area = bs.m_safe_area;
		m_size		= bs.m_size;
		m_buf		= new u8[m_size];
		ACE_ASSERT(m_buf!=NULL);
		m_last_err	= bs.m_last_err;
		m_write_off = bs.m_write_off;
		m_read_off  = bs.m_read_off;
		m_max_size	= bs.m_max_size;
		if(NULL!=m_buf)
			memcpy(m_buf,bs.m_buf,m_write_off); // copy up to write point
		m_byteAligned	 = bs.m_byteAligned;
		m_read_bit_off   = bs.m_read_bit_off;
		m_write_bit_off  = bs.m_write_bit_off;		
	}
	return *this;
}
/************************************************************************
Function:	 ~BitStream
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
Function:	 StoreBits/StoreBitsWithDebugInfo
Description: Stores a client-specified number of bits into the bit-
			 stream buffer.  The bits to store come from the dataBits
			 argument, starting from the least significant bit, to the
			 most significant bit
************************************************************************/
void BitStream::StoreBitsWithDebugInfo(u32 nBits, u32 dataBits)
{
	StoreBits(3, BS_BITS);
	StorePackedBits(5, nBits);
	StoreBits(dataBits, nBits);
}

void BitStream::StoreBits(u32 nBits, u32 dataBits)
{
	ACE_ASSERT(nBits <= BITS_PER_DWORD);

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
	//	If this stream is byte-aligned, then we'll need to use a byte-aligned
	//	value for nBits
	if(IsByteAligned())
	{
		if(nBits!=32) // mask out non-needed
			dataBits &= (1<<nBits)-1;
		nBits = BYTE_ALIGN(nBits);
	}
	uStoreBits(nBits,dataBits);
}
void BitStream::uStoreBits(u32 nBits, u32 dataBits)
{
	u64 *tp,r;
	ACE_ASSERT(nBits<=32);
	ACE_ASSERT(m_write_off+7<(m_size+m_safe_area));
	tp = (u64 *)write_ptr();
	r = dataBits;
	u64 mask_ = BIT_MASK(nBits)<<m_write_bit_off; // all bits in the mask are those that'll change
	(*tp) = (r<<m_write_bit_off)|((*tp)&~mask_); // put those bits in
	write_ptr((m_write_bit_off+nBits)>>3); //advance
	m_write_bit_off = (m_write_bit_off+nBits)&0x7;
}

/************************************************************************
Function:	 StorePackedBits/StorePackedBitsWithDebugInfo
Description: Stores bits in a special "packed" format.  Though i've
			 written a working implementation of it, I don't entirely
			 understand how it works

TODO: Learn more about the "packed bits" format, and write a better
	  description of it, and if necessary a better implementation
************************************************************************/
void BitStream::StorePackedBitsWithDebugInfo(u32 nBits, u32 dataBits)
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
void BitStream::StorePackedBits(u32 nBits, u32 dataBits)
{
	if(IsByteAligned())	return StoreBits(32, dataBits);

	while((nBits < 32) && (dataBits >= BIT_MASK(nBits)))
	{
		dataBits -= BIT_MASK(nBits);
		StoreBits(nBits, BIT_MASK(nBits));
		nBits *= 2;
		if(nBits > 32) nBits = 32;
	}

	StoreBits(nBits, dataBits);
}


/************************************************************************
Function:	 StoreBitArray/StoreBitArrayWithDebugInfo
Description: Stores an array of bits in the bit stream buffer.  The
			 main difference between StoreBitArray and StoreBits, is
			 that StoreBitArray can accept more than 32 bits at a time
************************	************************************************/
void BitStream::StoreBitArrayWithDebugInfo(const u8 *array,u32 nBits)
{
	StoreBits(3, BS_BITARRAY);
	StorePackedBits(5, nBits);
	StoreBitArray(array,nBits);
}

void BitStream::StoreBitArray(const u8 *src,u32 nBits)
{
	u32 nBytes = BITS_TO_BYTES(nBits);
	ACE_ASSERT(src);
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
Function:	 StoreString/StoreStringWithDebugInfo
Description: Stores a NULL terminated C-style string in the bit stream
			 buffer.  It includes the NULL terminator.
************************************************************************/
void BitStream::StoreStringWithDebugInfo(const char *str)
{
	StoreBits(3, BS_STRING);
	size_t str_length=0;
	if(str)
		str_length=strlen(str);
	StorePackedBits(5, (u32)str_length);
	StoreString(str);
}

void BitStream::StoreString(const char *str)
{
	//	strlen(str) + 1, because we want to include
	//	the NULL byte.
	if(IsByteAligned())
	{
		PutString(str);
		return;
	}
	size_t len = 1;
	if(str)
		len+=strlen(str);
	u32 idx;
	u8	rshift = 8-m_write_bit_off;
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
		u8 upperbits   = ((u16)((u8 *)str)[idx]) << m_write_bit_off;
		u8 lowerbits   = ((u8 *)str)[idx] >> rshift;
		u8 mask = (0xFF>> rshift);
		m_buf[m_write_off + idx] = (m_buf[m_write_off + idx] & mask) | upperbits;
		m_buf[m_write_off + idx + 1] = lowerbits;
	}

	m_write_off  += idx;
}


/************************************************************************
*************************************************************************
					Functions to retrieve bits
*************************************************************************
************************************************************************/


/************************************************************************
Function:	 GetBits/GetBitsWithDebugInfo
Description: Retrieves a client-specified number of bits from the bit
			 stream
************************************************************************/
s32 BitStream::GetBitsWithDebugInfo(u32 nBits)
{
	if(GetBits(3) != BS_BITS)	return 0;
	/*u32 datalength =*/ (void)GetPackedBits(5);
	return GetBits(nBits);
}

s32 BitStream::GetBits(u32 nBits)
{
	s32 tgt;
	if(nBits>GetReadableBits()) 
		return false;
	if(IsByteAligned())
		nBits= BYTE_ALIGN(nBits);
	tgt = uGetBits(nBits);
	return tgt;
}
s32 BitStream::uGetBits(u32 nBits)
{
	unsigned long long *tp,r;
	s32 tgt;
	ACE_ASSERT(nBits<=32);
	ACE_ASSERT(GetReadableBits()>=nBits);
	ACE_ASSERT(m_read_off+7<(m_size+m_safe_area));
	tp = (u64 *)read_ptr();
	r = *tp;//swap64(*tp);
	r>>=m_read_bit_off; // starting at the top
	tgt = s32(r & (~1ull)>>(64-nBits));
	read_ptr((m_read_bit_off+nBits)>>3);
	m_read_bit_off = (m_read_bit_off+nBits)&0x7; 
	return tgt;
}


/************************************************************************
Function:	 GetPackedBits/GetPackedBitsWithDebugInfo
Description: Retrieves an indefinite(though always less than 32) number
			 of bits.  It determines how many to retrieve based on how
			 the bits are packed.  I don't yet fully understand this
			 "packed bits" concept

TODO: Learn more about this format and write a better description, and 
	  if necessary, a better implementation
************************************************************************/
s32 BitStream::GetPackedBitsWithDebugInfo(u32 minbits)
{
	if(GetBits(3) != BS_PACKEDBITS) return 0;
	/*u32 datalength =*/ (void)GetPackedBits(5);
	return GetPackedBits(minbits);
}

s32 BitStream::GetPackedBits(u32 minbits)
{
	if(IsByteAligned())	return GetBits(32);

	u32 accumulator = 0;
	while(GetReadableBits()>0)
	{
		u32 bits    = GetBits(minbits);
		u32 bitMask = BIT_MASK(minbits);

		if(bits < bitMask || minbits == BITS_PER_DWORD) return bits + accumulator;

		minbits     *= 2;
		accumulator += bitMask;

		if(minbits > BITS_PER_DWORD) minbits = BITS_PER_DWORD;
	} 

	return -1;
}


/************************************************************************
Function:	 GetBitArray/GetBitArrayWithDebugInfo
Description: Retrieves a client-specified "array" of bits.  The main
			 difference between this function, and the GetBits function
			 is that this one can potentially retrieve more than 32 bits
************************************************************************/
void BitStream::GetBitArrayWithDebugInfo(u8 *array,size_t nBytes)
{
	if(GetBits(3) != BS_BITARRAY) return;
	/*u32 datalength =*/ (void)GetPackedBits(5);
	GetBitArray(array,nBytes);
}

void BitStream::GetBitArray(u8 *tgt, size_t nBits)
{
	ByteAlign(true,false);
	u32 nBytes(nBits>>3);
	GetBytes(tgt,nBytes);
}


/************************************************************************
Function:	 GetString/GetStringWithDebugInfo
Description: Retrieves a null-terminated C-style string from the bit
			 stream
************************************************************************/
void BitStream::GetStringWithDebugInfo(string &str)
{
	if(GetBits(3) != BS_STRING)	return;
	/*u32 datalength =*/ (void)GetPackedBits(5);
	GetString(str);
}

void BitStream::GetString(string &str)
{
	if(GetReadableBits()<8) 
	{
		m_last_err = 1;
		return;
	}
	str.clear();
	u32 bitsLeft = BITS_LEFT(m_read_bit_off);
	u8 chr;
	do {
		chr  = m_buf[m_read_off]  >> m_read_bit_off;
		chr	|= m_buf[++m_read_off] << bitsLeft;
        if(chr)
		    str += chr;

		if(GetReadableBits()<8) 
		{
			m_last_err = 1;
			return;
		}
	} while(chr != '\0');
}


/************************************************************************
Function:	 GetFloat/GetFloatWithDebugInfo()
Description: Retrieves a floating-point value from the bit stream.  This
			 will always be a 32-bit value.
************************************************************************/
f32 BitStream::GetFloatWithDebugInfo()
{
	if(GetBits(3) != BS_F32) return 0;
	/*u32 datalength =*/ (void)GetPackedBits(5);
	return GetFloat();
}

f32 BitStream::GetFloat()
{
	f32 res;
	if(IsByteAligned())
		Get(res);
	else
	{
		s32 to_convert = GetBits(32);
		res = *(reinterpret_cast<float *>(&to_convert));
		ACE_ASSERT(res==(*((f32 *)&to_convert)));
	}
	return res;
}
void BitStream::StoreFloatWithDebugInfo(f32 val)
{
	StoreBits(3,BS_F32);
	StoreBits(5,32);
	StoreBits(32,*(reinterpret_cast<u32 *>(&val)));
	//StoreBits(32,*((u32 *)&val));
}

void BitStream::StoreFloat(f32 val)
{
	if(IsByteAligned())
		Put(val);
	else
		StoreBits(32,*(reinterpret_cast<u32 *>(&val)));
//		StoreBits(32,*((u32 *)&val));
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
	//	If bitPos is 0, we're already aligned
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
u32 BitStream::GetPackedBitsLength(u32 nBits, u32 dataBits) const
{
	if(IsByteAligned())	return GetBitsLength(32, dataBits);

	u32 length = 0;
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

u32 BitStream::GetBitsLength(u32 nBits, u32 dataBits) const
{
	//	If this stream is byte-aligned, then we'll need to use a byte-aligned
	//	value for nBits
	u32 numbits = IsByteAligned() ? BYTE_ALIGN(nBits) : nBits;
	assert(numbits <= BITS_PER_DWORD);
	
	u32 bitsAdded = 0;
	for(u32 bits = 0; numbits; numbits -= bits)
	{
		//	If we still have more bits left to copy than are left in
		//	this byte, then we only copy the number of bits left in
		//	the current byte.
		bits       = numbits >= BITS_LEFT() ? GetBitsLeftInByte() : numbits;
		bitsAdded += bits;
	}

	return bitsAdded;
}*/


void BitStream::SetByteLength(u32 /*byteLength*/)
{
	ACE_ASSERT(!"Not implemented!");
}
/*
BitStream::BitStream(u8* arr,u32 bit_size)
{
	m_buf            = new u8[BITS_TO_BYTES(bit_size)];
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
	u32 decompLen = strlen(str) + 1;

	u32 len = ((u32)(decompLen * 1.0125)) + 12;
	u8 *buf = new u8[len];
	compress2(buf, (uLongf *)&len, (const Bytef *)str, decompLen, 5);
	StorePackedBits(1, len);		//	Store compressed len
	StorePackedBits(1, decompLen);	//	Store decompressed len
	StoreBitArray(buf,len << 3);	//	Store compressed string
	delete [] buf;
}
void BitStream::GetAndDecompressString(string &tgt)
{
	u32 decompLen = 0;

	u32 len = 0;
	len		  = GetPackedBits(1);		//	Store compressed len
	decompLen = GetPackedBits(1);		//	decompressed len
	u8 *dst = new u8[decompLen];
	u8 *src = new u8[len];
	GetBitArray(src,len<<3);
	uncompress(dst,(uLongf *)&decompLen,src,len);
	tgt.assign((char *)dst,decompLen);
	delete [] src;
	delete [] dst;
}
