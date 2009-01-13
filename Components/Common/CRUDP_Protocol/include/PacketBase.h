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
//#include "opcodes/Opcodes.h"
//#include "opcodes/ControlCodes.h"
class GameServer;
class Client;
class GamePacket
{
public:
	u8 m_opcode;
	virtual ~GamePacket(){}
	void dump();
	//eGameOpcode getType() const {return (eGameOpcode)m_opcode;}
	u8 getType() const {return m_opcode;}
	virtual void serializefrom(BitStream &src)=0;
	virtual void serializeto(BitStream &src) const=0;
protected:
	virtual void dependent_dump()=0;

};
class UnknownGamePacket : public GamePacket
{
protected:
	virtual void dependent_dump();

public:
	virtual ~UnknownGamePacket(){}
	UnknownGamePacket();	
	virtual void serializefrom(BitStream &src);;
	virtual void serializeto(BitStream &) const {ACE_ASSERT(false);};
	u32 vals[32];
	u8 arr[512];
	size_t num_vals;
	size_t arr_size;
};
class ControlPacket : public GamePacket
{
protected:
	ControlPacket();
	virtual ~ControlPacket(){}
	virtual void dependent_dump();
public:
	u8 m_comm_opcode;
	virtual void serializefrom(BitStream &) {}
};
class UnknownControlPacket : public ControlPacket
{
protected:
	virtual void dependent_dump();

public:
	UnknownControlPacket()
	{
		num_vals=arr_size=0;
	}
	virtual ~UnknownControlPacket(){}
	virtual void serializefrom(BitStream &src);;
	virtual void serializeto(BitStream &) const {ACE_ASSERT(false);};
	u32 vals[32];
	u8 arr[512];
	int num_vals;
	int arr_size;
};


class pktBlowfishKey : public ControlPacket
{
protected:
	virtual void dependent_dump();
public:
	pktBlowfishKey(int stage); // first step : sending my public key, and waiting for response;
	virtual ~pktBlowfishKey(){}
	virtual void serializefrom(BitStream &src);
	virtual void serializeto(BitStream &tgt) const;
	int m_stage;
	u8 key[64];

};
class pktSC_Connected : public ControlPacket
	
{
protected:
	virtual void dependent_dump();
public:
	virtual ~pktSC_Connected (){}
	pktSC_Connected();
	virtual void serializefrom(BitStream &) {}
	virtual void serializeto(BitStream &) const {}
};
class pktSC_Diconnect : public ControlPacket

{
protected:
	virtual void dependent_dump();
public:
	virtual ~pktSC_Diconnect(){}
	pktSC_Diconnect();
	virtual void serializefrom(BitStream &) {}
	virtual void serializeto(BitStream &) const {}
};

class pktIdle : public ControlPacket
{
protected:
	virtual void dependent_dump();
public:
	pktIdle();
	virtual void serializefrom(BitStream &) {}
	virtual void serializeto(BitStream &) const {}
};
class pktCS_Connect : public GamePacket	
{
protected:
	virtual void dependent_dump();
public:
	virtual ~pktCS_Connect(){}
	virtual void serializefrom(BitStream &src);
	virtual void serializeto(BitStream &tgt) const;
	u32 m_tickcount;
	u32 m_version;
};

class PacketFactory
{
public:
	virtual ~PacketFactory(){}
	virtual GamePacket *PacketFromStream(u8 opcode,BitStream &bs)const=0;
	virtual GamePacket *PacketFromStream(BitStream &bs) const;;
	virtual void Destroy(const GamePacket *) const=0 ;
};
class ControlPacketFactory : public PacketFactory
{
public:
        virtual ~ControlPacketFactory(){}
	virtual GamePacket *PacketFromStream(u8 opcode,BitStream &bs) const=0;
	virtual void Destroy(const GamePacket *) const;
};
