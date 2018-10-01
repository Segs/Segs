/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup MapServer Projects/CoX/Servers/MapServer
 * @{
 */

#include "MapInstance.h"

#include "Common/GameData/CoHMath.h"
#include "Common/Servers/Database.h"
#include "Common/Servers/HandlerLocator.h"
#include "GameServer/FriendshipService/FriendHandlerEvents.h"
#include "Common/Servers/InternalEvents.h"
#include "Common/Servers/MessageBus.h"
#include "DataHelpers.h"
#include "EntityStorage.h"
#include "GameData/chardata_serializers.h"
#include "GameData/clientoptions_serializers.h"
#include "GameData/entitydata_serializers.h"
#include "GameData/keybind_serializers.h"
#include "GameData/map_definitions.h"
#include "GameData/playerdata_definitions.h"
#include "GameData/playerdata_serializers.h"
#include "GameDatabase/GameDBSyncEvents.h"
#include "Logging.h"
#include "GameServer/GameEvents.h"
#include "MapEvents.h"
#include "MapManager.h"
#include "MapSceneGraph.h"
#include "MapServer.h"
#include "TimeEvent.h"
#include "GameData/GameDataStore.h"
#include "MapTemplate.h"
#include "NetStructures/Character.h"
#include "NetStructures/CharacterHelpers.h"
#include "NetStructures/Entity.h"
#include "NetStructures/Trade.h"
#include "SEGSTimer.h"
#include "SlashCommand.h"
#include "WorldSimulation.h"
#include "serialization_common.h"
#include "serialization_types.h"
#include "version.h"
#include "Events/MapXferWait.h"
#include "Events/MapXferRequest.h"

#include <ace/Reactor.h>

#include <QtCore/QDebug>
#include <QRegularExpression>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <random>
#include <stdlib.h>

using namespace SEGSEvents;

namespace
{
    enum
    {
        World_Update_Timer   = 1,
        State_Transmit_Timer = 2,
        Session_Reaper_Timer   = 3,
        Link_Idle_Timer   = 4,
        Sync_Service_Update_Timer = 5,
        Afk_Update_Timer = 6
    };

    const ACE_Time_Value reaping_interval(0,1000*1000);
    const ACE_Time_Value link_is_stale_if_disconnected_for(0,5*1000*1000);
    const ACE_Time_Value link_update_interval(0,500*1000);
    const ACE_Time_Value world_update_interval(0,1000*1000/WORLD_UPDATE_TICKS_PER_SECOND);
    const ACE_Time_Value sync_service_update_interval(0, 30000*1000);
    const ACE_Time_Value afk_update_interval(0, 1000 * 1000);
    const ACE_Time_Value resend_interval(0,250*1000);
    const CRUDLink::duration maximum_time_without_packets(2000);
    const constexpr int MinPacketsToAck=5;

    void loadAndRunLua(std::unique_ptr<ScriptingEngine> &lua,const QString &locations_scriptname)
    {
        if(QFile::exists(locations_scriptname))
        {
            lua->loadAndRunFile(locations_scriptname);
        }
        else
        {
            qDebug().noquote() << locations_scriptname <<"is missing; Process will continue without it.";
        }
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
    m_world = new World(m_entities, getGameData().m_player_fade_in);
    m_scripting_interface.reset(new ScriptingEngine);
    m_endpoint = new MapLinkEndpoint(m_addresses.m_listen_addr); //,this
    m_endpoint->set_downstream(this);
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
        m_npc_generators.m_generators["NPCDrones"] = {"Police_Drone",EntType::NPC,{}};
        for(int i=0; i<7; ++i)
        {
            QString right_whare=QString("Door_Right_Whare_0%1").arg(i);
            m_npc_generators.m_generators[right_whare] = {right_whare, EntType::DOOR, {}};
            QString left_whare=QString("Door_Left_Whare_0%1").arg(i);
            m_npc_generators.m_generators[left_whare] = {left_whare,EntType::DOOR,{}};
            QString left_city=QString("Door_Left_City_0%1").arg(i);
            m_npc_generators.m_generators[left_city] = {left_city,EntType::DOOR,{}};
            QString right_city=QString("Door_Right_City_0%1").arg(i);
            m_npc_generators.m_generators[right_city] = {right_city,EntType::DOOR,{}};
            QString right_store=QString("Door_Right_Store_0%1").arg(i);
            m_npc_generators.m_generators[right_store] = {right_store,EntType::DOOR,{}};
            QString left_store=QString("Door_Left_Store_0%1").arg(i);
            m_npc_generators.m_generators[left_store] = {left_store,EntType::DOOR,{}};
        }
        m_npc_generators.m_generators["Door_reclpad"] = {"Door_reclpad",EntType::DOOR,{}};
        m_npc_generators.m_generators["Door_elevator"] = {"Door_elevator",EntType::DOOR,{}};
        m_npc_generators.m_generators["Door_Left_Res_03"] = {"Door_Left_Res_03",EntType::DOOR,{}};
        m_npc_generators.m_generators["Door_Right_Res_03"] = {"Door_Right_Res_03",EntType::DOOR,{}};
        m_npc_generators.m_generators["Door_Right_Ind_01"] = {"Door_Right_Ind_01",EntType::DOOR,{}};
        m_npc_generators.m_generators["Door_Left_Ind_01"] = {"Door_Left_Ind_01",EntType::DOOR,{}};

        bool scene_graph_loaded = false;
        Q_UNUSED(scene_graph_loaded);
        TIMED_LOG({
                m_map_scenegraph = new MapSceneGraph;
                scene_graph_loaded = m_map_scenegraph->loadFromFile("./data/geobin/" + scenegraph_path);
                m_new_player_spawns = m_map_scenegraph->spawn_points("NewPlayer");
            }, "Loading original scene graph"
            );
        TIMED_LOG({
            m_map_scenegraph->spawn_npcs(this);
            m_npc_generators.generate(this);
            },"Spawning npcs");
        qInfo() << "Loading custom scripts";
        QString locations_scriptname=m_data_path+'/'+"locations.lua";
        QString plaques_scriptname=m_data_path+'/'+"plaques.lua";
        loadAndRunLua(m_scripting_interface,locations_scriptname);
        loadAndRunLua(m_scripting_interface,plaques_scriptname);
    }
    else
    {
        QDir::current().mkpath(m_data_path);
        qWarning() << "FAILED to load map instance data. Check to see if file exists:"<< m_data_path;
    }

    // create a GameDbSyncService
    m_sync_service = new GameDBSyncService();
    m_sync_service->set_db_handler(m_game_server_id);
    m_sync_service->activate();

    // world simulation ticks
    m_world_update_timer = std::make_unique<SEGSTimer>(this, World_Update_Timer, world_update_interval, false);
    // state broadcast ticks
    m_resend_timer = std::make_unique<SEGSTimer>(this, State_Transmit_Timer, resend_interval, false);
    m_link_timer   = std::make_unique<SEGSTimer>(this, Link_Idle_Timer, link_update_interval, false);
    m_sync_service_timer =
        std::make_unique<SEGSTimer>(this, Sync_Service_Update_Timer, sync_service_update_interval, false);
    m_afk_update_timer = std::make_unique<SEGSTimer>(this, Afk_Update_Timer, afk_update_interval, false );

    m_session_store.create_reaping_timer(this,Session_Reaper_Timer,reaping_interval); // session cleaning
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
    HandlerLocator::setMapInstance_Handler(owner_id,instance_id,this);
    if (ACE_Reactor::instance()->register_handler(m_endpoint,ACE_Event_Handler::READ_MASK) == -1)
    {
        qWarning() << "MapInstance::spin_up_for failed to register_handler, port already open";
        return false;
    }
    if (m_endpoint->open() == -1) // will register notifications with current reactor
        ACE_ERROR_RETURN ((LM_ERROR, "(%P|%t) MapInstance: ServerEndpoint::open\n"),false);

    qInfo() << "Spun up MapInstance" << m_instance_id << "for MapServer" << m_owner_id;

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
    m_session_store.reap_stale_links("MapInstance",link_is_stale_if_disconnected_for,[tgt](uint64_t tok) {
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
        case evInputState:
            on_input_state(static_cast<InputState *>(ev));
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
        case evSendStance:
            on_send_stance(static_cast<SendStance *>(ev));
            break;
        case evSetDestination:
            on_set_destination(static_cast<SetDestination *>(ev));
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
            on_switch_viewpoint(static_cast<SwitchViewPoint *>(ev));
            break;
        case evSaveClientOptions:
            on_client_options(static_cast<SaveClientOptions *>(ev));
            break;
        case evDescriptionAndBattleCry:
            on_description_and_battlecry(static_cast<DescriptionAndBattleCry *>(ev));
            break;
        case evSetDefaultPowerSend:
            on_set_default_power_send(static_cast<SetDefaultPowerSend *>(ev));
            break;
        case evSetDefaultPower:
            on_set_default_power(static_cast<SetDefaultPower *>(ev));
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
            on_select_keybind_profile(static_cast<SelectKeybindProfile *>(ev));
            break;
        case evSetKeybind:
            on_set_keybind(static_cast<SetKeybind *>(ev));
            break;
        case evRemoveKeybind:
            on_remove_keybind(static_cast<RemoveKeybind *>(ev));
            break;
        case evResetKeybinds:
            on_reset_keybinds(static_cast<ResetKeybinds *>(ev));
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
        case MapEventTypes::evTradeWasCancelledMessage:
            on_trade_cancelled(static_cast<TradeWasCancelledMessage *>(ev));
            break;
        case MapEventTypes::evTradeWasUpdatedMessage:
            on_trade_updated(static_cast<TradeWasUpdatedMessage *>(ev));
            break;
        case MapEventTypes::evInitiateMapXfer:
            on_initiate_map_transfer(static_cast<InitiateMapXfer *>(ev));
            break;
        case MapEventTypes::evMapXferComplete:
            on_map_xfer_complete(static_cast<MapXferComplete *>(ev));
            break;
        case evSendFriendListMessage:
            on_update_friendslist(static_cast<SendFriendListMessage *>(ev));
            break;
        case evSendNotifyFriendMessage:
            on_notify_friend(static_cast<SendNotifyFriendMessage *>(ev));
            break;
        case evAwaitingDeadNoGurney:
            on_awaiting_dead_no_gurney(static_cast<AwaitingDeadNoGurney *>(ev));
            break;
        case evBrowserClose:
            on_browser_close(static_cast<BrowserClose *>(ev));
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
         qCDebug(logMapXfers) << QString("Client Session %1 attempting to initiate transfer with no map data message received").arg(session.link()->session_token());
         return;
    }
    
    // This is used here to get the map idx to send to the client for the transfer, but we
    // remove it from the std::map after the client has sent us the ClientRenderingResumed event so we
    // can prevent motd showing every time.
    uint8_t map_idx = map_server->session_map_xfer_idx(lnk->session_token()); 
    QString map_path = getMapPath(map_idx).toLower();
    GameAccountResponseCharacterData c_data;
    QString serialized_data;
    
    fromActualCharacter(*session.m_ent->m_char, *session.m_ent->m_player, *session.m_ent->m_entity, c_data);
    serializeToQString(c_data, serialized_data);
    ExpectMapClientRequest *map_req = new ExpectMapClientRequest({session.auth_id(), session.m_access_level, lnk->peer_addr(),
                                    serialized_data, session.m_requested_slot_idx, session.m_name, map_path,
                                    session.m_max_slots},
                                    lnk->session_token(),this);
    map_server->putq(map_req);
}

void MapInstance::on_map_xfer_complete(MapXferComplete */*ev*/)
{
    // TODO: Do anything necessary after connecting to new map instance here.
}

void MapInstance::on_idle(Idle *ev)
{
    MapLink * lnk = (MapLink *)ev->src();
    // TODO: put idle sending on timer, which is reset each time some other packet is sent ?
    lnk->putq(new Idle);
}

void MapInstance::on_check_links()
{
    // walk the active sessions
    for(const auto &entry : m_session_store)
    {
        MapLink * client_link = entry->link();
        if(!client_link)
        {
            // don't send to disconnected clients :)
            continue;
        }
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
        if (!session.m_ent->m_char->m_char_data.m_is_on_auto_logout)
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
            ->putq(new ClientDisconnectedMessage({session_token,session.auth_id()},0));

    EventProcessor *f_tgt = HandlerLocator::getFriend_Handler();
    f_tgt->putq(new FriendDisconnectedMessage({ent->m_db_id},0));

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
            ->putq(new ClientDisconnectedMessage({session_token,session.auth_id()},0));

    EventProcessor *f_tgt = HandlerLocator::getFriend_Handler();
    f_tgt->putq(new FriendDisconnectedMessage({ent->m_db_id},0));

    removeLFG(*ent);
    leaveTeam(*ent);
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
    if (request_data.char_from_db_data.isEmpty())
    {
        EventProcessor *game_db = HandlerLocator::getGame_DB_Handler(m_game_server_id);
        game_db->putq(new WouldNameDuplicateRequest({request_data.m_character_name},ev->session_token(),this) );
        // while we wait for db response, mark session as waiting for reaping
        m_session_store.locked_mark_session_for_reaping(&map_session,ev->session_token());
        return;
    }
    GameAccountResponseCharacterData char_data;
    serializeFromQString(char_data,request_data.char_from_db_data);
    // existing character
    Entity *ent = m_entities.CreatePlayer();
    toActualCharacter(char_data, *ent->m_char,*ent->m_player, *ent->m_entity);
    ent->fillFromCharacter(getGameData());
    ent->m_client = &map_session;
    map_session.m_ent = ent;
    // Now we inform our game server that this Map server instance is ready for the client

    ev->src()->putq(new ExpectMapClientResponse({cookie, 0, m_addresses.m_location_addr}, ev->session_token()));
}

void MapInstance::on_update_friendslist(SendFriendListMessage *ev){
    MapClientSession &map_session(m_session_store.session_from_token(ev->m_data.m_session_token));
    Entity * e = map_session.m_ent;
    FriendsList flist = ev->m_data.m_friendlist;
    e->m_char->m_char_data.m_friendlist = flist;
    sendFriendsListUpdate(e,flist);
}

void MapInstance::on_notify_friend(SendNotifyFriendMessage *ev){
    //Get the session we want to notify, and the session of who connected (so we can get the name)
    MapClientSession &notify_session(m_session_store.session_from_token(ev->m_data.m_notify_token));
    MapClientSession &connected_session(m_session_store.session_from_token(ev->m_data.m_connected_token));
    Entity * e = connected_session.m_ent;

    QString notify_msg = "Your friend " + e->m_char->getName() + " is now online.";
    sendInfoMessage(MessageChannel::SERVER,notify_msg,notify_session);
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

    if (logSpawn().isDebugEnabled())
    {
        qCDebug(logSpawn).noquote() << "Dumping Entity Data during spawn:\n";
        map_session.m_ent->dump();
    }

    // Tell our game server we've got the client
    EventProcessor *tgt = HandlerLocator::getGame_Handler(m_game_server_id);
    tgt->putq(new ClientConnectedMessage(
        {ev->session_token(),m_owner_id,m_instance_id,map_session.auth_id()},0));

    map_session.m_current_map->enqueue_client(&map_session);
    setMapIdx(*map_session.m_ent, index());
    map_session.link()->putq(new MapInstanceConnected(this, 1, ""));

    EventProcessor *f_tgt = HandlerLocator::getFriend_Handler();
    f_tgt->putq(new FriendConnectedMessage({ev->session_token(),m_owner_id,m_instance_id, e->m_db_id, e->m_char->m_char_data.m_friendlist},0));
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

    if (logSpawn().isDebugEnabled())
    {
        qCDebug(logSpawn).noquote() << "Dumping Entity Data during spawn:\n";
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
    if (ev->m_new_character)
    {
        QString ent_data;
        Entity *e = m_entities.CreatePlayer();

        const GameDataStore &data(getGameData());

        fillEntityFromNewCharData(*e, ev->m_character_data, data);
        e->m_char->m_account_id = map_session.auth_id();
        e->m_client = &map_session;
        map_session.m_ent = e;
        glm::vec3 spawn_pos = glm::vec3(128.0f,16.0f,-198.0f);
        if(!m_new_player_spawns.empty())
            spawn_pos = glm::vec3(m_new_player_spawns[rand()%m_new_player_spawns.size()][3]);
        forcePosition(*e,spawn_pos);
        e->m_entity_data.m_access_level = map_session.m_access_level;
        // new characters are transmitted nameless, use the name provided in on_expect_client
        e->m_char->setName(map_session.m_name);
        GameAccountResponseCharacterData char_data;
        fromActualCharacter(*e->m_char,*e->m_player, *e->m_entity, char_data);
        serializeToDb(e->m_entity_data,ent_data);

        // create the character from the data.
        //fillGameAccountData(map_session.m_client_id, map_session.m_game_account);
        // FixMe: char_data members index, m_current_costume_idx, and m_villain are not initialized.
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
    srand(time(nullptr));

    EntitiesResponse *res=new EntitiesResponse(&session);
    res->m_map_time_of_day = m_world->time_of_day();
    res->is_incremental(false); // full world update = op 3
    res->ent_major_update = true;
    res->abs_time = 30*100*(m_world->accumulated_time);
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
    switch (timer_id) {
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
    }
}

void MapInstance::sendState() {

    if(m_session_store.num_sessions()==0)
        return;

    auto iter=m_session_store.begin();
    auto end=m_session_store.end();

    for(;iter!=end; ++iter)
    {

        MapClientSession *cl = *iter;
        EntitiesResponse *res=new EntitiesResponse(cl);
        res->m_map_time_of_day = m_world->time_of_day();

        if(cl->m_in_map==false) // send full updates until client `resumes`
        {
            //TODO: decide when we need full updates res->is_incremental(false);
        }
        else
        {
            res->is_incremental(true); // incremental world update = op 2
        }
        res->ent_major_update = true;
        res->abs_time = 30*100*(m_world->accumulated_time);
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
    sendEnhanceCombineResponse(session.m_ent, res.success, res.destroyed);
    session.m_ent->m_char->m_char_data.m_powers_updated = res.success || res.destroyed;

    qCDebug(logMapEvents) << "Entity: " << session.m_ent->m_idx << "wants to merge enhancements" /*<< ev->first_power << ev->second_power*/;
}

void MapInstance::on_input_state(InputState *st)
{
    MapClientSession &session(m_session_store.session_from_event(st));
    Entity *   ent = session.m_ent;
    if (st->m_data.m_has_input_commit_guess)
        ent->m_input_ack = st->m_data.m_send_id;
    ent->inp_state = st->m_data;

    if (st->m_data.m_input_received)
        ent->m_has_input_on_timeframe = st->m_data.m_input_received;

    // Set Target
    if(st->m_has_target && (getTargetIdx(*ent) != st->m_target_idx))
    {
        ent->m_has_input_on_timeframe = true;
        setTarget(*ent, st->m_target_idx);
    }

    // Set Orientation
    if(st->m_data.m_orientation_pyr.p || st->m_data.m_orientation_pyr.y || st->m_data.m_orientation_pyr.r)
    {
        ent->m_entity_data.m_orientation_pyr = st->m_data.m_orientation_pyr;
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
    qDebug("Received cookie confirm %x - %x\n", ev->cookie, ev->console);
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

QString process_replacement_strings(MapClientSession *sender,const QString &msg_text)
{
    /*
    // $$           - newline
    // $archetype   - the archetype of your character
    // $battlecry   - your character's battlecry, as entered on your character ID screen
    // $level       - your character's current level
    // $name        - your character's name
    // $origin      - your character's origin
    // $target      - your currently selected target's name

    msg_text = msg_text.replace("$target",sender->m_ent->target->name());
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
    QString  target_char_name;

    qCDebug(logChat) << "src -> tgt: " << sender->m_ent->m_idx  << "->" << target_idx;

    if(target_idx > 0)
    {
        Entity   *tgt    = getEntity(sender,target_idx);
        target_char_name = tgt->name();
    }
    else
        target_char_name = c.getName();

    foreach (const QString &str, replacements) {
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
                qCDebug(logChat) << "need to send newline for" << str; // TODO: Need method for returning newline in str
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

void MapInstance::process_chat(MapClientSession *sender,QString &msg_text)
{
    int first_space = msg_text.indexOf(QRegularExpression("\\s"), 0); // first whitespace, as the client sometimes sends tabs
    QString sender_char_name;
    QString prepared_chat_message;

    QStringRef cmd_str(msg_text.midRef(0,first_space));
    QStringRef msg_content(msg_text.midRef(first_space+1,msg_text.lastIndexOf("\n")));
    MessageChannel kind = getKindOfChatMessage(cmd_str);
    std::vector<MapClientSession *> recipients;

    if(!sender)
      return;

    if(sender->m_ent)
      sender_char_name = sender->m_ent->name();

    switch(kind)
    {
        case MessageChannel::LOCAL:
        {
            // send only to clients within range
            glm::vec3 senderpos = sender->m_ent->m_entity_data.m_pos;
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

            Entity *tgt = getEntity(sender,target_name);

            if(tgt == nullptr)
            {
                prepared_chat_message = QString("No player named \"%1\" currently online.").arg(target_name);
                sendInfoMessage(MessageChannel::USER_ERROR,prepared_chat_message,*sender);
                break;
            }
            else
            {
                prepared_chat_message = QString(" -->%1: %2").arg(target_name,msg_content.toString());
                sendChatMessage(MessageChannel::PRIVATE,prepared_chat_message,sender,*sender); // in this case, sender is target

                prepared_chat_message = QString(" %1: %2").arg(sender_char_name,msg_content.toString());
                sendChatMessage(MessageChannel::PRIVATE,prepared_chat_message,sender,*tgt->m_client);
            }

            break;
        }
        case MessageChannel::TEAM:
        {
            if(!sender->m_ent->m_has_team)
            {
                prepared_chat_message = "You are not a member of a Team.";
                sendInfoMessage(MessageChannel::USER_ERROR,prepared_chat_message,*sender);
                break;
            }

            // Only send the message to characters on sender's team
            for(MapClientSession *cl : m_session_store)
            {
                if(sender->m_ent->m_team->m_team_idx == cl->m_ent->m_team->m_team_idx)
                    recipients.push_back(cl);
            }
            prepared_chat_message = QString(" %1: %2").arg(sender_char_name,msg_content.toString());
            for(MapClientSession * cl : recipients)
            {
                sendChatMessage(MessageChannel::TEAM,prepared_chat_message,sender,*cl);
            }
            break;
        }
        case MessageChannel::SUPERGROUP:
        {
            if(!sender->m_ent->m_has_supergroup)
            {
                prepared_chat_message = "You are not a member of a SuperGroup.";
                sendInfoMessage(MessageChannel::USER_ERROR,prepared_chat_message,*sender);
                break;
            }

            // Only send the message to characters in sender's supergroup
            for(MapClientSession *cl : m_session_store)
            {
                if(sender->m_ent->m_supergroup.m_SG_id == cl->m_ent->m_supergroup.m_SG_id)
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
            FriendsList * fl = &sender->m_ent->m_char->m_char_data.m_friendlist;
            if(!fl->m_has_friends || fl->m_friends_count == 0)
            {
                prepared_chat_message = "You don't have any friends to message.";
                sendInfoMessage(MessageChannel::USER_ERROR,prepared_chat_message,*sender);
                break;
            }
            // Only send the message to characters in sender's friendslist
            prepared_chat_message = QString(" %1: %2").arg(sender_char_name,msg_content.toString());
            for(Friend &f : fl->m_friends)
            {
                if(f.m_online_status != true)
                    continue;
                assert(false);
                //TODO: this only work for friends on local server
                // introduce a message router, and send messages to EntityIDs instead of directly using sessions.
                Entity *tgt = nullptr; //getEntityByDBID(*sender,f.m_db_id);
                if(tgt == nullptr) // In case we didn't toggle online_status.
                    continue;

                sendChatMessage(MessageChannel::FRIENDS,prepared_chat_message,sender,*tgt->m_client);
            }
            sendChatMessage(MessageChannel::FRIENDS,prepared_chat_message,sender,*sender);
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
        contents = process_replacement_strings(&session, contents);

    //printf("Console command received %s\n",qPrintable(ev->contents));

    ent->m_has_input_on_timeframe = true;

    if(isChatMessage(contents))
    {
        process_chat(&session,contents);
    }
    else if(has_emote_prefix(contents))
    {
        on_emote_command(contents, ent);
    }
    else
    {
        runCommand(contents,session);
    }
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
    static const QStringList newspaperCommands = {"newspaper", "afk"};
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
        if(ent->m_is_flying)                                                    // Different versions when flying and on the ground.
            msg = "Unhandled flying Afraid emote";
        else
            msg = "Unhandled ground Afraid emote";
    }
    else if(akimboCommands.contains(lowerContents) && !ent->m_is_flying)        // Akimbo: Stands with fists on hips looking forward, hold stance.
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
    else if(boomBoxCommands.contains(lowerContents) && !ent->m_is_flying)       // BoomBox (has sound): Summons forth a boombox (it just appears) and leans over to turn it on, stands up and does a sort of dance. A random track will play.
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
    else if(bowCommands.contains(lowerContents) && !ent->m_is_flying)           // Bow: Chinese/Japanese style bow with palms together, returns to normal stance.
        msg = "Unhandled Bow emote";                                            // Not allowed when flying.
    else if(bowDownCommands.contains(lowerContents))                            // BowDown: Thrusts hands forward, then points down, as if ordering someone else to bow before you.
        msg = "Unhandled BowDown emote";
    else if(lowerContents == "burp" && !ent->m_is_flying)                       // Burp (has sound): A raunchy belch, wipes mouth with arm afterward, ape-like stance.
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
    else if(lowerContents == "crossarms" && !ent->m_is_flying)                  // CrossArms: Crosses arms, stance (slightly different from most other crossed arm stances).
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
    else if(lowerContents == "roar" && !ent->m_is_flying)                       // Roar: Claws air, roaring, ape-like stance.
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
        if(ent->m_is_flying)                                                    // Different versions when flying and on the ground.
            msg = "Unhandled flying Tarzan emote";
        else
            msg = "Unhandled ground Tarzan emote";
    }
    else if(taunt1Commands.contains(lowerContents))                             // Taunt1: Taunts, beckoning with one hand, then slaps fist into palm, repeating stance.

    {
        if(ent->m_is_flying)                                                    // Different versions when flying and on the ground.
            msg = "Unhandled flying Taunt1 emote";
        else
            msg = "Unhandled ground Taunt1 emote";
    }
    else if(taunt2Commands.contains(lowerContents))                             // Taunt2: Taunts, beckoning with both hands, combat stance.
    {
        if(ent->m_is_flying)                                                    // Different versions when flying and on the ground.
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
        if(ent->m_is_flying)                                                    // Different versions when flying and on the ground.
            msg = "Unhandled flying Yoga emote";
        else
            msg = "Unhandled ground Yoga emote";
    }
                                                                                // Boombox Emotes
    else if(lowerContents.startsWith("bb") && !ent->m_is_flying)                // Check if Boombox Emote.
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
        if(ent->m_is_flying)                                                    // Different versions when flying and on the ground.
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
        sendChatMessage(MessageChannel::EMOTE,msg,src,*cl);
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

    qCDebug(logMiniMap) << "MiniMapState tile "<< ev->tile_idx << " for player" << ent->name();
    // TODO: Save these tile #s to dbase and (presumably) load upon entering map to remove fog-of-war from map
}

void MapInstance::on_client_resumed(ClientResumedRendering *ev)
{
    // TODO only do this the first time a client connects, not after map transfers..
    MapClientSession &session(m_session_store.session_from_event(ev));
    MapServer *map_server = (MapServer *)HandlerLocator::getMap_Handler(m_game_server_id);
    if(session.m_in_map==false)
        session.m_in_map = true;
    if (!map_server->session_has_xfer_in_progress(session.link()->session_token()))
    {
        char buf[256];
        std::string welcome_msg = std::string("Welcome to SEGS ") + VersionInfo::getAuthVersion()+"\n";
        std::snprintf(buf, 256, "There are %zu active entities and %zu clients", m_entities.active_entities(),
                    m_session_store.num_sessions());
        welcome_msg += buf;
        sendInfoMessage(MessageChannel::SERVER,QString::fromStdString(welcome_msg),session);

        sendServerMOTD(&session);
    }
    else
    {
        // else don't send motd, as this is from a map transfer
        // TODO: check if there's a better place to complete the map transfer..
        map_server->session_xfer_complete(session.link()->session_token());
    }
}

void MapInstance::on_location_visited(LocationVisited *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    qCDebug(logMapEvents) << "Attempting a call to script location_visited with:"<<ev->m_name<<qHash(ev->m_name);
    auto val = m_scripting_interface->callFuncWithClientContext(&session,"location_visited",qHash(ev->m_name));
    sendInfoMessage(MessageChannel::DEBUG_INFO,QString::fromStdString(val),session);

    qCWarning(logMapEvents) << "Unhandled location visited event:" << ev->m_name <<
                  QString("(%1,%2,%3)").arg(ev->m_pos.x).arg(ev->m_pos.y).arg(ev->m_pos.z);
}

void MapInstance::on_plaque_visited(PlaqueVisited * ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    qCDebug(logMapEvents) << "Attempting a call to script plaque_visited with:"<<ev->m_name<<qHash(ev->m_name);
    auto val = m_scripting_interface->callFuncWithClientContext(&session,"plaque_visited",qHash(ev->m_name));
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
    
    // ev->name is the map_idx when using the map menu currently.
    if (!map_server->session_has_xfer_in_progress(session.link()->session_token()))
    {   
        uint8_t map_idx = ev->name.toInt();
        if (ev->location.x != 0 || ev->location.y != 0 || ev->location.z != 0)
            map_idx = std::rand() % 23;

        // TODO: change this to not be hacky.
        // change the map idx if you're trying to load the map you're currently on
        // this should only ever happen with /movezone commands after doors work correctly.
        if (map_idx == m_index)
            map_idx = (map_idx + 1) % 23;
        map_server->putq(new ClientMapXferMessage({session.link()->session_token(), map_idx},0));
        session.link()->putq(new MapXferWait(getMapPath(map_idx))); 
    }
    else
    {
        qCWarning(logMapXfers).noquote() << "Unhandled door entry request to:" << ev->name;
        if(ev->no_location)
            qCWarning(logMapXfers).noquote() << "    no location provided";
        else
            qCWarning(logMapXfers).noquote() << ev->location.x<< ev->location.y<< ev->location.z;
    }

    //pseudocode:
    //  auto door = get_door(ev->name,ev->location);
    //  if(door and player_can_enter(door)
    //    process_map_transfer(player,door->targetMap);
}

void MapInstance::on_change_stance(ChangeStance * ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));

    if(ev->enter_stance)
    {
        //session.m_ent->m_stance = &getPower(*session.m_ent, ev->pset_idx, ev->pow_idx);
        qCWarning(logMapEvents) << "Unhandled change stance request" << session.m_ent->m_idx << ev->powerset_index
                                << ev->power_index;
    }
    else
    {
        session.m_ent->m_stance = nullptr;
        qCWarning(logMapEvents) << "Unhandled exit stance request" << session.m_ent->m_idx;
    }
}

void MapInstance::on_send_stance(SendStance * ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));

    //ev->m_enter_stance = true;
    //ev->m_pset_idx = session.m_ent->m_stance->m_power_tpl.m_pset_idx;
    //ev->m_pow_idx = session.m_ent->m_stance->m_power_tpl.m_pow_idx;

    qCWarning(logMapEvents) << "Unhandled send stance request";

    if(ev->m_enter_stance)
        qCWarning(logMapEvents) << "Entity" << session.m_ent->name() << "SendStance" << ev->m_pset_idx << ev->m_pow_idx;
    else
        qCWarning(logMapEvents) << "Entity" << session.m_ent->name() << "SendStance is zero";
}

void MapInstance::on_set_destination(SetDestination * ev)
{
    qCWarning(logMapEvents) << "Unhandled set destination request"
               << "\n\t" << "index" << ev->point_index
               << "loc" << ev->destination.x << ev->destination.y << ev->destination.z;
}

void MapInstance::on_abort_queued_power(AbortQueuedPower * /*ev*/)
{
    qCWarning(logMapEvents) << "Unhandled abort queued power request";
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

    session.addCommandToSendNextUpdate(std::unique_ptr<EntityInfoResponse>(new EntityInfoResponse(description)));
    qCDebug(logDescription) << "Entity info requested" << ev->entity_idx << description;
}

void MapInstance::on_client_options(SaveClientOptions * ev)
{
    // Save options/keybinds to character entity and entry in the database.
    MapClientSession &session(m_session_store.session_from_event(ev));
    //LinkBase * lnk = (LinkBase *)ev->src();

    Entity *ent = session.m_ent;
    markEntityForDbStore(ent,DbStoreFlags::PlayerData);
    ent->m_player->m_options = ev->data;
}

void MapInstance::on_switch_viewpoint(SwitchViewPoint *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    Entity *ent = session.m_ent;

    ent->m_player->m_options.m_first_person_view = ev->new_viewpoint_is_firstperson;
    qCDebug(logMapEvents) << "Saving viewpoint mode to ClientOptions" << ev->new_viewpoint_is_firstperson;
}

void MapInstance::on_chat_reconfigured(ChatReconfigure *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    Entity *ent = session.m_ent;

    ent->m_player->m_gui.m_chat_top_flags = ev->m_chat_top_flags;
    ent->m_player->m_gui.m_chat_bottom_flags = ev->m_chat_bottom_flags;

    qCDebug(logMapEvents) << "Saving chat channel mask settings to GUISettings" << ev->m_chat_top_flags << ev->m_chat_bottom_flags;
}

void MapInstance::on_set_default_power_send(SetDefaultPowerSend *ev)
{
    qCWarning(logMapEvents) << "Unhandled Set Default Power Send request:" << ev->powerset_idx << ev->power_idx;
}

void MapInstance::on_set_default_power(SetDefaultPower */*ev*/)
{
    qCWarning(logMapEvents) << "Unhandled Set Default Power request.";
}

void MapInstance::on_unqueue_all(UnqueueAll *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    Entity *ent = session.m_ent;

    // What else could go here?
    ent->m_target_idx = 0;
    ent->m_assist_target_idx = 0;
    // cancelAttack(ent);
    // unqueuePowers(ent);
    // unqueueInspirations(ent); // merge this with unqueuePowers()?

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
    uint32_t tgt_idx = ev->target_idx;

    if(ev->target_idx == 0 || ev->target_idx == session.m_ent->m_idx)
        tgt_idx = session.m_ent->m_idx;
    else
        sendFaceEntity(session.m_ent, tgt_idx);

    usePower(*session.m_ent, ev->pset_idx, ev->pow_idx, tgt_idx, ev->target_db_id);
    qCDebug(logPowers) << "Entity: " << session.m_ent->m_idx << "has activated power" << ev->pset_idx << ev->pow_idx << ev->target_idx << ev->target_db_id;
}

void MapInstance::on_activate_power_at_location(ActivatePowerAtLocation *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    session.m_ent->m_has_input_on_timeframe = true;

    // TODO: Check that target is valid, then Do Power!
    QString contents = QString("To Location: <%1, %2, %3>").arg(ev->location.x).arg(ev->location.y).arg(ev->location.z);
    sendFloatingInfo(session, contents, FloatingInfoStyle::FloatingInfo_Attention, 4.0);
    sendFaceLocation(session.m_ent, ev->location);

    qCDebug(logPowers) << "Entity: " << session.m_ent->m_idx << "has activated power"<< ev->pset_idx << ev->pow_idx << ev->target_idx << ev->target_db_id;
}

void MapInstance::on_activate_inspiration(ActivateInspiration *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));

    session.m_ent->m_has_input_on_timeframe = true;
    useInspiration(*session.m_ent, ev->slot_idx, ev->row_idx);
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

void MapInstance::on_set_keybind(SetKeybind *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    Entity *ent = session.m_ent;

    KeyName key = static_cast<KeyName>(ev->key);
    ModKeys mod = static_cast<ModKeys>(ev->mods);


    ent->m_player->m_keybinds.setKeybind(ev->profile, key, mod, ev->command, ev->is_secondary);
    //qCDebug(logMapEvents) << "Setting keybind: " << ev->profile << QString::number(ev->key) << QString::number(ev->mods) << ev->command << ev->is_secondary;
}

void MapInstance::on_remove_keybind(RemoveKeybind *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    Entity *ent = session.m_ent;

    ent->m_player->m_keybinds.removeKeybind(ev->profile,(KeyName &)ev->key,(ModKeys &)ev->mods);
    //qCDebug(logMapEvents) << "Clearing Keybind: " << ev->profile << QString::number(ev->key) << QString::number(ev->mods);
}

glm::vec3 MapInstance::closest_safe_location(glm::vec3 v) const
{
    Q_UNUSED(v);
    if(!m_new_player_spawns.empty())
    {
        return m_new_player_spawns.front()[3];
    }
    return glm::vec3(0,0,0);
}

void MapInstance::serialize_from(istream &/*is*/)
{
    assert(false);
}

void MapInstance::serialize_to(ostream &/*is*/)
{
    assert(false);
}
void MapInstance::on_reset_keybinds(ResetKeybinds *ev)
{
    const GameDataStore &data(getGameData());
    const Parse_AllKeyProfiles &default_profiles(data.m_keybind_profiles);
    MapClientSession &session(m_session_store.session_from_event(ev));
    Entity *ent = session.m_ent;

    ent->m_player->m_keybinds.resetKeybinds(default_profiles);
    //qCDebug(logMapEvents) << "Resetting Keybinds to defaults.";
}

void MapInstance::on_select_keybind_profile(SelectKeybindProfile *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    Entity *ent = session.m_ent;

    ent->m_player->m_keybinds.setKeybindProfile(ev->profile);
    //qCDebug(logMapEvents) << "Saving currently selected Keybind Profile. Profile name: " << ev->profile;
}

void MapInstance::on_interact_with(InteractWithEntity *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));

    qCDebug(logMapEvents) << "Entity: " << session.m_ent->m_idx << "wants to interact with" << ev->m_srv_idx;
}

void MapInstance::on_move_inspiration(MoveInspiration *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));

    moveInspiration(session.m_ent->m_char->m_char_data, ev->src_col, ev->src_row, ev->dest_col, ev->dest_row);
}

void MapInstance::on_recv_selected_titles(RecvSelectedTitles *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    QString generic, origin, special;

    generic = getGenericTitle(ev->m_generic);
    origin  = getOriginTitle(ev->m_origin);
    special = getSpecialTitle(*session.m_ent->m_char);

    setTitles(*session.m_ent->m_char, ev->m_has_prefix, generic, origin, special);
    qCDebug(logMapEvents) << "Entity sending titles: " << session.m_ent->m_idx << ev->m_has_prefix << generic << origin << special;
}

void MapInstance::on_dialog_button(DialogButton *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));

    switch(ev->button_id)
    {
    case 0:
        // cancel?
        break;
    case 1:
        // accept?
        break;
    case 2:
        // no idea
        break;
    case 3:
        sendContactDialogClose(session);
        break;
    default:
        // close all windows?
        break;
    }

    qCDebug(logMapEvents) << "Entity: " << session.m_ent->m_idx << "has received DialogButton" << ev->button_id;
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

    buyEnhancementSlot(*session.m_ent, ev->m_num, ev->m_pset_idx, ev->m_pow_idx);
}

void MapInstance::on_recv_new_power(RecvNewPower *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));

    addPower(session.m_ent->m_char->m_char_data, ev->ppool);
}

