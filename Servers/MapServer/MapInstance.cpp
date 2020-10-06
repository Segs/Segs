/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
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
#include "Components/Logging.h"
#include "MapManager.h"
#include "MapSceneGraph.h"
#include "MapServer.h"
#include "MapTemplate.h"
#include "MessageHelpers.h"
#include "Components/SEGSTimer.h"
#include "SlashCommands/SlashCommand.h"
#include "Components/TimeEvent.h"
#include "Components/TimeHelpers.h"
#include "WorldSimulation.h"
#include "Components/serialization_common.h"
#include "Components/serialization_types.h"
#include "Version.h"
#include "ScriptingEngine/ScriptingEngine.h"
#include "Common/GameData/CoHMath.h"
#include "Common/GameData/LFG.h"
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
#include "Messages/Map/TeamLooking.h"
#include "Messages/Map/TeamOffer.h"
#include "Messages/EmailService/EmailEvents.h"
#include "Messages/Game/GameEvents.h"
#include "Messages/GameDatabase/GameDBSyncEvents.h"
#include "Messages/Map/ClueList.h"
#include "Messages/Map/ContactList.h"
#include "Messages/Map/EmailHeaders.h"
#include "Messages/Map/EmailMessageStatus.h"
#include "Messages/Map/EmailRead.h"
#include "Messages/Map/FloatingInfoStyles.h"
#include "Messages/Map/LevelUp.h"
#include "Messages/Map/MapEvents.h"
#include "Messages/Map/MapXferRequest.h"
#include "Messages/Map/MapXferWait.h"
#include "Messages/Map/PlayerInfo.h"
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
    const ACE_Time_Value reaping_interval(0,1000*1000);
    const ACE_Time_Value link_is_stale_if_disconnected_for(5,0);
    const ACE_Time_Value link_update_interval(0,500*1000);
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
MapInstance::MapInstance(const QString &mapdir_path, const ListenAndLocationAddresses &listen_addr, const bool is_mission_map)
  : m_data_path(mapdir_path),
    m_index(getMapIndex(mapdir_path.mid(mapdir_path.indexOf('/')))),
    m_addresses(listen_addr), m_is_mission_map(is_mission_map)
{
    m_world = new World(m_entities, getGameData().m_player_fade_in, this);
    m_scripting_interface.reset(new ScriptingEngine);
    m_scripting_interface->setIncludeDir(mapdir_path);
    m_endpoint = new MapLinkEndpoint(m_addresses.m_listen_addr); //,this
    m_endpoint->set_downstream(this);

    initServices();
}

void MapInstance::initServices()
{
    m_email_service = {};
    m_client_option_service = {};
}

void MapInstance::startTimers()
{
    // world simulation ticks
    m_world_update_timer = addTimer(m_world_update_interval); //World_Update_Timer
    startTimer(m_world_update_timer,[this](const ACE_Time_Value &at) {m_world->update(at);});
    // state broadcast ticks
    m_resend_timer = addTimer(resend_interval);
    startTimer(m_resend_timer,&MapInstance::sendState);
    m_link_timer   = addTimer(link_update_interval);
    startTimer(m_link_timer, &MapInstance::on_check_links);
    m_sync_service_timer = addTimer(sync_service_update_interval);
    startTimer(m_sync_service_timer,&MapInstance::on_update_entities);
    m_afk_update_timer = addTimer(afk_update_interval);
    startTimer(m_afk_update_timer,&MapInstance::on_afk_update);

    //Lua timer
    m_lua_timer_id = addTimer(lua_timer_interval);
    startTimer(m_lua_timer_id,&MapInstance::on_lua_update);

    // session cleaning
    m_session_reaping_timer = addTimer(reaping_interval);
    startTimer(m_session_reaping_timer,&MapInstance::reap_stale_links);
}
void MapInstance::start(const QString &scenegraph_path)
{
    assert(m_game_server_id!=255);
    m_registered_timers.clear();
    m_scripting_interface->registerTypes();
    QFileInfo mapDataDirInfo(m_data_path);
    if(mapDataDirInfo.exists() && mapDataDirInfo.isDir())
    {
        qInfo() << "Loading map instance data...";
        bool scene_graph_loaded = false;
        Q_UNUSED(scene_graph_loaded)

        TIMED_LOG({
                m_map_scenegraph = new MapSceneGraph;
                scene_graph_loaded = m_map_scenegraph->loadFromFile(("./data/geobin/" + scenegraph_path).toUtf8());
                m_map_transfers = m_map_scenegraph->get_map_transfers();
            }, "Loading original scene graph")

        TIMED_LOG({
            m_map_scenegraph->spawn_npcs(this);         // handles persistents, Spawndef, npc/vehicle encounters
            m_npc_generators.generate(this);            // handles doors, monorails, trains
            m_all_spawners = m_map_scenegraph->getSpawnPoints();    // used for locating player spawn points
            }, "Spawning npcs")

        // Set correct MapInstance in scripting engine
        m_scripting_interface->updateMapInstance(this);
        // Load Lua Scripts for this Map Instance
        load_map_lua();
    }
    else
    {
        QDir::current().mkpath(m_data_path);
        qWarning() << "FAILED to load map instance data. Check to see if file exists:" << m_data_path;
    }

    // create a GameDbSyncService
    m_sync_service = new GameDBSyncService();
    m_sync_service->set_db_handler(m_game_server_id);
    m_sync_service->activate();

    startTimers();
}

///
/// \fn MapInstance::load_lua
/// \brief This function should load the lua files from m_data_path
void MapInstance::load_map_lua()
{
    qInfo() << "Loading custom scripts";

    QStringList script_paths = {
        "scripts/global.lua",                   // global helper script
        "scripts/Universal_Spawns.lua",         // Spawndef and functionality for universal critters
        "scripts/Encounter_Manager.lua",        // used by all maps for encounter generation
        "scripts/ES_OL_Functions.lua",          // used by all maps for object library reference functionality
        "scripts/Global_NPC_Extras.lua",        // universal "faction table" for civilian NPCs, vehicles and later police drones
        "scripts/Global_Persistents.lua",       // universal file for referencing persistent NPCs/model lookup
        "scripts/spawners.lua",                 // handles exposed Scenegraph data for all Lua-side managed spawning activity

        // per zone scripts
        m_data_path + '/'+"ES_Library_Objects.lua",
        m_data_path + '/'+"contacts.lua",
        m_data_path + '/'+"locations.lua",
        m_data_path + '/'+"plaques.lua",
        m_data_path + '/'+"entities.lua",
        m_data_path + '/'+"missions.lua"
    };

    for(const QString &path : script_paths)
        loadAndRunLua(m_scripting_interface, path);
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
    delete m_sync_service;
}

void MapInstance::on_client_connected_to_other_server(ClientConnectedMessage * /*ev*/)
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


void MapInstance::dispatch( Event *ev )
{
    assert(ev);
    switch(ev->type())
    {
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
        case evEnterDoor:
            on_enter_door(static_cast<EnterDoor *>(ev));
            break;
        case evChangeStance:
            on_change_stance(static_cast<ChangeStance *>(ev));
            break;
        case evSetDestination:
            on_set_destination(static_cast<SetDestination *>(ev));
            break;
        case evHasEnteredDoor:
            on_has_entered_door(static_cast<HasEnteredDoor *>(ev));
            break;
        case evWindowState:
            on_window_state(static_cast<WindowState *>(ev));
            break;
        case evInspirationDockMode:
            on_inspiration_dockmode(static_cast<InspirationDockMode *>(ev));
            break;
        case evPowersDockMode:
            on_powers_dockmode(static_cast<PowersDockMode *>(ev));
            break;
        case evAbortQueuedPower:
            on_abort_queued_power(static_cast<AbortQueuedPower *>(ev));
            break;
        case evConsoleCommand:
            on_console_command(static_cast<ConsoleCommand *>(ev));
            break;
        case evChatDividerMoved:
            on_command_chat_divider_moved(static_cast<ChatDividerMoved *>(ev));
            break;
        case evClientResumedRendering:
            on_client_resumed(static_cast<ClientResumedRendering *>(ev));
            break;
        case evMiniMapState:
            on_minimap_state(static_cast<MiniMapState *>(ev));
            break;
        case evLocationVisited:
            on_location_visited(static_cast<LocationVisited *>(ev));
            break;
        case evChatReconfigure:
            on_chat_reconfigured(static_cast<ChatReconfigure *>(ev));
            break;
        case evPlaqueVisited:
            on_plaque_visited(static_cast<PlaqueVisited *>(ev));
            break;
        case evSwitchViewPoint:
            on_service_to_entity_response(m_client_option_service.on_switch_viewpoint(ev));
            break;
        case evSaveClientOptions:
            on_service_to_entity_response(m_client_option_service.on_save_client_options(ev));
            break;
        case evDescriptionAndBattleCry:
            on_description_and_battlecry(static_cast<DescriptionAndBattleCry *>(ev));
            break;
        case evSetDefaultPower:
            on_set_default_power(static_cast<SetDefaultPower *>(ev));
            break;
        case evUnsetDefaultPower:
            on_unset_default_power(static_cast<UnsetDefaultPower *>(ev));
            break;
        case evUnqueueAll:
            on_unqueue_all(static_cast<UnqueueAll *>(ev));
            break;
        case evActivatePower:
            on_activate_power(static_cast<ActivatePower *>(ev));
            break;
        case evActivatePowerAtLocation:
            on_activate_power_at_location(static_cast<ActivatePowerAtLocation *>(ev));
            break;
        case evActivateInspiration:
            on_activate_inspiration(static_cast<ActivateInspiration *>(ev));
            break;
        case evInteractWithEntity:
            on_interact_with(static_cast<InteractWithEntity *>(ev));
            break;
        case evSwitchTray:
            on_switch_tray(static_cast<SwitchTray *>(ev));
            break;
        case evTargetChatChannelSelected:
            on_target_chat_channel_selected(static_cast<TargetChatChannelSelected *>(ev));
            break;
        case evEntityInfoRequest:
            on_entity_info_request(static_cast<EntityInfoRequest *>(ev));
            break;
        case evSelectKeybindProfile:
            on_service_to_entity_response(m_client_option_service.on_select_keybind_profile(ev));
            break;
        case evSetKeybind:
            on_service_to_entity_response(m_client_option_service.on_set_keybind(ev));
            break;
        case evRemoveKeybind:
            on_service_to_entity_response(m_client_option_service.on_remove_keybind(ev));
            break;
        case evResetKeybinds:
            on_service_to_entity_response(m_client_option_service.on_reset_keybinds(ev));
            break;
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
        case evMoveInspiration:
            on_move_inspiration(static_cast<MoveInspiration *>(ev));
            break;
        case evRecvSelectedTitles:
            on_recv_selected_titles(static_cast<RecvSelectedTitles *>(ev));
            break;
        case evDialogButton:
            on_dialog_button(static_cast<DialogButton *>(ev));
            break;
        case evCombineEnhancementsReq:
            on_combine_enhancements(static_cast<CombineEnhancementsReq *>(ev));
            break;
        case evMoveEnhancement:
            on_move_enhancement(static_cast<MoveEnhancement *>(ev));
            break;
        case evSetEnhancement:
            on_set_enhancement(static_cast<SetEnhancement *>(ev));
            break;
        case evTrashEnhancement:
            on_trash_enhancement(static_cast<TrashEnhancement *>(ev));
            break;
        case evTrashEnhancementInPower:
            on_trash_enhancement_in_power(static_cast<TrashEnhancementInPower *>(ev));
            break;
        case evBuyEnhancementSlot:
            on_buy_enhancement_slot(static_cast<BuyEnhancementSlot *>(ev));
            break;
        case evRecvNewPower:
            on_recv_new_power(static_cast<RecvNewPower *>(ev));
            break;
        case evTradeWasCancelledMessage:
            on_trade_cancelled(static_cast<TradeWasCancelledMessage *>(ev));
            break;
        case evTradeWasUpdatedMessage:
            on_trade_updated(static_cast<TradeWasUpdatedMessage *>(ev));
            break;
        case evInitiateMapXfer:
            on_initiate_map_transfer(static_cast<InitiateMapXfer *>(ev));
            break;
        case evMapXferComplete:
            on_map_xfer_complete(static_cast<MapXferComplete *>(ev));
            break;
        case evMapSwapCollisionMessage:
            on_map_swap_collision(static_cast<MapSwapCollisionMessage *>(ev));
            break;
        case evAwaitingDeadNoGurney:
            on_awaiting_dead_no_gurney(static_cast<AwaitingDeadNoGurney *>(ev));
            break;
        case evDeadNoGurneyOK:
            on_dead_no_gurney_ok(static_cast<DeadNoGurneyOK *>(ev));
            break;
        case evBrowserClose:
            on_browser_close(static_cast<BrowserClose *>(ev));
            break;
        case evRecvCostumeChange:
            on_recv_costume_change(static_cast<RecvCostumeChange *>(ev));
            break;
        case evLevelUpResponse:
            on_levelup_response(static_cast<LevelUpResponse *>(ev));
            break;
        case evReceiveContactStatus:
            on_receive_contact_status(static_cast<ReceiveContactStatus *>(ev));
            break;
        case evTeamMemberInvitedMessage:
            on_team_member_invited(static_cast<TeamMemberInvitedMessage *>(ev));
            break;
        case evTeamToggleLFGMessage:
            on_team_toggle_lfg(static_cast<TeamToggleLFGMessage *>(ev));
            break;
        case evTeamRefreshLFGMessage:
            on_team_refresh_lfg(static_cast<TeamRefreshLFGMessage *>(ev));
            break;
        case evTeamUpdatedMessage:
            on_team_updated(static_cast<TeamUpdatedMessage *>(ev));
            break;
        case evTeamMemberKickedMessage:
            on_team_member_kicked(static_cast<TeamMemberKickedMessage *>(ev));
            break;
        case evTeamLeaveTeamMessage:
            on_team_leave_team(static_cast<TeamLeaveTeamMessage *>(ev));
            break;
        case evReceiveTaskDetailRequest:
            on_receive_task_detail_request(static_cast<ReceiveTaskDetailRequest *>(ev));
            break;
        case evSouvenirDetailRequest:
            on_souvenir_detail_request(static_cast<SouvenirDetailRequest *>(ev));
            break;
        case evStoreSellItem:
            on_store_sell_item(static_cast<StoreSellItem *>(ev));
            break;
        case evStoreBuyItem:
            on_store_buy_item(static_cast<StoreBuyItem *>(ev));
            break;
        default:
            qCWarning(logMapEvents, "Unhandled MapEventTypes %u\n", ev->type()-MapEventTypes::base_MapEventTypes);
    }
}

