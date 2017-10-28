/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *

 */

#pragma once

#include "EntityStorage.h"
#include "EventProcessor.h"
#include "ClientManager.h"
#include "ScriptingEngine.h"
#include <map>
#include <memory>
#include <vector>
#define WORLD_UPDATE_TICKS_PER_SECOND 30

class IdleEvent;
class SceneRequest;
class CombineRequest;
class EntitiesRequest;
class DisconnectRequest;
class ShortcutsRequest;
class ExpectMapClient;
class CookieRequest;
class WindowState;
class NewEntity;
class ConsoleCommand;
class ClientQuit;
class ConnectRequest;
class ChatDividerMoved;
class MiniMapState;

class MapServer;
class SEGSTimer;
class InputState;
class World;

class MapInstance : public EventProcessor
{
    std::unique_ptr<ScriptingEngine> m_scripting_interface;
    std::string            m_name;
    SEGSTimer *            m_world_update_timer;
    SEGSTimer *            m_resend_timer;
    ClientStore<MapClient> m_clients;

    // vClients        m_queued_clients;
    World *    m_world;
    MapServer *m_server;

public:
    EntityManager m_entities;

    MapInstance(const std::string &name);
    virtual ~MapInstance();
    void       dispatch(SEGSEvent *ev);
    SEGSEvent *dispatch_sync(SEGSEvent *ev);

    void   enqueue_client(MapClient *clnt);
    void   start();
    void   set_server(MapServer *s) { m_server = s; }
    size_t num_active_clients();

protected:
    void process_chat(MapClient *sender, const QString &msg_text);

    void on_expect_client(ExpectMapClient *ev);
    void on_link_lost(SEGSEvent *ev);
    void on_disconnect(DisconnectRequest *ev);
    void on_scene_request(SceneRequest *ev);
    void on_entities_request(EntitiesRequest *ev);
    void on_create_map_entity(NewEntity *ev);
    void on_timeout(TimerEvent *ev);
    void on_combine_boosts(CombineRequest *);
    void on_input_state(InputState *st);
    void on_idle(IdleEvent *ev);
    void on_shortcuts_request(ShortcutsRequest *ev);

    void sendState();
    void on_cookie_confirm(CookieRequest *ev);
    void on_window_state(WindowState *ev);
    void on_console_command(ConsoleCommand *ev);
    void on_client_quit(ClientQuit *ev);
    void on_connection_request(ConnectRequest *ev);
    void on_command_chat_divider_moved(ChatDividerMoved *ev);
    void on_minimap_state(MiniMapState *ev);
};
