/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

// Inclusion guards
#pragma once
#ifndef SERVERMANAGER_H
#define SERVERMANAGER_H

#include <ace/Singleton.h>
#include <ace/Synch.h>
#include <deque>
#include <string>
//#include <boost/filesystem/path.hpp>
#include <algorithm>
#include "AdminServerInterface.h"
#include "AuthServerInterface.h"
#include "GameServerInterface.h"
#include "MapServerInterface.h"

using namespace std;

class ServerManagerC
{
public:
    typedef deque<GameServerInterface *> dGameServer;

                                ServerManagerC(void);
virtual                         ~ServerManagerC(void){};

	    bool                    LoadConfiguration(const std::string &config_file_path); // this loads this process configuration
	    bool                    StartLocalServers(void); //! this function will create all server instances local to this process
	    bool                    CreateServerConnections(void); //! using configuration info, this will connect all remote servers to their local proxy objects
	    void                    StopLocalServers(void);

virtual AdminServerInterface *	GetAdminServer(void);
virtual AuthServerInterface *	GetAuthServer(void);
virtual GameServerInterface *	GetGameServer(size_t idx); //! If called from standalone MapServer it contains it's controlling GameServer interface
virtual MapServerInterface *	GetMapServer(size_t idx);
	
            void                SetAuthServer(IAuthServer *srv);
            void                SetAdminServer(IAdminServer *srv);
virtual     size_t              GameServerCount(void);
virtual     void	            AddGameServer(IGameServer *srv);
            const dGameServer * getGameServerList() const {return &m_GameServers;};

virtual     size_t              MapServerCount(void);
virtual     void                AddMapServer(IMapServer *srv);

protected:
            dGameServer         m_GameServers;
            deque<MapServerInterface *> m_MapServers;
            AuthServerInterface *m_authserv;
            AdminServerInterface *m_adminserv;
};
typedef ACE_Singleton<ServerManagerC,ACE_Thread_Mutex> ServerManager;

#endif // SERVERMANAGER_H
