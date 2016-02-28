/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */
#include <cassert>
#include "ServerManager.h"
//! If called from standalone MapServer it contains it's controlling GameServer interface
GameServerInterface *ServerManagerC::GetGameServer(size_t idx)
{
    if(idx>m_GameServers.size())
        return NULL;
    return m_GameServers[idx];
}
ServerManagerC::ServerManagerC() : m_authserv(NULL),m_adminserv(NULL)
{
}
//! this loads this process configuration
bool ServerManagerC::LoadConfiguration(const std::string &config_file_full_path)
{
    m_adminserv->ReadConfig(config_file_full_path);
    m_authserv->ReadConfig(config_file_full_path);
    for(size_t idx=0; idx<m_GameServers.size(); idx++)
    {
        m_GameServers[idx]->ReadConfig(config_file_full_path);
    }
    for(size_t idx=0; idx<m_MapServers.size(); idx++)
    {
        m_MapServers[idx]->ReadConfig(config_file_full_path);
    }
    return true;
}

//! this function will create all server instances local to this process
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
//! using configuration info, this will connect all remote servers to their local proxy objects
bool ServerManagerC::CreateServerConnections()
{
    return true; // for now, this is just a stub
}
/*
void ServerManagerC::RemoveGameServer(IGameServer *srv)
{
    deque<GameServerInterface *>::iterator iter = find(m_GameServers.begin(),m_GameServers.end(),srv);
    assert(iter!=m_GameServers.end());
    delete *iter;
    m_GameServers.erase(iter);
};
void ServerManagerC::RemoveMapServer(MapServerInterface *srv)
{
    deque<MapServerInterface *>::iterator iter = find(m_MapServers.begin(),m_MapServers.end(),srv);
    assert(iter!=m_MapServers.end());
    delete *iter;
    m_MapServers.erase(iter);
};
*/
MapServerInterface *ServerManagerC::GetMapServer(size_t idx)
{
    if(idx>m_MapServers.size())
        return NULL;
    return m_MapServers[idx];
};

void ServerManagerC::AddGameServer( IGameServer *srv )
{
    m_GameServers.push_back(new GameServerInterface(srv));
}

size_t ServerManagerC::GameServerCount( void )
{
    return m_GameServers.size();
}

size_t ServerManagerC::MapServerCount( void )
{
    return m_MapServers.size();
}

void ServerManagerC::AddMapServer( IMapServer *srv )
{
    m_MapServers.push_back(new MapServerInterface(srv));
}

void ServerManagerC::SetAuthServer( IAuthServer *srv )
{
    assert(m_authserv==0);
    m_authserv=new AuthServerInterface(srv);
}
void ServerManagerC::SetAdminServer( IAdminServer *srv )
{
    assert(m_adminserv==0);
    m_adminserv=new AdminServerInterface(srv);
}
