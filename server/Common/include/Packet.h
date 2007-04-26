/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: Packet.h 253 2006-08-31 22:00:14Z malign $
 */

/************************************************************************
Class:		 Packet
Author:		 Darawk
Description: The Packet class is an abstract factory for network packets.
************************************************************************/
#pragma once
#include "Base.h"

class Net;
static const u32 maxPacketSize    = 0x5C0;
static const u32 packetHeaderSize = 8;

class Packet
{
public:
	friend class Net;

	Packet();
	Packet(BitStream *stream, bool hasDebugInfo);
	~Packet();

	virtual void serializefrom() { return; };
	virtual void serializeto() { return; };
	
	void serializeandsend(Net *netptr);

	u32     GetBits(u32 nBits);
	void    GetBitArray(u32 nBytes, u8 *array);
	u32     GetPackedBits(u32 nBits);
	string *GetString();

	f32  GetFloat();
	void StoreBits(u32 nBits, u32 dataBits)		  {		m_stream->StoreBits(nBits, dataBits);		};
	void StoreBitArray(u32 nBits, u8 *array)	  {		m_stream->StoreBitArray(nBits, array);		};
	void StorePackedBits(u32 nBits, u32 dataBits) {		m_stream->StorePackedBits(nBits, dataBits); };
	void StoreString(char *str)					  {		m_stream->StoreString(str);					};

	//	Accessors
	////////////////
	u8  *GetBuffer()       const { return m_stream->GetBuffer();		 };
	u32  GetBytePosition() const { return m_stream->GetBytePosition();	 };
	u32  GetBitPosition()  const { return m_stream->GetBitPosition();	 };
	u32  GetPacketLength() const { return m_stream->GetByteLength();	 };
	u32  GetBitLength()	   const { return m_stream->GetBitLength();		 };
	u32  GetBufferLength() const { return m_stream->GetBufferLength();	 };
	u32  GetOpcode()       const { return m_opcode;						 };
	BitStream *GetStream() const { return m_stream;						 };
	bool HasDebugInfo()    const { return m_hasDebugInfo;				 };
	bool HasSiblings()	   const { return (m_numSibs > 0);				 };

	u32  GetPackedBitsLength(u32 len, u32 dataBits) { return m_stream->GetPackedBitsLength(len, dataBits); };
	void SetBufferLength(u32 length) { m_stream->SetByteLength(length); };
	void SetPosition(u32 pos) { m_stream->SetPosition(pos); };

	u32 GetSequenceNumber()  { return m_seqNo;  };
	u32 GetSiblingPosition() { return m_sibPos; };

	void ByteAlign() { m_stream->ByteAlign(); };

protected:

	BitStream *m_stream;
	u32 m_opcode;
	bool m_hasDebugInfo, m_compressed;
	u32 m_checksum;
	u32 m_seqNo;
	u32 m_numSibs;
	u32 m_sibId;
	u32 m_sibPos;

private:
	//	These functions are to be used only by the Net class
	bool ParseHeader();
	void BuildHeader();

	static u32 Checksum(u8 *buf, u32 length);
};

#define REGISTER_PACKET(name, handler) class handler##_Maker : public PacketMaker	\
{																					\
public:																				\
	handler##_Maker() : PacketMaker(cmdName) { };									\
private:																			\
	Packet *makepacket(BitStream *stream, bool hasDebugInfo) const					\
	{																				\
		Packet *pak = new handler##(stream, hasDebugInfo);							\
		pak->serializefrom();														\
		return pak;																	\
	};																				\
};																					\
static const handler##_Maker handler##m_registerthis;

class PacketMaker
{
public:
	PacketMaker(u32 opcode) { Registry()[opcode] = this; };

	static Packet *constructmessage(Packet *pak)
	{
		return constructmessage(pak->GetStream(), pak->HasDebugInfo());
	}

	static Packet *constructmessage(BitStream *stream, bool hasDebugInfo)
	{
		BitStream *newstream = new BitStream(*stream);

		//	We'll have to figure out how to use this opcode later on
		u32 opcode			 = newstream->GetPackedBits(1);
		u32 controlCmd		 = newstream->GetPackedBits(1);
		PacketMaker *pak	 = Registry()[controlCmd];

		try
		{
			return pak->makepacket(newstream, hasDebugInfo);
		}
		catch(...)
		{
			LOG("Packet") << "Packet handler for control command: " << controlCmd << " not found";
		}
        
		return NULL;
	}

private:
	virtual Packet *makepacket(BitStream *stream, bool hasDebugInfo) const = 0;

	static _declspec(noinline) map<u32, PacketMaker *> &Registry()
	{
		static map<u32, PacketMaker *> m_registry;
		return m_registry;
	}
};
