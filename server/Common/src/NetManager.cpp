/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: NetManager.cpp 253 2006-08-31 22:00:14Z malign $
 */

#include "NetManager.h"
#include "PacketCodec.h"
#include "CRUDP_Protocol.h"
#include "GameProtocol.h"
/*
IGameProtocol *GameProtocolManager::getProtocolFor(ACE_INET_Addr &clientaddr)
{
	static GameProtocol<PacketSerializer> a;
	CrudP_Protocol *crudp = new CrudP_Protocol;
	a.setHandler(new HelloHandler);
	a.setCrudP(crudp);
	crudp->setCodec(new PacketCodec);
	crudp->setGameLayer(&a);
	return &a;
}
*/
IGameProtocol *GameProtocolManager::CreateGameProtocol()
{
	GameProtocol<PacketSerializer> *res = new GameProtocol<PacketSerializer>;
	CrudP_Protocol *crudp = new CrudP_Protocol;
	res->setCrudP(crudp);
	crudp->setCodec(new PacketCodecNull);
	crudp->setGameLayer(res);
	return res;
}
