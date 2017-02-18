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
#include "MapLink.h"
#include "CharacterDatabase.h"

#include <QtCore/QString>
#include <string>
#include <vector>
#include <map>
// skeleton class used during authentication
class MapHandler;
class MapInstance;
class NetCommand;
class GamePacket;
class SEGSTimer;
class MapClient : public ClientSession
{
    friend class CharacterDatabase;
    typedef std::map<int,NetCommand *> mNetCommands;
        mNetCommands        m_shortcuts;
        MapInstance *       m_current_map;
        Entity *            m_ent;
        QString             m_name; // current character name, stored here for quick lookups
        SEGSTimer *         m_tick_source;
public:
                            MapClient();
virtual                     ~MapClient(){}

        void                AddShortcut(int index, NetCommand *command);
        NetCommand *        GetCommand(int index) {return m_shortcuts[index];}
        void                SendCommand(NetCommand *command,...);
        void                current_map(MapInstance *pmap){m_current_map=pmap;}
        MapInstance *       current_map(){return m_current_map;}
        void                char_entity(Entity *ent){m_ent=ent;}
        Entity *            char_entity(){return m_ent;}
        void                reset();
        MapLink *           link() {return static_cast<MapLink *>(m_link.link());}
        const QString &     name() const { return m_name; }
        void                name(const QString &val) { m_name = val; }
        void                entity(Entity * val);
        bool                db_create(); // creates a new character from Entity data
};
