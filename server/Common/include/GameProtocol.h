/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: GameProtocol.h 253 2006-08-31 22:00:14Z malign $
 */

#pragma once
#include <deque>
#include <algorithm>

#include <map>
#include <set>
#include "CRUDP_Packet.h"
using namespace std;
#ifndef WIN32
#include <ext/hash_map>
using namespace __gnu_cxx;
#else
#include <hash_map>
using namespace stdext;
#endif
class GamePacket;
class PacketFactory;
class IGameProtocol;
class PacketCodecNull;
class CrudP_Protocol;
//class IGamePacketHandler;
class LinkCommandHandler;
class PacketSerializer
{
public:
	//CrudP_Packet * IncomingPreProcess(BitStream &src); // base packet is constructed, -> Collector
	//CrudP_Packet * OutgoingPreProcess(BitStream &tgt);
	GamePacket *serializefrom(CrudP_Packet *);
	bool serializeto(CrudP_Packet *p,const GamePacket *src);
protected:
	PacketFactory *m_factory;
};
/*
mixins are used here because this must class be as efficient as possible without sacrificing readability
This class has one mixin which is used to serialize concrete packets to/from bitstreams: 
	And two strategy objects:
		- Producer/Consumer of GamePackets
		- Producer/Consumer of BitStream Data

	Usually incoming packets are processed like this:
		Serializer pre-processes a packet
		Packet is pushed into a scheduler.
		If there are available packets in the scheduler, they're passed back to serializer for final de-serialization
		finished packet is passed to IGamePacketHandler
	Outgoing packet is processed like this:
		IGamePacketHandler calls SendPacket();
		outgoing packet is passed to serializer, which produces clean BitStream payload.
		LinkProtocol : 
		[split]

		Packet -> Serializer -> BitStream 

		this BitStream is passed to collector, which splits it if it's needed,fills are
		if outgoing queue size is greater then 0, then this Net instance schedules a timer which will send those packets after a given period
*/
// this defines the basic interface for GameProtocol templates
class IGameProtocol
{
public:

virtual						~IGameProtocol(){}
//virtual void				setHandler(IGamePacketHandler *)=0;
virtual void				setHandler(LinkCommandHandler *)=0;
virtual void				Received(CrudP_Packet *p)=0;
virtual void				SendPacket(const GamePacket *)=0;
virtual PacketCodecNull *	getCodec()=0;
virtual CrudP_Protocol *	getCrudP()=0;
virtual void				setCrudP(CrudP_Protocol *l)=0;
virtual void				setPacketFactory(PacketFactory *f)=0;
};

template<class SERIALIZER_TYPE>
class GameProtocol : public SERIALIZER_TYPE,public IGameProtocol
{
public:
							GameProtocol(){}
virtual						~GameProtocol(){};
virtual void				Received(CrudP_Packet *p);
virtual void				SendPacket(const GamePacket *);
virtual PacketCodecNull *	getCodec();
		CrudP_Protocol *	getCrudP(){return m_link;}
		void				setCrudP(CrudP_Protocol *l){m_link=l;}
virtual void				setHandler(LinkCommandHandler *ph);
virtual void				setPacketFactory(PacketFactory *f){SERIALIZER_TYPE::m_factory=f;}
protected:
//	IGamePacketHandler *m_handler;
	LinkCommandHandler *m_handler;
	CrudP_Protocol *m_link;
};
