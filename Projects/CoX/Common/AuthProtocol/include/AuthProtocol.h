/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: AuthProtocol.h 317 2007-01-25 15:44:29Z nemerle $
 */

// Inclusion guards
#pragma once
#ifndef AUTHPROTOCOL_H
#define AUTHPROTOCOL_H

#include "Buffer.h"
#include "AuthOpcodes.h"
#include "AuthPacketCodec.h"
//#include "AuthFSM.h"
class AuthConnection;
class AuthPacket;
class AuthFSM_Default;
class AuthSerializer
{
public:
	AuthSerializer();
	AuthPacket *serializefrom(GrowingBuffer &buffer); // returns null if it's not possible to serialize the packet
	bool serializeto(AuthPacket *pkt,GrowingBuffer &buffer); // returns false if it's not possible to serialize the packet, i.e. buffer to small etc.
	static eAuthPacketType OpcodeToType(u8 opcode,bool direction); // false -> client->server packet.
	AuthPacketCodec m_codec;
	static const int m_protocol_version = 30206;
};
// This allows us to pass pointers to derived Protocol's with mixins 
class IAuthProtocol
{
public:
	virtual ~IAuthProtocol(){}; // my_conn doesn't belong to us, therefore no delete.
	virtual u32 getVersion()=0;
	virtual void ReceivedBytes(GrowingBuffer &buf)=0;
	virtual void setConnection(AuthConnection *conn)=0;
	virtual void Established()=0;
	virtual void Closed()=0;
protected:
	AuthConnection *my_conn;
};
/*
	This class handles every client connection, so it uses only stack variables, and threadsafe data 
	passed to it by	callers.
*/
template<class Auth_FSM>
class AuthProtocol : public IAuthProtocol,public AuthSerializer,public Auth_FSM
{
	void sendPacket(AuthPacket *pkt)
	{
		GrowingBuffer output(0x10000,0,64);
		if(this->serializeto(pkt,output))
		{
			my_conn->sendBytes(output);
		}
		//TODO: handle this error!
		AuthPacketFactory::Destroy(pkt);

	}


public:
	AuthProtocol(){};
	virtual ~AuthProtocol(){};

	void ReceivedBytes(GrowingBuffer &buf)
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

	void Established()
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
	void Closed()
	{
		this->ConnectionClosed(my_conn);
	}

	virtual void setConnection(AuthConnection *conn) {my_conn = conn;};
	virtual u32 getVersion(){return this->m_protocol_version;};

};

#endif // AUTHPROTOCOL_H
