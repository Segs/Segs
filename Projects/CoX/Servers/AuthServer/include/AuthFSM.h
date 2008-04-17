/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: AuthFSM.h 262 2006-09-15 07:46:05Z nemerle $
 */

// Inclusion guards
#pragma once
#ifndef AUTHFSM_H
#define AUTHFSM_H
#include "AuthOpcodes.h"
/**
AuthFSM_Default is used as mixin for the Protocol class.
*/
class AuthPacket;
class AuthConnection;
class AuthFSM_Default
{

	typedef enum
	{
		CLIENT_UNCONNECTED=0,
		CLIENT_CONNECTED,
		CLIENT_AUTHORIZED,
		CLIENT_SERVSELECT,
		CLIENT_AWAITING_DISCONNECT
	} eClientState;
	static AuthPacket * BuildServerListPacket( void );
	//! this method will create error packet if recv_type!=expect_type, NULL othrwise
	static AuthPacket * ExpectingPacket(eAuthPacketType expect_type,eAuthPacketType recv_type,char *waitng_for_txt); 
private:
	static AuthPacket *auth_error(char,char);
public:
	static AuthPacket *ReceivedPacket(AuthConnection *caller,AuthPacket *pkt);

	static AuthPacket *ConnectionEstablished(AuthConnection *conn);
	static void SendError(AuthConnection *conn,int err);
	static void ConnectionClosed(AuthConnection *conn);
};

#endif // AUTHFSM_H
