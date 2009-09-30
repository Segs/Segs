/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: MapHandler.h 319 2007-01-26 17:03:18Z nemerle $
 */

// Inclusion guards
#pragma once
#ifndef MAPHANDLER_H
#define MAPHANDLER_H

#include "ServerProtocolHandler.h"
class MapClient;
class MapServer;
class IClient;
class MapHandler : public ServerCommandHandler
{
	MapClient *m_client;
	MapServer *m_server;
public:
					MapHandler(MapServer *srv);
virtual	bool		ReceivePacket(GamePacket *pak);
		MapServer * getServer() const { return m_server; }
		void		setServer(MapServer * val) { m_server = val; }
		void		setClient(IClient *cl);

};

#endif // MAPHANDLER_H
