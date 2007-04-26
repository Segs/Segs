/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: AuthFSM.h 262 2006-09-15 07:46:05Z nemerle $
 */

#pragma once
/**
AuthFSM_Default is used as mixin for the Protocol class.
*/
class AuthPacket;
class ClientConnection;
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
private:
	static AuthPacket *auth_error(char,char);
public:
	static AuthPacket *ReceivedPacket(ClientConnection *caller,AuthPacket *pkt);
	static AuthPacket *ConnectionEstablished(ClientConnection *conn);
	static void SendError(ClientConnection *conn,int err);
	static void ConnectionClosed(ClientConnection *conn);
};
