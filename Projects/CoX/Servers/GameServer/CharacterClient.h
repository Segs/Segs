/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *

 */
#pragma once

#include "ServerManager.h"
#include "Client.h"
#include "GameLink.h"
#include "CharacterDatabase.h"
#include <string>
#include <vector>
// skeleton class used during authentication
class Character;
class GameServer;
//class CharacterHandler;
class CharacterClient : public ClientSession
{
        GameServer *    m_server;
public:
                        CharacterClient(){}
virtual                 ~CharacterClient();
        void            setServer(GameServer * val) { m_server = val; }
        Character *     getCharacter(size_t idx);
        bool            deleteCharacter(Character *chr);
        bool            getCharsFromDb();
        size_t          max_slots();
        void            reset();
};
