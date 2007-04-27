/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: AuthProtocol.h 317 2007-01-25 15:44:29Z nemerle $
 */

#pragma once
#include "Buffer.h"
#include "AuthOpcodes.h"
#include "AuthPacketCodec.h"
#include "AuthFSM.h"
class ClientConnection;
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
	virtual void setConnection(ClientConnection *conn)=0;
	virtual void Established()=0;
	virtual void Closed()=0;
protected:
	ClientConnection *my_conn;
};
/*
	This class handles every client connection, so it uses only stack variables, and threadsafe data 
	passed to it by	callers.
*/
class AuthProtocol : public IAuthProtocol,public AuthSerializer,public AuthFSM_Default
{
	void sendPacket(AuthPacket *);
public:
	AuthProtocol(){};
	virtual ~AuthProtocol(){};
	void Established();
	void Closed();
	void ReceivedBytes(GrowingBuffer &buf);
	virtual void setConnection(ClientConnection *conn) {my_conn = conn;};
	virtual u32 getVersion(){return this->m_protocol_version;};

};
