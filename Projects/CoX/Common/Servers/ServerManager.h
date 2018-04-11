/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once

#include <ace/Singleton.h>
#include <ace/Synch.h>
#include <deque>
#include <string>
//#include <boost/filesystem/path.hpp>
#include <algorithm>
#include "AdminServerInterface.h"
#include "GameServerInterface.h"
#include "MapServerInterface.h"

class ServerManagerC
{
public:
    typedef std::deque<GameServerInterface *> dGameServer;
    typedef std::deque<MapServerInterface *> dMapServer;

                                ServerManagerC(void);
virtual                         ~ServerManagerC(void){}

        bool                    LoadConfiguration();
        bool                    StartLocalServers(void);
        bool                    CreateServerConnections(void);
        void                    StopLocalServers(void);

virtual AdminServerInterface *  GetAdminServer(void);
virtual GameServerInterface *   GetGameServer(size_t idx);
virtual MapServerInterface *    GetMapServer(size_t idx);

        void                    SetAuthServer(Server *srv);
        void                    SetAdminServer(IAdminServer *srv);
virtual void                    AddGameServer(IGameServer *srv);
        const dGameServer *     getGameServerList() const {return &m_GameServers;}

virtual size_t                  MapServerCount(void);
virtual void                    AddMapServer(IMapServer *srv);

protected:
        dGameServer             m_GameServers;
        dMapServer              m_MapServers;
        Server *                m_authserv;
        AdminServerInterface *  m_adminserv;
};
typedef ACE_Singleton<ServerManagerC,ACE_Thread_Mutex> ServerManager;
