/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: GameServerInterface.cpp 301 2006-12-26 15:50:44Z nemerle $
 */

#include "GameServerInterface.h"
#include "GameServer/include/GameServer.h"
#include <ace/Log_Msg.h>

bool GameServerInterface::Run()
{
	ACE_ASSERT(m_instance);
	return m_instance->Run();
}
bool GameServerInterface::ReadConfig(const std::string &name)
{
	ACE_ASSERT(m_instance);
	return m_instance->ReadConfig(name);
}
bool GameServerInterface::ShutDown(const std::string &reason)
{
	ACE_ASSERT(m_instance);
	return m_instance->ShutDown(reason);
}
bool GameServerInterface::Online()
{ 
	ACE_ASSERT(m_instance);
	return m_instance->Online();
}
u8 GameServerInterface::getId()
{ 
	ACE_ASSERT(m_instance);
	return m_instance->getId();
}
u16 GameServerInterface::getCurrentPlayers()
{ 
	ACE_ASSERT(m_instance);
	return m_instance->getCurrentPlayers();
}
u16 GameServerInterface::getMaxPlayers()
{ 
	ACE_ASSERT(m_instance);
	return m_instance->getMaxPlayers();
}
u8 GameServerInterface::getUnkn1()
{ 
	ACE_ASSERT(m_instance);
	return m_instance->getUnkn1();
}
u8 GameServerInterface::getUnkn2()
{ 
	ACE_ASSERT(m_instance);
	return m_instance->getUnkn2();
}
std::string GameServerInterface::getName()
{ 
	ACE_ASSERT(m_instance);
	return m_instance->getName();
}
const ACE_INET_Addr &GameServerInterface::getAddress()
{ 
	ACE_ASSERT(m_instance);
	return m_instance->getAddress();
}
u32 GameServerInterface::ExpectClient(const ACE_INET_Addr &from,u64 id,u16 access_level)
{
	ACE_ASSERT(m_instance);
	return m_instance->ExpectClient(from,id,access_level);
}
void GameServerInterface::checkClientConnection(u64 id)
{
	ACE_ASSERT(m_instance);
	m_instance->checkClientConnection(id);
}
bool GameServerInterface::isClientConnected(u64 id)
{
	ACE_ASSERT(m_instance);
	return m_instance->isClientConnected(id);
}
int	GameServerInterface::GetAccessKeyForServer(const ServerHandle<MapServer> &h_map)
{
	ACE_ASSERT(m_instance);
	return m_instance->GetAccessKeyForServer(h_map);
}
bool GameServerInterface::MapServerReady(const ServerHandle<MapServer> &h_map)
{
	ACE_ASSERT(m_instance);
	return m_instance->MapServerReady(h_map);
}