void MapInstance::on_awaiting_dead_no_gurney(AwaitingDeadNoGurney *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    session.m_ent->m_client->addCommandToSendNextUpdate(std::unique_ptr<DeadNoGurney>(new DeadNoGurney()));
}

void MapInstance::on_browser_close(BrowserClose *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));

    qCDebug(logMapEvents) << "Entity: " << session.m_ent->m_idx << "has received BrowserClose";
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

        if (e->m_has_input_on_timeframe == false)
            cd->m_idle_time += afk_update_interval.sec();
        else
        {
            msg = QString("Receiving input from player: ") + &e->m_char->getName();
            qCDebug(logInput) << msg;
            cd->m_idle_time = 0;

            if (cd->m_afk)
                toggleAFK(*e->m_char, false);
            cd->m_is_on_auto_logout = false;

            e->m_has_input_on_timeframe = false;
        }

        if (cd->m_idle_time >= data.m_time_to_afk && !cd->m_afk)
        {
            toggleAFK(* e->m_char, true, "Auto AFK");
            msg = QString("You are AFKed after %1 seconds of inactivity.").arg(data.m_time_to_afk);
            sendInfoMessage(MessageChannel::DEBUG_INFO, msg, *sess);
        }


        if (data.m_uses_auto_logout && cd->m_idle_time >= data.m_time_to_logout_msg)
        {
            // give message that character will be auto logged out in 2 mins
            // player must not be on task force and is not on mission map for this to happen
            if (!cd->m_is_on_task_force && !isEntityOnMissionMap(e->m_entity_data) && !cd->m_is_on_auto_logout)
            {
                cd->m_is_on_auto_logout = true;
                msg = QString("You have been inactive for %1 seconds. You will automatically ").arg(data.m_time_to_logout_msg) +
                      QString("be logged out if you stay idle for %1 seconds").arg(data.m_time_to_auto_logout);
                sendInfoMessage(MessageChannel::DEBUG_INFO, msg, *sess);
            }
        }

        if (cd->m_is_on_auto_logout && cd->m_idle_time >=
                data.m_time_to_logout_msg + data.m_time_to_auto_logout
                && !e->m_is_logging_out)
        {
            e->beginLogout(30);
            msg = "You have been inactive for too long. Beginning auto-logout process...";
            sendInfoMessage(MessageChannel::DEBUG_INFO, msg, *sess);
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
        send_character_update(e);

        /* at the moment we are forcing full character updates, so I'll leave this commented for now

        // full character update
        if (e->m_db_store_flags & uint32_t(DbStoreFlags::Full))
            send_character_update(e);
        // update only player data
        else if (e->m_db_store_flags & uint32_t(DbStoreFlags::PlayerData))
            send_player_update(e);

        */
    }
}

