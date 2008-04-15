/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: Client.h 253 2006-08-31 22:00:14Z malign $
 */

// Inclusion guards
#pragma once
#ifndef MAPCLIENT_H
#define MAPCLIENT_H

#include <string>
#include <ace/OS_NS_time.h>
#include "ServerManager.h"
#include "Client.h"
#include <vector>
#include <map>

// skeleton class used during authentication
class GameServerInterface;
class MapHandler;
class CharacterHandler;
class SEGSMap;
class NetCommand;

class IMapClient: public IClient
{
public:

	virtual void			AddShortcut(int index, NetCommand *command)=0; // part of user app model
	virtual NetCommand *	GetCommand(int index)=0; // part of user app model
	virtual void			SendCommand(NetCommand *command,...)=0; // part of user app model
	virtual void			setCurrentMap(SEGSMap *pmap)=0;
	virtual SEGSMap *		getCurrentMap()=0;
	virtual void			setCharEntity(Entity *ent)=0;
	virtual Entity *		getCharEntity()=0;

};
class pktCS_SendEntity;
class GamePacket;
class MapClient : public IMapClient
{
	//MapHandler *m_handler;
	u64 m_game_server_acc_id;
	friend class CharacterDatabase;
	void setGameServerAccountId(u64 id){m_game_server_acc_id=id;}
	std::map<int,NetCommand *> m_shortcuts;
	SEGSMap * m_current_map;
	Entity *m_ent;
public:
	MapClient();
	virtual ~MapClient(){};

	GamePacket *	HandleClientPacket(pktCS_SendEntity *ent);
	void			AddShortcut(int index, NetCommand *command);
	NetCommand *	GetCommand(int index) {return m_shortcuts[index];}
	void			SendCommand(NetCommand *command,...);
	void			setCurrentMap(SEGSMap *pmap){m_current_map=pmap;}
	SEGSMap *		getCurrentMap(){return m_current_map;}
	void			setCharEntity(Entity *ent){m_ent=ent;}
	Entity *		getCharEntity(){return m_ent;}
	//void			setHandler(MapHandler *) {m_hamdler=}
//	void setHandler(MapHandler * val) { m_handler = val; }
//	bool serializeFromDb();
//	bool getCharsFromDb();
	void reset();
};

#endif // MAPCLIENT_H
