/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: PacketBase.cpp 253 2006-08-31 22:00:14Z malign $
 */
// From opcodes, later on this should be removed in favor of game specific implementations
enum 
{
	COMM_CONTROLCOMMAND				=		0x00,
	COMM_CONNECT						=	0x01,
	SERVER_UPDATE						=	0x02,
};
enum 
{
	CTRL_IDLE = 0x00,
	CTRL_CONNECT,
	CTRL_KEY_REQUEST,
	CTRL_KEY_REPLY,
	CTRL_CONNECTED,	//	?
	CTRL_UNK5,
	CTRL_DISCONNECT,
	CTRL_RESIZE,		//	?
	CTRL_UNK8,
	CTRL_UNK9,
	CTRL_NET_CMD,			//	Contains a textual command string
	CTRL_UNK11,
	CTRL_UNK12,
	CTRL_UNK13
}; 
#include "PacketBase.h"
GamePacket *ControlPacketFactory::PacketFromStream(u8 opcode, BitStream &bs) const
{
	GamePacket *res=NULL;
	if(opcode==0)
	{
		if(bs.GetReadableBits()==0)
			return new pktIdle();
		u8 command_opcode = bs.GetPackedBits(1);
		switch(command_opcode)
		{
		case CTRL_KEY_REQUEST:
		case CTRL_KEY_REPLY:
			res= new pktBlowfishKey(1);
			((ControlPacket *)res)->m_comm_opcode = (u8)command_opcode;
			break;
		case CTRL_IDLE:
			res= new pktIdle();
			break;
		default:
			res=new UnknownControlPacket;
			((ControlPacket *)res)->m_comm_opcode = (u8)command_opcode;
			res->serializefrom(bs);
			res->dump();
			delete res;
			res=NULL;
		}
	}
	return res;
}
void ControlPacketFactory::Destroy(const GamePacket *pkt) const
{
	delete pkt;
}

pktBlowfishKey::pktBlowfishKey( int stage ) /* first step : sending my public key, and waiting for response */
{
	m_opcode=0;
	m_comm_opcode=(u8)CTRL_KEY_REQUEST;
	m_stage = stage;
}

void pktBlowfishKey::serializefrom( BitStream &src )
{
	m_stage = src.GetPackedBits(1);
	src.GetBitArray((u8 *)key,512);
}

void pktBlowfishKey::serializeto( BitStream &tgt ) const
{
	tgt.StorePackedBits(1,m_stage);
	tgt.StoreBitArray((u8 *)key,512);
}

void pktBlowfishKey::dependent_dump()
{
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IKey exchange\n%I{\n")));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    step %d;\n"),m_stage));
	for(int i=0; i<8; i++)
	{
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    key: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x;\n"),key[i],key[i+1],key[i+2],key[i+3],key[i+4],key[i+5],key[i+6],key[i+7]));
	}
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
}

pktSC_Connected::pktSC_Connected()
{
	m_opcode=0;
	m_comm_opcode=(u8)CTRL_CONNECTED;
}
pktSC_Diconnect::pktSC_Diconnect()
{
	m_opcode=0;
	m_comm_opcode=(u8)CTRL_DISCONNECT;
}
void pktSC_Diconnect::dependent_dump()
{
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IDisconnect request\n%I{\n")));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
}
void pktSC_Connected::dependent_dump()
{
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IConnected\n%I{\n")));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
}

pktIdle::pktIdle()
{
	m_opcode=0;
	m_comm_opcode=(u8)CTRL_IDLE;
}

void pktIdle::dependent_dump()
{
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IConnected\n%I{\n")));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
}

void pktCS_Connect::dependent_dump()
{
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IDb Connect\n%I{\n")));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    tickcount 0x%08x;\n"),m_tickcount));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    version 0x%08x;\n"),m_version));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
}

void pktCS_Connect::serializefrom( BitStream &src )
{
	m_tickcount = src.GetPackedBits(1);
	m_version = src.GetPackedBits(1);
}

void pktCS_Connect::serializeto( BitStream &tgt ) const
{
	tgt.StorePackedBits(1, m_tickcount);
	tgt.StorePackedBits(1, m_version);
}

void UnknownControlPacket::serializefrom( BitStream &src )
{
	while((src.GetReadableBits()&7)!=0)
	{
		vals[num_vals++] = src.GetPackedBits(1);
		if(num_vals>31)
			return;
	}
	src.GetBitArray((u8*)arr,src.GetReadableBits());
}

void UnknownControlPacket::dependent_dump()
{
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IUnknown CTRL packet %x\n%I{\n"),m_comm_opcode));
	for(int i=0; i<num_vals; i++)
	{
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    packed val[%d]:0x%08x;\n"),i,vals[i]));
	}
	ACE_HEX_DUMP((LM_DEBUG,(char *)arr,arr_size,"Bit Array"));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
}

ControlPacket::ControlPacket()
{
	m_opcode = 0;
	m_comm_opcode=0;
}

void ControlPacket::dependent_dump()
{
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IControl Packet\n{\n")));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
}

void GamePacket::dump()
{
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IGamePacket\n%I{\n")));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    opcode 0x%08x;\n"),m_opcode));
	ACE_Log_Msg::instance()->inc();
	this->dependent_dump();
	ACE_Log_Msg::instance()->dec();
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
}

void UnknownGamePacket::dependent_dump()
{
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IUnknown packet\n%I{\n")));
	for(size_t i=0; i<num_vals; i++)
	{
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    packed val[%d]:0x%08x;\n"),i,vals[i]));
	}
	ACE_HEX_DUMP((LM_DEBUG,(char *)arr,arr_size,"Bit Array"));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
}

UnknownGamePacket::UnknownGamePacket()
{
	num_vals=arr_size=0;
}

void UnknownGamePacket::serializefrom( BitStream &src )
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
}

GamePacket * PacketFactory::PacketFromStream( BitStream &bs ) const
{
	s32 opcode=bs.GetPackedBits(1);
	return this->PacketFromStream((u8)opcode,bs);
}