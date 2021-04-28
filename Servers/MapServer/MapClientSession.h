/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Components/FixedPointValue.h"
#include "NetCommandManager.h"
#include "Messages/Map/GameCommand.h"
#include "Common/Servers/ClientManager.h"

#include <map>
#include <memory>


class MapHandler;
class MapInstance;
class NetCommand;
struct MapLink;
class GamePacket;
class SEGSTimer;
class Entity;

struct ClientEntityStateBelief
{
    const Entity *m_entity = nullptr; // this is the entity this belief is linked to.
    Vector3_FPV   position;           // we think client considers m_entity to be located here.
    bool          m_is_logging_out = false;
};

struct MapClientSession : public ClientSession
{
    using mNetCommands = std::map<int, NetCommand *>;
    using vBelief      = std::map<int, ClientEntityStateBelief>;
    using vStoredCommands = std::vector<std::unique_ptr<SEGSEvents::GameCommandEvent>>;
    friend class CharacterDatabase;

        uint32_t                m_client_id    = 0;
        uint8_t                 m_access_level = 0;
        uint16_t                m_requested_slot_idx=0;
        uint16_t                m_max_slots;
        mNetCommands            m_shortcuts;
        vStoredCommands         m_contents;
        MapInstance *           m_current_map = nullptr;
        Entity *                m_ent         = nullptr;
        QString                 m_name; // current character name, stored here for quick lookups
        SEGSTimer *             m_tick_source = nullptr;
        vBelief                 m_worldstate_belief;
        bool                    m_in_map                        = false;

        // The values below might be needed for map<->map handover ?
        uint32_t                is_connected_to_game_server_id = 0;
        uint32_t                is_connected_to_map_server_id   = 0;
        uint32_t                is_connected_to_map_instance_id = 0;
        void                    addCommandToSendNextUpdate(std::unique_ptr<SEGSEvents::GameCommandEvent> &&v);
        void                    AddShortcut(int index, NetCommand *command);

        uint32_t                auth_id() const { return m_client_id; }
                                // TODO: for now session reuse does not retain any data, should it ?
        void                    reset() { *this = {}; }
                                // Map sessions do not support temporary ones right now.
        void                    set_temporary(bool v) { assert(v==false); }
        bool                    is_temporary() const { return false; }
        MapLink *               link() { return m_link; }
        /// \note setting the link does not preserve the state of the previous one.
        void                    link(MapLink *l) { m_link = l; }
        template<class T, typename... Args>
        void addCommand(Args&&... args)
        {
            addCommandToSendNextUpdate(std::make_unique<T>(std::forward<Args>(args)...));
        }

protected:
        MapLink *               m_link = nullptr;
};
template<class T, typename... Args>
void addSessionCommand(MapClientSession &sess,Args&&... args)
{
    sess.addCommand(std::make_unique<T>(std::forward<Args>(args)...));
}
