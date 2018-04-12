/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *

 */

#pragma once
#include "Client.h"
#include "MapLink.h"
#include "CharacterDatabase.h"
#include "FixedPointValue.h"
#include "Events/GameCommandList.h"

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
class Entity;
///
/// \brief The ClientEntityStateBelief struct represents the server's belief about the client's internal state
/// in regards to given entity.
///
struct ClientEntityStateBelief
{
    const Entity *m_entity=nullptr; // this is the entity this belief is linked to.
    Vector3_FPV position;   // we think client considers m_entity to be located here.
    bool m_is_logging_out=false;
};

class MapClientSession : public ClientSession
{
    friend class CharacterDatabase;
    using mNetCommands = std::map<int,NetCommand *>;
    using vBelief = std::map<int,ClientEntityStateBelief>;

        mNetCommands        m_shortcuts;
        MapInstance *       m_current_map;
        Entity *            m_ent = nullptr;
        QString             m_name; // current character name, stored here for quick lookups
        SEGSTimer *         m_tick_source = nullptr;

public:
virtual                     ~MapClientSession() = default;

        void                AddShortcut(int index, NetCommand *command);
        NetCommand *        GetCommand(int index) {return m_shortcuts[index];}
        void                current_map(MapInstance *pmap){m_current_map=pmap;}
        MapInstance *       current_map(){return m_current_map;}
        void                char_entity(Entity *ent){m_ent=ent; m_ent->m_client=this;}
        Entity *            char_entity(){return m_ent;}
        void                reset();
        MapLink *           link() {return static_cast<MapLink *>(m_link.link());}
        const QString &     name() const { return m_name; }
        void                name(const QString &val) { m_name = val; }
        void                entity(Entity * val);
        void                addCommandToSendNextUpdate(std::unique_ptr<GameCommand> &&v) { m_contents.emplace_back(std::move(v));}
        bool                db_create(); // creates a new character from Entity data
        vBelief             m_worldstate_belief;
        std::vector<std::unique_ptr<GameCommand> > m_contents;
        bool                m_in_map = false;
};
