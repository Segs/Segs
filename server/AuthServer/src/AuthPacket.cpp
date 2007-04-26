/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: AuthPacket.cpp 253 2006-08-31 22:00:14Z malign $
 */

#include "AuthPacket.h"
AuthPacket *AuthPacketFactory::PacketForType(eAuthPacketType type)
{
	switch(type)
	{
	case SMSG_AUTHVERSION:
		return new pktAuthVersion();
	case SMSG_AUTH_ERROR:
		return new pktAuthErrorPacket();
	case CMSG_AUTH_SELECT_DBSERVER:
		return new pktAuthSelectServer();
	case CMSG_DB_CONN_FAILURE:
		return new pktDbErrorPacket;
	case PKT_AUTH_LOGIN_SUCCESS:
		return new pktAuthLoginResponse();
	case CMSG_AUTH_LOGIN:
		return new pktAuthLogin();
	case SMSG_AUTH_SERVER_LIST:
		return new pktAuthServerList;
	case CMSG_AUTH_REQUEST_SERVER_LIST:
		return new pktAuthRequestServerList();
	case PKT_SELECT_SERVER_RESPONSE:
		return new pktAuthSelectServerResponse();
	case MSG_AUTH_UNKNOWN:
		return NULL;
	}
	return NULL;
}
void AuthPacketFactory::Destroy(AuthPacket *what)
{
	delete what;
}