void MapInstance::on_initiate_map_transfer(InitiateMapXfer *ev)
{

    MapClientSession &session(m_session_store.session_from_event(ev));
    MapLink *lnk = session.link();
    MapServer *map_server = (MapServer *)HandlerLocator::getMap_Handler(m_game_server_id);
    if (!map_server->session_has_xfer_in_progress(lnk->session_token()))
    {
        qCDebug(logMapXfers) << QString(
                                    "Client Session %1 attempting to initiate transfer with no map data message received")
                                    .arg(session.link()->session_token());
        return;
    }

    // This is used here to get the map idx to send to the client for the transfer, but we
    // remove it from the std::map after the client has sent us the ClientRenderingResumed event so we
    // can prevent motd showing every time.
    MapXferData &map_xfer = map_server->session_map_xfer_idx(lnk->session_token());
    GameAccountResponseCharacterData c_data;
    QString serialized_data;

    fromActualCharacter(*session.m_ent->m_char, *session.m_ent->m_player, *session.m_ent->m_entity, c_data);
    serializeToQString(c_data, serialized_data);
    QString map_path = getMapPath(map_xfer.m_target_map_name);
    qInfo() << "Map transfer initiated to map path: " << map_path;
    ExpectMapClientRequest *map_req = new ExpectMapClientRequest({session.auth_id(), session.m_access_level, lnk->peer_addr(),
                                    serialized_data, session.m_requested_slot_idx, session.m_name, map_path,
                                    session.m_max_slots},
                                    lnk->session_token(),this);
    map_server->putq(map_req);
}

void MapInstance::on_map_xfer_complete(MapXferComplete *ev)
{
    // TODO: Do anything necessary after connecting to new map instance here.
    MapClientSession &session(m_session_store.session_from_event(ev));
    forcePosition(*session.m_ent, session.m_current_map->closest_safe_location(session.m_ent->m_entity_data.m_pos));
    session.m_ent->m_map_swap_collided = false;
}

