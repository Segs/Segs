/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "EntityStorage.h"
#include "EventProcessor.h"
#include "Common/Servers/ClientManager.h"
#include "Servers/ServerEndpoint.h"
#include "Servers/GameDatabase/GameDBSyncService.h"
#include "ScriptingEngine.h"
#include "MapClientSession.h"
#include "NpcGenerator.h"
#include "Events/FriendHandlerEvents.h"

#include <map>
#include <memory>
#include <vector>

#define WORLD_UPDATE_TICKS_PER_SECOND 30

class IdleEvent;
class MapServer;
class SEGSTimer;
class InputState;
class World;
class MapServerData;
struct MapSceneGraph;

// server<-> server event types
struct ExpectMapClientRequest;

class MapLinkEndpoint;

class MapInstance final : public EventProcessor
{
        using SessionStore = ClientSessionStore<MapClientSession>;
        using ScriptEnginePtr = std::unique_ptr<ScriptingEngine>;
        QString                m_data_path;
        uint32_t               m_index = 1; // what does client expect this to store, and where do we send it?
        std::unique_ptr<SEGSTimer> m_world_update_timer;
        std::unique_ptr<SEGSTimer> m_resend_timer;
        std::unique_ptr<SEGSTimer> m_link_timer;
        std::unique_ptr<SEGSTimer> m_sync_service_timer;
        std::vector<glm::mat4>  m_new_player_spawns;
        World *                 m_world;
        GameDBSyncService*      m_sync_service;
        uint8_t                 m_game_server_id=255; // 255 is `invalid` id
        uint32_t                m_owner_id;
        uint32_t                m_instance_id;

public:
        SessionStore            m_session_store;
        EntityManager           m_entities;
        ScriptEnginePtr         m_scripting_interface;
        MapLinkEndpoint *       m_endpoint = nullptr;
        ListenAndLocationAddresses m_addresses; //! this value is sent to the clients
        MapSceneGraph *         m_map_scenegraph;
        NpcGeneratorStore       m_npc_generators;

public:
                                MapInstance(const QString &name,const ListenAndLocationAddresses &listen_addr);
                                ~MapInstance() override;
        void                    dispatch(SEGSEvent *ev) override;

        void                    enqueue_client(MapClientSession *clnt);
        void                    start(const QString &scenegraph_path);
        const QString &         name() const { return m_data_path; }
        uint32_t                index() const { return m_index; }
        void                    spin_down();
        bool                    spin_up_for(uint8_t game_server_id, uint32_t owner_id, uint32_t instance_id);
        const MapServerData &   serverData() const;
        glm::vec3               closest_safe_location(glm::vec3 v) const;
protected:
        void                    reap_stale_links();
        void                    on_client_connected_to_other_server(ClientConnectedMessage *ev);
        void                    on_client_disconnected_from_other_server(ClientDisconnectedMessage *ev);
        void                    process_chat(MapClientSession *sender, QString &msg_text);
        // DB -> Server messages
        void                    on_name_clash_check_result(WouldNameDuplicateResponse *ev);
        void                    on_character_created(CreateNewCharacterResponse *ev);
        void                    on_entity_response(GetEntityResponse *ev);
        // Server->Server messages
        void on_expect_client(ExpectMapClientRequest *ev);
        void on_update_friendslist(SendFriendListMessage *ev);

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
        void on_check_links();

        void on_cookie_confirm(class CookieRequest *ev);
        void on_window_state(class WindowState *ev);
        void on_console_command(class ConsoleCommand *ev);
        void on_client_quit(class ClientQuit *ev);
        void on_connection_request(class ConnectRequest *ev);
        void on_command_chat_divider_moved(class ChatDividerMoved *ev);
        void on_minimap_state(class MiniMapState *ev);
        void on_client_resumed(class ClientResumedRendering *ev);
        void on_location_visited(class LocationVisited *ev);
        void on_plaque_visited(class PlaqueVisited *ev);
        void on_inspiration_dockmode(class InspirationDockMode *ev);
        void on_enter_door(class EnterDoor *ev);
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
        void on_interact_with(class InteractWithEntity *ev);
};
