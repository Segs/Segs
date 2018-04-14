#pragma once
#include "FixedPointValue.h"
#include "Events/GameCommandList.h"
#include "AuthDatabase/AuthDBSyncEvents.h"
#include "GameDatabase/GameDBSyncEvents.h"

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

struct MapClientSession
{
    using mNetCommands = std::map<int, NetCommand *>;
    using vBelief      = std::map<int, ClientEntityStateBelief>;
    using vStoredCommands = std::vector<std::unique_ptr<GameCommand>>;
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
        uint32_t                is_connected_to_map_server_id   = 0;
        uint32_t                is_connected_to_map_instance_id = 0;
        void                    addCommandToSendNextUpdate(std::unique_ptr<GameCommand> &&v) {
                                    m_contents.emplace_back(std::move(v));
                                }
        void                    AddShortcut(int index, NetCommand *command)
                                {
                                    if (m_shortcuts.find(index) != m_shortcuts.end())
                                    {
                                        qDebug() << "Replacing command" << index << m_shortcuts[index]->m_name <<
                                                    "->" << command->m_name;
                                    }
                                    m_shortcuts[index] = command;
                                }

        uint32_t                auth_id() const { return m_client_id; }
                                // TODO: for now session reuse does not retain any data, should it ?
        void                    reset() { *this = {}; }
                                // Map sessions do not support temporary ones right now.
        void                    set_temporary(bool v) { assert(v==false); }
        bool                    is_temporary() const { return false; }
        MapLink *               link() { return m_link; }
        /// \note setting the link does not preserver the state of the previous one.
        void                    link(MapLink *l) { m_link = l; }

protected:
        MapLink *               m_link = nullptr;
};
