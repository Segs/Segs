/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: GameProtocol.cpp 253 2006-08-31 22:00:14Z malign $
 */

#include "GameProtocol.h"
#include <ace/Log_Msg.h>
#include "PacketCodec.h"
#include "BitStream.h"
#include "CRUDP_Packet.h"
#include "CRUDP_Protocol.h"
#include "PacketBase.h"
#include "GameProtocolHandler.h"

GamePacket *PacketSerializer::serializefrom(CrudP_Packet *tgt)
{
	ACE_ASSERT(m_factory);
	GamePacket *res = m_factory->PacketFromStream(*tgt->GetStream());
	if(res)
	{
		res->serializefrom(*tgt->GetStream());
		//res->dump();
	}
	return res;
}
bool PacketSerializer::serializeto(CrudP_Packet *p,const GamePacket *src)
{
	BitStream *tgt = p->GetStream();
	tgt->StorePackedBits(1,src->m_opcode);
	if(src->m_opcode==0)
		tgt->StorePackedBits(1,static_cast<const ControlPacket *>(src)->m_comm_opcode);
	src->serializeto(*tgt);
	m_factory->Destroy(src);
	return p->GetStream()->getLastError()==0;
}
template<class SERIALIZER_TYPE>
void GameProtocol<SERIALIZER_TYPE>::Received(CrudP_Packet *pak)
{
	GamePacket *gp;
	ACE_ASSERT(pak);
	
	while(pak->GetStream()->GetReadableBits() && (gp = this->serializefrom(pak)))
	{
		m_handler->ReceivePacket(gp);
		delete gp;
	}
	if(pak->GetStream()->GetReadableBits())
	{
		ACE_ASSERT(!"bits_are_left");
	}
	//PacketFactory::Destroy(pak);
}
template<class SERIALIZER_TYPE>
void GameProtocol<SERIALIZER_TYPE>::SendPacket(const GamePacket *pkt)
{	
	CrudP_Packet *res=new CrudP_Packet;
	if(this->serializeto(res,pkt))
		m_link->SendPacket(res);
}
//void GameProtocol<SERIALIZER_TYPE>::setHandler(IGamePacketHandler *ph)
template<class SERIALIZER_TYPE>
void GameProtocol<SERIALIZER_TYPE>::setHandler(LinkCommandHandler *ph)
{
	m_handler = ph;
	m_handler->setProtocol(this);
};
template<class SERIALIZER_TYPE>
PacketCodecNull *GameProtocol<SERIALIZER_TYPE>::getCodec(void)
{
    return m_link->getCodec();
}

template class GameProtocol<PacketSerializer >;
