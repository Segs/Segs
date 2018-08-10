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

#include <map>
#include <memory>
#include <vector>

#define WORLD_UPDATE_TICKS_PER_SECOND 30

class MapServer;
class SEGSTimer;
class World;
class MapServerData;
class MapSceneGraph;
namespace SEGSEvents
{
class InputState;
class IdleEvent;
class DisconnectRequest;
class SceneRequest;
class EntitiesRequest;
class NewEntity;
// server<-> server event types
struct ExpectMapClientRequest;

}

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
        void                    dispatch(SEGSEvents::Event *ev) override;

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
        void                    on_client_connected_to_other_server(SEGSEvents::ClientConnectedMessage *ev);
        void                    on_client_disconnected_from_other_server(SEGSEvents::ClientDisconnectedMessage *ev);
        void                    process_chat(MapClientSession *sender, QString &msg_text);
        // DB -> Server messages
        void                    on_name_clash_check_result(SEGSEvents::WouldNameDuplicateResponse *ev);
        void                    on_character_created(SEGSEvents::CreateNewCharacterResponse *ev);
        void                    on_entity_response(SEGSEvents::GetEntityResponse *ev);
        void                    on_entity_by_name_response(SEGSEvents::GetEntityByNameResponse *ev);
        // Server->Server messages
        void on_expect_client(SEGSEvents::ExpectMapClientRequest *ev);

        void on_link_lost(SEGSEvents::Event *ev);
        void on_disconnect(SEGSEvents::DisconnectRequest *ev);
        void on_scene_request(SEGSEvents::SceneRequest *ev);
        void on_entities_request(SEGSEvents::EntitiesRequest *ev);
        void on_create_map_entity(SEGSEvents::NewEntity *ev);
        void on_timeout(SEGSEvents::Timeout *ev);
        void on_combine_boosts(SEGSEvents::CombineRequest *);
        void on_input_state(SEGSEvents::InputState *st);
        void on_idle(SEGSEvents::IdleEvent *ev);
        void on_shortcuts_request(SEGSEvents::ShortcutsRequest *ev);

        void sendState();
        void on_check_links();

        void on_cookie_confirm(SEGSEvents::CookieRequest *ev);
        void on_window_state(SEGSEvents::WindowState *ev);
        void on_console_command(SEGSEvents::ConsoleCommand *ev);
        void on_client_quit(SEGSEvents::ClientQuit *ev);
        void on_connection_request(SEGSEvents::ConnectRequest *ev);
        void on_command_chat_divider_moved(SEGSEvents::ChatDividerMoved *ev);
        void on_minimap_state(SEGSEvents::MiniMapState *ev);
        void on_client_resumed(SEGSEvents::ClientResumedRendering *ev);
        void on_location_visited(SEGSEvents::LocationVisited *ev);
        void on_plaque_visited(SEGSEvents::PlaqueVisited *ev);
        void on_inspiration_dockmode(SEGSEvents::InspirationDockMode *ev);
        void on_enter_door(SEGSEvents::EnterDoor *ev);
        void on_change_stance(SEGSEvents::ChangeStance *ev);
        void on_set_destination(SEGSEvents::SetDestination *ev);
        void on_abort_queued_power(SEGSEvents::AbortQueuedPower *ev);
        void on_description_and_battlecry(SEGSEvents::DescriptionAndBattleCry *ev);
        void on_entity_info_request(SEGSEvents::EntityInfoRequest *ev);
        void on_chat_reconfigured(SEGSEvents::ChatReconfigure *ev);
        void on_switch_viewpoint(SEGSEvents::SwitchViewPoint *ev);
        void on_client_options(SEGSEvents::SaveClientOptions *ev);
        void on_set_default_power_send(SEGSEvents::SetDefaultPowerSend *ev);
        void on_set_default_power(SEGSEvents::SetDefaultPower *ev);
        void on_unqueue_all(SEGSEvents::UnqueueAll *ev);
        void on_target_chat_channel_selected(SEGSEvents::TargetChatChannelSelected *ev);
        void on_activate_inspiration(SEGSEvents::ActivateInspiration *ev);
        void on_powers_dockmode(SEGSEvents::PowersDockMode *ev);
        void on_switch_tray(SEGSEvents::SwitchTray *ev);
        void on_select_keybind_profile(SEGSEvents::SelectKeybindProfile *ev);
        void on_reset_keybinds(SEGSEvents::ResetKeybinds *ev);
        void on_set_keybind(SEGSEvents::SetKeybind *ev);
        void on_remove_keybind(SEGSEvents::RemoveKeybind *ev);
        void on_emote_command(const QString &command, Entity *ent);
        void on_interact_with(SEGSEvents::InteractWithEntity *ev);
};
