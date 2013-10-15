/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */
#include <cassert>
#include "GameServerInterface.h"
#include <ace/Log_Msg.h>

bool GameServerInterface::Run()
{
        assert(m_instance);
        return m_instance->Run();
}
bool GameServerInterface::ReadConfig(const std::string &name)
{
        assert(m_instance);
        return m_instance->ReadConfig(name);
}
bool GameServerInterface::ShutDown(const std::string &reason)
{
        assert(m_instance);
        return m_instance->ShutDown(reason);
}
bool GameServerInterface::Online()
{
        assert(m_instance);
        return m_instance->Online();
}
uint8_t GameServerInterface::getId()
{
        assert(m_instance);
        return m_instance->getId();
}
uint16_t GameServerInterface::getCurrentPlayers()
{
        assert(m_instance);
        return m_instance->getCurrentPlayers();
}
uint16_t GameServerInterface::getMaxPlayers()
{
        assert(m_instance);
        return m_instance->getMaxPlayers();
}
uint8_t GameServerInterface::getUnkn1()
{
        assert(m_instance);
        return m_instance->getUnkn1();
}
uint8_t GameServerInterface::getUnkn2()
{
        assert(m_instance);
        return m_instance->getUnkn2();
}
std::string GameServerInterface::getName()
{
        assert(m_instance);
        return m_instance->getName();
}
const ACE_INET_Addr &GameServerInterface::getAddress()
{
        assert(m_instance);
        return m_instance->getAddress();
}
int	GameServerInterface::getAccessKeyForServer(const ServerHandle<IMapServer> &h_map)
{
        assert(m_instance);
        return m_instance->getAccessKeyForServer(h_map);
}
bool GameServerInterface::isMapServerReady(const ServerHandle<IMapServer> &h_map)
{
        assert(m_instance);
        return m_instance->isMapServerReady(h_map);
}
EventProcessor * GameServerInterface::event_target()
{
    assert(m_instance);
    return m_instance->event_target();
}