void MapInstance::send_character_update(Entity *e)
{
    QString cerealizedCharData, cerealizedEntityData, cerealizedPlayerData;

    PlayerData playerData = PlayerData({
                e->m_player->m_gui,
                e->m_player->m_keybinds,
                e->m_player->m_options
                });

    serializeToQString(e->m_char->m_char_data, cerealizedCharData);
    serializeToQString(e->m_entity_data, cerealizedEntityData);
    serializeToQString(playerData, cerealizedPlayerData);

    CharacterUpdateMessage* msg = new CharacterUpdateMessage(
                CharacterUpdateData({
                                        e->m_char->getName(),

                                        // cerealized blobs
                                        cerealizedCharData,
                                        cerealizedEntityData,
                                        cerealizedPlayerData,

                                        // plain values
                                        e->m_char->getCurrentCostume()->m_body_type,
                                        e->m_char->getCurrentCostume()->m_height,
                                        e->m_char->getCurrentCostume()->m_physique,
                                        (uint32_t)e->m_supergroup.m_SG_id,
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
                e->m_player->m_options
                });

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
    cancelTrade(*session.m_ent);
}

void MapInstance::on_trade_updated(TradeWasUpdatedMessage* ev)
{
    MapClientSession& session = m_session_store.session_from_event(ev);
    updateTrade(*session.m_ent, ev->m_info);
}


//! @}