void MapInstance::on_idle(Idle */*ev*/)
{
    // MapLink * lnk = (MapLink *)ev->src();

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

    m_session_store.session_link_lost(session_token,"MapInstance: link was lost");
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

    m_session_store.session_link_lost(session_token,"MapInstance: client disconnected");
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
    if (request_data.char_from_db_data.isEmpty())
    {
        EventProcessor *game_db = HandlerLocator::getGame_DB_Handler(m_game_server_id);
        game_db->putq(new WouldNameDuplicateRequest({request_data.m_character_name}, ev->session_token(), this));
        // while we wait for db response, mark session as waiting for reaping
        m_session_store.locked_mark_session_for_reaping(&map_session, ev->session_token(),
                                                        "MapSession: Awaiting DB response - duplicate name check");
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
    m_session_store.locked_mark_session_for_reaping(&map_session,ev->session_token(),
                                                    "MapSession: Awaiting DB response - retrieve char");
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
    bool was_afk = isAFK(*e->m_char);
    setAFK(*e->m_char, false);
    if(was_afk)
        sendInfoMessage(MessageChannel::DEBUG_INFO, "You are no longer AFK", map_session);

    e->m_entity_update_flags.setFlag(e->UpdateFlag::AFK); // status of afk has changed

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
    m_session_store.locked_mark_session_for_reaping(&map_session,lnk->session_token(),
                                                    "MapSession: Awaiting DB response - create/retrieve a character");
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
    qInfo() << "Scene Request for map path: " << map_desc_from_path;

    MapClientSession &session(m_session_store.session_from_event(ev));

    QString map_path       = getMapPath(map_desc_from_path);
    res->m_map_desc        = map_path;
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
    session.m_ent->m_entity_update_flags.setFlag(session.m_ent->UpdateFlag::FULL);

    res->abs_time = 30*100*(m_world->accumulated_time);
    buildEntityResponse(res,session,EntityUpdateMode::FULL,false);
    session.link()->putq(res);
    m_session_store.add_to_active_sessions(&session);
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

void MapInstance::on_combine_enhancements(CombineEnhancementsReq *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    CombineResult res=combineEnhancements(*session.m_ent, ev->first_power, ev->second_power);
    sendEnhanceCombineResponse(session, res.success, res.destroyed);
    session.m_ent->m_char->m_char_data.m_has_updated_powers = res.success || res.destroyed;

    qCDebug(logMapEvents) << "Entity: " << session.m_ent->m_idx << "wants to merge enhancements" /*<< ev->first_power << ev->second_power*/;
}

void MapInstance::on_input_state(RecvInputState *st)
{
    MapClientSession &session(m_session_store.session_from_event(st));
    Entity *   ent = session.m_ent;

    // Add new input state change
    ent->m_input_state.m_queued_changes.push_back(st->m_input_state_change);

    // Set current Input Packet ID
    if(st->m_input_state_change.m_control_state_changes.size())
    {
        // todo: when corrections are turned on, I think this needs to be m_input_ack?
        // Also perhaps it should be the last csc id in this packet, not the first?
        ent->m_input_pkt_id = st->m_input_state_change.m_first_control_state_change_id;
    }

    // Check for input
    ent->m_has_input_on_timeframe = st->m_input_state_change.hasInput();

    // Set Target
    if(st->m_input_state_change.m_has_target && (getTargetIdx(*ent) != st->m_input_state_change.m_target_idx))
    {
        ent->m_has_input_on_timeframe = true;
        setTarget(*ent, st->m_input_state_change.m_target_idx);
        //Not needed currently
        //auto val = m_scripting_interface->callFuncWithClientContext(&session,"set_target", st->m_next_state.m_target_idx);
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

void MapInstance::on_window_state(WindowState * ev)
{
    // Save GUISettings to character entity and entry in the database.
    MapClientSession &session(m_session_store.session_from_event(ev));
    Entity *e = session.m_ent;

    int idx = ev->wnd.m_idx;
    e->m_player->m_gui.m_wnds.at(idx) = ev->wnd;

    qCDebug(logGUI) << "Received window state" << ev->wnd.m_idx << "-" << ev->wnd.m_mode;
    if(logGUI().isDebugEnabled())
        e->m_player->m_gui.m_wnds.at(idx).guiWindowDump();
}

QString processReplacementStrings(MapClientSession *sender, const QString &msg_text)
{
    /*
    // $$           - newline
    // $archetype   - the archetype of your character
    // $battlecry   - your character's battlecry, as entered on your character ID screen
    // $level       - your character's current level
    // $name        - your character's name
    // $origin      - your character's origin
    // $target      - your currently selected target's name
    */

    QString new_msg = msg_text;
    static const QStringList replacements = {
        "\\$\\$",
        "\\$archetype",
        "\\$battlecry",
        "\\$level",
        "\\$name",
        "\\$origin",
        "\\$target"
    };

    const Character &c(*sender->m_ent->m_char);

    QString  sender_class       = QString(getClass(c)).remove("Class_");
    QString  sender_battlecry   = getBattleCry(c);
    uint32_t sender_level       = getLevel(c);
    QString  sender_char_name   = c.getName();
    QString  sender_origin      = getOrigin(c);
    uint32_t target_idx         = getTargetIdx(*sender->m_ent);

    qCDebug(logChat) << "src -> tgt: " << sender->m_ent->m_idx  << "->" << target_idx;

    Entity *tgt = getEntity(sender, target_idx);
    QString target_char_name = tgt->name(); // change name

    foreach (const QString &str, replacements)
    {
        if(str == "\\$archetype")
            new_msg.replace(QRegularExpression(str), sender_class);
        else if(str == "\\$battlecry")
            new_msg.replace(QRegularExpression(str), sender_battlecry);
        else if(str == "\\$level")
            new_msg.replace(QRegularExpression(str), QString::number(sender_level));
        else if(str == "\\$name")
            new_msg.replace(QRegularExpression(str), sender_char_name);
        else if(str == "\\$origin")
            new_msg.replace(QRegularExpression(str), sender_origin);
        else if(str == "\\$target")
            new_msg.replace(QRegularExpression(str), target_char_name);
        else if(str == "\\$\\$")
        {
            if(new_msg.contains(str))
                qCDebug(logChat) << "need to send newline for" << str; // This apparently works client-side.
        }
    }
    return new_msg;
}

static bool isChatMessage(const QString &msg)
{
    static const QStringList chat_prefixes = {
            "l", "local",
            "b", "broadcast", "y", "yell",
            "g", "group", "sg", "supergroup",
            "req", "request",
            "f",
            "t", "tell", "w", "whisper", "p", "private"
    };
    QString space(msg.mid(0,msg.indexOf(' ')));
    return chat_prefixes.contains(space);
}

static MessageChannel getKindOfChatMessage(const QStringRef &msg)
{
    if(msg=="l" || msg=="local")                                                            // Aliases: local, l
        return MessageChannel::LOCAL;
    if(msg=="b" || msg=="broadcast" || msg=="y" || msg=="yell")                             // Aliases: broadcast, yell, b, y
        return MessageChannel::BROADCAST;
    if(msg=="g" || msg=="group" || msg=="team")                                             // Aliases: team, g, group
        return MessageChannel::TEAM;
    if(msg=="sg" || msg=="supergroup")                                                      // Aliases: sg, supergroup
        return MessageChannel::SUPERGROUP;
    if(msg=="req" || msg=="request" || msg=="auction" || msg=="sell")                       // Aliases: request, req, auction, sell
        return MessageChannel::REQUEST;
    if(msg=="f")                                                                            // Aliases: f
        return MessageChannel::FRIENDS;
    if(msg=="t" || msg=="tell" || msg=="w" || msg=="whisper" || msg=="p" || msg=="private") // Aliases: t, tell, whisper, w, private, p
        return MessageChannel::PRIVATE;
    // unknown chat types are processed as local chat
    return MessageChannel::LOCAL;
}




void MapInstance::process_chat(Entity *sender, QString &msg_text)
{
    int first_space = msg_text.indexOf(QRegularExpression("\\s"), 0); // first whitespace, as the client sometimes sends tabs
    QString sender_char_name;
    QString prepared_chat_message;

    QStringRef cmd_str(msg_text.midRef(0,first_space));
    QStringRef msg_content(msg_text.midRef(first_space+1,msg_text.lastIndexOf("\n")));
    MessageChannel kind = getKindOfChatMessage(cmd_str);
    std::vector<MapClientSession *> recipients;

    MapClientSession *sender_sess;

    if(!sender)
      return;

    sender_char_name = sender->name();

    for(MapClientSession *cl : m_session_store)
    {
        if(cl->m_ent->m_db_id == sender->m_db_id)
        {
            sender_sess = cl;
            break;
        }
    }

    switch(kind)
    {
        case MessageChannel::LOCAL:
        {
            // send only to clients within range
            glm::vec3 senderpos = sender->m_entity_data.m_pos;
            for(MapClientSession *cl : m_session_store)
            {
                glm::vec3 recpos = cl->m_ent->m_entity_data.m_pos;
                float range = 50.0f; // range of "hearing". I assume this is in yards
                float dist = glm::distance(senderpos,recpos);

                qCDebug(logChat, "senderpos: %f %f %f", senderpos.x, senderpos.y, senderpos.z);
                qCDebug(logChat, "recpos: %f %f %f", recpos.x, recpos.y, recpos.z);
                qCDebug(logChat, "sphere: %f", range);
                qCDebug(logChat, "dist: %f", dist);

                if(dist<=range)
                    recipients.push_back(cl);
            }
            prepared_chat_message = QString("[Local] %1: %2").arg(sender_char_name,msg_content.toString());
            for(MapClientSession * cl : recipients)
            {
                sendChatMessage(MessageChannel::LOCAL,prepared_chat_message,sender,*cl);
            }
            break;
        }
        case MessageChannel::BROADCAST:
        {
            // send the message to everyone on this map
            std::copy(m_session_store.begin(),m_session_store.end(),std::back_insert_iterator<std::vector<MapClientSession *>>(recipients));
            prepared_chat_message = QString(" %1: %2").arg(sender_char_name,msg_content.toString()); // where does [Broadcast] come from? The client?
            for(MapClientSession * cl : recipients)
            {
                sendChatMessage(MessageChannel::BROADCAST,prepared_chat_message,sender,*cl);
            }
            break;
        }
        case MessageChannel::REQUEST:
        {
            // send the message to everyone on this map
            std::copy(m_session_store.begin(),m_session_store.end(),std::back_insert_iterator<std::vector<MapClientSession *>>(recipients));
            prepared_chat_message = QString(" %1: %2").arg(sender_char_name,msg_content.toString());
            for(MapClientSession * cl : recipients)
            {
                sendChatMessage(MessageChannel::REQUEST,prepared_chat_message,sender,*cl);
            }
            break;
        }
        case MessageChannel::PRIVATE:
        {
            int first_comma = msg_text.indexOf(',');
            QStringRef target_name_ref(msg_text.midRef(first_space+1,(first_comma - first_space-1)));
            msg_content = msg_text.midRef(first_comma+1,msg_text.lastIndexOf("\n"));

            QString target_name = target_name_ref.toString();
            qCDebug(logChat) << "Private Chat:"
                             << "\n\t" << "target_name:" << target_name
                             << "\n\t" << "msg_text:" << msg_text;

            Entity *tgt = nullptr;
            for(MapClientSession *cl : m_session_store)
            {
                if(cl->m_ent->name() == target_name)
                {
                    tgt = cl->m_ent;
                    break;
                }
            }

            qWarning() << "Private chat: this only work for players on local server. We should introduce a message router, and send messages to EntityIDs instead of directly using sessions.";

            if(tgt == nullptr)
            {
                prepared_chat_message = QString("No player named \"%1\" currently online.").arg(target_name);
                sendInfoMessage(MessageChannel::USER_ERROR,prepared_chat_message, *sender_sess);
                break;
            }
            else
            {
                prepared_chat_message = QString(" --> %1: %2").arg(target_name,msg_content.toString());
                sendInfoMessage(MessageChannel::PRIVATE, prepared_chat_message, *sender_sess); // in this case, sender is target

                prepared_chat_message = QString(" %1: %2").arg(sender_char_name,msg_content.toString());
                sendChatMessage(MessageChannel::PRIVATE, prepared_chat_message, sender, *tgt->m_client);
            }

            break;
        }
        case MessageChannel::TEAM:
        {
            if(!sender->m_has_team)
            {
                prepared_chat_message = "You are not a member of a Team.";
                sendInfoMessage(MessageChannel::USER_ERROR, prepared_chat_message, *sender_sess);
                break;
            }

            qWarning() << "Team chat: this only work for members on local server. We should introduce a message router, and send messages to EntityIDs instead of directly using sessions.";

            // Only send the message to characters on sender's team
            for(MapClientSession *cl : m_session_store)
            {
                if(sender->m_team->m_data.m_team_idx == cl->m_ent->m_team->m_data.m_team_idx)
                    recipients.push_back(cl);
            }
            prepared_chat_message = QString(" %1: %2").arg(sender_char_name,msg_content.toString());
            for(MapClientSession * cl : recipients)
            {
                sendChatMessage(MessageChannel::TEAM, prepared_chat_message, sender, *cl);
            }
            break;
        }
        case MessageChannel::SUPERGROUP:
        {
            if(!sender->m_has_supergroup)
            {
                prepared_chat_message = "You are not a member of a SuperGroup.";
                sendInfoMessage(MessageChannel::USER_ERROR, prepared_chat_message, *sender_sess);
                break;
            }

            qWarning() << "SuperGroup chat: this only work for members on local server. We should introduce a message router, and send messages to EntityIDs instead of directly using sessions.";

            // Only send the message to characters in sender's supergroup
            for(MapClientSession *cl : m_session_store)
            {
                if(sender->m_supergroup.m_SG_id == cl->m_ent->m_supergroup.m_SG_id)
                    recipients.push_back(cl);
            }
            prepared_chat_message = QString(" %1: %2").arg(sender_char_name,msg_content.toString());
            for(MapClientSession * cl : recipients)
            {
                sendChatMessage(MessageChannel::SUPERGROUP,prepared_chat_message,sender,*cl);
            }
            break;
        }
        case MessageChannel::FRIENDS:
        {
            FriendsList * fl = &sender->m_char->m_char_data.m_friendlist;
            if(!fl->m_has_friends || fl->m_friends_count == 0)
            {
                prepared_chat_message = "You don't have any friends to message.";
                sendInfoMessage(MessageChannel::USER_ERROR,prepared_chat_message,*sender_sess);
                break;
            }

            qWarning() << "Friend chat: this only work for friends on local server. We should introduce a message router, and send messages to EntityIDs instead of directly using sessions.";

            // Only send the message to characters in sender's friendslist
            prepared_chat_message = QString(" %1: %2").arg(sender_char_name,msg_content.toString());
            for(Friend &f : fl->m_friends)
            {
                if(f.m_online_status != true)
                    continue;

                //TODO: this only work for friends on local server
                // introduce a message router, and send messages to EntityIDs instead of directly using sessions.
                Entity *tgt = getEntityByDBID(sender_sess->m_current_map, f.m_db_id);
                if(tgt == nullptr) // In case we didn't toggle online_status.
                    continue;

                sendChatMessage(MessageChannel::FRIENDS, prepared_chat_message, sender, *tgt->m_client);
            }
            sendChatMessage(MessageChannel::FRIENDS, prepared_chat_message, sender, *sender_sess);
            break;
        }
        default:
        {
            qCDebug(logChat) << "Unhandled MessageChannel type" << int(kind);
            break;
        }
    }
}

static bool has_emote_prefix(const QString &cmd) // ERICEDIT: This encompasses all emotes.
{
    return cmd.startsWith("em ",Qt::CaseInsensitive) || cmd.startsWith("e ",Qt::CaseInsensitive)
                || cmd.startsWith("me ",Qt::CaseInsensitive) || cmd.startsWith("emote ",Qt::CaseInsensitive);
}

void MapInstance::on_console_command(ConsoleCommand * ev)
{
    QString contents = ev->contents.simplified();
    MapClientSession &session(m_session_store.session_from_event(ev));
    Entity *ent = session.m_ent;

    if(contents.contains("$")) // does it contain replacement strings?
        contents = processReplacementStrings(&session, contents);

    //qCDebug(logChat, "Console command received %1").arg(qPrintable(ev->contents);

    ent->m_has_input_on_timeframe = true;

    if(isChatMessage(contents))
        process_chat(ent,contents);
    else if(has_emote_prefix(contents))
        on_emote_command(contents, ent);
    else
        runCommand(contents, session);
}

void MapInstance::on_emote_command(const QString &command, Entity *ent)
{
    QString msg;                                                                // Initialize the variable to hold the debug message.
    MapClientSession *src = ent->m_client;
    std::vector<MapClientSession *> recipients;

    QString cmd_str = command.section(QRegularExpression("\\s+"), 0, 0);
    QString original_emote = command.section(QRegularExpression("\\s+"), 1, -1);
    QString lowerContents = original_emote.toLower();
                                                                             // Normal Emotes
    static const QStringList afraidCommands = {"afraid", "cower", "fear", "scared"};
    static const QStringList akimboCommands = {"akimbo", "wings"};
    static const QStringList bigWaveCommands = {"bigwave", "overhere"};
    static const QStringList boomBoxCommands = {"boombox", "bb", "dropboombox"};
    static const QStringList bowCommands = {"bow", "sorry"};
    static const QStringList bowDownCommands = {"bowdown", "down"};
    static const QStringList coinCommands = {"coin", "cointoss", "flipcoin"};
    static const QStringList diceCommands = {"dice", "rolldice"};
    static const QStringList evilLaughCommands = {"evillaugh", "elaugh", "muahahaha", "villainlaugh", "villainouslaugh"};
    static const QStringList fancyBowCommands = {"fancybow", "elegantbow"};
    static const QStringList flex1Commands = {"flex1", "flexa"};
    static const QStringList flex2Commands = {"flex2", "flex", "flexb"};
    static const QStringList flex3Commands = {"flex3", "flexc"};
    static const QStringList hiCommands = {"hi", "wave"};
    static const QStringList hmmCommands = {"hmmm", "plotting"};
    static const QStringList laugh2Commands = {"laugh2", "biglaugh", "laughtoo"};
    static const QStringList martialArtsCommands = {"martialarts", "kata"};
    static const QStringList newspaperCommands = {"newspaper"};
    static const QStringList noCommands = {"no", "dontattack"};
    static const QStringList plotCommands = {"plot", "scheme"};
    static const QStringList stopCommands = {"stop", "raisehand"};
    static const QStringList tarzanCommands = {"tarzan", "beatchest"};
    static const QStringList taunt1Commands = {"taunt1", "taunta"};
    static const QStringList taunt2Commands = {"taunt2", "taunt", "tauntb"};
    static const QStringList thanksCommands = {"thanks", "thankyou"};
    static const QStringList waveFistCommands = {"wavefist", "rooting"};
    static const QStringList winnerCommands = {"winner", "champion"};
    static const QStringList yesCommands = {"yes", "thumbsup"};
    static const QStringList yogaCommands = {"yoga", "lotus"};
    static const QStringList snowflakesCommands = {"snowflakes", "throwsnowflakes"};

    if(afraidCommands.contains(lowerContents))                                  // Afraid: Cower in fear, hold stance.
    {
        if(ent->m_motion_state.m_is_flying)                                                    // Different versions when flying and on the ground.
            msg = "Unhandled flying Afraid emote";
        else
            msg = "Unhandled ground Afraid emote";
    }
    else if(akimboCommands.contains(lowerContents) && !ent->m_motion_state.m_is_flying)        // Akimbo: Stands with fists on hips looking forward, hold stance.
        msg = "Unhandled Akimbo emote";                                         // Not allowed when flying.
    else if(lowerContents == "angry")                                           // Angry: Fists on hips and slouches forward, as if glaring or grumbling, hold stance.
        msg = "Unhandled Angry emote";
    else if(lowerContents == "atease")                                          // AtEase: Stands in the 'at ease' military position (legs spread out slightly, hands behind back) stance, hold stance.
        msg = "Unhandled AtEase emote";
    else if(lowerContents == "attack")                                          // Attack: Gives a charge! type point, fists on hips stance.
        msg = "Unhandled Attack emote";
    else if(lowerContents == "batsmash")                                        // BatSmash: Hit someone or something with a bat, repeat.
        msg = "Unhandled BatSmash emote";
    else if(lowerContents == "batsmashreact")                                   // BatSmashReact: React as if getting hit with a bat, often used in duo with BatSmash.
        msg = "Unhandled BatSmashReact emote";
    else if(bigWaveCommands.contains(lowerContents))                            // BigWave: Waves over the head, fists on hips stance.
        msg = "Unhandled BigWave emote";
    else if(boomBoxCommands.contains(lowerContents) && !ent->m_motion_state.m_is_flying)       // BoomBox (has sound): Summons forth a boombox (it just appears) and leans over to turn it on, stands up and does a sort of dance. A random track will play.
    {                                                                           // Not allowed when flying.
        int rSong = rand() % 25 + 1;                                            // Randomly pick a song.
        switch(rSong)
        {
            case 1:                                                             // 1: BBAltitude
            {
                msg = "1: Unhandled \"BBAltitude\" BoomBox emote";
                break;
            }
            case 2:                                                             // 2: BBBeat
            {
                msg = "2: Unhandled \"BBBeat\" BoomBox emote";
                break;
            }
            case 3:                                                             // 3: BBCatchMe
            {
                msg = "3: Unhandled \"BBCatchMe\" BoomBox emote";
                break;
            }
            case 4:                                                             // 4: BBDance
            {
                msg = "4: Unhandled \"BBDance\" BoomBox emote";
                break;
            }
            case 5:                                                             // 5: BBDiscoFreak
            {
                msg = "5: Unhandled \"BBDiscoFreak\" BoomBox emote";
                break;
            }
            case 6:                                                             // 6: BBDogWalk
            {
                msg = "6: Unhandled \"BBDogWalk\" BoomBox emote";
                break;
            }
            case 7:                                                             // 7: BBElectroVibe
            {
                msg = "7: Unhandled \"BBElectroVibe\" BoomBox emote";
                break;
            }
            case 8:                                                             // 8: BBHeavyDude
            {
                msg = "8: Unhandled \"BBHeavyDude\" BoomBox emote";
                break;
            }
            case 9:                                                             // 9: BBInfoOverload
            {
                msg = "9: Unhandled \"BBInfoOverload\" BoomBox emote";
                break;
            }
            case 10:                                                            // 10: BBJumpy
            {
                msg = "10: Unhandled \"BBJumpy\" BoomBox emote";
                break;
            }
            case 11:                                                            // 11: BBKickIt
            {
                msg = "11: Unhandled \"BBKickIt\" BoomBox emote";
                break;
            }
            case 12:                                                            // 12: BBLooker
            {
                msg = "12: Unhandled \"BBLooker\" BoomBox emote";
                break;
            }
            case 13:                                                            // 13: BBMeaty
            {
                msg = "13: Unhandled \"BBMeaty\" BoomBox emote";
                break;
            }
            case 14:                                                            // 14: BBMoveOn
            {
                msg = "14: Unhandled \"BBMoveOn\" BoomBox emote";
                break;
            }
            case 15:                                                            // 15: BBNotorious
            {
                msg = "15: Unhandled \"BBNotorious\" BoomBox emote";
                break;
            }
            case 16:                                                            // 16: BBPeace
            {
                msg = "16: Unhandled \"BBPeace\" BoomBox emote";
                break;
            }
            case 17:                                                            // 17: BBQuickie
            {
                msg = "17: Unhandled \"BBQuickie\" BoomBox emote";
                break;
            }
            case 18:                                                            // 18: BBRaver
            {
                msg = "18: Unhandled \"BBRaver\" BoomBox emote";
                break;
            }
            case 19:                                                            // 19: BBShuffle
            {
                msg = "19: Unhandled \"BBShuffle\" BoomBox emote";
                break;
            }
            case 20:                                                            // 20: BBSpaz
            {
                msg = "20: Unhandled \"BBSpaz\" BoomBox emote";
                break;
            }
            case 21:                                                            // 21: BBTechnoid
            {
                msg = "21: Unhandled \"BBTechnoid\" BoomBox emote";
                break;
            }
            case 22:                                                            // 22: BBVenus
            {
                msg = "22: Unhandled \"BBVenus\" BoomBox emote";
                break;
            }
            case 23:                                                            // 23: BBWindItUp
            {
                msg = "23: Unhandled \"BBWindItUp\" BoomBox emote";
                break;
            }
            case 24:                                                            // 24: BBWahWah
            {
                msg = "24: Unhandled \"BBWahWah\" BoomBox emote";
                break;
            }
            case 25:                                                            // 25: BBYellow
            {
                msg = "25: Unhandled \"BBYellow\" BoomBox emote";
            }
        }
    }
    else if(bowCommands.contains(lowerContents) && !ent->m_motion_state.m_is_flying)           // Bow: Chinese/Japanese style bow with palms together, returns to normal stance.
        msg = "Unhandled Bow emote";                                            // Not allowed when flying.
    else if(bowDownCommands.contains(lowerContents))                            // BowDown: Thrusts hands forward, then points down, as if ordering someone else to bow before you.
        msg = "Unhandled BowDown emote";
    else if(lowerContents == "burp" && !ent->m_motion_state.m_is_flying)                       // Burp (has sound): A raunchy belch, wipes mouth with arm afterward, ape-like stance.
        msg = "Unhandled Burp emote";                                           // Not allowed when flying.
    else if(lowerContents == "cheer")                                           // Cheer: Randomly does one of 3 cheers, 1 fist raised, 2 fists raised or 2 fists lowered, repeats.
    {
        int rNum = rand() % 3 + 1;                                              // Randomly pick the cheer.
        switch(rNum)
        {
            case 1:                                                             // 1: 1 fist raised
            {
                msg = "1: Unhandled \"1 fist raised\" Cheer emote";
                break;
            }
            case 2:                                                             // 2: 2 fists raised
            {
                msg = "2: Unhandled \"2 fists raised\" Cheer emote";
                break;
            }
            case 3:                                                             // 3: 2 fists lowered
            {
                msg = "3: Unhandled \"2 fists lowered\" Cheer emote";
            }
        }
    }
    else if(lowerContents == "clap")                                            // Clap (has sound): Claps hands several times, crossed arms stance.
        msg = "Unhandled Clap emote";
    else if(coinCommands.contains(lowerContents))                               // Coin: Flips a coin, randomly displays heads or tails, and hold stance. Coin image remains until stance broken.
    {
        int rFlip = rand() % 2 + 1;                                             // Randomly pick heads or tails.
        switch(rFlip)
        {
            case 1:                                                             // 1: Heads
            {
                msg = "1: Unhandled heads Coin emote";
                break;
            }
            case 2:                                                             // 2: Tails
            {
                msg = "2: Unhandled tails Coin emote";
            }
        }
    }
    else if(lowerContents == "crossarms" && !ent->m_motion_state.m_is_flying)                  // CrossArms: Crosses arms, stance (slightly different from most other crossed arm stances).
        msg = "Unhandled CrossArms emote";                                      // Not allowed when flying.
    else if(lowerContents == "dance")                                           // Dance: Randomly performs one of six dances.
    {
        int rDance = rand() % 6 + 1;                                            // Randomly pick the dance.
        switch(rDance)
        {
            case 1:                                                             // 1: Dances with elbows by hips.
            {
                msg = "1: Unhandled \"Dances with elbows by hips\" Dance emote";
                break;
            }
            case 2:                                                             // 2: Dances with fists raised.
            {
                msg = "2: Unhandled \"Dances with fists raised\" Dance emote";
                break;
            }
            case 3:                                                             // 3: Swaying hands by hips, aka "Really Bad" dancing.
            {
                msg = "3: Unhandled \"Swaying hands by hips, aka 'Really Bad' dancing\" Dance emote";
                break;
            }
            case 4:                                                             // 4: Swaying hands up in the air, like in a breeze.
            {
                msg = "4: Unhandled \"Swaying hands up in the air, like in a breeze\" Dance emote";
                break;
            }
            case 5:                                                             // 5: As Dance4, but jumping as well.
            {
                msg = "5: Unhandled \"As Dance4, but jumping as well\" Dance emote";
                break;
            }
            case 6:                                                             // 6: The monkey.
            {
                msg = "6: Unhandled \"The monkey\" Dance emote";
            }
        }
    }
    else if(diceCommands.contains(lowerContents))                               // Dice: Picks up, shakes and rolls a die, randomly displays the results (1-6), default stance. Die image quickly fades.
    {
        int rDice = rand() % 6 + 1;                                             // Randomly pick a die result.
        switch(rDice)
        {
            case 1:                                                             // 1: 1
            {
                msg = "1: Unhandled \"1\" Dice emote";
                break;
            }
            case 2:                                                             // 2: 2
            {
                msg = "2: Unhandled \"2\" Dice emote";
                break;
            }
            case 3:                                                             // 3: 3
            {
                msg = "3: Unhandled \"3\" Dice emote";
                break;
            }
            case 4:                                                             // 4: 4
            {
                msg = "4: Unhandled \"4\" Dice emote";
                break;
            }
            case 5:                                                             // 5: 5
            {
                msg = "5: Unhandled \"5\" Dice emote";
                break;
            }
            case 6:                                                             // 6: 6
            {
                msg = "6: Unhandled \"6\" Dice emote";
            }
        }
    }
    else if(lowerContents == "dice1")                                           // Dice1: Picks up, shakes and rolls a die, displays a 1, default stance.
        msg = "Unhandled Dice1 emote";
    else if(lowerContents == "dice2")                                           // Dice2: Picks up, shakes and rolls a die, displays a 2, default stance.
        msg = "Unhandled Dice2 emote";
    else if(lowerContents == "dice3")                                           // Dice3: Picks up, shakes and rolls a die, displays a 3, default stance.
        msg = "Unhandled Dice3 emote";
    else if(lowerContents == "dice4")                                           // Dice4: Picks up, shakes and rolls a die, displays a 4, default stance.
        msg = "Unhandled Dice4 emote";
    else if(lowerContents == "dice5")                                           // Dice5: Picks up, shakes and rolls a die, displays a 5, default stance.
        msg = "Unhandled Dice5 emote";
    else if(lowerContents == "dice6")                                           // Dice6: Picks up, shakes and rolls a die, displays a 6, default stance.
        msg = "Unhandled Dice6 emote";
    else if(lowerContents == "disagree")                                        // Disagree: Shakes head, crosses hand in front, then offers an alternative, crossed arms stance.
        msg = "Unhandled Disagree emote";
    else if(lowerContents == "drat")                                            // Drat: Raises fists up, then down, stomping at the same time, same ending stance as Frustrated.
        msg = "Unhandled Drat emote";
    else if(lowerContents == "explain")                                         // Explain: Hold arms out in a "wait a minute" gesture, motion alternatives, then shrug.
        msg = "Unhandled Explain emote";
    else if(evilLaughCommands.contains(lowerContents))                          // EvilLaugh: Extremely melodramatic, overacted evil laugh.
        msg = "Unhandled EvilLaugh emote";
    else if(fancyBowCommands.contains(lowerContents))                           // FancyBow: A much more elegant, ball-room style bow, falls into neutral forward facing stance.
        msg = "Unhandled FancyBow emote";
    else if(flex1Commands.contains(lowerContents))                              // Flex1: Fists raised, flexing arms stance, hold stance. This is called a "double biceps" pose.
        msg = "Unhandled Flex1 emote";
    else if(flex2Commands.contains(lowerContents))                              // Flex2: A side-stance flexing arms, hold stance. This is a sideways variation on the "most muscular" pose.
        msg = "Unhandled Flex2 emote";
    else if(flex3Commands.contains(lowerContents))                              // Flex3: Another side-stance, flexing arms, hold stance. This is an open variation on the "side chest" pose.
        msg = "Unhandled Flex3 emote";
    else if(lowerContents == "frustrated")                                      // Frustrated: Raises both fists and leans backwards, shaking fists and head, leads into a quick-breathing angry-looking stance.
        msg = "Unhandled Frustrated emote";
    else if(lowerContents == "grief")                                           // Grief: Falls to knees, hands on forehead, looks up and gestures a sort of "why me?" look with hands, goes into a sort of depressed slump while on knees, holds stance.
        msg = "Unhandled Grief emote";
    else if(hiCommands.contains(lowerContents))                                 // Hi: Simple greeting wave, fists on hips stance.
        msg = "Unhandled Hi emote";
    else if(hmmCommands.contains(lowerContents))                                // Hmmm: Stare into the sky, rubbing chin, thinking.
        msg = "Unhandled Hmmm emote";
    else if(lowerContents == "jumpingjacks")                                    // JumpingJacks (has sound): Does jumping jacks, repeats.
        msg = "Unhandled JumpingJacks emote";
    else if(lowerContents == "kneel")                                           // Kneel: Quickly kneels on both knees with hands on thighs (looks insanely uncomfortable), holds stance.
        msg = "Unhandled Kneel emote";
    else if(lowerContents == "laugh")                                           // Laugh: Fists on hips, tosses head back and laughs.
        msg = "Unhandled Laugh emote";
    else if(laugh2Commands.contains(lowerContents))                             // Laugh2: Another style of laugh.
        msg = "Unhandled Laugh2 emote";
    else if(lowerContents == "lecture")                                         // Lecture: Waves/shakes hands in different motions in a lengthy lecture, fists on hips stance.
        msg = "Unhandled Lecture emote";
    else if(martialArtsCommands.contains(lowerContents))                        // MartialArts (has sound): Warm up/practice punches and blocks.
        msg = "Unhandled MartialArts emote";
    else if(lowerContents == "militarysalute")                                  // MilitarySalute: Stands in the military-style heads-high hand on forehead salute stance.
        msg = "Unhandled MilitarySalute emote";
    else if(newspaperCommands.contains(lowerContents))                          // Newspaper: Materializes a newspaper and reads it.
        msg = "Unhandled Newspaper emote";
    else if(noCommands.contains(lowerContents))                                 // No: Shakes head and waves hands in front of character, crossed arms stance.
        msg = "Unhandled No emote";
    else if(lowerContents == "nod")                                             // Nod: Fists on hips, nod yes, hold stance.
        msg = "Unhandled Nod emote";
    else if(lowerContents == "none")                                            // None: Cancels the current emote, if any, and resumes default standing animation cycle.
        msg = "Unhandled None emote";
    else if(lowerContents == "paper")                                           // Paper: Plays rock/paper/scissors, picking paper (displays all three symbols for about 6 seconds, then displays and holds your choice until stance is broken).
        msg = "Unhandled Paper emote";
    else if(plotCommands.contains(lowerContents))                               // Plot: Rubs hands together while hunched over.
        msg = "Unhandled Plot emote";
    else if(lowerContents == "point")                                           // Point: Extends left arm and points in direction character is facing, hold stance.
        msg = "Unhandled Point emote";
    else if(lowerContents == "praise")                                          // Praise: Kneel prostrate and repeatedly bow in adoration.
        msg = "Unhandled Praise emote";
    else if(lowerContents == "protest")                                         // Protest: Hold hold up one of several randomly selected mostly unreadable protest signs.
        msg = "Unhandled Protest emote";
    else if(lowerContents == "roar" && !ent->m_motion_state.m_is_flying)                       // Roar: Claws air, roaring, ape-like stance.
        msg = "Unhandled Roar emote";                                           // Not allowed when flying.
    else if(lowerContents == "rock")                                            // Rock: Plays rock/paper/scissors, picking rock (displays all three symbols for about 6 seconds, then displays and holds your choice until stance is broken).
        msg = "Unhandled Rock emote";
    else if(lowerContents == "salute")                                          // Salute: A hand-on-forehead salute, fists on hips stance.
        msg = "Unhandled Salute emote";
    else if(lowerContents == "scissors")                                        // Scissors: Plays rock/paper/scissors, picking scissors (displays all three symbols for about 6 seconds, then displays and holds your choice until stance is broken).
        msg = "Unhandled Scissors emote";
    else if(lowerContents == "score1")                                          // Score1: Holds a black on white scorecard up, displaying a 1, holds stance.
        msg = "Unhandled Score1 emote";
    else if(lowerContents == "score2")                                          // Score2: Holds a black on white scorecard up, displaying a 2, holds stance.
        msg = "Unhandled Score2 emote";
    else if(lowerContents == "score3")                                          // Score3: Holds a black on white scorecard up, displaying a 3, holds stance.
        msg = "Unhandled Score3 emote";
    else if(lowerContents == "score4")                                          // Score4: Holds a black on white scorecard up, displaying a 4, holds stance.
        msg = "Unhandled Score4 emote";
    else if(lowerContents == "score5")                                          // Score5: Holds a black on white scorecard up, displaying a 5, holds stance.
        msg = "Unhandled Score5 emote";
    else if(lowerContents == "score6")                                          // Score6: Holds a black on white scorecard up, displaying a 6, holds stance.
        msg = "Unhandled Score6 emote";
    else if(lowerContents == "score7")                                          // Score7: Holds a black on white scorecard up, displaying a 7, holds stance.
        msg = "Unhandled Score7 emote";
    else if(lowerContents == "score8")                                          // Score8: Holds a black on white scorecard up, displaying a 8, holds stance.
        msg = "Unhandled Score8 emote";
    else if(lowerContents == "score9")                                          // Score9: Holds a black on white scorecard up, displaying a 9, holds stance.
        msg = "Unhandled Score9 emote";
    else if(lowerContents == "score10")                                         // Score10: Holds a black on white scorecard up, displaying a 10, holds stance.
        msg = "Unhandled Score10 emote";
    else if(lowerContents == "shucks")                                          // Shucks: Swings fist and head dejectedly, neutral forward facing stance (not the default stance, same as huh/shrug).
        msg = "Unhandled Shucks emote";
    else if(lowerContents == "sit")                                             // Sit: Sits down, legs forward, with knees bent, elbows on knees, and slightly slumped over, stance.
        msg = "Unhandled Sit emote";
    else if(lowerContents == "smack")                                           // Smack: Backhand slap.
        msg = "Unhandled Smack emote";
    else if(stopCommands.contains(lowerContents))                               // Stop: Raises your right hand above your head, hold stance.
        msg = "Unhandled Stop emote";
    else if(tarzanCommands.contains(lowerContents))                             // Tarzan: Beats chest and howls, angry-looking stance.
    {
        if(ent->m_motion_state.m_is_flying)                                                    // Different versions when flying and on the ground.
            msg = "Unhandled flying Tarzan emote";
        else
            msg = "Unhandled ground Tarzan emote";
    }
    else if(taunt1Commands.contains(lowerContents))                             // Taunt1: Taunts, beckoning with one hand, then slaps fist into palm, repeating stance.

    {
        if(ent->m_motion_state.m_is_flying)                                                    // Different versions when flying and on the ground.
            msg = "Unhandled flying Taunt1 emote";
        else
            msg = "Unhandled ground Taunt1 emote";
    }
    else if(taunt2Commands.contains(lowerContents))                             // Taunt2: Taunts, beckoning with both hands, combat stance.
    {
        if(ent->m_motion_state.m_is_flying)                                                    // Different versions when flying and on the ground.
            msg = "Unhandled flying Taunt2 emote";
        else
            msg = "Unhandled ground Taunt2 emote";
    }
    else if(thanksCommands.contains(lowerContents))                             // Thanks: Gestures with hand, neutral forward facing stance.
        msg = "Unhandled Thanks emote";
    else if(lowerContents == "thewave")                                         // Thewave: Does the wave (as seen in stadiums at sporting events), neutral facing forward stance.
        msg = "Unhandled Thewave emote";
    else if(lowerContents == "victory")                                         // Victory: Raises hands excitedly, and then again less excitedly, and then a third time almost non-chalantly, falls into neutral forward facing stance.
        msg = "Unhandled Victory emote";
    else if(waveFistCommands.contains(lowerContents))                           // WaveFist (has sound): Waves fist, hoots and then claps (its a cheer), crossed arms stance.
        msg = "Unhandled WaveFist emote";
    else if(lowerContents == "welcome")                                         // Welcome: Open arms welcoming, fists on hips stance.
        msg = "Unhandled Welcome emote";
    else if(lowerContents == "whistle")                                         // Whistle (has sound): Whistles (sounds like a police whistle), ready-stance.
        msg = "Unhandled Whistle emote";
    else if(winnerCommands.contains(lowerContents))                             // Winner: Fist in fist cheer, right, and then left, neutral forward facing stance.
        msg = "Unhandled Winner emote";
    else if(lowerContents == "yourewelcome")                                    // YoureWelcome: Bows head and gestures with hand, neutral forward facing stance.
        msg = "Unhandled YoureWelcome emote";
    else if(yesCommands.contains(lowerContents))                                // Yes: Big (literally) thumbs up and an affirmative nod, fists on hips stance.
        msg = "Unhandled Yes emote";
    else if(yogaCommands.contains(lowerContents))                               // Yoga: Sits down cross legged with hands on knees/legs, holds stance.
    {
        if(ent->m_motion_state.m_is_flying)                                                    // Different versions when flying and on the ground.
            msg = "Unhandled flying Yoga emote";
        else
            msg = "Unhandled ground Yoga emote";
    }
                                                                                // Boombox Emotes
    else if(lowerContents.startsWith("bb") && !ent->m_motion_state.m_is_flying)                // Check if Boombox Emote.
    {                                                                           // Not allowed when flying.
        lowerContents.replace(0, 2, "");                                        // Remove the "BB" prefix for conciseness.
        if(lowerContents == "altitude")                                         // BBAltitude
            msg = "Unhandled BBAltitude emote";
        else if(lowerContents == "beat")                                        // BBBeat
            msg = "Unhandled BBBeat emote";
        else if(lowerContents == "catchme")                                     // BBCatchMe
            msg = "Unhandled BBCatchMe emote";
        else if(lowerContents == "dance")                                       // BBDance
            msg = "Unhandled BBDance emote";
        else if(lowerContents == "discofreak")                                  // BBDiscoFreak
            msg = "Unhandled BBDiscoFreak emote";
        else if(lowerContents == "dogwalk")                                     // BBDogWalk
            msg = "Unhandled BBDogWalk emote";
        else if(lowerContents == "electrovibe")                                 // BBElectroVibe
            msg = "Unhandled BBElectroVibe emote";
        else if(lowerContents == "heavydude")                                   // BBHeavyDude
            msg = "Unhandled BBHeavyDude emote";
        else if(lowerContents == "infooverload")                                // BBInfoOverload
            msg = "Unhandled BBInfoOverload emote";
        else if(lowerContents == "jumpy")                                       // BBJumpy
            msg = "Unhandled BBJumpy emote";
        else if(lowerContents == "kickit")                                      // BBKickIt
            msg = "Unhandled BBKickIt emote";
        else if(lowerContents == "looker")                                      // BBLooker
            msg = "Unhandled BBLooker emote";
        else if(lowerContents == "meaty")                                       // BBMeaty
            msg = "Unhandled BBMeaty emote";
        else if(lowerContents == "moveon")                                      // BBMoveOn
            msg = "Unhandled BBMoveOn emote";
        else if(lowerContents == "notorious")                                   // BBNotorious
            msg = "Unhandled BBNotorious emote";
        else if(lowerContents == "peace")                                       // BBPeace
            msg = "Unhandled BBPeace emote";
        else if(lowerContents == "quickie")                                     // BBQuickie
            msg = "Unhandled BBQuickie emote";
        else if(lowerContents == "raver")                                       // BBRaver
            msg = "Unhandled BBRaver emote";
        else if(lowerContents == "shuffle")                                     // BBShuffle
            msg = "Unhandled BBShuffle emote";
        else if(lowerContents == "spaz")                                        // BBSpaz
            msg = "Unhandled BBSpaz emote";
        else if(lowerContents == "technoid")                                    // BBTechnoid
            msg = "Unhandled BBTechnoid emote";
        else if(lowerContents == "venus")                                       // BBVenus
            msg = "Unhandled BBVenus emote";
        else if(lowerContents == "winditup")                                    // BBWindItUp
            msg = "Unhandled BBWindItUp emote";
        else if(lowerContents == "wahwah")                                      // BBWahWah
            msg = "Unhandled BBWahWah emote";
        else if(lowerContents == "yellow")                                      // BBYellow
            msg = "Unhandled BBYellow emote";
    }
                                                                                // Unlockable Emotes
                                                                                // TODO: Implement logic and variables for unlocking these emotes.
    else if(lowerContents == "dice7")                                           // Dice7: Picks up, shakes and rolls a die, displays a 7, default stance.
        msg = "Unhandled Dice7 emote";                                          // Unlocked by earning the Burkholder's Bane Badge (from the Ernesto Hess Task Force).
    else if(lowerContents == "listenpoliceband")                                // ListenPoliceBand: Listens in on the heroes' PPD police band radio.
        msg = "Unhandled ListenPoliceBand emote";                               // Heroes can use this without any unlock requirement. For villains, ListenStolenPoliceBand unlocks by earning the Outlaw Badge.
    else if(snowflakesCommands.contains(lowerContents))                         // Snowflakes: Throws snowflakes.
    {
        if(ent->m_motion_state.m_is_flying)                                                    // Different versions when flying and on the ground.
            msg = "Unhandled flying Snowflakes emote";                          // Unlocked by purchasing from the Candy Keeper during the Winter Event.
        else
            msg = "Unhandled ground Snowflakes emote";
    }
    else                                                                        // If not specific command, output EMOTE message.
    {
        // "CharacterName {emote message}"
        msg = QString("%1 %2").arg(ent->name(),original_emote);
    }

    // send only to clients within range
    glm::vec3 senderpos = src->m_ent->m_entity_data.m_pos;
    for(MapClientSession *cl : m_session_store)
    {
        glm::vec3 recpos = cl->m_ent->m_entity_data.m_pos;
        float range = 50.0f; // range of "hearing". I assume this is in yards
        float dist = glm::distance(senderpos,recpos);

        qCDebug(logEmotes, "senderpos: %f %f %f", senderpos.x, senderpos.y, senderpos.z);
        qCDebug(logEmotes, "recpos: %f %f %f", recpos.x, recpos.y, recpos.z);
        qCDebug(logEmotes, "sphere: %f", range);
        qCDebug(logEmotes, "dist: %f", dist);

        if(dist<=range)
            recipients.push_back(cl);
    }
    for(MapClientSession * cl : recipients)
    {
        sendChatMessage(MessageChannel::EMOTE,msg,src->m_ent,*cl);
        qCDebug(logEmotes) << msg;
    }
}

void MapInstance::on_command_chat_divider_moved(ChatDividerMoved *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    Entity *ent = session.m_ent;

    ent->m_player->m_gui.m_chat_divider_pos = ev->m_position;
    qCDebug(logMapEvents) << "Chat divider moved to " << ev->m_position << " for player" << ent->name();
}

void MapInstance::on_minimap_state(MiniMapState *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    Entity *ent = session.m_ent;
    uint32_t map_idx = session.m_current_map->m_index;

    std::vector<bool> * map_cells = &ent->m_player->m_player_progress.m_visible_map_cells[map_idx];

    if (map_cells->empty())
        map_cells->resize(1024);

    if (ev->tile_idx > map_cells->size())
        map_cells->resize(map_cells->size() + (ev->tile_idx - map_cells->size() + 1023) / 1024 * 1024);

    // #818 map_cells of type array with a size of 1024 threw
    // out of range exception on maps that had index tiles larger than 1024
    map_cells->at(ev->tile_idx) = true;

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
        session.m_ent->m_entity_update_flags.setFlag(session.m_ent->UpdateFlag::FULL);

        char buf[256];
        std::string welcome_msg = std::string("Welcome to ") + VersionInfo::getAuthVersion()+"\n";
        std::snprintf(buf, 256, "There are %zu active entities and %zu clients", m_entities.active_entities(),
                    m_session_store.num_sessions());
        welcome_msg += buf;
        sendInfoMessage(MessageChannel::SERVER,QString::fromStdString(welcome_msg),session);

        // Show MOTD only if it's been more than X amount of time since last online
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

    std::vector<bool> * visible_map_cells = &session.m_ent->m_player->m_player_progress.m_visible_map_cells[session.m_current_map->m_index];

    if (!visible_map_cells->empty())
    {
        // TODO: Check map type to determine if is_opaque is true / false
        sendVisitMapCells(session, false, *visible_map_cells);
    }

    initializeCharacter(*session.m_ent->m_char);

    // Call Lua Connected function.
    auto val = m_scripting_interface->callFuncWithClientContext(&session,"player_connected", session.m_ent->m_idx);
}

void MapInstance::on_location_visited(LocationVisited *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    qCDebug(logMapEvents) << "Attempting a call to script location_visited with:"<<ev->m_name<<qHash(ev->m_name);

    auto val = m_scripting_interface->callFuncWithClientContext(&session,"location_visited", qPrintable(ev->m_name), ev->m_pos);
    sendInfoMessage(MessageChannel::DEBUG_INFO,qPrintable(ev->m_name),session);

    qCWarning(logMapEvents) << "Unhandled location visited event:" << ev->m_name <<
                  QString("(%1,%2,%3)").arg(ev->m_pos.x).arg(ev->m_pos.y).arg(ev->m_pos.z);
}

void MapInstance::on_plaque_visited(PlaqueVisited * ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    qCDebug(logMapEvents) << "Attempting a call to script plaque_visited with:"<<ev->m_name<<qHash(ev->m_name);

    auto val = m_scripting_interface->callFuncWithClientContext(&session,"plaque_visited", qPrintable(ev->m_name), ev->m_pos);
    qCWarning(logMapEvents) << "Unhandled plaque visited event:" << ev->m_name <<
                  QString("(%1,%2,%3)").arg(ev->m_pos.x).arg(ev->m_pos.y).arg(ev->m_pos.z);
}

void MapInstance::on_inspiration_dockmode(InspirationDockMode *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    Entity *ent = session.m_ent;

    ent->m_player->m_gui.m_insps_tray_mode = ev->dock_mode;
    qCDebug(logMapEvents) << "Saving inspirations dock mode to GUISettings:" << ev->dock_mode;
}

void MapInstance::on_enter_door(EnterDoor *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    MapServer *map_server = (MapServer *)HandlerLocator::getMap_Handler(m_game_server_id);

    QString output_msg = "Door entry request to: " + ev->name;
    if(ev->no_location)
    {
        qCDebug(logMapXfers).noquote() << output_msg << " No location provided";

        // Doors with no location may be a /mapmenu call.
        if(session.m_ent->m_is_using_mapmenu)
        {
            // ev->name is the map_idx when using /mapmenu
            if(!map_server->session_has_xfer_in_progress(session.link()->session_token()))
            {
                uint32_t map_idx = ev->name.toInt();
                if (map_idx == m_index)
                {
                    QString door_msg = "You're already here!";
                    sendDoorMessage(session, 2, door_msg);
                }
                else
                {
                    MapXferData map_data = MapXferData();
                    map_data.m_target_map_name = getMapName(map_idx);
                    map_server->putq(new ClientMapXferMessage({session.link()->session_token(), map_data},0));
                    QString map_path = getMapPath(map_idx);
                    qInfo() << "On enter door map_path: " << map_path;
                    session.link()->putq(new MapXferWait(map_path));
                }
            }
            session.m_ent->m_is_using_mapmenu = false;
        }
        else
        {
            QString door_msg = "Door coordinates unavailable.";
            sendDoorMessage(session, 2, door_msg);
        }
    }
    else
    {
        qCDebug(logMapXfers).noquote() << output_msg << " loc:" << ev->location.x << ev->location.y << ev->location.z;

        // Check if any doors in range have the GotoSpawn property.
        // TODO: if the node also has a GotoMap property, start a map transfer
        //       and put them in the given SpawnLocation in the target map.
        QString gotoSpawn = getNearestDoor(ev->location);

        if (gotoSpawn.isEmpty())
        {
            QString door_msg = "You cannot enter.";
            sendDoorMessage(session, 2, door_msg);
        }
        else
        {
            // Attempt to send the player to that SpawnLocation in the current map.
            QString anim_name = "RUNIN";
            glm::vec3 offset = ev->location + glm::vec3 {0,0,2};
            sendDoorAnimStart(session, ev->location, offset, true, anim_name);
            session.m_current_map->setSpawnLocation(*session.m_ent, gotoSpawn);
        }
    }
}

void MapInstance::on_change_stance(ChangeStance * ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));

    session.m_ent->m_stance = ev->m_stance;
    if(ev->m_stance.has_stance)
        qCDebug(logMapEvents) << "Change stance request" << session.m_ent->m_idx << ev->m_stance.pset_idx << ev->m_stance.pow_idx;
    else
        qCDebug(logMapEvents) << "Exit stance request" << session.m_ent->m_idx;
}

void MapInstance::on_set_destination(SetDestination * ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));

    qCWarning(logMapEvents) << QString("SetDestination request: %1 <%2, %3, %4>")
                                .arg(ev->point_index)
                                .arg(ev->destination.x, 0, 'f', 1)
                                .arg(ev->destination.y, 0, 'f', 1)
                                .arg(ev->destination.z, 0, 'f', 1);

    // store destination, confirm accuracy and send back to client as waypoint.
    setCurrentDestination(*session.m_ent, ev->point_index, ev->destination);
    sendWaypoint(session, ev->point_index, ev->destination);
}

