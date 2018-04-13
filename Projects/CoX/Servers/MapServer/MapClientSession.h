#pragma once
#include "AuthDatabase/AccountData.h"
#include "FixedPointValue.h"
#include "GameDatabase/GameDBSyncEvents.h"
#include "Events/GameCommandList.h"

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
    friend class CharacterDatabase;
    MapLink *       m_link         = nullptr;
    uint32_t        m_client_id    = 0;
    uint8_t         m_access_level = 0;
    GameAccountResponseData m_game_account;
    mNetCommands    m_shortcuts;
    MapInstance *   m_current_map = nullptr;
    Entity *        m_ent         = nullptr;
    QString         m_name; // current character name, stored here for quick lookups
    SEGSTimer *     m_tick_source = nullptr;
    vBelief         m_worldstate_belief;
    bool            m_in_map = false;
    // The values below might be needed for map<->map handover ?
    uint32_t is_connected_to_map_server_id   = 0;
    uint32_t is_connected_to_map_instance_id = 0;

    std::vector<std::unique_ptr<GameCommand>> m_contents;
    void addCommandToSendNextUpdate(std::unique_ptr<GameCommand> &&v) { m_contents.emplace_back(std::move(v)); }
    void AddShortcut(int index, NetCommand *command)
    {
        if (m_shortcuts.find(index) != m_shortcuts.end())
        {
            ACE_DEBUG((LM_DEBUG, ACE_TEXT("Replacing command %d %s->%s\n"), index,
                       qPrintable(m_shortcuts[index]->m_name), qPrintable(command->m_name)));
        }
        m_shortcuts[index] = command;
    }
    uint32_t auth_id() const { return m_client_id; }
    // TODO: for now session reuse does not retain any data, should it ?
    void reset() { *this = {}; }
};
