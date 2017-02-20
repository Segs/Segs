/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */
#pragma once

#include "Client.h"

#include <string>

class GameServerInterface;
class IGameServer;
// skeleton class used during authentication
class AuthClient : public ClientSession
{

public:
                    AuthClient();
    bool            isLoggedIn();
    void            forceGameServerConnectionCheck();
    void            setSelectedServer(IGameServer *gs){m_game_server=gs;}
protected:
    IGameServer *   m_game_server; //!< every auth client knows where it is connected.

};