void MapInstance::on_has_entered_door(HasEnteredDoor *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    //MapServer *map_server = (MapServer *)HandlerLocator::getMap_Handler(m_game_server_id);

    sendDoorAnimExit(session, false);

    QString output_msg = "Enter door animation has finished.";
    qCDebug(logAnimations).noquote() << output_msg;
}

void MapInstance::on_abort_queued_power(AbortQueuedPower * ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));

    if(session.m_ent->m_queued_powers.empty())
        return;

    // remove last queued power
    session.m_ent->m_queued_powers.back().m_activation_state = false;
    session.m_ent->m_queued_powers.back().m_active_state_change = true;
    session.m_ent->m_char->m_char_data.m_has_updated_powers = true; // this must be true, because we're updating queued powers

    qCWarning(logMapEvents) << "Aborting queued power";
}

void MapInstance::on_description_and_battlecry(DescriptionAndBattleCry * ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    Entity *ent = session.m_ent;
    Character &c(*ent->m_char);

    setBattleCry(c,ev->battlecry);
    setDescription(c,ev->description);
    qCDebug(logDescription) << "Saving description and battlecry:" << ev->description << ev->battlecry;
}

void MapInstance::on_entity_info_request(EntityInfoRequest * ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));

    Entity *tgt = getEntity(&session,ev->entity_idx);
    if(tgt == nullptr)
    {
        qCDebug(logMapEvents) << "No target active, doing nothing";
        return;
    }

    QString description = getDescription(*tgt->m_char);

    session.addCommandToSendNextUpdate(std::make_unique<EntityInfoResponse>(description));
    qCDebug(logDescription) << "Entity info requested" << ev->entity_idx << description;
}

