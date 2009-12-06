/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

// Inclusion guards
#pragma once

#include <string>
#include <ace/OS_NS_time.h>
#include "ServerManager.h"
#include "Client.h"
#include "MapLink.h"
#include "CharacterDatabase.h"
#include <vector>
#include <map>

// skeleton class used during authentication
class MapHandler;
class CoXMap;
class NetCommand;
class GamePacket;
class MapClient : public ClientSession
{
    friend class CharacterDatabase;
    typedef std::map<int,NetCommand *> mNetCommands;
    mNetCommands            m_shortcuts;
    CoXMap *               m_current_map;
    Entity *                m_ent;
    std::string             m_name; // current character name, stored here for quick lookups
public:
                            MapClient();
virtual                     ~MapClient(){};

        void                AddShortcut(int index, NetCommand *command);
        NetCommand *        GetCommand(int index) {return m_shortcuts[index];}
        void                SendCommand(NetCommand *command,...);
        void                current_map(CoXMap *pmap){m_current_map=pmap;}
        CoXMap *            current_map(){return m_current_map;}
        void                char_entity(Entity *ent){m_ent=ent;}
        Entity *            char_entity(){return m_ent;}
        void                reset();
        const std::string & name() const { return m_name; }
        void                name(const std::string &val) { m_name = val; }
        void                entity(Entity * val);
        bool                db_create(); // creates a new character from Entity data
};
