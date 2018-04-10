/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */
#include "MapServerInterface.h"
#include <cassert>

bool MapServerInterface::Run()
{
        assert(m_instance);
        return m_instance->Run();
}
bool MapServerInterface::ReadConfig()
{
        assert(m_instance);
        return m_instance->ReadConfig();
}
bool MapServerInterface::ShutDown(const QString &reason)
{
        assert(m_instance);
        return m_instance->ShutDown(reason);
}
const ACE_INET_Addr &MapServerInterface::getAddress()
{
        assert(m_instance);
        return m_instance->getAddress();
}

EventProcessor * MapServerInterface::event_target()
{
    assert(m_instance);
    return m_instance->event_target();
}