void MapInstance::on_chat_reconfigured(ChatReconfigure *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    Entity *ent = session.m_ent;

    ent->m_player->m_gui.m_chat_top_flags = ev->m_chat_top_flags;
    ent->m_player->m_gui.m_chat_bottom_flags = ev->m_chat_bottom_flags;

    qCDebug(logMapEvents) << "Saving chat channel mask settings to GUISettings" << ev->m_chat_top_flags << ev->m_chat_bottom_flags;
}

void MapInstance::on_set_default_power(SetDefaultPower *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    PowerTrayGroup *ptray = &session.m_ent->m_char->m_char_data.m_trays;

    ptray->m_has_default_power = true;
    ptray->m_default_pset_idx = ev->powerset_idx;
    ptray->m_default_pow_idx = ev->power_idx;

    qCDebug(logMapEvents) << "Set Default Power:" << ev->powerset_idx << ev->power_idx;
}

void MapInstance::on_unset_default_power(UnsetDefaultPower *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    PowerTrayGroup *ptray = &session.m_ent->m_char->m_char_data.m_trays;

    ptray->m_has_default_power = false;
    ptray->m_default_pset_idx = 0;
    ptray->m_default_pow_idx = 0;

    qCDebug(logMapEvents) << "Unset Default Power.";
}

