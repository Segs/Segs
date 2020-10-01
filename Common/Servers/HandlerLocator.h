/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <deque>
#include <unordered_map>
#include <stdint.h>

class EventProcessor;
class MessageBus;
///
/// \brief The HandlerLocator class is meant as a central point where each service registers it's presence,
/// and messages can be passed to it.
///
class HandlerLocator
{
    static MessageBus *m_message_bus;
    static EventProcessor *m_db_sync_handler;
    static EventProcessor *m_auth_handler;
    static EventProcessor *m_email_handler;
    static EventProcessor *m_friend_handler;
    static EventProcessor *m_team_handler;
    static std::deque<EventProcessor *> m_game_servers;
    static std::deque<EventProcessor *> m_map_servers;
    static std::deque<EventProcessor *> m_game_db_servers;
    static std::deque<std::deque<EventProcessor *>> m_map_instances;
    static std::unordered_map<uint32_t,std::deque<EventProcessor *>> m_all_event_processors;
public:
    HandlerLocator();
    static void setMessageBus(MessageBus *h) { m_message_bus=h; }
    static MessageBus *getMessageBus() { return m_message_bus; }

    static void setAuthDB_Handler(EventProcessor *h) { m_db_sync_handler=h; }
    static EventProcessor *getAuthDB_Handler() { return m_db_sync_handler; }

    static void setAuth_Handler(EventProcessor *h) { m_auth_handler=h; }
    static EventProcessor *getAuth_Handler() { return m_auth_handler; }

    static void setEmail_Handler(EventProcessor *h) {m_email_handler=h;}
    static EventProcessor *getEmail_Handler() { return m_email_handler; }

    static void setFriend_Handler(EventProcessor *h) { m_friend_handler=h; }
    static EventProcessor *getFriend_Handler() { return m_friend_handler; }

    static void setTeam_Handler(EventProcessor *h) { m_team_handler=h; }
    static EventProcessor *getTeam_Handler() { return m_team_handler; }

    static const std::deque<EventProcessor *> &allGameDBHandlers() { return m_game_db_servers; }
    static const std::deque<EventProcessor *> &allGameHandlers() { return m_game_servers; }
    static EventProcessor *getGame_Handler(uint8_t id)
    {
        if(id>=m_game_servers.size())
            return nullptr;
        return m_game_servers[id];
    }
    static EventProcessor *getGame_DB_Handler(uint8_t id)
    {
        if(id>=m_game_db_servers.size())
            return nullptr;
        return m_game_db_servers[id];
    }
    static void setGame_Handler(uint8_t id,EventProcessor *h)
    {
        if(id>=m_game_servers.size())
            m_game_servers.resize(id+1);
        m_game_servers[id] = h;
    }
    static void setGame_DB_Handler(uint8_t id,EventProcessor *h)
    {
        if(id>=m_game_db_servers.size())
            m_game_db_servers.resize(id+1);
        m_game_db_servers[id] = h;
    }
    // The ID here must be the same as the GameServer ID
    // the MapServer is the point of contact for the GameServer, and will create many MapInstances with running maps
    static EventProcessor *getMap_Handler(uint8_t id)
    {
        if(id>=m_map_servers.size())
            return nullptr;
        return m_map_servers[id];
    }
    static void setMap_Handler(uint8_t id,EventProcessor *h)
    {
        if(id>=m_map_servers.size())
            m_map_servers.resize(id+1);
        m_map_servers[id] = h;
    }

    static EventProcessor *getMapInstance_Handler(uint8_t id,uint8_t subserver_id)
    {
        if(id>=m_map_instances.size())
            return nullptr;
        if(subserver_id>=m_map_instances[id].size())
            return nullptr;
        return m_map_instances[id][subserver_id];
    }
    static void setMapInstance_Handler(uint8_t id,uint8_t subserver_id,EventProcessor *h)
    {
        if(id>=m_map_instances.size())
            m_map_instances.resize(id+1);
        if(subserver_id>=m_map_instances[id].size())
            m_map_instances[id].resize(subserver_id+1);
        m_map_instances[id][subserver_id] = h;
    }
    template<class T>
    static EventProcessor *lookup(uint32_t instance_id)
    {
        auto iter = m_all_event_processors.find(T::processor_id);
        if(iter==m_all_event_processors.end())
            return nullptr;
        if(instance_id>=iter->second.size())
            return nullptr;
        return iter->second[instance_id];
    }
};
extern void shutDownAllActiveHandlers();
