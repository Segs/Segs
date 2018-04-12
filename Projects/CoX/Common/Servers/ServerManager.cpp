/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */
#include "ServerManager.h"
#include <cassert>
#include <QDebug>

 //! If called from standalone MapServer it contains it's controlling GameServer interface
GameServerInterface *ServerManagerC::GetGameServer(size_t idx)
{
    if(idx>m_GameServers.size())
        return nullptr;
    return m_GameServers[idx];
}
ServerManagerC::ServerManagerC() : m_authserv(nullptr),m_adminserv(nullptr)
{
}
//! this loads this process configuration
bool ServerManagerC::LoadConfiguration()
{
    bool loaded_ok = m_adminserv->ReadConfig();
    loaded_ok &= m_authserv->ReadConfig();
    for(GameServerInterface * serv : m_GameServers)
    {
        loaded_ok &= serv->ReadConfig();
    }
    for(MapServerInterface *serv : m_MapServers)
    {
        loaded_ok &= serv->ReadConfig();
    }
    return loaded_ok;
}

//! this function will create all server instances local to this process
bool ServerManagerC::StartLocalServers()
{
    m_adminserv->Run();
    m_authserv->Run();
    for(GameServerInterface * serv : m_GameServers)
    {
        if (!serv->Run())
        {
            qCritical() << "Failed to start game server";
            return false;
        }
    }
    for(MapServerInterface *serv : m_MapServers)
    {
        if (!serv->Run())
        {
            qCritical() << "Failed to start map server";
            return false;
        }
    }
    return true;
}
void ServerManagerC::StopLocalServers()
{
    for(GameServerInterface * serv : m_GameServers)
    {
        if(!serv->ShutDown("Full system shutdown"))
        {
            // Log it
        }
    }
    for(MapServerInterface *serv : m_MapServers)
    {
        if(!serv->ShutDown("Full system shutdown"))
        {
            // Log it
        }
    }
    m_authserv->ShutDown("Full system shutdown");
}
AdminServerInterface *ServerManagerC::GetAdminServer()
{
    return m_adminserv;
}
//! using configuration info, this will connect all remote servers to their local proxy objects
bool ServerManagerC::CreateServerConnections()
{
    return true; // for now, this is just a stub
}

MapServerInterface *ServerManagerC::GetMapServer(size_t idx)
{
    if(idx>m_MapServers.size())
        return nullptr;
    return m_MapServers[idx];
}

void ServerManagerC::AddGameServer( IGameServer *srv )
{
    m_GameServers.push_back(new GameServerInterface(srv));
}

size_t ServerManagerC::MapServerCount( )
{
    return m_MapServers.size();
}

void ServerManagerC::AddMapServer( IMapServer *srv )
{
    m_MapServers.push_back(new MapServerInterface(srv));
}

void ServerManagerC::SetAuthServer( Server *srv )
{
    assert(m_authserv==nullptr);
    m_authserv=srv;
}
void ServerManagerC::SetAdminServer( IAdminServer *srv )
{
    assert(m_adminserv==nullptr);
    m_adminserv=new AdminServerInterface(srv);
}
