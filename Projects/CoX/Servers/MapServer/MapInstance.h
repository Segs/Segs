/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2018 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *

 */

#pragma once

#include "EntityStorage.h"
#include "EventProcessor.h"
#include "ClientManager.h"
#include "ScriptingEngine.h"
#include "MapClientSession.h"

#include <map>
#include <memory>
#include <vector>
#define WORLD_UPDATE_TICKS_PER_SECOND 30

class IdleEvent;
class MapServer;
class SEGSTimer;
class InputState;
class World;
// server<-> server event types
struct ExpectMapClientRequest;

class MapInstance final : public EventProcessor
{
    QString                m_name;
    uint32_t               m_index = 1; // what does client expect this to store, and where do we send it?
    SEGSTimer *            m_world_update_timer;
    SEGSTimer *            m_resend_timer;

    World *    m_world;
    MapServer *m_server;

    uint8_t                 m_game_server_id=255; // 255 is `invalid` id
public:
    using SessionStore = ClientSessionStore<MapClientSession>;
    EntityManager m_entities;
    SessionStore            m_session_store;

    std::unique_ptr<ScriptingEngine> m_scripting_interface;

public:
                            MapInstance(const QString &name);
                            ~MapInstance() override;
    void                    dispatch(SEGSEvent *ev) override;

    void                    enqueue_client(MapClientSession *clnt);
    void                    start();
    void                    set_server(MapServer *s) { m_server = s; }
    const QString &         name() const { return m_name; }
    uint32_t                index() const { return m_index; }
    void                    spin_down();
    void                    spin_up_for(uint8_t game_server_id);
protected:
    void process_chat(MapClientSession *sender, QString &msg_text);

    void on_expect_client(ExpectMapClientRequest *ev);
    void on_link_lost(SEGSEvent *ev);
    void on_disconnect(class DisconnectRequest *ev);
    void on_scene_request(class SceneRequest *ev);
    void on_entities_request(class EntitiesRequest *ev);
    void on_create_map_entity(class NewEntity *ev);
    void on_timeout(TimerEvent *ev);
    void on_combine_boosts(class CombineRequest *);
    void on_input_state(InputState *st);
    void on_idle(IdleEvent *ev);
    void on_shortcuts_request(class ShortcutsRequest *ev);

    void sendState();
    void on_cookie_confirm(class CookieRequest *ev);
    void on_window_state(class WindowState *ev);
    void on_console_command(class ConsoleCommand *ev);
    void on_client_quit(class ClientQuit *ev);
    void on_connection_request(class ConnectRequest *ev);
    void on_command_chat_divider_moved(class ChatDividerMoved *ev);
    void on_minimap_state(class MiniMapState *ev);
    void on_client_resumed(class ClientResumedRendering *ev);
    void on_location_visited(class LocationVisited * ev);
    void on_plaque_visited(class PlaqueVisited * ev);
    void on_inspiration_dockmode(class InspirationDockMode * ev);
    void on_enter_door(class EnterDoor * ev);
    void on_change_stance(class ChangeStance *ev);
    void on_set_destination(class SetDestination *ev);
    void on_abort_queued_power(class AbortQueuedPower *ev);
    void on_description_and_battlecry(class DescriptionAndBattleCry *ev);
    void on_entity_info_request(class EntityInfoRequest *ev);
    void on_chat_reconfigured(class ChatReconfigure *ev);
    void on_switch_viewpoint(class SwitchViewPoint *ev);
    void on_client_options(class SaveClientOptions *ev);
    void on_set_default_power_send(class SetDefaultPowerSend *ev);
    void on_set_default_power(class SetDefaultPower *ev);
    void on_unqueue_all(class UnqueueAll *ev);
    void on_target_chat_channel_selected(class TargetChatChannelSelected *ev);
    void on_activate_inspiration(class ActivateInspiration *ev);
    void on_powers_dockmode(class PowersDockMode *ev);
    void on_switch_tray(class SwitchTray *ev);
    void on_select_keybind_profile(class SelectKeybindProfile *ev);
    void on_reset_keybinds(class ResetKeybinds *ev);
    void on_set_keybind(class SetKeybind *ev);
    void on_remove_keybind(class RemoveKeybind *ev);
    void on_emote_command(const QString &command, Entity *ent);
};
