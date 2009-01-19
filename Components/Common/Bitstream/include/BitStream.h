/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: BitStream.h 253 2006-08-31 22:00:14Z malign $
 */

/************************************************************************
Class:		 BitStream
Author:		 Darawk
Description: The BitStream class allows it's user to manipulate data in
			 terms of individual bits, thus allowing said data to achieve
			 much greater levels of density.  It abstracts the nuts and
			 bolts of this functionality away from it's user.
************************************************************************/
#pragma once
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

	/************************************************************************
	Function:	 BitStream
	Description: BitStream's main constructor, initializes various internal
			 	 values and buffers
	************************************************************************/
	BitStream(u32 size);

	/************************************************************************
	Function:	 BitStream
	Description: BitStream's constructor, initializes various internal
				 values and buffers
	************************************************************************/
	BitStream(u8 *from,u32 bitsize);

	/************************************************************************
	Function:	 BitStream (copy constructor)
	Description: BitStream's copy constructor.  The copy constructor creates
				 a deep copy of a BitStream, by copying the old buffer into
				 a new one. Only explicit copying allowed. Compiler will 
				 complain if some method will try to pass BitStream by value.
	************************************************************************/
	BitStream(const BitStream &bs);
	BitStream &operator=(const BitStream &bs);


	/************************************************************************
	Function:	 ~BitStream
	Description: BitStream's destructor, de-allocates allocated memory and
				 cleans up anything else that BitStream has left behind.
	************************************************************************/
	~BitStream();

	/************************************************************************
	*************************************************************************
							Functions to store bits
	*************************************************************************
	************************************************************************/
	
	/***********************grr*************************************************
	Function:	 StoreBits/StoreBitsWithDebugInfo
	Description: Stores a client-specified number of bits into the bit-
				 stream buffer.  The bits to store come from the dataBits
				 argument, starting from the least significant bit, to the
				 most significant bit
	************************************************************************/
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
	/************************************************************************
	Function:	 StorePackedBits/StorePackedBitsWithDebugInfo
	Description: Stores bits in a special "packed" format.  Though i've
				 written a working implementation of it, I don't entirely
				 understand how it works

	 TODO: Learn more about the "packed bits" format, and write a better
		   description of it, and if necessary a better implementation
	************************************************************************/
	void StorePackedBits(u32 nBits, u32 dataBits);
	void StorePackedBitsWithDebugInfo(u32 nBits, u32 dataBits);

	/************************************************************************
	Function:	 StoreBitArray/StoreBitArrayWithDebugInfo
	Description: Stores an array of bits in the bit stream buffer.  The
				 main difference between StoreBitArray and StoreBits, is
				 that StoreBitArray can accept more than 32 bits at a time
	************************************************************************/
	void StoreBitArray(const u8 *array,u32 nBits);
	void StoreBitArrayWithDebugInfo(const u8 *array,u32 nBits);

	/************************************************************************
	Function:	 StoreString/StoreStringWithDebugInfo
	Description: Stores a NULL terminated C-style string in the bit stream
				 buffer.  It includes the NULL terminator.
	************************************************************************/
	void StoreString(const char *str);
	void StoreString(const string &str){StoreString(str.c_str());};
	void StoreStringWithDebugInfo(const char *str);


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
	s32 GetBits(u32 nBits);
	s32 GetBits_2_10_24_32()
	{
		static const int numbits[]={2,10,24,32};
		int type=GetBits(2);
		return GetBits(numbits[type]);
	}
	s32 uGetBits(u32 nBits);
	s32 GetBitsWithDebugInfo(u32 nBits);

	/************************************************************************
	Function:	 GetPackedBits/GetPackedBitsWithDebugInfo
	Description: Retrieves an indefinite(though always less than 32) number
				 of bits.  It determines how many to retrieve based on how
				 the bits are packed.  I don't yet fully understand this
				 "packed bits" concept

	TODO: Learn more about this format and write a better description, and 
		  if necessary, a better implementation
	************************************************************************/
	s32 GetPackedBits(u32 minbits);
	s32 GetPackedBitsWithDebugInfo(u32 minbits);

	/************************************************************************
	Function:	 GetBitArray/GetBitArrayWithDebugInfo
	Description: Retrieves a client-specified "array" of bits.  The main
				 difference between this function, and the GetBits function
				 is that this one can potentially retrieve more than 32 bits
	************************************************************************/
	void GetBitArray(u8 *array,size_t nBits);
	void GetBitArrayWithDebugInfo(u8 *array,size_t nBits);

	/************************************************************************
	Function:	 GetString/GetStringWithDebugInfo
	Description: Retrieves a null-terminated C-style string from the bit
				 stream
	************************************************************************/
	void GetString(string &str);
	void GetStringWithDebugInfo(string &str);

	/************************************************************************
	Function:	 GetFloat/GetFloatWithDebugInfo()
	Description: Retrieves a floating-point value from the bit stream.  This
				 will always be a 32-bit value.
	************************************************************************/
	f32 GetFloat();
	f32 GetFloatWithDebugInfo();


	/************************************************************************
	*************************************************************************
									Accessors
	*************************************************************************
	************************************************************************/
//	u32 GetBitLength()			const {	return m_bitLength;												};
	size_t GetWritableBits()		const { return (GetAvailSize()<<3)-m_write_bit_off;};
	size_t GetReadableBits()		const { return (GetReadableDataSize()<<3)+(m_write_bit_off-m_read_bit_off);};
	size_t	  GetAvailSize()		const
	{ 
		ssize_t res = (ssize_t)((m_size-m_write_off)-(m_write_bit_off!=0));
		return (size_t)max((ssize_t)0,res);
	}

//	u32 GetByteLength()			const {	return m_byteLength + BITS_TO_BYTES(m_bitPos);					};
//	u32 GetBufferLength()		const {	return m_allocatedBytes;										};
//	u32 GetBitPosition()		const {	return BYTES_TO_BITS(m_bytePos) + m_bitPos;						};
//	u32 GetBytePosition()		const {	return m_bytePos;												};
//	u32 GetBitsLeftInByte()		const {	return BITS_PER_BYTE - m_bitPos;								};

	bool IsByteAligned()		const {	return m_byteAligned;											};
//	bool IsCurByteFull()		const {	return (m_bitPos >= BITS_PER_BYTE);								};
	
	void SetReadPos(u32 pos)	{	m_read_off  = pos >> 3;		m_read_bit_off  = (u8)(pos & 0x7);};
	size_t GetReadPos()			{	return (m_read_off<<3)  + m_read_bit_off;};
	void SetWritePos(u32 pos)	{	m_write_off = pos >> 3;		m_write_bit_off = (u8)(pos & 0x7);};
	
	void SetByteLength(u32 length);
	void UseByteAlignedMode(bool toggle);
	void ByteAlign(bool read_part=true,bool write_part=true);
	void Reset();

	u32 GetPackedBitsLength(u32 nBits, u32 dataBits) const;
	u32 GetBitsLength(u32 nBits, u32 dataBits)       const;
	/************************************************************************
	Function:	 shift_left
	Description: Shifts all contents of the buffer a given amount left
	************************************************************************/
	void shift_left(u32 amount);
	void CompressAndStoreString(const char *str);
	void GetAndDecompressString(string &tgt);

private:
	bool m_byteAligned;
	u8 m_read_bit_off;
	u8 m_write_bit_off;
//	bool IsOverflow() const { return (GetBitPosition() > GetBitLength()); };
};
