/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: authversion.h 253 2006-08-31 22:00:14Z malign $
 */

#pragma once
#include "AuthPacket.h"
#include "Base.h"

class pktAuthVersion : public AuthPacket
{
	u32 seed;
public:
	u32 m_proto_version;
	virtual ~pktAuthVersion(){}
	pktAuthVersion():seed(0),m_proto_version(0)
	{
		m_packet_type = SMSG_AUTHVERSION;
		m_op= 0;
	}
	u16 ExpectedSize(){return 9;}
	u16 serializefrom(GrowingBuffer &buf)
	{
		// might change based on m_proto_version
		ACE_ASSERT(buf.GetReadableDataSize()>=9);
		buf.uGet(m_op);
		buf.uGet(seed);
		buf.uGet(m_proto_version);
		return 5;
	}
	bool serializeto(GrowingBuffer &buf) 
	{
		buf.uPut(m_op);
		buf.uPut(seed);
		buf.uPut(m_proto_version);
		return true;
	}
	virtual u32 GetVersion() const {return m_proto_version;}
	virtual u32 GetSeed() const {return seed;}
	virtual void SetSeed(u32 s) {seed=s;}
};
