/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: Client.h 253 2006-08-31 22:00:14Z malign $
 */

// Inclusion guards
#pragma once
#ifndef MAPCLIENT_H
#define MAPCLIENT_H

#include <string>
#include <ace/OS_NS_time.h>
#include "ServerManager.h"
#include "Client.h"
#include "MapLink.h"
#include <vector>
#include <map>

// skeleton class used during authentication
class GameServerInterface;
class MapHandler;
class CharacterHandler;
class SEGSMap;
class NetCommand;

class IMapClient: public IClient
{
public:

    virtual void            AddShortcut(int index, NetCommand *command)=0; // part of user app model
    virtual NetCommand *    GetCommand(int index)=0; // part of user app model
    virtual void            SendCommand(NetCommand *command,...)=0; // part of user app model
    virtual void            current_map(SEGSMap *pmap)=0;
    virtual SEGSMap *       current_map()=0;
    virtual void            char_entity(Entity *ent)=0;
    virtual Entity *        char_entity()=0;

};
class pktCS_SendEntity;
class GamePacket;
class MapClient : public IMapClient
{
    friend class CharacterDatabase;
    typedef std::map<int,NetCommand *> mNetCommands;

    u64                     m_game_server_acc_id;
    void                    setGameServerAccountId(u64 id){m_game_server_acc_id=id;}
    mNetCommands            m_shortcuts;
    SEGSMap *               m_current_map;
    Entity *                m_ent;
    MapLink *               m_link;
    std::string             m_name; // stored here for quick lookups
public:
                            MapClient();
virtual                     ~MapClient(){};

        GamePacket *        HandleClientPacket(pktCS_SendEntity *ent);
        void                AddShortcut(int index, NetCommand *command);
        NetCommand *        GetCommand(int index) {return m_shortcuts[index];}
        void                SendCommand(NetCommand *command,...);
        void                current_map(SEGSMap *pmap){m_current_map=pmap;}
        SEGSMap *            current_map(){return m_current_map;}
        void                char_entity(Entity *ent){m_ent=ent;}
        Entity *            char_entity(){return m_ent;}
        void                reset();
        MapLink *           link() const { return m_link; }
        void                link(MapLink * val) { m_link = val; }
        const std::string & name() const { return m_name; }
        void                name(const std::string &val) { m_name = val; }
};

#endif // MAPCLIENT_H
