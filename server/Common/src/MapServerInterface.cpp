/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: MapServerInterface.cpp 305 2007-01-21 08:53:16Z nemerle $
 */

#include "MapServerInterface.h"
#include "MapServer/include/MapServer.h"
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
u32 MapServerInterface::ExpectClient(const ACE_INET_Addr &from,u64 id,u16 access_level)
{
	return m_instance->ExpectClient(from,id,access_level);
}
void MapServerInterface::AssociatePlayerWithMap(u64 player_id,int map_number)
{
	m_instance->AssociatePlayerWithMap(player_id, map_number);
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
