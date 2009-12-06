/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

#include "GameServerInterface.h"
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
int	GameServerInterface::getAccessKeyForServer(const ServerHandle<IMapServer> &h_map)
{
	ACE_ASSERT(m_instance);
	return m_instance->getAccessKeyForServer(h_map);
}
bool GameServerInterface::isMapServerReady(const ServerHandle<IMapServer> &h_map)
{
	ACE_ASSERT(m_instance);
	return m_instance->isMapServerReady(h_map);
}
EventProcessor * GameServerInterface::event_target()
{
    ACE_ASSERT(m_instance);
    return m_instance->event_target();
}
