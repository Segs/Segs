/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: ClientHandler.h 319 2007-01-26 17:03:18Z nemerle $
 */

#pragma once
#include "types.h"
#include "GameProtocolHandler.h"
class CharacterClient;
class GameServer;
class CharacterDatabase;
class CharacterHandler : public LinkCommandHandler
{
		CharacterClient *	m_client;
		GameServer *		m_server;
private:
		bool				sendNotAuthorized();
		bool				sendAllowConnection();
		bool				sendWrongVersion(u32 version);
public:
							CharacterHandler(GameServer *);
virtual						~CharacterHandler();

virtual bool				ReceivePacket(GamePacket *pak);
		CharacterDatabase *	getDb();
		void				setClient(IClient *cl);
};
