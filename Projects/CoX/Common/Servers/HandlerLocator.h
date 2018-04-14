#pragma once
#include <deque>
#include <stdint.h>
#include <QHash>
#include <QString>

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
    static std::deque<EventProcessor *> m_game_servers;
    static std::deque<EventProcessor *> m_game_db_servers;
    static QHash<QString,int> m_map_name_to_id;
    static QHash<int,std::deque<EventProcessor *>> m_map_handlers;
public:
    HandlerLocator();
    static void setMessageBus(MessageBus *h) { m_message_bus=h; }
    static MessageBus *getMessageBus() { return m_message_bus; }

    static void setAuthDB_Handler(EventProcessor *h) { m_db_sync_handler=h; }
    static EventProcessor *getAuthDB_Handler() { return m_db_sync_handler; }

    static void setAuth_Handler(EventProcessor *h) { m_auth_handler=h; }
    static EventProcessor *getAuth_Handler() { return m_auth_handler; }


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
    //TODO: this whole instance selection code should be moved from here to GameServer
    static EventProcessor *getMap_Handler(const QString &name)
    {
        auto iter = m_map_name_to_id.find(name);
        if(iter==m_map_name_to_id.end())
        {
            // unknown map names requests are put in atlas park by default.
            iter = m_map_name_to_id.find("City_01_01");
        }
        auto handler_container_iter = m_map_handlers.find(iter.value());
        if(handler_container_iter==m_map_handlers.end())
            return nullptr;
        return handler_container_iter->front();
    }
    static EventProcessor *getMap_Handler(int id)
    {
        auto handler_container_iter = m_map_handlers.find(id);
        if(handler_container_iter==m_map_handlers.end())
            return nullptr;
        return handler_container_iter->front();
    }

    static void registerMapInstance(const char *map_template,int id,EventProcessor *handler)
    {
        m_map_name_to_id[map_template] = id;
        m_map_handlers[id].emplace_back(handler);
    }

};
extern void shutDownAllActiveHandlers();
