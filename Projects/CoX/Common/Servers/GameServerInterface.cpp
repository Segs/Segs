/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */
#include <cassert>
#include "GameServerInterface.h"

bool GameServerInterface::Run()
{
    assert(m_instance);
    return m_instance->Run();
}
bool GameServerInterface::ReadConfig()
{
    assert(m_instance);
    return m_instance->ReadConfig();
}
bool GameServerInterface::ShutDown(const QString &reason)
{
    assert(m_instance);
    return m_instance->ShutDown(reason);
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
QString GameServerInterface::getName()
{
    assert(m_instance);
    return m_instance->getName();
}
const ACE_INET_Addr &GameServerInterface::getAddress()
{
    assert(m_instance);
    return m_instance->getAddress();
}
EventProcessor * GameServerInterface::event_target()
{
    assert(m_instance);
    return m_instance->event_target();
}

int GameServerInterface::getMaxCharacterSlots() const
{
    assert(m_instance);
    return m_instance->getMaxCharacterSlots();
}
