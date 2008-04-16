/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: AuthProtocol.cpp 319 2007-01-26 17:03:18Z nemerle $
 */

#include "AuthProtocol.h"
#include "AuthPacket.h"
#include "AuthConnection.h"
/*! 
	@class AuthSerializer
	@brief Responsible for packet serialization to/from GrowingBuffer
	This class uses m_codec to preproces a packet, and hands of the rest to given packet's serialization logic
 */
static long long  KeyPrepare(const char *co_string)
{
	long long t = 0;
	char *p_llt = (char *)&t;
	//    long tmp_codingval = CodingValue;
	int index = 0;

	while(*co_string)
	{
		char val = *co_string;
		if( index >= 40 ) 
			break;
		p_llt[index&7] = (p_llt[index&7] ^ val);
		co_string ++;
		index ++;
	}
	return t;
}
AuthSerializer::AuthSerializer()
{
	static char key_30207[] = {	0x39, 0x3D, 0x33, 0x2A, 0x32, 0x3B, 0x78, 0x5D,
		0x31, 0x31, 0x73, 0x61, 0x3B, 0x2F, 0x34, 0x73,
		0x64, 0x2E, 0x25, 0x2B, 0x24, 0x40, 0x61, 0x2A,
		0x27, 0x21, 0x32, 0x7A, 0x30, 0x7E, 0x67, 0x60,
		0x7B, 0x7A, 0x5D, 0x3F, 0x6E, 0x38, 0x28, 0};

	if(m_protocol_version==30206)
		m_codec.SetDesKey(KeyPrepare("TEST\0\0\0"));
	else
		m_codec.SetDesKey(KeyPrepare((char *)key_30207));

}
AuthPacket *AuthSerializer::serializefrom(GrowingBuffer &buffer)
{
	u16 packet_size(0);
	u8 *tmp(NULL);
	while(true) // we loop and loop and loop loopy loop through the buffery contents!
	{
		if(buffer.GetReadableDataSize()<=2) // no more bytes for us, so we'll go hungry for a while
			return NULL;
		// And the skies are clear on the Packet Fishing Waters
		buffer.uGet(packet_size); // Ah ha! I smell packet in there!
		if(buffer.GetReadableDataSize()<packet_size) // tis' a false trail capt'n !
		{
			buffer.PopFront(1); // Crew, Dead Slow Ahead ! we're in hunting mode !
			continue;
		}
		// this might be a live packet in there
		tmp = (u8 *)&(buffer.GetBuffer()[2]);
		
		m_codec.XorDecodeBuf(tmp, packet_size+1); // Let's see what's in those murky waters
		eAuthPacketType recv_type = OpcodeToType(tmp[0],false);
		AuthPacket *pkt = AuthPacketFactory::PacketForType(recv_type); // Crow's nest, report !			
		if(!pkt)
		{
			if(buffer.GetReadableDataSize()>2) // False alarm Skipper!
				buffer.uGet(packet_size); // On to next adventure crew. Slow Ahead !
			continue;
		}
		// A catch !
		if(pkt->GetPacketType()==CMSG_AUTH_LOGIN) // Is tis' on of those pesky AuthLogin Packets ?!!?
		{
			// Bring out the Codec Cannon, an' load it with Des
			m_codec.DesDecode(static_cast<u8*>(&tmp[1]),24); // It'll crack it's chitinous armor
		}
		pkt->serializefrom(buffer);
		buffer.PopFront(packet_size+3); //Let's sail away from this depleted fishery.
		return pkt; // And throw our catch to the Cook.
	}
}
bool AuthSerializer::serializeto(AuthPacket *pkt,GrowingBuffer &buffer)
{
	u16 expected_size = pkt->ExpectedSize();
	if(expected_size!=0xFFFF)
	{
		ACE_ASSERT(buffer.GetAvailSize()>=static_cast<u32>(expected_size+2)); // 2 additional bytes for packet size
	}
	else
	{
		ACE_ASSERT(buffer.GetMaxSize()>=0xFFFF); // We need a reaaaalllyyy stretchy buffer here
	}	
	buffer.uPut((u16)(expected_size-1)); // not counting the opcode
	if(!pkt->serializeto(buffer))
		return false;
	if(expected_size==0xFFFF) // we didn't know what size the packet has
		*reinterpret_cast<u16 *>(buffer.GetBuffer()) = (u16)buffer.GetDataSize()-3; // so fix it now (2 for length 1 for opcode)
	// ok now for encoding this
	if(pkt->GetPacketType()!=SMSG_AUTHVERSION)
	{
		m_codec.XorCodeBuf(static_cast<u8 *>(buffer.GetBuffer())+2,buffer.GetDataSize()-2); // opcode gets encrypted
	}
	if(pkt->GetPacketType()!=CMSG_AUTH_LOGIN) 
		return true;
	//only one is encoded
	m_codec.DesCode((u8*)buffer.GetBuffer()+3,24); //only part of packet is des-crypted
	return true;
}

eAuthPacketType AuthSerializer::OpcodeToType( u8 opcode,bool direction )
{
	switch(opcode)
	{	
	case 0:
		if(direction)
			return SMSG_AUTHVERSION;
		else
			return CMSG_AUTH_LOGIN;
	case 2:
			return CMSG_AUTH_SELECT_DBSERVER;
	case 3:
			return CMSG_DB_CONN_FAILURE;
	case 4:
			return CMSG_AUTH_LOGIN;
	case 5:
			return CMSG_AUTH_REQUEST_SERVER_LIST;
	case 6:
		return CMSG_AUTH_LOGIN;
	}
	return MSG_AUTH_UNKNOWN;

}
/*! 
	@class AuthProtocol
	@brief Responsible for binding serializer with authorization finite state machine.

	Turns bytes into AuthFSM::ReceivedPacket calls, and uses serializer to build buffers ready to send.
	Also it handles 'connection Established' condition.
 */

void AuthProtocol::ReceivedBytes(GrowingBuffer &buf)
{
	AuthPacket *res,*pkt = this->serializefrom(buf);
	if(pkt)
	{
		res = this->ReceivedPacket(my_conn,pkt);
		// if there is a response, send it
		if(res)
			sendPacket(res);
	}
}
void AuthProtocol::Established()
{
	AuthPacket *res = this->ConnectionEstablished(my_conn); // this will call static method from current FSM
	if(!res)
	{
		return;
	}
	if(res->GetPacketType()==SMSG_AUTHVERSION)
	{
		u32 seed = 1;
		static_cast<pktAuthVersion *>(res)->SetSeed(seed);
		static_cast<pktAuthVersion *>(res)->m_proto_version = this->m_protocol_version;
		this->m_codec.SetXorKey(seed);
	}
	sendPacket(res);
}
void AuthProtocol::Closed()
{
	this->ConnectionClosed(my_conn);
}
void AuthProtocol::sendPacket(AuthPacket *pkt)
{
	GrowingBuffer output(0x10000,0,64);
	if(this->serializeto(pkt,output))
	{
		my_conn->sendBytes(output);
	}
	//TODO: handle this error!
	AuthPacketFactory::Destroy(pkt);

}

//template class AuthProtocol< AuthSerializer >; // instantiation of protocol 30206
//template class AuthProtocol< AuthSerializer<AuthPacketCodec,30207> >; // instantiation of protocol 30207
