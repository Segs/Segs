/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: ClientHandler.h 319 2007-01-26 17:03:18Z nemerle $
 */

// Inclusion guards
#pragma once
#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H

#include "types.h"
#include "ServerProtocolHandler.h"
class CharacterClient;
class GameServer;
class CharacterDatabase;
class CharacterHandler : public ServerCommandHandler
{
		CharacterClient *	m_client;
		GameServer *		m_server;
private:
		bool				sendNotAuthorized();
		bool				sendAllowConnection();
		bool				sendWrongVersion(u32 version);
		// 'event' handlers
		void				onCharacterUpdate	( GamePacket * pak );
		void				onCharacterDelete	( GamePacket * pak );
		void				onOnterMap			( GamePacket * pak );
		bool				onServerUpdate		( GamePacket * pak );

public:
							CharacterHandler(GameServer *);
virtual						~CharacterHandler();

virtual bool				ReceivePacket(GamePacket *pak);

CharacterDatabase *	getDb();
		void				setClient(IClient *cl);
};

#endif // CLIENTHANDLER_H
