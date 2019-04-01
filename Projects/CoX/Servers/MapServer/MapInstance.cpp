/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup MapServer Projects/CoX/Servers/MapServer
 * @{
 */

#include "MapInstance.h"

#include "DataHelpers.h"
#include "EntityStorage.h"
#include "Logging.h"
#include "MapManager.h"
#include "MapSceneGraph.h"
#include "MapServer.h"
#include "MapTemplate.h"
#include "MessageHelpers.h"
#include "SEGSTimer.h"
#include "SlashCommand.h"
#include "TimeEvent.h"
#include "TimeHelpers.h"
#include "WorldSimulation.h"
#include "serialization_common.h"
#include "serialization_types.h"
#include "Version.h"
#include "Common/GameData/CoHMath.h"
#include "Common/Servers/Database.h"
#include "Common/Servers/HandlerLocator.h"
#include "Common/Servers/InternalEvents.h"
#include "Common/Servers/MessageBus.h"
#include "GameData/Character.h"
#include "GameData/CharacterHelpers.h"
#include "GameData/Entity.h"
#include "GameData/GameDataStore.h"
#include "GameData/Trade.h"
#include "GameData/chardata_serializers.h"
#include "GameData/clientoptions_serializers.h"
#include "GameData/entitydata_serializers.h"
#include "GameData/keybind_serializers.h"
#include "GameData/map_definitions.h"
#include "GameData/playerdata_definitions.h"
#include "GameData/playerdata_serializers.h"
#include "GameData/Store.h"
#include "Messages/EmailService/EmailEvents.h"
#include "Messages/Game/GameEvents.h"
#include "Messages/GameDatabase/GameDBSyncEvents.h"
#include "Messages/Map/ClueList.h"
#include "Messages/Map/ContactList.h"
#include "Messages/Map/MapEvents.h"
#include "Messages/Map/MapXferRequest.h"
#include "Messages/Map/MapXferWait.h"
#include "Messages/Map/StoresEvents.h"
#include "Messages/Map/Tasks.h"

#include <ace/Reactor.h>

#include <QRegularExpression>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <random>
#include <stdlib.h>

using namespace SEGSEvents;
struct EntityIdxCompare;

namespace
{
    enum
    {
        World_Update_Timer   = 1,
        State_Transmit_Timer = 2,
        Session_Reaper_Timer   = 3,
        Link_Idle_Timer   = 4,
        Sync_Service_Update_Timer = 5,
        Afk_Update_Timer = 6,
        Lua_Timer = 7
    };

    const ACE_Time_Value reaping_interval(0,1000*1000);
    const ACE_Time_Value link_is_stale_if_disconnected_for(0,5*1000*1000);
    const ACE_Time_Value link_update_interval(0,500*1000);
    const ACE_Time_Value world_update_interval(0,1000*1000/WORLD_UPDATE_TICKS_PER_SECOND);
    const ACE_Time_Value sync_service_update_interval(0, 30000*1000);
    const ACE_Time_Value afk_update_interval(0, 1000 * 1000);
    const ACE_Time_Value lua_timer_interval(0, 1000 * 1000);
    const ACE_Time_Value resend_interval(0,250*1000);
    const CRUDLink::duration maximum_time_without_packets(2000);
    const constexpr int MinPacketsToAck = 5;

    void loadAndRunLua(std::unique_ptr<ScriptingEngine> &lua, const QString &locations_scriptname)
    {
        if(!QFile::exists(locations_scriptname))
            qCDebug(logScripts).noquote() << locations_scriptname << "is missing; Process will continue without it.";

        qCDebug(logScripts).noquote() << "Loading" << locations_scriptname;
        lua->loadAndRunFile(locations_scriptname);
    }
} // namespace

class MapLinkEndpoint final : public ServerEndpoint
{
public:
    MapLinkEndpoint(const ACE_INET_Addr &local_addr) : ServerEndpoint(local_addr) {}
    ~MapLinkEndpoint() override =default;
protected:
    CRUDLink *createLink(EventProcessor *down) override
    {
        return new MapLink(down,this);
    }
};

using namespace std;
MapInstance::MapInstance(const QString &mapdir_path, const ListenAndLocationAddresses &listen_addr)
  : m_data_path(mapdir_path), m_index(getMapIndex(mapdir_path.mid(mapdir_path.indexOf('/')))),
    m_addresses(listen_addr)
{
    m_world = new World(m_entities, getGameData().m_player_fade_in, this);
    m_scripting_interface.reset(new ScriptingEngine);
    m_scripting_interface->setIncludeDir(mapdir_path);
    m_endpoint = new MapLinkEndpoint(m_addresses.m_listen_addr); //,this
    m_endpoint->set_downstream(this);

    m_chat_service = new ChatService(m_entities);
}

void MapInstance::start(const QString &scenegraph_path)
{
    assert(m_world_update_timer==nullptr);
    assert(m_game_server_id!=255);
    m_scripting_interface->registerTypes();
    QFileInfo mapDataDirInfo(m_data_path);
    if(mapDataDirInfo.exists() && mapDataDirInfo.isDir())
    {
        qInfo() << "Loading map instance data...";
        bool scene_graph_loaded = false;
        Q_UNUSED(scene_graph_loaded);

        TIMED_LOG({
                m_map_scenegraph = new MapSceneGraph;
                scene_graph_loaded = m_map_scenegraph->loadFromFile("./data/geobin/" + scenegraph_path);
                m_all_spawners = m_map_scenegraph->getSpawnPoints();
            }, "Loading original scene graph");

        TIMED_LOG({
            m_map_scenegraph->spawn_npcs(this);
            m_npc_generators.generate(this);
            m_map_scenegraph->spawn_critters(this);
            m_critter_generators.generate(this);
            }, "Spawning npcs");

        // Load Lua Scripts for this Map Instance
        load_map_lua();
    }
    else
    {
        QDir::current().mkpath(m_data_path);
        qWarning() << "FAILED to load map instance data. Check to see if file exists:" << m_data_path;
    }

    m_session_store.create_reaping_timer(this,Session_Reaper_Timer,reaping_interval); // session cleaning
    init_timers();
    init_services();
}

void MapInstance::init_timers()
{
    // world simulation ticks
    m_world_update_timer = std::make_unique<SEGSTimer>(this, World_Update_Timer, world_update_interval, false);
    // state broadcast ticks
    m_resend_timer = std::make_unique<SEGSTimer>(this, State_Transmit_Timer, resend_interval, false);
    m_link_timer   = std::make_unique<SEGSTimer>(this, Link_Idle_Timer, link_update_interval, false);
    m_sync_service_timer =
        std::make_unique<SEGSTimer>(this, Sync_Service_Update_Timer, sync_service_update_interval, false);
    m_afk_update_timer = std::make_unique<SEGSTimer>(this, Afk_Update_Timer, afk_update_interval, false );

    //Lua timer
    m_lua_timer = std::make_unique<SEGSTimer>(this, Lua_Timer, lua_timer_interval, false );
    m_session_store.create_reaping_timer(this,Session_Reaper_Timer,reaping_interval); // session cleaning
}

void MapInstance::init_services()
{
    // create a GameDbSyncService
    m_sync_service = std::make_unique<GameDBSyncService>();
    m_sync_service->set_db_handler(m_game_server_id);
    m_sync_service->activate();

    m_email_service = EmailService();
    m_client_option_service = ClientOptionService();
    m_character_service = CharacterService();
    m_enhancement_service = EnhancementService();
    m_inspiration_service = InspirationService();
    m_power_service = PowerService();
    m_location_service = LocationService();
    m_trading_service = TradingService();
    m_zone_transfer_service = ZoneTransferService(this);
}

///
/// \fn MapInstance::load_lua
/// \brief This function should load the lua files from m_data_path
void MapInstance::load_map_lua()
{
    qInfo() << "Loading custom scripts";

    QStringList script_paths = {
        "scripts/global.lua", // global helper script
        // per zone scripts
        m_data_path+'/'+"contacts.lua",
        m_data_path+'/'+"locations.lua",
        m_data_path+'/'+"plaques.lua",
        m_data_path+'/'+"entities.lua",
        m_data_path+'/'+"missions.lua"

    };

    for(const QString &path : script_paths)
        loadAndRunLua(m_scripting_interface, path);
}

///
/// \fn MapInstance::spin_down
/// \brief This function should stop the world simulation, and save all entities to db
/// It is meant to be ran when MapInstance is getting `scrubbed` for reuse
void MapInstance::spin_down()
{
    qDebug() << "spin down ? I don't know how to stop!";
}

///
/// \fn MapInstance::spin_up_for
/// \brief This function should prepare the map for use by the specified game server
/// \param game_server_id
///
bool MapInstance::spin_up_for(uint8_t game_server_id,uint32_t owner_id,uint32_t instance_id)
{
    m_game_server_id = game_server_id;
    m_owner_id = owner_id;
    m_instance_id = instance_id;
    if(ACE_Reactor::instance()->register_handler(m_endpoint,ACE_Event_Handler::READ_MASK) == -1)
    {
        qWarning() << "MapInstance::spin_up_for failed to register_handler, port already open";
        return false;
    }
    if(m_endpoint->open() == -1) // will register notifications with current reactor
        ACE_ERROR_RETURN ((LM_ERROR, "(%P|%t) MapInstance: ServerEndpoint::open\n"),false);

    qInfo() << "Spun up MapInstance" << m_instance_id << "for MapServer" << m_owner_id;
    HandlerLocator::setMapInstance_Handler(m_owner_id, m_instance_id, this);

    return true;
}

MapInstance::~MapInstance()
{
    delete m_world;
    delete m_endpoint;

    // one last update on entities before termination of MapInstance, and in turn the SyncService as well
    on_update_entities();

    m_sync_service.reset();
}

void MapInstance::on_client_connected_to_other_server(ClientConnectedMessage */*ev*/)
{
    assert(false);
//    assert(ev->m_data.m_sub_server_id);
//    MapClientSession &session(m_session_store.sessionFromEvent(ev));
//    {
//        ACE_Guard<ACE_Thread_Mutex> guard(m_reaping_mutex);
//        // check if this session perhaps is in ready for reaping set
//        for(size_t idx=0,total=m_session_ready_for_reaping.size(); idx<total; ++idx)
//        {
//            if(m_session_ready_for_reaping[idx].m_session==&session)
//            {
//                std::swap(m_session_ready_for_reaping[idx],m_session_ready_for_reaping.back());
//                m_session_ready_for_reaping.pop_back();
//                break;
//            }
//        }
//    }
//    session.is_connected_to_map_server_id = ev->m_data.m_server_id;
//    session.is_connected_to_map_instance_id = ev->m_data.m_sub_server_id;
}

void MapInstance::on_client_disconnected_from_other_server(ClientDisconnectedMessage *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    session.is_connected_to_map_server_id = 0;
    session.is_connected_to_map_instance_id = 0;
    {
        SessionStore::MTGuard guard(m_session_store.reap_lock());
        m_session_store.mark_session_for_reaping(&session,ev->session_token());
    }
}

void MapInstance::reap_stale_links()
{
    ACE_Time_Value              time_now = ACE_OS::gettimeofday();
    EventProcessor *            tgt      = HandlerLocator::getGame_Handler(m_game_server_id);

    SessionStore::MTGuard guard(m_session_store.reap_lock());
    // TODO: How to get m_char_db_id in this?
    m_session_store.reap_stale_links("MapInstance",link_is_stale_if_disconnected_for,[tgt](uint64_t tok)
    {
        tgt->putq(new ClientDisconnectedMessage({tok, 0},0));
    });
}

void MapInstance::enqueue_client(MapClientSession *clnt)
{
    // m_world stores a ref to m_entities, so its entity mgr is updated as well
    m_entities.InsertPlayer(clnt->m_ent);

    //m_queued_clients.push_back(clnt); // enter this client on the waiting list
}

// Here we would add the handler call in case we get evCombineRequest :)
void MapInstance::dispatch( Event *ev )
{
    assert(ev);
    switch(ev->type())
    {
        case evTimeout:
            on_timeout(static_cast<Timeout *>(ev));
            break;
        case evDisconnect:
            on_link_lost(ev);
            break;
        case evExpectMapClientRequest:
            on_expect_client(static_cast<ExpectMapClientRequest *>(ev));
            break;
        case evExpectMapClientResponse:
            on_expect_client_response(static_cast<ExpectMapClientResponse *>(ev));
            break;
        case evWouldNameDuplicateResponse:
            on_name_clash_check_result(static_cast<WouldNameDuplicateResponse *>(ev));
            break;
        case evCreateNewCharacterResponse:
            on_character_created(static_cast<CreateNewCharacterResponse *>(ev));
            break;
        case evGetEntityResponse:
            on_entity_response(static_cast<GetEntityResponse *>(ev));
            break;
        case evGetEntityByNameResponse:
            on_entity_by_name_response(static_cast<GetEntityByNameResponse *>(ev));
            break;
        case evIdle:
            on_idle(static_cast<Idle *>(ev));
            break;
        case evConnectRequest:
            on_connection_request(static_cast<ConnectRequest *>(ev));
            break;
        case evSceneRequest:
            on_scene_request(static_cast<SceneRequest *>(ev));
            break;
        case evDisconnectRequest:
            on_disconnect(static_cast<DisconnectRequest *>(ev));
            break;
        case evNewEntity:
            on_create_map_entity(static_cast<NewEntity *>(ev));
            break;
        case evClientQuit:
            on_client_quit(static_cast<ClientQuit*>(ev));
            break;
        case evEntitiesRequest:
            on_entities_request(static_cast<EntitiesRequest *>(ev));
            break;
        case evShortcutsRequest:
            on_shortcuts_request(static_cast<ShortcutsRequest *>(ev));
            break;
        case evRecvInputState:
            on_input_state(static_cast<RecvInputState *>(ev));
            break;
        case evCookieRequest:
            on_cookie_confirm(static_cast<CookieRequest *>(ev));
            break;
        case evConsoleCommand:
            on_console_command(static_cast<ConsoleCommand *>(ev));
            break;
        case evClientResumedRendering:
            on_client_resumed(static_cast<ClientResumedRendering *>(ev));
            break;
        case evMiniMapState:
            on_minimap_state(static_cast<MiniMapState *>(ev));
            break;
        case evChatReconfigure:
            on_chat_reconfigured(static_cast<ChatReconfigure *>(ev));
            break;
        case evTargetChatChannelSelected:
            on_target_chat_channel_selected(static_cast<TargetChatChannelSelected *>(ev));
            break;
            // --------------- Power Service ---------------
        case evActivatePower:
            m_power_service.on_activate_power(m_session_store.session_from_event(ev).m_ent, ev);
            break;
        case evActivatePowerAtLocation:
            m_power_service.on_activate_power_at_location(m_session_store.session_from_event(ev).m_ent, ev);
            break;
        case evSetDefaultPower:
            m_power_service.on_set_default_power(m_session_store.session_from_event(ev).m_ent, ev);
            break;
        case evUnsetDefaultPower:
            m_power_service.on_unset_default_power(m_session_store.session_from_event(ev).m_ent, ev);
            break;
        case evPowersDockMode:
            m_power_service.on_powers_dockmode(m_session_store.session_from_event(ev).m_ent, ev);
            break;
        case evAbortQueuedPower:
            m_power_service.on_abort_queued_power(m_session_store.session_from_event(ev).m_ent, ev);
            break;
        case evRecvNewPower:
            m_power_service.on_recv_new_power(m_session_store.session_from_event(ev).m_ent, ev);
            break;
        case evChangeStance:
            m_power_service.on_change_stance(m_session_store.session_from_event(ev).m_ent, ev);
            break;
        case evUnqueueAll:
            m_power_service.on_unqueue_all(m_session_store.session_from_event(ev).m_ent, ev);
            break;
            // ---------- Client Option Service ------------
        case evSelectKeybindProfile:
            m_client_option_service.on_select_keybind_profile(m_session_store.session_from_event(ev).m_ent, ev);
            break;
        case evSetKeybind:
            m_client_option_service.on_set_keybind(m_session_store.session_from_event(ev).m_ent, ev);
            break;
        case evRemoveKeybind:
            m_client_option_service.on_remove_keybind(m_session_store.session_from_event(ev).m_ent, ev);
            break;
        case evResetKeybinds:
            m_client_option_service.on_reset_keybinds(m_session_store.session_from_event(ev).m_ent, ev);
            break;
        case evSwitchViewPoint:
            m_client_option_service.on_switch_viewpoint(m_session_store.session_from_event(ev).m_ent, ev);
            break;
        case evSaveClientOptions:
            m_client_option_service.on_client_options(m_session_store.session_from_event(ev).m_ent, ev);
            break;
        case evWindowState:
            m_client_option_service.on_window_state(m_session_store.session_from_event(ev).m_ent, ev);
            break;
        case evBrowserClose:
            m_client_option_service.on_browser_close(m_session_store.session_from_event(ev).m_ent, ev);
            break;
        case evChatDividerMoved:
            m_client_option_service.on_command_chat_divider_moved(m_session_store.session_from_event(ev).m_ent, ev);
            break;
            // ---------- Email Service ----------------
        case evEmailHeaderResponse:
            on_service_to_client_response(m_email_service.on_email_header_response(ev));
            break;
        case evEmailHeaderToClientMessage:
            on_service_to_client_response(m_email_service.on_email_header_to_client(ev));
            break;
        case evEmailHeadersToClientMessage:
            on_service_to_client_response(m_email_service.on_email_headers_to_client(ev));
            break;
        case evEmailReadResponse:
            on_service_to_client_response(m_email_service.on_email_read_response(ev));
            break;
        case evEmailWasReadByRecipientMessage:
            on_service_to_client_response(m_email_service.on_email_read_by_recipient(ev));
            break;
        case evEmailCreateStatusMessage:
            on_service_to_client_response(m_email_service.on_email_create_status(ev));
            break;
            // --------------- Inspiration Service -----------------
        case evMoveInspiration:
            m_inspiration_service.on_move_inspiration(m_session_store.session_from_event(ev).m_ent, ev);
            break;
        case evInspirationDockMode:
            m_inspiration_service.on_inspiration_dockmode(m_session_store.session_from_event(ev).m_ent, ev);
            break;
        case evActivateInspiration:
            on_service_to_client_response(m_inspiration_service.on_activate_inspiration(m_session_store.session_from_event(ev).m_ent, ev));
            break;
            // ------------- Enhancement Service --------------------
        case evCombineEnhancementsReq:
            m_enhancement_service.on_combine_enhancements(m_session_store.session_from_event(ev).m_ent, ev);
            break;
        case evMoveEnhancement:
            m_enhancement_service.on_move_enhancement(m_session_store.session_from_event(ev).m_ent, ev);
            break;
        case evSetEnhancement:
            m_enhancement_service.on_set_enhancement(m_session_store.session_from_event(ev).m_ent, ev);
            break;
        case evTrashEnhancement:
            m_enhancement_service.on_trash_enhancement(m_session_store.session_from_event(ev).m_ent, ev);
            break;
        case evTrashEnhancementInPower:
            m_enhancement_service.on_trash_enhancement_in_power(m_session_store.session_from_event(ev).m_ent, ev);
            break;
        case evBuyEnhancementSlot:
            m_enhancement_service.on_buy_enhancement_slot(m_session_store.session_from_event(ev).m_ent, ev);
            break;
            // ------------------ Character Service ---------------
        case evLevelUpResponse:
            m_character_service.on_levelup_response(m_session_store.session_from_event(ev).m_ent, ev);
            break;
        case evSwitchTray:
            m_character_service.on_switch_tray(m_session_store.session_from_event(ev).m_ent, ev);
            break;
        case evRecvSelectedTitles:
            m_character_service.on_recv_selected_titles(m_session_store.session_from_event(ev).m_ent, ev);
            break;
        case evRecvCostumeChange:
            m_character_service.on_levelup_response(m_session_store.session_from_event(ev).m_ent, ev);
            break;
        case evDescriptionAndBattleCry:
            m_character_service.on_description_and_battlecry(m_session_store.session_from_event(ev).m_ent, ev);
            break;
            // -------------------- Transaction Service ---------------
        case evTradeWasCancelledMessage:
            on_service_to_client_response(m_trading_service.on_trade_cancelled(m_session_store.session_from_event(ev).m_ent, ev));
            break;
        case evTradeWasUpdatedMessage:
            on_service_to_client_response(m_trading_service.on_trade_updated(m_session_store.session_from_event(ev).m_ent, ev));
            break;
        case evStoreSellItem:
            on_service_to_client_response(m_trading_service.on_store_sell_item(m_session_store.session_from_event(ev).m_ent, ev));
            break;
        case evStoreBuyItem:
            on_service_to_client_response(m_trading_service.on_store_buy_item(m_session_store.session_from_event(ev).m_ent, ev));
            break;
            // ------------------------- Location Service --------------------
        case evLocationVisited:
            on_service_to_client_response(m_location_service.on_location_visited(m_session_store.session_from_event(ev).m_ent, ev));
            break;
        case evPlaqueVisited:
            on_service_to_client_response(m_location_service.on_plaque_visited(m_session_store.session_from_event(ev).m_ent, ev));
            break;
        case evSetDestination:
            on_service_to_client_response(m_location_service.on_set_destination(m_session_store.session_from_event(ev).m_ent, ev));
            break;
            // ----------------------- Zone Transfer Service -----------------
        case evInitiateMapXfer:
            on_service_to_client_response(m_zone_transfer_service.on_initiate_map_transfer(
                                              (MapServer *)HandlerLocator::getMap_Handler(m_game_server_id),
                                              m_session_store.session_from_event(ev).link(),
                                              m_session_store.session_from_event(ev).m_ent, ev));
            break;
        case evMapXferComplete:
            m_zone_transfer_service.on_map_xfer_complete(m_session_store.session_from_event(ev).m_ent, closest_safe_location(m_session_store.session_from_event(ev).m_ent->m_entity_data.m_pos), ev);
            break;
        case evMapSwapCollisionMessage:
            on_service_to_client_response(m_zone_transfer_service.on_map_swap_collision(
                                              m_session_store.session_from_event(ev).link(),
                                              m_session_store.session_from_event(ev).m_ent, ev));
            break;
        case evEnterDoor:
            on_service_to_client_response(m_zone_transfer_service.on_enter_door(
                                              (MapServer *)HandlerLocator::getMap_Handler(m_game_server_id),
                                              m_session_store.session_from_event(ev).link(),
                                              m_session_store.session_from_event(ev).m_ent,
                                              m_index, ev));
            break;
        case evHasEnteredDoor:
            on_service_to_client_response(m_zone_transfer_service.on_has_entered_door(m_session_store.session_from_event(ev).m_ent, ev));
            break;
        case evAwaitingDeadNoGurney:
            on_service_to_client_response(m_zone_transfer_service.on_awaiting_dead_no_gurney(m_session_store.session_from_event(ev).m_ent, ev));
            break;
        case evDeadNoGurneyOK:
            on_service_to_client_response(m_zone_transfer_service.on_dead_no_gurney_ok(m_session_store.session_from_event(ev).m_ent, ev));
            break;
            // ------------------------- Interaction Service --------------------------
        case evInteractWithEntity:
            on_service_to_client_response(m_interaction_service.on_interact_with(m_session_store.session_from_event(ev).m_ent, ev));
            break;
        case evEntityInfoRequest:
            on_service_to_client_response(m_interaction_service.on_entity_info_request(m_session_store.session_from_event(ev).m_ent, ev));
            break;
        case evDialogButton:
            on_service_to_client_response(m_interaction_service.on_dialog_button(m_session_store.session_from_event(ev).m_ent, ev));
            break;
        case evReceiveContactStatus:
            on_service_to_client_response(m_interaction_service.on_receive_contact_status(m_session_store.session_from_event(ev).m_ent, ev));
            break;
        case evReceiveTaskDetailRequest:
            on_service_to_client_response(m_interaction_service.on_receive_task_detail_request(m_session_store.session_from_event(ev).m_ent, ev));
            break;
        case evSouvenirDetailRequest:
            on_service_to_client_response(m_interaction_service.on_souvenir_detail_request(m_session_store.session_from_event(ev).m_ent, ev));
            break;
        default:
            qCWarning(logMapEvents, "Unhandled MapEventTypes %u\n", ev->type()-MapEventTypes::base_MapEventTypes);
    }
}

void MapInstance::on_idle(Idle *ev)
{
    MapLink * lnk = (MapLink *)ev->src();
    // TODO: put idle sending on timer, which is reset each time some other packet is sent ?
    // we don't have to respond with an idle message here, check links will send idle events as needed.
    //lnk->putq(new Idle);
}

void MapInstance::on_check_links()
{
    // walk the active sessions
    for(const auto &entry : m_session_store)
    {
        MapLink * client_link = entry->link();
        if(!client_link)
            continue; // don't send to disconnected clients :)

        // Send at least one packet within maximum_time_without_packets
        if(client_link->last_sent_packets()>maximum_time_without_packets)
            client_link->putq(new Idle); // Threading trouble, last_sent_packets will not get updated until the packet is actually sent.
        else if(client_link->client_packets_waiting_for_ack()>MinPacketsToAck)
            client_link->putq(new Idle); // Threading trouble, last_sent_packets will not get updated until the packet is actually sent.
    }
}
void MapInstance::on_connection_request(ConnectRequest *ev)
{
    ev->src()->putq(new ConnectResponse);
}

void MapInstance::on_shortcuts_request(ShortcutsRequest *ev)
{
    // TODO: expend this to properly access the data from :
    // Shortcuts are part of UserData and that should be a part of Client entity which is a part of InstanceData
    // TODO: use the access level and send proper commands
    MapClientSession &session(m_session_store.session_from_event(ev));
    Shortcuts *res = new Shortcuts(&session);

    NetCommandManagerSingleton::instance()->UpdateCommandShortcuts(&session,res->m_commands);
    session.link()->putq(res);
}

void MapInstance::on_client_quit(ClientQuit*ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    // process client removal -> sending delete event to all clients etc.
    assert(session.m_ent);

    if(ev->abort_disconnect)
    {
        // forbid the player from aborting logout,
        // if logout is called automatically (through a loooong period of AFK)
        if(!session.m_ent->m_char->m_char_data.m_is_on_auto_logout)
            abortLogout(session.m_ent);
    }
    else
        session.m_ent->beginLogout(10);
}

void MapInstance::on_link_lost(Event *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    MapLink *lnk = session.link();
    uint64_t session_token = lnk->session_token();
    Entity *ent = session.m_ent;
    assert(ent);
    //todo: notify all clients about entity removal

    HandlerLocator::getGame_Handler(m_game_server_id)
            ->putq(new ClientDisconnectedMessage({session_token,session.m_ent->m_char->m_db_id},0));

    // one last character update for the disconnecting entity
    send_character_update(ent);
    m_entities.removeEntityFromActiveList(ent);

    m_session_store.session_link_lost(session_token);
    m_session_store.remove_by_token(session_token, session.auth_id());
     // close the link by puting an disconnect event there
    lnk->putq(new Disconnect(session_token));
}

void MapInstance::on_disconnect(DisconnectRequest *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    MapLink *lnk = session.link();
    uint64_t session_token = lnk->session_token();

    Entity *ent = session.m_ent;
    assert(ent);
    //todo: notify all clients about entity removal
    HandlerLocator::getGame_Handler(m_game_server_id)
            ->putq(new ClientDisconnectedMessage({session_token,session.m_ent->m_char->m_db_id},0));

    removeLFG(*ent);
    leaveTeam(*ent);
    updateLastOnline(*ent->m_char);

    // one last character update for the disconnecting entity
    send_character_update(ent);
    m_entities.removeEntityFromActiveList(ent);

    m_session_store.session_link_lost(session_token);
    m_session_store.remove_by_token(session_token, session.auth_id());

    lnk->putq(new DisconnectResponse);
    lnk->putq(new Disconnect(session_token)); // this should work, event if different threads try to do it in parallel
}

void MapInstance::on_name_clash_check_result(WouldNameDuplicateResponse *ev)
{
    if(ev->m_data.m_would_duplicate)
    {
        // name is taken, inform by setting cookie to 0.
        EventProcessor *tgt = HandlerLocator::getGame_Handler(m_game_server_id);
        tgt->putq(new ExpectMapClientResponse({0, 0, m_addresses.m_location_addr}, ev->session_token()));
    }
    else
    {
        uint32_t cookie = m_session_store.get_cookie_for_session(ev->session_token());
        assert(cookie!=0);
        // Now we inform our game server that this Map server instance is ready for the client
        MapClientSession &map_session(m_session_store.session_from_event(ev));
        m_session_store.locked_unmark_session_for_reaping(&map_session);
        HandlerLocator::getGame_Handler(m_game_server_id)
            ->putq(new ExpectMapClientResponse({2+cookie, 0, m_addresses.m_location_addr}, ev->session_token()));
    }
}

void MapInstance::on_expect_client_response(ExpectMapClientResponse *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    MapXferRequest *map_xfer_req = new MapXferRequest();
    map_xfer_req->m_address = ev->m_data.m_connection_addr;
    map_xfer_req->m_map_cookie = ev->m_data.cookie;
    session.link()->putq(map_xfer_req);
}

void MapInstance::on_expect_client( ExpectMapClientRequest *ev )
{
    // TODO: handle contention while creating 2 characters with the same name from different clients
    // TODO: SELECT account_id from characters where name=ev->m_character_name
    const ExpectMapClientRequestData &request_data(ev->m_data);

    // make sure that this is not a duplicate session
    uint32_t cookie = 0; // name in use
    // fill the session with data, this will get discarded if the name is already in use...
    MapClientSession &map_session(*m_session_store.create_or_reuse_session_for(ev->session_token()));
    map_session.m_name        = request_data.m_character_name;
    // TODO: this code is wrong on the logical level
    map_session.m_current_map = this;
    map_session.m_max_slots   = request_data.m_max_slots;
    map_session.m_access_level = request_data.m_access_level;
    map_session.m_client_id    = request_data.m_client_id;
    map_session.is_connected_to_game_server_id = m_game_server_id;
    cookie                    = 2 + m_session_store.expect_client_session(ev->session_token(), request_data.m_from_addr,
                                                     request_data.m_client_id);
    if(request_data.char_from_db_data.isEmpty())
    {
        EventProcessor *game_db = HandlerLocator::getGame_DB_Handler(m_game_server_id);
        game_db->putq(new WouldNameDuplicateRequest({request_data.m_character_name},ev->session_token(),this) );
        // while we wait for db response, mark session as waiting for reaping
        m_session_store.locked_mark_session_for_reaping(&map_session,ev->session_token());
        return;
    }
    GameAccountResponseCharacterData char_data;
    serializeFromQString(char_data,request_data.char_from_db_data);
    qCDebug(logCharSel).noquote() << "expected_client: Costume:" << char_data.m_serialized_costume_data;
    // existing character
    Entity *ent = m_entities.CreatePlayer();
    toActualCharacter(char_data, *ent->m_char, *ent->m_player, *ent->m_entity);
    ent->m_char->finalizeLevel(); // done here to fix spawn with 0 hp/end bug
    ent->fillFromCharacter(getGameData());
    ent->m_client = &map_session;
    map_session.m_ent = ent;
    // Now we inform our game server that this Map server instance is ready for the client

    ev->src()->putq(new ExpectMapClientResponse({cookie, 0, m_addresses.m_location_addr}, ev->session_token()));
}

void MapInstance::on_character_created(CreateNewCharacterResponse *ev)
{
    MapClientSession &map_session(m_session_store.session_from_event(ev));
    m_session_store.locked_unmark_session_for_reaping(&map_session);
    if(ev->m_data.slot_idx<0)
    {
        //TODO: this requires sending an actual error message to client and then disconnecting ?
        map_session.link()->putq(new DisconnectResponse);
        assert(false); //this code needs work;
        return;
    }

    map_session.m_ent->m_char->m_db_id = ev->m_data.m_char_id;
    map_session.m_ent->m_char->setIndex(ev->m_data.slot_idx);
    EventProcessor *game_db = HandlerLocator::getGame_DB_Handler(m_game_server_id);
    // now we go back to standard path -> asking db about the entity
    // TODO: we've just put the entity in the db, and now we have to load it back ??
    game_db->putq(new GetEntityRequest({ev->m_data.m_char_id},ev->session_token(),this));
    // while we wait for db response, mark session as waiting for reaping
    m_session_store.locked_mark_session_for_reaping(&map_session,ev->session_token());
}

void MapInstance::on_entity_response(GetEntityResponse *ev)
{
    MapClientSession &map_session(m_session_store.session_from_event(ev));
    m_session_store.locked_unmark_session_for_reaping(&map_session);
    Entity * e = map_session.m_ent;

    e->m_db_id              = e->m_char->m_db_id;
    e->m_supergroup.m_SG_id = ev->m_data.m_supergroup_id;
    serializeFromDb(e->m_entity_data, ev->m_data.m_ent_data);

    // Can't pass direction through cereal, so let's update it here.
    e->m_direction = fromCoHYpr(e->m_entity_data.m_orientation_pyr);

    // make sure to 'off' the AFK from the character in db first
    toggleAFK(*e->m_char, false);

    if(logPlayerSpawn().isDebugEnabled())
    {
        qCDebug(logPlayerSpawn).noquote() << "Dumping Entity Data during spawn:\n";
        map_session.m_ent->dump();
    }

    // Finalize level to calculate max attribs etc.
    e->m_char->finalizeLevel();

    // Tell our game server we've got the client
    EventProcessor *tgt = HandlerLocator::getGame_Handler(m_game_server_id);
    tgt->putq(new ClientConnectedMessage(
        {ev->session_token(),m_owner_id,m_instance_id,map_session.m_ent->m_char->m_db_id},0));

    map_session.m_current_map->enqueue_client(&map_session);
    setMapIdx(*map_session.m_ent, index());
    map_session.link()->putq(new MapInstanceConnected(this, 1, ""));
}

void MapInstance::on_entity_by_name_response(GetEntityByNameResponse *ev)
{
    // exactly the same function as above, this is just a test to see if getting entitydata by name is working
    MapClientSession &map_session(m_session_store.session_from_event(ev));
    m_session_store.locked_unmark_session_for_reaping(&map_session);
    Entity * e = map_session.m_ent;

    e->m_db_id              = e->m_char->m_db_id;
    e->m_supergroup.m_SG_id = ev->m_data.m_supergroup_id;
    serializeFromDb(e->m_entity_data, ev->m_data.m_ent_data);

    // Can't pass direction through cereal, so let's update it here.
    e->m_direction = fromCoHYpr(e->m_entity_data.m_orientation_pyr);

    if(logPlayerSpawn().isDebugEnabled())
    {
        qCDebug(logPlayerSpawn).noquote() << "Dumping Entity Data during spawn:\n";
        map_session.m_ent->dump();
    }

    // Tell our game server we've got the client
    EventProcessor *tgt = HandlerLocator::getGame_Handler(m_game_server_id);
    tgt->putq(new ClientConnectedMessage(
        {ev->session_token(),m_owner_id,m_instance_id, map_session.auth_id()},0));

    map_session.m_current_map->enqueue_client(&map_session);
    setMapIdx(*map_session.m_ent, index());
    map_session.link()->putq(new MapInstanceConnected(this, 1, ""));
}

void MapInstance::on_create_map_entity(NewEntity *ev)
{
    // TODO: At this point we should pre-process the NewEntity packet and let the proper CoXMapInstance handle the rest
    // of processing
    auto *lnk   = (MapLink *)ev->src();
    uint64_t token = m_session_store.connected_client(ev->m_cookie - 2);
    EventProcessor *game_db = HandlerLocator::getGame_DB_Handler(m_game_server_id);

    assert(token != ~0U);
    MapClientSession &map_session(m_session_store.session_from_token(token));
    map_session.link(lnk);
    lnk->session_token(token);
    if(ev->m_new_character)
    {
        QString ent_data;
        Entity *e = m_entities.CreatePlayer();

        const GameDataStore &data(getGameData());
        fillEntityFromNewCharData(*e, ev->m_character_data, data);

        e->m_char->m_account_id = map_session.auth_id();
        e->m_client = &map_session;
        map_session.m_ent = e;

        // Set player spawn position and PYR
        setPlayerSpawn(*e);

        e->m_entity_data.m_access_level = map_session.m_access_level;
        // new characters are transmitted nameless, use the name provided in on_expect_client
        e->m_char->setName(map_session.m_name);
        e->m_char->setIndex(map_session.m_requested_slot_idx);

        GameAccountResponseCharacterData char_data;
        fromActualCharacter(*e->m_char, *e->m_player, *e->m_entity, char_data);
        serializeToDb(e->m_entity_data, ent_data);

        qCDebug(logDB).noquote() << "received serialized Costume:" << char_data.m_serialized_costume_data;

        // create the character from the data.
        //fillGameAccountData(map_session.m_client_id, map_session.m_game_account);
        game_db->putq(new CreateNewCharacterRequest({char_data,ent_data, map_session.m_requested_slot_idx,
                                                     map_session.m_max_slots,map_session.m_client_id},
                                                    token,this));
    }
    else
    {
        assert(map_session.m_ent);

        // this is just to test if GetEntityByNameRequest is working, the plan is for other EventProcessors to use this event
        // game_db->putq(new GetEntityByNameRequest({map_session.m_ent->m_char->getName()}, lnk->session_token(), this));

        // this is what should be used
        game_db->putq(new GetEntityRequest({map_session.m_ent->m_char->m_db_id},lnk->session_token(),this));
    }
    // while we wait for db response, mark session as waiting for reaping
    m_session_store.locked_mark_session_for_reaping(&map_session,lnk->session_token());
}

void MapInstance::on_scene_request(SceneRequest *ev)
{
    auto *lnk = (MapLink *)ev->src();
    auto *res = new Scene;

    res->undos_PP              = 0;
    res->is_new_world          = true;
    res->m_outdoor_mission_map = false;
    res->m_map_number          = 1;

    assert(m_data_path.contains('_'));
    int city_idx = m_data_path.indexOf('/') + 1;
    int end_or_slash = m_data_path.indexOf('/',city_idx);
    assert(city_idx!=0);
    QString map_desc_from_path = m_data_path.mid(city_idx,end_or_slash==-1 ? -1 : m_data_path.size()-end_or_slash);
    res->m_map_desc        = QString("maps/City_Zones/%1/%1.txt").arg(map_desc_from_path);
    res->current_map_flags = true; // off 1
    res->unkn1             = 1;
    qDebug("Scene Request: unkn1: %d, undos_PP: %d, current_map_flags: %d", res->unkn1, res->undos_PP, res->current_map_flags);
    res->unkn2 = true;
    lnk->putq(res);
}
void MapInstance::on_entities_request(EntitiesRequest *ev)
{
    // this packet should start the per-client send-world-state-update timer
    // actually I think the best place for this timer would be the map instance.
    // so this method should call MapInstace->initial_update(MapClient *);
    MapClientSession &session(m_session_store.session_from_event(ev));
    EntitiesResponse *res=new EntitiesResponse();
    res->m_map_time_of_day = m_world->time_of_day();
    res->ent_major_update = true;
    res->abs_time = 30*100*(m_world->accumulated_time);
    buildEntityResponse(res,session,EntityUpdateMode::FULL,false);
    session.link()->putq(res);
    m_session_store.add_to_active_sessions(&session);
}

//! Handle instance-wide timers
void MapInstance::on_timeout(Timeout *ev)
{
    // TODO: This should send 'ping' packets on all client links to which we didn't send
    // anything in the last time quantum
    // 1. Find all links that have inactivity_time() > ping_time && <disconnect_time
    // For each found link
    //   If there is no ping_pending on this link, add a ping event to queue
    // 2. Find all links with inactivity_time() >= disconnect_time
    //   Disconnect given link.

    auto timer_id = ev->timer_id();
    switch (timer_id)
    {
        case World_Update_Timer:
            m_world->update(ev->arrival_time());
            break;
        case State_Transmit_Timer:
            sendState();
            break;
        case Link_Idle_Timer:
            on_check_links();
            break;
        case Session_Reaper_Timer:
            reap_stale_links();
            break;
        case Sync_Service_Update_Timer:
            on_update_entities();
            break;
        case Afk_Update_Timer:
            on_afk_update();
            break;
        case Lua_Timer:
            on_lua_update();
            break;
    }
}

void MapInstance::sendState()
{
    if(m_session_store.num_sessions()==0)
        return;

    auto iter=m_session_store.begin();
    auto end=m_session_store.end();

    for(;iter!=end; ++iter)
    {
        MapClientSession *cl = *iter;
        if(!cl->m_in_map)
            continue;

        EntitiesResponse *res = new EntitiesResponse();
        res->m_map_time_of_day = m_world->time_of_day();
        //while cl->m_in_map==false we send full updates until client `resumes`
        res->ent_major_update = true;
        res->abs_time = 30 * 100 * (m_world->accumulated_time);
        buildEntityResponse(res, *cl, cl->m_in_map ? EntityUpdateMode::INCREMENTAL : EntityUpdateMode::FULL, false);
        cl->link()->putq(res);
    }

    // This is handling instance-wide timers

    //TODO: Move timer processing to per-client EventHandler ?
    //1. Find the client that this timer corresponds to.
    //2. Call appropriate method ( keep-alive, Entities update etc .. )
    //3. Maybe use one timer for all links ?

}

void MapInstance::on_input_state(RecvInputState *st)
{
    MapClientSession &session(m_session_store.session_from_event(st));
    Entity *   ent = session.m_ent;

    // Save current position to last_pos
    ent->m_motion_state.m_last_pos      = ent->m_entity_data.m_pos;
    ent->m_states.current()->m_pos_end  = ent->m_entity_data.m_pos;
    st->m_next_state.m_pos_start        = ent->m_entity_data.m_pos;

    // Add new input state
    ent->m_states.addNewState(st->m_next_state);

    // Set current Input Packet ID
    if(st->m_next_state.m_full_input_packet)
        ent->m_input_pkt_id = st->m_next_state.m_send_id;

    // Check for input
    if(st->m_next_state.m_input_received)
        ent->m_has_input_on_timeframe = st->m_next_state.m_input_received;

    // Set Target
    if(st->m_next_state.m_has_target && (getTargetIdx(*ent) != st->m_next_state.m_target_idx))
    {
        ent->m_has_input_on_timeframe = true;
        setTarget(*ent, st->m_next_state.m_target_idx);
        //Not needed currently
        //auto val = m_scripting_interface->callFuncWithClientContext(&session,"set_target", st->m_next_state.m_target_idx);
    }

    // Set Orientation
    if(st->m_next_state.m_orientation_pyr.p || st->m_next_state.m_orientation_pyr.y || st->m_next_state.m_orientation_pyr.r)
    {
        ent->m_entity_data.m_orientation_pyr = st->m_next_state.m_orientation_pyr;
        ent->m_direction = fromCoHYpr(ent->m_entity_data.m_orientation_pyr);
    }

    // Input state messages can be followed by multiple commands.
    assert(st->m_user_commands.GetReadableBits()<32*1024*8); // simple sanity check ?
    // here we will try to extract all of them and put them on our processing queue
    std::vector<const char *> prev_commands = {};
    while(st->m_user_commands.GetReadableBits()>1)
    {
        MapLinkEvent *ev = MapEventFactory::CommandEventFromStream(st->m_user_commands);
        if(!ev)
            break;
        ev->serializefrom(st->m_user_commands);
        // copy source packet seq number to created command
        ev->m_seq_number = st->m_seq_number;
        ev->src(st->src());
        prev_commands.emplace_back(ev->info());
        // post the event to ourselves for dispatch
        putq(ev);
    }
    if(st->m_user_commands.GetReadableBits()!=0)
    {
        qCDebug(logMapEvents) << "bits: " << st->m_user_commands.GetReadableBits();
        qCCritical(logMapEvents) << "Not all bits were consumed by previous commands:";
        for(const char *cmd : prev_commands)
            qCWarning(logMapEvents) << cmd;
    }

    //TODO: do something here !
}

void MapInstance::on_cookie_confirm(CookieRequest * ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    qDebug("Received cookie confirm %x - %x\n", ev->cookie, ev->console);

    // just after sending this packet the client started waiting for resume , so we send it the update right now.
    EntitiesResponse *res = new EntitiesResponse();
    res->m_map_time_of_day = m_world->time_of_day();
    res->ent_major_update = true;
    res->abs_time = 30 * 100 * (m_world->accumulated_time);
    buildEntityResponse(res, session, EntityUpdateMode::FULL, false);
    session.link()->putq(res);
}

void MapInstance::on_console_command(ConsoleCommand * ev)
{
    QString contents = ev->contents.simplified();
    MapClientSession &session(m_session_store.session_from_event(ev));
    Entity *ent = session.m_ent;

    if(contents.contains("$")) // does it contain replacement strings?
        contents = m_chat_service->process_replacement_strings(ent, contents);

    //printf("Console command received %s\n",qPrintable(ev->contents));

    ent->m_has_input_on_timeframe = true;

    if(m_chat_service->isChatMessage(contents))
    {
        on_chat_service_to_client_response(m_chat_service->process_chat(ent,contents));
    }
    else if(m_chat_service->has_emote_prefix(contents))
    {
        on_chat_service_to_client_response(m_chat_service->on_emote_command(ent, contents));
    }
    else
    {
        runCommand(contents,session);
    }
}

void MapInstance::on_minimap_state(MiniMapState *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    Entity *ent = session.m_ent;

    qCDebug(logMiniMap) << "MiniMapState tile "<< ev->tile_idx << " for player" << ent->name();
    // TODO: Save these tile #s to dbase and (presumably) load upon entering map to remove fog-of-war from map
}

void MapInstance::on_client_resumed(ClientResumedRendering *ev)
{
    // TODO only do this the first time a client connects, not after map transfers..
    MapClientSession &session(m_session_store.session_from_event(ev));
    MapServer *map_server = (MapServer *)HandlerLocator::getMap_Handler(m_game_server_id);
    if(!session.m_in_map)
        session.m_in_map = true;
    if(!map_server->session_has_xfer_in_progress(session.link()->session_token()))
    {
        // Send current contact list
        sendContactStatusList(session);
        sendTaskStatusList(session);

        //Should these be combined?
        sendSouvenirList(session);
        sendClueList(session);

        // Force position and orientation to fix #617 spawn at 0,0,0 bug
        forcePosition(*session.m_ent, session.m_ent->m_entity_data.m_pos);
        forceOrientation(*session.m_ent, session.m_ent->m_entity_data.m_orientation_pyr);

        char buf[256];
        std::string welcome_msg = std::string("Welcome to SEGS ") + VersionInfo::getAuthVersion()+"\n";
        std::snprintf(buf, 256, "There are %zu active entities and %zu clients", m_entities.active_entities(),
                    m_session_store.num_sessions());
        welcome_msg += buf;
        sendInfoMessage(MessageChannel::SERVER,QString::fromStdString(welcome_msg),session);

        // Show MOTD only if it's been more than hour since last online
        // TODO: Make length of motd suppression configurable in settings.cfg
        QDateTime last_online = QDateTime::fromString(getLastOnline(*session.m_ent->m_char));
        QDateTime today = QDateTime::currentDateTime();
        const GameDataStore &data(getGameData()); // for motd timer
        if(last_online.addSecs(data.m_motd_timer) < today)
            sendServerMOTD(&session);

        // send Lua connection method?
        // auto val = m_scripting_interface->callFuncWithClientContext(&session,"player_connected", session.m_ent->m_idx);
    }
    else
    {
        MapXferData &map_data = map_server->session_map_xfer_idx(session.link()->session_token());
        
        if(!m_all_spawners.empty() && m_all_spawners.contains(map_data.m_target_spawn_name))
        {
            setSpawnLocation(*session.m_ent, map_data.m_target_spawn_name);
        }
        else
        {
            setPlayerSpawn(*session.m_ent);
        }        

        // else don't send motd, as this is from a map transfer
        // TODO: check if there's a better place to complete the map transfer..
        map_server->session_xfer_complete(session.link()->session_token());
    }

    // Call Lua Connected function.
    auto val = m_scripting_interface->callFuncWithClientContext(&session,"player_connected", session.m_ent->m_idx);
}

void MapInstance::on_chat_reconfigured(ChatReconfigure *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    Entity *ent = session.m_ent;

    ent->m_player->m_gui.m_chat_top_flags = ev->m_chat_top_flags;
    ent->m_player->m_gui.m_chat_bottom_flags = ev->m_chat_bottom_flags;

    qCDebug(logMapEvents) << "Saving chat channel mask settings to GUISettings" << ev->m_chat_top_flags << ev->m_chat_bottom_flags;
}

void MapInstance::on_target_chat_channel_selected(TargetChatChannelSelected *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    Entity *ent = session.m_ent;

    ent->m_player->m_gui.m_cur_chat_channel = ev->m_chat_type;
    qCDebug(logMapEvents) << "Saving chat channel type to GUISettings:" << ev->m_chat_type;
}

void MapInstance::setPlayerSpawn(Entity &e)
{
    // Spawn player position and PYR
    glm::vec3 spawn_pos = glm::vec3(128.0f,16.0f,-198.0f);
    glm::vec3 spawn_pyr = glm::vec3(0.0f, 0.0f, 0.0f);

    if(!m_all_spawners.empty())
    {
        glm::mat4 v = glm::mat4(1.0f);

        if(m_all_spawners.contains("NewPlayer"))
            v = m_all_spawners.values("NewPlayer")[rand() % m_all_spawners.values("NewPlayer").size()];
        else if(m_all_spawners.contains("PlayerSpawn"))
            v = m_all_spawners.values("PlayerSpawn")[rand() % m_all_spawners.values("PlayerSpawn").size()];
        else if(m_all_spawners.contains("Citywarp01"))
            v = m_all_spawners.values("Citywarp01")[rand() % m_all_spawners.values("Citywarp01").size()];
        else
        {
            qWarning() << "No default spawn location found. Spawning at random spawner";
            v = m_all_spawners.values()[rand() % m_all_spawners.values().size()];
        }

        // Position
        spawn_pos = glm::vec3(v[3]);

        // Orientation
        auto valquat = glm::quat_cast(v);
        spawn_pyr = toCoH_YPR(valquat);
    }

    forcePosition(e, spawn_pos);
    forceOrientation(e, spawn_pyr);
}

// Teleport to a specific SpawnLocation; do nothing if the SpawnLocation is not found.
void MapInstance::setSpawnLocation(Entity &e, const QString &spawnLocation)
{
    if(m_all_spawners.empty() || !m_all_spawners.contains(spawnLocation))
        return;

    glm::mat4 v = m_all_spawners.values(spawnLocation)[rand() % m_all_spawners.values(spawnLocation).size()];

    // Position
    glm::vec3 spawn_pos = glm::vec3(v[3]);

    // Orientation
    auto valquat = glm::quat_cast(v);
    glm::vec3 spawn_pyr = toCoH_YPR(valquat);
    forcePosition(e, spawn_pos);
    forceOrientation(e, spawn_pyr);
}

glm::vec3 MapInstance::closest_safe_location(glm::vec3 v) const
{
    // In the future this should get the closet NAV or NAVCMBT
    // node and return it. For now let's just pick some known
    // safe spawn locations

    Q_UNUSED(v);
    glm::vec3 loc = glm::vec3(0,0,0);

    // If no default spawners if available, spawn at 0,0,0
    if(m_all_spawners.empty())
        return loc;

    // Try NewPlayer spawners first, then hospitals, then random
    if(m_all_spawners.contains("NewPlayer"))
        loc = m_all_spawners.values("NewPlayer")[rand() % m_all_spawners.values("NewPlayer").size()][3];
    else if(m_all_spawners.contains("PlayerSpawn"))
        loc = m_all_spawners.values("PlayerSpawn")[rand() % m_all_spawners.values("PlayerSpawn").size()][3];
    else if(m_all_spawners.contains("LinkFrom_Monorail_Red"))
        loc = m_all_spawners.values("LinkFrom_Monorail_Red")[rand() % m_all_spawners.values("LinkFrom_Monorail_Red").size()][3];
    else if(m_all_spawners.contains("LinkFrom_Monorail_Blue"))
        loc = m_all_spawners.values("LinkFrom_Monorail_Blue")[rand() % m_all_spawners.values("LinkFrom_Monorail_Blue").size()][3];
    else if(m_all_spawners.contains("Citywarp01"))
        loc = m_all_spawners.values("Citywarp01")[rand() % m_all_spawners.values("Citywarp01").size()][3];
    else if(m_all_spawners.contains("Hospital_Exit"))
        loc = m_all_spawners.values("Hospital_Exit")[rand() % m_all_spawners.values("Hospital_Exit").size()][3];
    else
        loc = m_all_spawners.values()[rand() % m_all_spawners.values().size()][3];

    return loc;
}

void MapInstance::serialize_from(istream &/*is*/)
{
    assert(false);
}

void MapInstance::serialize_to(ostream &/*is*/)
{
    assert(false);
}

void MapInstance::on_afk_update()
{
    const std::vector<MapClientSession *> &active_sessions (m_session_store.get_active_sessions());
    const GameDataStore &data(getGameData());
    QString msg;

    for (const auto &sess : active_sessions)
    {
        Entity *e = sess->m_ent;
        CharacterData* cd = &e->m_char->m_char_data;

        if(e->m_has_input_on_timeframe == false)
            cd->m_idle_time += afk_update_interval.sec();
        else
        {
            msg = QString("Receiving input from player: ") + &e->m_char->getName();
            qCDebug(logInput) << msg;
            cd->m_idle_time = 0;

            if(cd->m_afk)
                toggleAFK(*e->m_char, false);
            cd->m_is_on_auto_logout = false;

            e->m_has_input_on_timeframe = false;
        }

        if(cd->m_idle_time >= data.m_time_to_afk && !cd->m_afk)
        {
            toggleAFK(* e->m_char, true, "Auto AFK");
            msg = QString("You are AFKed after %1 seconds of inactivity.").arg(data.m_time_to_afk);
            sendInfoMessage(MessageChannel::DEBUG_INFO, msg, *sess);
        }


        if(data.m_uses_auto_logout && cd->m_idle_time >= data.m_time_to_logout_msg)
        {
            // give message that character will be auto logged out in 2 mins
            // player must not be on task force and is not on mission map for this to happen
            if(!cd->m_is_on_task_force && !isEntityOnMissionMap(e->m_entity_data) && !cd->m_is_on_auto_logout)
            {
                cd->m_is_on_auto_logout = true;
                msg = QString("You have been inactive for %1 seconds. You will automatically ").arg(data.m_time_to_logout_msg) +
                      QString("be logged out if you stay idle for %1 seconds").arg(data.m_time_to_auto_logout);
                sendInfoMessage(MessageChannel::DEBUG_INFO, msg, *sess);
            }
        }

        if(cd->m_is_on_auto_logout && cd->m_idle_time >=
                data.m_time_to_logout_msg + data.m_time_to_auto_logout
                && !e->m_is_logging_out)
        {
            e->beginLogout(30);
            msg = "You have been inactive for too long. Beginning auto-logout process...";
            sendInfoMessage(MessageChannel::DEBUG_INFO, msg, *sess);
        }
    }

}

void MapInstance::on_lua_update()
{
    int count = 0;
    for(const auto &t: m_lua_timers)
    {
        if(t.m_remove)
        {
            m_lua_timers.erase(m_lua_timers.begin() + count);
            break;
        }

        if(t.m_is_enabled && t.m_on_tick_callback != NULL)
        {
            m_scripting_interface->updateMapInstance(this);
            int64_t time = getSecsSince2000Epoch();
            int64_t diff = time - t.m_start_time;
            t.m_on_tick_callback(t.m_start_time, diff, time);
        }

        ++count;
    }
}

void MapInstance::on_update_entities()
{
    const std::vector<MapClientSession *> &active_sessions (m_session_store.get_active_sessions());

    // all active sessions are for player, so we don't need to verify if db_id != 0
    for (const auto &sess : active_sessions)
    {
        Entity *e = sess->m_ent;
        send_character_update(e);
        updateLastOnline(*e->m_char); // set this here, in case we disconnect unexpectedly

        /* at the moment we are forcing full character updates, so I'll leave this commented for now

        // full character update
        if(e->m_db_store_flags & uint32_t(DbStoreFlags::Full))
            send_character_update(e);
        // update only player data
        else if(e->m_db_store_flags & uint32_t(DbStoreFlags::PlayerData))
            send_player_update(e);

        */
    }
}

void MapInstance::send_character_update(Entity *e)
{
    QString cerealizedCharData, cerealizedEntityData, cerealizedPlayerData, cerealizedCostumeData;

    PlayerData playerData = PlayerData({
                e->m_player->m_gui,
                e->m_player->m_keybinds,
                e->m_player->m_options,
                e->m_player->m_contacts,
                e->m_player->m_tasks_entry_list,
                e->m_player->m_clues,
                e->m_player->m_souvenirs,
                e->m_player->m_player_statistics });

    serializeToQString(*e->m_char->getAllCostumes(), cerealizedCostumeData);
    serializeToQString(e->m_char->m_char_data, cerealizedCharData);
    serializeToQString(e->m_entity_data, cerealizedEntityData);
    serializeToQString(playerData, cerealizedPlayerData);

    CharacterUpdateMessage* msg = new CharacterUpdateMessage(
                CharacterUpdateData({
                                        e->m_char->getName(),

                                        // cerealized blobs
                                        cerealizedCostumeData,
                                        cerealizedCharData,
                                        cerealizedEntityData,
                                        cerealizedPlayerData,

                                        // plain values
                                        uint32_t(e->m_supergroup.m_SG_id),
                                        e->m_char->m_db_id
        }), (uint64_t)1);

    m_sync_service->putq(msg);
    unmarkEntityForDbStore(e, DbStoreFlags::Full);
}

void MapInstance::send_player_update(Entity *e)
{
    QString cerealizedPlayerData;

    PlayerData playerData = PlayerData({
                e->m_player->m_gui,
                e->m_player->m_keybinds,
                e->m_player->m_options,
                e->m_player->m_contacts,
                e->m_player->m_tasks_entry_list,
                e->m_player->m_clues,
                e->m_player->m_souvenirs,
                e->m_player->m_player_statistics });

    serializeToQString(playerData, cerealizedPlayerData);

    PlayerUpdateMessage* msg = new PlayerUpdateMessage(
                PlayerUpdateData({
                                     e->m_char->m_db_id,
                                     cerealizedPlayerData
                                 }), (uint64_t)1);

    m_sync_service->putq(msg);
    unmarkEntityForDbStore(e, DbStoreFlags::PlayerData);
}

void MapInstance::on_service_to_client_response(std::unique_ptr<ServiceToClientData> data)
{
    if (data == nullptr)
        return;

    // generally only ONE of these is filled
    // if both of them are not filled, we'd be in trouble
    if (data->m_token == 0 && data->m_ent == nullptr)
        return;

    try
    {
        // if token is empty, get it from the entity
        MapClientSession& session = data->m_token != 0
                ? m_session_store.session_from_token(data->m_token)
                : *data->m_ent->m_client;

        for (auto &command : data->m_commands)
            session.addCommandToSendNextUpdate(std::move(command));

        MapServer *map_server = (MapServer *)HandlerLocator::getMap_Handler(m_game_server_id);

        for (auto &mapServerEvent : data->m_map_server_events)
        {
            // it's a nullptr when initialized in the service
            EventSrc* src = mapServerEvent->src();
            src = this;
            map_server->putq(mapServerEvent);
        }


        for (auto &script : data->m_scripts)
        {
            if (script->flags & uint32_t(ScriptingServiceFlags::CallFuncWithClientContext))
            {
                string val;
                if (!script->charArg.isEmpty())
                    val = m_scripting_interface->callFuncWithClientContext(&session, qPrintable(script->funcName), qPrintable(script->charArg), script->locArg);
                // uses intArg
                else if (script->intArg != 0)
                {
                    if (script->locArg != glm::vec3(999, 999, 999))
                        val = m_scripting_interface->callFuncWithClientContext(&session, qPrintable(script->funcName), script->intArg, script->locArg);
                    else
                        val = m_scripting_interface->callFuncWithClientContext(&session, qPrintable(script->funcName), script->intArg);
                }

                if (val.empty() && script->on_val_empty != nullptr)
                    script->on_val_empty(*data->m_ent);

                if (!val.empty() && script->on_val_not_empty != nullptr)
                    script->on_val_not_empty(*data->m_ent);
            }

            if (script->flags & uint32_t(ScriptingServiceFlags::UpdateClientContext))
                m_scripting_interface->updateClientContext(&session);

            if (script->flags & uint32_t(ScriptingServiceFlags::UpdateMapInstance))
                m_scripting_interface->updateMapInstance(this);
        }

        // is not null and is not empty
        if (data->m_message.isEmpty() && data->m_message.isNull())
            sendInfoMessage(MessageChannel::DEBUG_INFO, data->m_message, session);
    }
    catch(std::exception &e)
    {
        qCritical() << e.what();
    }
}

void MapInstance::on_chat_service_to_client_response(std::unique_ptr<ChatServiceToClientData> data)
{
    if (data == nullptr)
        return;

    assert(data->m_source != nullptr);
    assert(data->m_source->m_client != nullptr);

    for (Entity* ent : data->m_targets)
    {
        assert(ent->m_client != nullptr);
        sendChatMessage(data->m_message_channel, data->m_message, data->m_source, *ent->m_client);
    }

    if (!data->m_send_info_msg_to_self)
        return;

    sendInfoMessage(data->m_channel_for_self, data->m_message_to_self, *data->m_source->m_client);
}

void MapInstance::add_chat_message(Entity *sender, QString &msg_text)
{
    on_chat_service_to_client_response(m_chat_service->process_chat(sender, msg_text));
}

QHash<QString, MapXferData> MapInstance::get_map_transfers()
{
    return m_map_transfers;
}

QHash<QString, MapXferData> MapInstance::get_map_door_transfers()
{
    if (!m_door_transfers_checked)
    {
        QHash<QString, MapXferData>::const_iterator i = m_map_transfers.constBegin();
        while (i != m_map_transfers.constEnd())
        {
            if (i.value().m_transfer_type == MapXferType::DOOR)
            {
                m_map_door_transfers.insert(i.key(), i.value());
            }
            i++;
        }
        m_door_transfers_checked = true;
    }

    return m_map_door_transfers;
}

QHash<QString, MapXferData> MapInstance::get_map_zone_transfers()
{
    if (!m_zone_transfers_checked)
    {
        QHash<QString, MapXferData>::const_iterator i = m_map_transfers.constBegin();
        while (i != m_map_transfers.constEnd())
        {
            if (i.value().m_transfer_type == MapXferType::ZONE)
            {
                m_map_zone_transfers.insert(i.key(), i.value());
            }
            i++;
        }
        m_zone_transfers_checked = true;
    }
    return m_map_zone_transfers;
}

QString MapInstance::getNearestDoor(glm::vec3 location)
{
    float door_distance_check = 15.f;
    QHash<QString, MapXferData>::const_iterator i = get_map_door_transfers().constBegin();
    while (i != get_map_door_transfers().constEnd())
    {
        if (glm::distance(location, i.value().m_position) < door_distance_check)
        {
            return i.value().m_target_spawn_name;
        }
        i++;
    }
    return QString();
}

void MapInstance::startTimer(uint32_t entity_idx)
{
    int count = 0;
    bool found = false;
    for(auto &t: this->m_lua_timers)
    {
        if(t.m_entity_idx == entity_idx)
        {
            found = true;
            break;
        }
        ++count;
    }
    if(found)
    {
        this->m_lua_timers[count].m_is_enabled = true;
        this->m_lua_timers[count].m_start_time = getSecsSince2000Epoch();
    }
}

void MapInstance::stopTimer(uint32_t entity_idx)
{
    int count = 0;
    bool found = false;
    for(auto &t: this->m_lua_timers)
    {
        if(t.m_entity_idx == entity_idx)
        {
            found = true;
            break;
        }
        ++count;
    }
    if(found)
        this->m_lua_timers[count].m_is_enabled = false;
}

void MapInstance::clearTimer(uint32_t entity_idx)
{
    int count = 0;
    bool found = false;
    for(auto &t: this->m_lua_timers)
    {
        if(t.m_entity_idx == entity_idx)
        {
            found = true;
            break;
        }
        ++count;
    }
    if(found)
        this->m_lua_timers[count].m_remove = true;
}


//! @}
