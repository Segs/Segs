/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: AuthPacket.h 253 2006-08-31 22:00:14Z malign $
 */

#pragma once
#include "Base.h"
#include <ace/Log_Msg.h>
#include "AuthOpcodes.h"
//	Packets

class AuthNet;
// this class knows how to translate stream->packet,packet->stream
class AuthPacket
{
public:
	AuthPacket() { };
	virtual ~AuthPacket() { };
	virtual u16 ExpectedSize()=0; // if this returns 0xFFFF then packet size is variable
	virtual u16 serializefrom(GrowingBuffer &buf)=0; // those must be overridden in derived classes
	virtual bool serializeto(GrowingBuffer &buf)=0; // this method assumes that it has a valid buffe it can write to
	
	//	Accessors
	//////////////
	virtual eAuthPacketType GetPacketType() const {return m_packet_type;};
protected:
	u8 m_op;
	eAuthPacketType m_packet_type;
	u8 GetOpcode() const { return m_op; };
	int m_proto_version; // this allows derived classes to handle various auth protocol versions
//	AuthNet *m_pMyConnection; // multiple authentication streams will need this
};
class pktAuthErrorPacket : public AuthPacket
{
public:
	pktAuthErrorPacket()
	{
		m_op = 1;
		m_reason= 0;
		m_packet_type = SMSG_AUTH_ERROR;
	}
	virtual ~pktAuthErrorPacket(){}
	u16 ExpectedSize(){return 2;}
	u16 serializefrom(GrowingBuffer &buf)
	{
		ACE_ASSERT(buf.GetReadableDataSize()>=2);
		buf.uGet(m_op); // 1,2,5,6
		buf.uGet(m_reason);
		return 2;
	}
	bool serializeto(GrowingBuffer &buf) 
	{
		buf.uPut(m_op);
		buf.uPut(m_reason);
		return true;
	}
	void setError(u8 error,u8 reason)
	{
		m_op = error;
		m_reason = reason;
	}
	u8 m_reason;
};
class pktDbErrorPacket : public AuthPacket
{
public:
	pktDbErrorPacket()
	{
		m_op = 3;
	}
	virtual ~pktDbErrorPacket(){}
	u16 ExpectedSize(){return 2;}
	u16 serializefrom(GrowingBuffer &buf)
	{
		ACE_ASSERT(buf.GetReadableDataSize()>=9);
		buf.uGet(m_op); // 1,2,5,6
		buf.uGetBytes(err_arr,8);
		return 9;
	}
	bool serializeto(GrowingBuffer &buf) 
	{
		buf.uPut(m_op);
		buf.uPutBytes(err_arr,8);
		return true;
	}
	void setError(u8 error,u8 reason)
	{
		m_op = error;
		m_reason=reason;
	}
	u8 m_reason;
	u8 err_arr[8];
};
#include "authversion.h"
#include "authserverlist.h"
#include "authselectserverresponse.h"
#include "authloginresponse.h"
#include "authrequestserverlist.h"
#include "authlogin.h"
#include "authselectserver.h"
// later this class will use Memory Pools for all packet types
class AuthPacketFactory
{
public:
	static AuthPacket *PacketForType(eAuthPacketType type);
	static void Destroy(AuthPacket *what);
};
