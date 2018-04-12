#pragma once
#include "AuthDatabase/AccountData.h"
#include "FixedPointValue.h"
#include "GameDatabase/GameAccountData.h"
#include "Events/GameCommandList.h"

#include <map>
#include <memory>

class MapHandler;
class MapInstance;
class NetCommand;
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
    struct MapLink * m_link;
    uint32_t m_client_id;
    // The values below might be needed for map<->map handover ?
    uint32_t is_connected_to_map_server_id=0;
    uint32_t is_connected_to_map_instance_id=0;
    uint8_t m_access_level;
    GameAccountData m_game_account;

    mNetCommands m_shortcuts;
    MapInstance *m_current_map;
    Entity *     m_ent = nullptr;
    QString      m_name; // current character name, stored here for quick lookups
    SEGSTimer *  m_tick_source = nullptr;
    vBelief      m_worldstate_belief;
    bool         m_in_map = false;

    std::vector<std::unique_ptr<GameCommand> > m_contents;
    void    addCommandToSendNextUpdate(std::unique_ptr<GameCommand> &&v) { m_contents.emplace_back(std::move(v));}
    void AddShortcut(int index, NetCommand *command)
    {
        if(m_shortcuts.find(index)!=m_shortcuts.end())
        {
            ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("Replacing command %d %s->%s\n"),index,qPrintable(m_shortcuts[index]->m_name),qPrintable(command->m_name)));
        }
        m_shortcuts[index]=command;
    }

};