void MapInstance::on_unqueue_all(UnqueueAll *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    Entity *ent = session.m_ent;

    // What else could go here?
    ent->m_target_idx = 0;
    ent->m_assist_target_idx = 0;

    qCWarning(logMapEvents) << "Incomplete Unqueue all request. Setting Target and Assist Target to 0";
}

void MapInstance::on_target_chat_channel_selected(TargetChatChannelSelected *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    Entity *ent = session.m_ent;

    ent->m_player->m_gui.m_cur_chat_channel = ev->m_chat_type;
    qCDebug(logMapEvents) << "Saving chat channel type to GUISettings:" << ev->m_chat_type;
}

void MapInstance::on_activate_power(ActivatePower *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    session.m_ent->m_has_input_on_timeframe = true;
    int tgt_idx = ev->target_idx;

    checkPower(*session.m_ent, ev->pset_idx, ev->pow_idx, tgt_idx);
}

void MapInstance::on_activate_power_at_location(ActivatePowerAtLocation *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    session.m_ent->m_has_input_on_timeframe = true;

    CharacterPower * ppower = nullptr;
    ppower = getOwnedPowerByVecIdx(*session.m_ent, ev->pset_idx, ev->pow_idx);
    const Power_Data powtpl = ppower->getPowerTemplate();
    int tgt_idx = ev->target_idx;

    if ((powtpl.EntsAffected[0] == StoredEntEnum::Caster ))
        tgt_idx = session.m_ent->m_idx;
    session.m_ent->m_target_loc = ev->location;
    checkPower(*session.m_ent, ev->pset_idx, ev->pow_idx, tgt_idx);
    sendFaceLocation(session, ev->location);
}

