/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

#pragma once
#include "Base.h"
#include <ace/Log_Msg.h>
#include "BitStream.h"
#include <set>
#include <list>
//#include "Opcodes.h"

class PacketCollector;
static const u32 maxPacketSize    = 0x5C0;
static const u32 packetHeaderSize = 8;
class CrudP_Packet
{
public:
	friend class PacketCollector;

	CrudP_Packet();
	explicit CrudP_Packet(const CrudP_Packet &);
	CrudP_Packet(BitStream *stream, bool hasDebugInfo);
	~CrudP_Packet();

	u32  GetBits(u32 nBits);
	void GetBitArray(u32 nBytes, u8 *array);
	u32  GetPackedBits(u32 nBits);
	void GetString(std::string &str);

	f32  GetFloat();
	void StoreBits(u32 nBits, u32 dataBits)		  {		m_stream->StoreBits(nBits, dataBits);		};
	void StoreBitArray(u8 *array, u32 nBits)	  {		m_stream->StoreBitArray(array,nBits);		};
	void StorePackedBits(u32 nBits, u32 dataBits) {		m_stream->StorePackedBits(nBits, dataBits); };
	void StoreString(const char *str)			  {		m_stream->StoreString(str);					};
	void CompressAndStoreString(const char *str);

	//	Sets the packet to the "finalized" state.
	//	This means that it's header is built, acks
	//	are stored in it, etc..
	void SetFinalized() { m_finalized = true; };

	//	Accessors
	//////////////////////////////////////////////////////////////////////////
	u8  *       GetBuffer()       const { return (u8 *)m_stream->GetBuffer();	 };
	size_t      GetPacketLength() const { return m_stream->GetReadableDataSize();};
	BitStream * GetStream()		 { return m_stream;						 };
	bool        HasDebugInfo()    const { return m_hasDebugInfo;				 };
	bool        getIsCompressed() const { return m_compressed;					 };
	bool        HasSiblings()	   const { return (m_numSibs > 0);				 };
	bool        IsFinalized()	   const { return m_finalized;					 };

	u32  GetPackedBitsLength(u32 len, u32 dataBits) { return m_stream->GetPackedBitsLength(len, dataBits); };
	void SetBufferLength(u32 length) { m_stream->SetByteLength(length); };

	u32 GetSequenceNumber()		const { return m_seqNo;  };
	u32 GetSiblingPosition()	const { return m_sibPos; };
	u32 getNumSibs()			const { return m_numSibs;}
	u32 getSibId()				const { return m_sibId;}
	u32 getSibPos()				const { return m_sibPos;}
	void setContents(const BitStream &t);
	void ByteAlign() { m_stream->ByteAlign(); };
	void SetStream(BitStream *stream) { m_stream = stream; };
	void SetHasDebugInfo(bool hasDebugInfo) { m_hasDebugInfo = hasDebugInfo; };
	void SetIsCompressed(bool compressed) { m_compressed = compressed; };
	void setSeqNo(u32 n) {m_seqNo=n;}
	void setNumSibs(u32 n) {m_numSibs=n;}
	void setSibId(u32 n) {m_sibId=n;}
	void setSibPos(u32 n) {m_sibPos=n;}
	void addAck(u32 id)
	{
		m_acks.insert(id);
	}
	u32 getNextAck();
	size_t getNumAcks()
	{
		return m_acks.size();
	}
	void dump();
	u32 m_checksum;
protected:

	BitStream *m_stream;
	bool m_hasDebugInfo, m_compressed, m_finalized;
	u32 m_seqNo;
	u32 m_numSibs;
	u32 m_sibId;
	u32 m_sibPos;
	std::set<u32> m_acks;
};
typedef std::list<CrudP_Packet *> lCrudP_Packet;
typedef std::vector<CrudP_Packet *> vCrudP_Packet;
typedef vCrudP_Packet::iterator ivCrudP_Packet;

