/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: CRUDP_Packet.cpp 253 2006-08-31 22:00:14Z malign $
 */

#include "Base.h"
#include "CRUDP_Packet.h"
//#include "Net.h"

CrudP_Packet::CrudP_Packet()
{ 
	m_stream		= new BitStream(maxPacketSize);
	m_hasDebugInfo	= false;
	m_finalized		= false;
	m_compressed	= false;
	m_checksum		= 0;
	m_seqNo			= 0;
	m_numSibs		= 0;
	m_sibId			= 0;
	m_sibPos		= 0;
}

CrudP_Packet::CrudP_Packet(BitStream *stream, bool hasDebugInfo)
{
	m_stream		= stream;
	m_hasDebugInfo	= hasDebugInfo;
	m_finalized		= false;
	m_compressed	= false;
	m_checksum		= 0;
	m_seqNo			= 0;
	m_numSibs		= 0;
	m_sibId			= 0;
	m_sibPos		= 0;

}
CrudP_Packet::CrudP_Packet(const CrudP_Packet &from)
{
	m_stream		= NULL; // we're not copying streams !!!
	m_hasDebugInfo	= from.m_hasDebugInfo;
	m_finalized		= from.m_finalized;
	m_compressed	= from.m_compressed;
	m_checksum		= from.m_checksum;
	m_seqNo			= from.m_seqNo;
	m_numSibs		= from.m_numSibs;
	m_sibId			= from.m_sibId;
	m_sibPos		= from.m_sibPos;

}
CrudP_Packet::~CrudP_Packet()
{ 
	if(m_stream) delete m_stream;
}

/*
void CrudP_Packet::serializeandsend(Net *netptr)
{
	serializeto();				//	Fills in the data of the packet
	netptr->SendPacket(this);
}
*/


/*
bool CrudP_Packet::ParseHeader()
{
	u32 bitLength = GetBits(32);	//	We don't really need this value for anything
	m_checksum    = GetBits(32);
	u32 realcsum  = Checksum(GetBuffer() + packetHeaderSize, GetPacketLength() - packetHeaderSize);
	
	if(m_checksum != realcsum)
	{
		LOG("CrudP_Packet") << "CrudP_Packet buffer checksum failed";
		return false;
	}

	m_hasDebugInfo = (bool)GetBits(1);
	m_seqNo        = GetBits(32);
	m_numSibs      = GetPackedBits(1);

	if(m_numSibs > 0)
	{
		m_sibPos = GetPackedBits(1);		//	The purpose/name of this value is currently unknown
		m_sibId  = GetBits(32);
	}

    return true;
}
*/

/*
void CrudP_Packet::BuildHeader()
{
	//	Reserve space for the final bitLength and checksum
	StoreBits(32, 0);
	StoreBits(32, 0);

	StoreBits(1, HasDebugInfo());
	StoreBits(32, m_seqNo);

	StorePackedBits(1, m_numSibs);
	if(m_numSibs > 0)
	{
		StorePackedBits(1, m_sibPos);
		StoreBits(32, m_sibId);
	}
}
*/

/*
u32 CrudP_Packet::Checksum(u8 *buf, u32 length)
{
	u16 v1 = 1, v2 = 0;
	u32 sum1 = v1, sum2 = v2;

	for(u32 i = 0; i < length; i++)
	{
		sum1 += buf[i];
		sum2 += sum1;

		if(sum1 >= 0xFFF1) sum1 -= 0xFFF1;
	}

	sum2 %= 0xFFF1;

	v1 = (u16)sum1;
	v2 = (u16)sum2;

	return MAKELONG(htons(v2), htons(v1));
}
*/


u32 CrudP_Packet::GetBits(u32 nBits)
{
	if(m_hasDebugInfo)
		return m_stream->GetBitsWithDebugInfo(nBits);
	else
		return m_stream->GetBits(nBits); 
}

void CrudP_Packet::GetBitArray(u32 nBytes, u8 *array)
{
	if(m_hasDebugInfo)
		return m_stream->GetBitArrayWithDebugInfo(array,nBytes);
	else
		return m_stream->GetBitArray(array,nBytes);
};

u32 CrudP_Packet::GetPackedBits(u32 nBits) 
{ 
	if(m_hasDebugInfo)
		return m_stream->GetPackedBitsWithDebugInfo(nBits);
	else
		return m_stream->GetPackedBits(nBits); 
}

void CrudP_Packet::GetString(string &tgt) 
{
	if(m_hasDebugInfo)
		m_stream->GetStringWithDebugInfo(tgt);
	else
		m_stream->GetString(tgt); 
}

f32 CrudP_Packet::GetFloat()
{
	if(m_hasDebugInfo)
		return m_stream->GetFloatWithDebugInfo();
	else
		return m_stream->GetFloat();
}
void CrudP_Packet::setContents(const BitStream &t)
{
	if(m_stream)
		*m_stream = t;
	else
		m_stream = new BitStream(t);
}