void MapInstance::on_activate_inspiration(ActivateInspiration *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));

    session.m_ent->m_has_input_on_timeframe = true;
    bool success = useInspiration(*session.m_ent, ev->slot_idx, ev->row_idx);

    if(!success)
        return;

    QString contents = "Inspired!";
    sendFloatingInfo(session, contents, FloatingInfoStyle::FloatingInfo_Attention, 4.0);
    // qCWarning(logPowers) << "Unhandled use inspiration request." << ev->row_idx << ev->slot_idx;
}

void MapInstance::on_powers_dockmode(PowersDockMode *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    Entity *ent = session.m_ent;

    ent->m_player->m_gui.m_powers_tray_mode = ev->toggle_secondary_tray;
    //qCDebug(logMapEvents) << "Saving powers tray dock mode to GUISettings:" << ev->toggle_secondary_tray;
}

void MapInstance::on_switch_tray(SwitchTray *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    Entity *ent = session.m_ent;

    ent->m_player->m_gui.m_tray1_number = ev->tray_group.m_primary_tray_idx;
    ent->m_player->m_gui.m_tray2_number = ev->tray_group.m_second_tray_idx;
    ent->m_char->m_char_data.m_trays = ev->tray_group;
    markEntityForDbStore(ent, DbStoreFlags::PlayerData);

   //qCDebug(logMapEvents) << "Saving Tray States to GUISettings. Tray1:" << ev->tray_group.m_primary_tray_idx+1 << "Tray2:" << ev->tray_group.m_second_tray_idx+1;
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
    e.m_entity_update_flags.setFlag(e.UpdateFlag::FULL);
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

void MapInstance::on_interact_with(InteractWithEntity *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    Entity *entity = getEntity(&session, ev->m_srv_idx);

    session.m_ent->m_has_input_on_timeframe = true;

    qCDebug(logMapEvents) << "Entity: " << session.m_ent->m_idx << "wants to interact with" << ev->m_srv_idx;
    auto val = m_scripting_interface->callFuncWithClientContext(&session,"entity_interact",ev->m_srv_idx, entity->m_entity_data.m_pos);
}

void MapInstance::on_receive_contact_status(ReceiveContactStatus *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));

    qCDebug(logMapEvents) << "ReceiveContactStatus Entity: " << session.m_ent->m_idx << "wants to interact with" << ev->m_srv_idx;
    auto val = m_scripting_interface->callFuncWithClientContext(&session,"contact_call",ev->m_srv_idx);
}

void MapInstance::on_receive_task_detail_request(ReceiveTaskDetailRequest *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));

    qCDebug(logMapEvents) << "ReceiveTaskDetailRequest Entity: " << session.m_ent->m_idx << "wants detail for task " << ev->m_task_idx;
    QString detail = "Testind Task Detail Request";

    TaskDetail test_task;
    test_task.m_task_idx = ev->m_task_idx;
    test_task.m_db_id = ev->m_db_id;

    vTaskEntryList task_entry_list = session.m_ent->m_player->m_tasks_entry_list;
    //find task
    bool found = false;

    for (uint32_t i = 0; i < task_entry_list.size(); ++i)
    {
       for(uint32_t t = 0; t < task_entry_list[i].m_task_list.size(); ++t)
        if(task_entry_list[i].m_task_list[t].m_task_idx == ev->m_task_idx)
        {
            found = true;
            //contact already in list, update task;
            test_task.m_task_detail = task_entry_list[i].m_task_list[t].m_detail;
            break;
        }

       if(found)
           break;
    }

    if(!found)
    {
       qCDebug(logMapEvents) << "ReceiveTaskDetailRequest m_task_idx: " << ev->m_task_idx << " not found.";
       test_task.m_task_detail = "Not found";
    }

    session.addCommandToSendNextUpdate(std::make_unique<TaskDetail>(test_task.m_db_id, test_task.m_task_idx, test_task.m_task_detail));
}


void MapInstance::on_move_inspiration(MoveInspiration *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));

    moveInspiration(session.m_ent->m_char->m_char_data, ev->src_col, ev->src_row, ev->dest_col, ev->dest_row);
}

void MapInstance::on_recv_selected_titles(RecvSelectedTitles *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    sendContactDialogClose(session); // must do this here, due to I1 client bug

    QString generic, origin, special;
    generic = getGenericTitle(ev->m_generic);
    origin  = getOriginTitle(ev->m_origin);
    special = getSpecialTitle(*session.m_ent->m_char);

    setTitles(*session.m_ent->m_char, ev->m_has_prefix, generic, origin, special);
    session.m_ent->m_entity_update_flags.setFlag(session.m_ent->UpdateFlag::TITLES);
    qCDebug(logMapEvents) << "Entity sending titles: " << session.m_ent->m_idx << ev->m_has_prefix << generic << origin << special;
}

void MapInstance::on_dialog_button(DialogButton *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));

    session.m_ent->m_has_input_on_timeframe = true;

    if(ev->success) // only sent by contactresponse
        qCDebug(logMapEvents) << "Dialog success" << ev->success;

    switch(ev->button_id)
    {
    case 0:
        callScriptingDialogButtonCallback(ev);
        break;
    case 1:
        if(session.m_ent->m_char->m_client_window_state == ClientWindowState::Training) // if training, raise level
        {
            increaseLevel(*session.m_ent);
        }
        else
        {
            callScriptingDialogButtonCallback(ev);
        }
        break;
    case 2:
        callScriptingDialogButtonCallback(ev);
        break;
    case 3: // Close Dialog - CONTACTLINK_BYE
        sendContactDialogClose(session);
        session.m_ent->m_active_dialog = NULL; // Clear scripting callback
        break;
    default:
        callScriptingDialogButtonCallback(ev);
        break;
    }

    qCDebug(logMapEvents) << "Entity: " << session.m_ent->m_idx << "has received DialogButton" << ev->button_id << ev->success;

}

void MapInstance::callScriptingDialogButtonCallback(DialogButton *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));

    if(session.m_ent->m_active_dialog != NULL)
    {
        m_scripting_interface->updateClientContext(&session);
        session.m_ent->m_active_dialog(ev->button_id);
    }
    else
    {
        auto val = m_scripting_interface->callFuncWithClientContext(&session,"dialog_button", ev->button_id);
    }
}

void MapInstance::on_move_enhancement(MoveEnhancement *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));

    moveEnhancement(session.m_ent->m_char->m_char_data, ev->m_src_idx, ev->m_dest_idx);
}

void MapInstance::on_set_enhancement(SetEnhancement *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));

    setEnhancement(*session.m_ent, ev->m_pset_idx, ev->m_pow_idx, ev->m_src_idx, ev->m_dest_idx);
}

void MapInstance::on_trash_enhancement(TrashEnhancement *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));

    trashEnhancement(session.m_ent->m_char->m_char_data, ev->m_idx);
}

void MapInstance::on_trash_enhancement_in_power(TrashEnhancementInPower *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));

    trashEnhancementInPower(session.m_ent->m_char->m_char_data, ev->m_pset_idx, ev->m_pow_idx, ev->m_eh_idx);
}

void MapInstance::on_buy_enhancement_slot(BuyEnhancementSlot *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));

    buyEnhancementSlots(*session.m_ent, ev->m_available_slots, ev->m_pset_idx, ev->m_pow_idx);
}

void MapInstance::on_recv_new_power(RecvNewPower *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));

    addPower(session.m_ent->m_char->m_char_data, ev->ppool);
}

void MapInstance::on_awaiting_dead_no_gurney(AwaitingDeadNoGurney *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    qCDebug(logMapEvents) << "Entity: " << session.m_ent->m_idx << "has received AwaitingDeadNoGurney";

    // TODO: Check if disablegurney
    /*
    setStateMode(*session.m_ent, ClientStates::AWAITING_GURNEY_XFER);
    sendClientState(session, ClientStates::AWAITING_GURNEY_XFER);
    sendDeadNoGurney(session);
    */
    // otherwise

    auto val = m_scripting_interface->callFuncWithClientContext(&session,"revive_ok", session.m_ent->m_idx);

    if (val.empty())
    {
        // Set statemode to Resurrect
        setStateMode(*session.m_ent, ClientStates::RESURRECT);
        // TODO: spawn in hospital, resurrect animations, "summoning sickness"
        revivePlayer(*session.m_ent, ReviveLevel::FULL);
    }

}

void MapInstance::on_dead_no_gurney_ok(DeadNoGurneyOK *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    qCDebug(logMapEvents) << "Entity: " << session.m_ent->m_idx << "has received DeadNoGurneyOK";

    // Set statemode to Ressurrect
    setStateMode(*session.m_ent, ClientStates::RESURRECT);
    revivePlayer(*session.m_ent, ReviveLevel::FULL);

    // TODO: Spawn where you go with no gurneys (no hospitals)
}

void MapInstance::on_browser_close(BrowserClose *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));

    qCDebug(logMapEvents) << "Entity: " << session.m_ent->m_idx << "has received BrowserClose";
}

void MapInstance::on_recv_costume_change(RecvCostumeChange *ev)
{
    // has changed costume in tailor
    MapClientSession &session(m_session_store.session_from_event(ev));
    qCDebug(logTailor) << "Entity: " << session.m_ent->m_idx << "has received CostumeChange";

    uint32_t idx = getCurrentCostumeIdx(*session.m_ent->m_char);
    session.m_ent->m_char->saveCostume(idx, ev->m_new_costume);

    session.m_ent->m_char->m_client_window_state = ClientWindowState::None;
    session.m_ent->m_entity_update_flags.setFlag(session.m_ent->UpdateFlag::COSTUMES);
    markEntityForDbStore(session.m_ent, DbStoreFlags::Full);
}

void MapInstance::on_levelup_response(LevelUpResponse *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));

    // if successful, set level
    if(session.m_ent->m_char->m_client_window_state == ClientWindowState::Training) // if training, raise level
        increaseLevel(*session.m_ent);

    qCDebug(logMapEvents) << "Entity: " << session.m_ent->m_idx << "has received LevelUpResponse" << ev->button_id << ev->result;
}

void MapInstance::on_afk_update()
{
    const std::vector<MapClientSession *> &active_sessions (m_session_store.get_active_sessions());
    const GameDataStore &data(getGameData());
    QString msg;

    for (MapClientSession *sess : active_sessions)
    {
        Entity *e = sess->m_ent;
        CharacterData* cd = &e->m_char->m_char_data;

        qCDebug(logAFK) << "Idle Time:" << cd->m_idle_time;

        if(!e->m_has_input_on_timeframe)
            cd->m_idle_time += afk_update_interval.sec();
        else
        {
            msg = QString("Receiving input after %1 seconds of inactivity from player: %2")
                    .arg(cd->m_idle_time)
                    .arg(e->m_char->getName());
            qCDebug(logAFK).noquote() << msg;

            cd->m_idle_time = 0;
            cd->m_is_on_auto_logout = false;
            e->m_has_input_on_timeframe = false;
            if(cd->m_afk)
            {
                setAFK(*e->m_char, false);
                e->m_entity_update_flags.setFlag(e->UpdateFlag::AFK, false);
                sendInfoMessage(MessageChannel::DEBUG_INFO, "You are no longer AFK", *sess);
            }
        }

        if(cd->m_idle_time >= data.m_time_to_afk && !cd->m_afk)
        {
            setAFK(*e->m_char, true, "Auto AFK");
            e->m_entity_update_flags.setFlag(e->UpdateFlag::AFK);
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
    // move all to-remove timers to the end of the m_lua_timers
    auto first_to_remove=std::partition(m_lua_timers.begin(),m_lua_timers.end(),[](const auto &t) {
        return !t.m_remove;
    });
    // remove all dead timers first
    if(first_to_remove!=m_lua_timers.end())
        m_lua_timers.erase(first_to_remove,m_lua_timers.end());
    for(const auto &t: m_lua_timers)
    {
        if(t.m_is_enabled && t.m_on_tick_callback != nullptr)
        {
            m_scripting_interface->updateMapInstance(this);
            int64_t time = getSecsSince2000Epoch();
            int64_t diff = time - t.m_start_time;
            t.m_on_tick_callback(t.m_start_time, diff, time);
        }
    }

}

void MapInstance::on_update_entities()
{
    const std::vector<MapClientSession *> &active_sessions (m_session_store.get_active_sessions());

    // all active sessions are for player, so we don't need to verify if db_id != 0
    for (const auto &sess : active_sessions)
    {
        Entity *e = sess->m_ent;
        updateLastOnline(*e->m_char); // set this here, in case we disconnect unexpectedly
        send_character_update(e);

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
                e->m_player->m_player_statistics,
                e->m_player->m_player_progress });

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
                e->m_player->m_player_statistics,
                e->m_player->m_player_progress });

    serializeToQString(playerData, cerealizedPlayerData);

    PlayerUpdateMessage* msg = new PlayerUpdateMessage(
                PlayerUpdateData({
                                     e->m_char->m_db_id,
                                     cerealizedPlayerData
                                 }), (uint64_t)1);

    m_sync_service->putq(msg);
    unmarkEntityForDbStore(e, DbStoreFlags::PlayerData);
}

