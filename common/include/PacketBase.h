/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: PacketBase.h 319 2007-01-26 17:03:18Z nemerle $
 */

#pragma once
#include <ace/INET_Addr.h>
#include <ace/Log_Msg.h>
#include <string>
#include "Base.h"
#include "BitStream.h"
#include "opcodes/Opcodes.h"
#include "opcodes/ControlCodes.h"
class GameServer;
class Client;
class GamePacket
{
public:
	u8 m_opcode;
	virtual ~GamePacket(){}
	void dump()
	{
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IGamePacket\n%I{\n")));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    opcode 0x%08x;\n"),m_opcode));
		ACE_Log_Msg::instance()->inc();
		this->dependent_dump();
		ACE_Log_Msg::instance()->dec();
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
	}
	eGameOpcode getType() const {return (eGameOpcode)m_opcode;}
	virtual void serializefrom(BitStream &src)=0;
	virtual void serializeto(BitStream &src) const=0;
protected:
	virtual void dependent_dump()=0;

};
class UnknownGamePacket : public GamePacket
{
protected:
	virtual void dependent_dump()
	{
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IUnknown packet\n%I{\n")));
	for(size_t i=0; i<num_vals; i++)
	{
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    packed val[%d]:0x%08x;\n"),i,vals[i]));
	}
	ACE_HEX_DUMP((LM_DEBUG,(char *)arr,arr_size,"Bit Array"));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
	}

public:
	virtual ~UnknownGamePacket(){}
	UnknownGamePacket()
	{
		num_vals=arr_size=0;
	}	
	virtual void serializefrom(BitStream &src)
	{
		while((src.GetReadableBits()&7)!=0)
		{
			vals[num_vals++] = src.GetPackedBits(1);
			if(num_vals>31)
				return;
		}
		arr_size = src.GetReadableBits()>>3;
		if(arr_size>512)
		arr_size = 512;
		src.GetBitArray((u8*)arr,arr_size<<3);
	};
	virtual void serializeto(BitStream &) const {ACE_ASSERT(false);};
	u32 vals[32];
	u8 arr[512];
	size_t num_vals;
	size_t arr_size;
};
class ControlPacket : public GamePacket
{
protected:
	ControlPacket()
	{
		m_opcode = 0;
		m_comm_opcode=0;
	}
	virtual ~ControlPacket(){}
	virtual void dependent_dump()
	{
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IControl Packet\n{\n")));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
	}
public:
	u8 m_comm_opcode;
	virtual void serializefrom(BitStream &) {}
};
class UnknownControlPacket : public ControlPacket
{
protected:
	virtual void dependent_dump()
	{
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IUnknown CTRL packet %x\n%I{\n"),m_comm_opcode));
		for(int i=0; i<num_vals; i++)
		{
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    packed val[%d]:0x%08x;\n"),i,vals[i]));
		}
		ACE_HEX_DUMP((LM_DEBUG,(char *)arr,arr_size,"Bit Array"));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
	}

public:
	UnknownControlPacket()
	{
		num_vals=arr_size=0;
	}
	virtual ~UnknownControlPacket(){}
	virtual void serializefrom(BitStream &src)
	{
		while((src.GetReadableBits()&7)!=0)
		{
			vals[num_vals++] = src.GetPackedBits(1);
			if(num_vals>31)
				return;
		}
		src.GetBitArray((u8*)arr,src.GetReadableBits());
	};
	virtual void serializeto(BitStream &) const {ACE_ASSERT(false);};
	u32 vals[32];
	u8 arr[512];
	int num_vals;
	int arr_size;
};


class pktBlowfishKey : public ControlPacket
{
protected:
	virtual void dependent_dump()
	{
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IKey exchange\n%I{\n")));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    step %d;\n"),m_stage));
		for(int i=0; i<8; i++)
		{
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    key: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x;\n"),key[i],key[i+1],key[i+2],key[i+3],key[i+4],key[i+5],key[i+6],key[i+7]));
		}
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
	}
public:
	pktBlowfishKey(int stage) // first step : sending my public key, and waiting for response
	{
		m_opcode=0;
		m_comm_opcode=(u8)CTRL_KEY_REQUEST;
		m_stage = stage;
	}
	virtual ~pktBlowfishKey(){}
	virtual void serializefrom(BitStream &src)
	{
		m_stage = src.GetPackedBits(1);
		src.GetBitArray((u8 *)key,512);
	}
	virtual void serializeto(BitStream &tgt) const
	{
		tgt.StorePackedBits(1,m_stage);
		tgt.StoreBitArray((u8 *)key,512);
	}
	int m_stage;
	u8 key[64];

};
class pktConnected : public ControlPacket
	
{
protected:
	virtual void dependent_dump()
	{
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IConnected\n%I{\n")));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
	}
public:
	virtual ~pktConnected (){}
	pktConnected()
	{
		m_opcode=0;
		m_comm_opcode=(u8)CTRL_CONNECTED;
	}
	virtual void serializefrom(BitStream &) {}
	virtual void serializeto(BitStream &) const {}
};
class pktIdle : public ControlPacket
{
protected:
	virtual void dependent_dump()
	{
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IConnected\n%I{\n")));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
	}
public:
	pktIdle()
	{
		m_opcode=0;
		m_comm_opcode=(u8)CTRL_IDLE;
	}
	virtual void serializefrom(BitStream &) {}
	virtual void serializeto(BitStream &) const {}
};
class pktCS_Connect : public GamePacket	
{
protected:
	virtual void dependent_dump()
	{
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IDb Connect\n%I{\n")));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    tickcount 0x%08x;\n"),m_tickcount));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    version 0x%08x;\n"),m_version));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
	}
public:
	virtual ~pktCS_Connect(){}
	virtual void serializefrom(BitStream &src)
	{
		m_tickcount = src.GetPackedBits(1);
		m_version = src.GetPackedBits(1);
	}
	virtual void serializeto(BitStream &tgt) const
	{
		tgt.StorePackedBits(1, m_tickcount);
		tgt.StorePackedBits(1, m_version);

	}
	u32 m_tickcount;
	u32 m_version;
};

class PacketFactory
{
public:
	virtual ~PacketFactory(){}
	virtual GamePacket *PacketFromStream(u8 opcode,BitStream &bs)const=0;
	virtual GamePacket *PacketFromStream(BitStream &bs) const
	{
		s32 opcode=bs.GetPackedBits(1);
		return this->PacketFromStream((u8)opcode,bs);
	};
	virtual void Destroy(const GamePacket *) const=0 ;
};
class ControlPacketFactory : public PacketFactory
{
public:
        virtual ~ControlPacketFactory(){}
	virtual GamePacket *PacketFromStream(u8 opcode,BitStream &bs) const=0;
	virtual void Destroy(const GamePacket *) const;
};
