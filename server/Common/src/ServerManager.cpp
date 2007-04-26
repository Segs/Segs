/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: ServerManager.cpp 291 2006-10-12 10:52:55Z nemerle $
 */

#include "ServerManager.h"
#include "GameServer/include/GameServer.h"
#include "MapServer/include/MapServer.h"
GameServerInterface *ServerManagerC::GetGameServer(size_t idx) 
{
	if(idx>m_GameServers.size()) 
		return NULL;
	return m_GameServers[idx];
};
ServerManagerC::ServerManagerC() : m_adminserv(NULL),m_authserv(NULL)
{
}
bool ServerManagerC::LoadConfiguration(const std::string &config_file_path)
{
	GameServer *game_instance		= new GameServer;
	MapServer * map_instance		= new MapServer;
	m_adminserv						= new AdminServerInterface;	
	m_authserv						= new AuthServerInterface;
	GameServerInterface *iface		= new GameServerInterface(game_instance); // takes over game_instance
	MapServerInterface *iface_map	= new MapServerInterface(map_instance); // takes over map_instance

	m_adminserv->ReadConfig(config_file_path+"/local_1.cfg");
	m_authserv->ReadConfig(config_file_path+"/local_1.cfg");
	iface->ReadConfig(config_file_path+"/local_1.cfg");
	iface_map->ReadConfig(config_file_path+"/local_1.cfg");
	AddGameServer(iface);   // takes over iface
	AddMapServer(iface_map); // takes over iface_map
	return true;
}
bool ServerManagerC::StartLocalServers()

{
	m_adminserv->Run();
	m_authserv->Run();
	for(size_t i=0; i<m_GameServers.size(); i++)
	{
		if(!m_GameServers[i]->Run())
			return false;
	}
	for(size_t i=0; i<m_MapServers.size(); i++)
	{
		if(!m_MapServers[i]->Run())
			return false;
	}
	return true;
}
void ServerManagerC::StopLocalServers()
{
	for(size_t i=0; i<m_GameServers.size(); i++)
	{
		if(!m_GameServers[i]->ShutDown("Full system shutdown"))
		{
			// Log it
		}
	}
}
AdminServerInterface *ServerManagerC::GetAdminServer(void)
{
	return m_adminserv;
}
AuthServerInterface *ServerManagerC::GetAuthServer(void)
{
	return m_authserv;
}
bool ServerManagerC::CreateServerConnections()
{
	return true; // for now, this is just a stub
}
void ServerManagerC::RemoveGameServer(GameServerInterface *srv)
{
	deque<GameServerInterface *>::iterator iter = find(m_GameServers.begin(),m_GameServers.end(),srv);
	ACE_ASSERT(iter!=m_GameServers.end());
	delete *iter;
	m_GameServers.erase(iter);
};
void ServerManagerC::RemoveMapServer(MapServerInterface *srv)
{
	deque<MapServerInterface *>::iterator iter = find(m_MapServers.begin(),m_MapServers.end(),srv);
	ACE_ASSERT(iter!=m_MapServers.end());
	delete *iter;
	m_MapServers.erase(iter);
};
MapServerInterface *ServerManagerC::GetMapServer(size_t idx) 
{
	if(idx>m_MapServers.size()) 
		return NULL;
	return m_MapServers[idx];
};