void MapInstance::on_trade_cancelled(TradeWasCancelledMessage* ev)
{
    MapClientSession& session = m_session_store.session_from_event(ev);

    if(session.m_ent->m_trade == nullptr)
    {
        // Trade already cancelled.
        // The client sends this many times while closing the trade window for some reason.
        return;
    }

    const uint32_t tgt_db_id = session.m_ent->m_trade->getOtherMember(*session.m_ent).m_db_id;
    Entity* const tgt = getEntityByDBID(session.m_ent->m_client->m_current_map, tgt_db_id);
    if(tgt == nullptr)
    {
        // Only one side left in the game.
        discardTrade(*session.m_ent);

        const QString msg = "Trade cancelled because the other player left.";
        sendTradeCancel(session, msg);

        qCDebug(logTrades) << session.m_ent->name() << "cancelled a trade where target has disappeared";
        return;
    }

    discardTrade(*session.m_ent);
    discardTrade(*tgt);

    const QString msg_src = "You cancelled the trade with " + tgt->name() + ".";
    const QString msg_tgt = session.m_ent->name() + " canceled the trade.";
    sendTradeCancel(session, msg_src);
    sendTradeCancel(*tgt->m_client, msg_tgt);

    qCDebug(logTrades) << session.m_ent->name() << "cancelled a trade with" << tgt->name();
}

void MapInstance::on_trade_updated(TradeWasUpdatedMessage* ev)
{
    MapClientSession& session = m_session_store.session_from_event(ev);

    Entity* const tgt = getEntityByDBID(session.m_current_map, ev->m_info.m_db_id);
    if(tgt == nullptr)
        return;

    QString msg;
    TradeSystemMessages result;
    result = updateTrade(*session.m_ent, *tgt, ev->m_info);
    switch(result)
    {
    case TradeSystemMessages::HAS_SENT_NO_TRADE:
        msg = "You have not sent a trade offer.";
        break;
    case TradeSystemMessages::TGT_RECV_NO_TRADE:
        msg = QString("%1 has not received a trade offer.").arg(tgt->name());
        break;
    case TradeSystemMessages::SRC_RECV_NO_TRADE:
        msg = QString("You are not considering a trade offer from %1.").arg(tgt->name());
        break;
    case TradeSystemMessages::SUCCESS:
    {
        // send tradeUpdate pkt to client
        Trade& trade = *session.m_ent->m_trade;
        TradeMember& trade_src = trade.getMember(*session.m_ent);
        TradeMember& trade_tgt = trade.getMember(*tgt);
        sendTradeUpdate(session, *tgt->m_client, trade_src, trade_tgt);

        if(session.m_ent->m_trade->isAccepted())
        {
            finishTrade(*session.m_ent, *tgt); // finish handling trade
            sendTradeSuccess(session, *tgt->m_client); // send tradeSuccess pkt to client
        }
        break;
    }
    default:
        msg = "Something went wrong with trade update!"; // this should never happen
    }

    sendInfoMessage(MessageChannel::SERVER, msg, session);
}

void MapInstance::on_souvenir_detail_request(SouvenirDetailRequest* ev)
{
    MapClientSession& session = m_session_store.session_from_event(ev);
    vSouvenirList sl = session.m_ent->m_player->m_souvenirs;

    Souvenir souvenir_detail;
    bool found = false;
    for(const Souvenir &s: sl)
    {
        if(s.m_idx == ev->m_souvenir_idx)
        {
            souvenir_detail = s;
            found = true;
            qCDebug(logScripts) << "SouvenirDetail Souvenir " << ev->m_souvenir_idx << " found";
            break;
        }
    }

    if(!found)
    {
        qCDebug(logScripts) << "SouvenirDetail Souvenir " << ev->m_souvenir_idx << " not found";
        souvenir_detail.m_idx = 0; // Should always be found?
        souvenir_detail.m_description = "Data not found";

    }
    session.addCommand<SouvenirDetail>(souvenir_detail);
}

void MapInstance::on_store_sell_item(StoreSellItem* ev)
{
    qCDebug(logStores) << "on_store_sell_item. NpcId: " << ev->m_npc_idx << " isEnhancement: " << ev->m_is_enhancement << " TrayNumber: " << ev->m_tray_number << " enhancement_idx: " << ev->m_enhancement_idx;
    MapClientSession& session = m_session_store.session_from_event(ev);
    Entity *e = getEntity(&session, ev->m_npc_idx);

    QString enhancement_name;
    CharacterEnhancement enhancement = session.m_ent->m_char->m_char_data.m_enhancements[ev->m_enhancement_idx];
    enhancement_name = enhancement.m_name + "_" + QString::number(enhancement.m_level);

    if(enhancement_name.isEmpty())
    {
        qCDebug(logStores) << "on_store_sell_item. EnhancementId " << ev->m_enhancement_idx << " not found";
        return;
    }

    if(e->m_is_store && !e->m_store_items.empty())
    {
        //Find store in entity store list
        StoreTransactionResult result = Store::sellItem(e, enhancement_name);

        if(result.m_is_success)
        {
            modifyInf(session, result.m_inf_amount);
            trashEnhancement(session.m_ent->m_char->m_char_data, ev->m_enhancement_idx);
            sendChatMessage(MessageChannel::SERVER,result.m_message,session.m_ent,session);
        }
        else
            qCDebug(logStores) << "Error processing sellItem";
    }
    else
        qCDebug(logStores) << "Entity is not a store or has no items";
}

void MapInstance::on_store_buy_item(StoreBuyItem* ev)
{
    qCDebug(logMapEvents) << "on_store_buy_item. NpcId: " <<ev->m_npc_idx << " ItemName: " << ev->m_item_name;
    MapClientSession& session = m_session_store.session_from_event(ev);
    Entity *e = getEntity(&session, ev->m_npc_idx);

    StoreTransactionResult result = Store::buyItem(e, ev->m_item_name);
    if(result.m_is_success)
    {
        modifyInf(session, result.m_inf_amount);
        if(result.m_is_insp)
            giveInsp(session, result.m_item_name);
        else
            giveEnhancement(session, result.m_item_name, result.m_enhancement_lvl);

        sendChatMessage(MessageChannel::SERVER,result.m_message,session.m_ent,session);
    }
    else
        qCDebug(logStores) << "Error processing buyItem";
}

void MapInstance::on_map_swap_collision(MapSwapCollisionMessage *ev)
{
    if (!m_map_transfers.contains(ev->m_data.m_node_name))
    {
        qCDebug(logMapXfers) << QString("Map swap collision triggered on node_name %1, but that node_name doesn't exist in the list of map_transfers.");
        return;
    }

    MapXferData map_transfer_data = m_map_transfers[ev->m_data.m_node_name];
    Entity *e = getEntityByDBID(this, ev->m_data.m_ent_db_id);
    MapClientSession &sess = *e->m_client;

    sess.link()->putq(new MapXferWait(getMapPath(map_transfer_data.m_target_map_name)));
    MapServer *map_server = (MapServer *)HandlerLocator::getMap_Handler(m_game_server_id);
    map_server->putq(new ClientMapXferMessage({ sess.link()->session_token(), map_transfer_data}, 0));
}

void MapInstance::add_chat_message(Entity *sender,QString &msg_text)
{
    process_chat(sender, msg_text);
}

void MapInstance::startLuaTimer(uint32_t entity_idx)
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

void MapInstance::stopLuaTimer(uint32_t entity_idx)
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

void MapInstance::clearLuaTimer(uint32_t entity_idx)
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

void MapInstance::on_service_to_client_response(SEGSEvents::UPtrServiceToClientData data)
{
    // if the token is 0, that means it's not set to any account's token :)
    if (data == nullptr || data->m_token == 0)
        return;

    // the required session is no longer stored
    if (!m_session_store.has_session_for(data->m_token))
        return;

    MapClientSession& session = m_session_store.session_from_token(data->m_token);

    for (auto &command : data->m_commands)
        session.addCommandToSendNextUpdate(std::move(command));

    // is not null and is not empty
    if (!data->m_message.isEmpty() && !data->m_message.isNull())
        sendInfoMessage(MessageChannel::DEBUG_INFO, data->m_message, session);
}

void MapInstance::on_service_to_entity_response(SEGSEvents::UPtrServiceToEntityData data)
{
    // if the token is 0, that means it's not set to any account's token :)
    if (data == nullptr || data->m_token == 0)
        return;

    // the required session is no longer stored
    if (!m_session_store.has_session_for(data->m_token))
        return;

    MapClientSession& session = m_session_store.session_from_token(data->m_token);
    Entity* ent = session.m_ent;

    if (ent != nullptr)
        data->m_entity_found_action(ent);
}
void MapInstance::on_team_member_invited(TeamMemberInvitedMessage *msg)
{
    for (MapClientSession *cl : m_session_store)
    {
        if (cl->m_ent->name() != msg->m_data.m_invitee_name)
			continue;

		QString name = msg->m_data.m_leader_name;
		uint32_t db_id = cl->m_ent->m_db_id;
		TeamOfferType type = TeamOfferType::NoMission;

		qCDebug(logLogging) << "Sending Teamup Offer" << db_id << name << uint32_t(type);

		cl->m_ent->m_client->addCommandToSendNextUpdate(std::unique_ptr<TeamOffer>(new TeamOffer(db_id, name, type)));
    }
}

void MapInstance::on_team_toggle_lfg(TeamToggleLFGMessage *msg)
{
    MapClientSession &map_session(m_session_store.session_from_token(msg->session_token()));
	map_session.m_ent->m_char->m_char_data.m_lfg = msg->m_data.m_char_data.m_lfg;
}

void MapInstance::on_team_refresh_lfg(TeamRefreshLFGMessage *msg)
{
    MapClientSession &map_session(m_session_store.session_from_token(msg->session_token()));
    map_session.addCommand<TeamLooking>(msg->m_data.m_lfg_list);
}

void MapInstance::on_team_updated(TeamUpdatedMessage *msg)
{
    qCDebug(logTeams) << "team updated: " << msg->m_data.m_team_data.m_team_idx;

    for (const auto &mem : msg->m_data.m_team_data.m_team_members)
    {
        for (MapClientSession *cl : m_session_store)
        {
            if (cl->m_ent->m_db_id != mem.tm_idx)
                continue;

            qCDebug(logTeams) << "updating team" << msg->m_data.m_team_data.m_team_idx << mem.tm_pending << mem.tm_idx << mem.tm_name;

            cl->m_ent->m_has_team = !msg->m_data.m_disbanded;
            if(!cl->m_ent->m_team)
            {
                cl->m_ent->m_team = new Team;
            }
            cl->m_ent->m_team->m_data = msg->m_data.m_team_data;
        }
    }
}

void MapInstance::on_team_member_kicked(TeamMemberKickedMessage *msg)
{
    for (MapClientSession *cl : m_session_store)
    {
        if (cl->m_ent->name() != msg->m_data.m_kickee_name)
            continue;

        qCDebug(logTeams) << "kicking from team:" << msg->m_data.m_kickee_name;
        cl->m_ent->m_has_team = false;
        cl->m_ent->m_team = nullptr;
    }
}

void MapInstance::on_team_leave_team(TeamLeaveTeamMessage *msg)
{
    for (MapClientSession *cl : m_session_store)
    {
        if (cl->m_ent->m_db_id != msg->m_data.m_id)
            continue;

        qCDebug(logTeams) << "leaving team:" << msg->m_data.m_id;
        cl->m_ent->m_has_team = false;
        cl->m_ent->m_team = nullptr;
    }
}


//! @}
