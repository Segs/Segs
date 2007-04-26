/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: PacketBase.cpp 253 2006-08-31 22:00:14Z malign $
 */

#include "PacketBase.h"
GamePacket *ControlPacketFactory::PacketFromStream(u8 opcode, BitStream &bs) const
{
	GamePacket *res=NULL;
	if(opcode==0)
	{
		eControlCodes command_opcode = (eControlCodes)bs.GetPackedBits(1);
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
