/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */
#include <cassert>
#include "MapServerInterface.h"
#include <ace/Log_Msg.h>

bool MapServerInterface::Run()
{
        assert(m_instance);
        return m_instance->Run();
}
bool MapServerInterface::ReadConfig(const std::string &name)
{
        assert(m_instance);
        return m_instance->ReadConfig(name);
}
bool MapServerInterface::ShutDown(const std::string &reason)
{
        assert(m_instance);
        return m_instance->ShutDown(reason);
}
bool MapServerInterface::Online()
{
        assert(m_instance);
        return m_instance->Online();
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
