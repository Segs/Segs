/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

#include "MapServerInterface.h"
#include <ace/Log_Msg.h>

bool MapServerInterface::Run()
{
	ACE_ASSERT(m_instance);
	return m_instance->Run();
}
bool MapServerInterface::ReadConfig(const std::string &name)
{
	ACE_ASSERT(m_instance);
	return m_instance->ReadConfig(name);
}
bool MapServerInterface::ShutDown(const std::string &reason)
{
	ACE_ASSERT(m_instance);
	return m_instance->ShutDown(reason);
}
bool MapServerInterface::Online()
{ 
	ACE_ASSERT(m_instance);
	return m_instance->Online();
}
const ACE_INET_Addr &MapServerInterface::getAddress()
{ 
	ACE_ASSERT(m_instance);
	return m_instance->getAddress();
}

EventProcessor * MapServerInterface::event_target()
{
    ACE_ASSERT(m_instance);
    return m_instance->event_target();
}