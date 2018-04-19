#include "GameHandler.h"

#include "GameEvents.h"
#include "Servers/HandlerLocator.h"
#include "Servers/MessageBus.h"
#include "GameData/chardata_serializers.h"
#include "Common/Servers/InternalEvents.h"
#include "GameData/serialization_common.h"
#include "GameLink.h"
#include "GameEvents.h"
#include "GameServer.h"
#include "NetStructures/Character.h"
#include "SEGSTimer.h"

static const uint32_t supported_version=20040422;
namespace {
enum {
    Link_Idle_Timer   = 1,
    Session_Reaper_Timer   = 2,
    Service_Status_Timer = 3,
};
const ACE_Time_Value link_update_interval(0,500*1000);
const ACE_Time_Value service_update_interval(5,0);
const ACE_Time_Value session_reaping_interval(1,0);
const ACE_Time_Value maximum_time_without_packets(1,0);
const ACE_Time_Value link_is_stale_if_disconnected_for(5,0);
const constexpr int MinPacketsToAck=5;
}

GameHandler::GameHandler() : EventProcessor ()
{
    m_server = nullptr;
}

GameHandler::~GameHandler()
{
}

void GameHandler::start() {
    ACE_ASSERT(m_link_checker==nullptr);
    m_link_checker.reset(new SEGSTimer(this,(void *)Link_Idle_Timer,link_update_interval,false));
    m_service_status_timer.reset(new SEGSTimer(this,(void *)Service_Status_Timer,service_update_interval,false));
    m_session_store.create_reaping_timer(this,Session_Reaper_Timer,session_reaping_interval);
}

void GameHandler::dispatch( SEGSEvent *ev )
{
    assert(ev);
    switch(ev->type())
    {
    case SEGS_EventTypes::evTimeout:
        on_timeout(static_cast<TimerEvent *>(ev));
        break;
    case SEGS_EventTypes::evDisconnect: // link layer tells us that a link is not responsive/dead
        on_link_lost(ev);
        break;
    // Server <-> Server messages
    case Internal_EventTypes::evExpectClientRequest:
        on_expect_client(static_cast<ExpectClientRequest *>(ev));
        break;
    case Internal_EventTypes::evExpectMapClientResponse:
        on_client_expected(static_cast<ExpectMapClientResponse *>(ev));
        break;
    case Internal_EventTypes::evClientConnected:
        on_client_connected_to_other_server(static_cast<ClientConnectedMessage *>(ev));
        break;
    case Internal_EventTypes::evClientDisconnected:
        on_client_disconnected_from_other_server(static_cast<ClientDisconnectedMessage *>(ev));
        break;
    case GameEventTypes::evServerReconfigured:
        // reconfiguring service forces status broadcast
        report_service_status();
        break;
    // Client -> Server messages
    case GameEventTypes::evIdle:
        on_idle(static_cast<IdleEvent*>(ev));
        break;
    case GameEventTypes::evDisconnectRequest:
        on_disconnect(static_cast<DisconnectRequest *>(ev));
        break;
    case GameEventTypes::evConnectRequest:
        on_connection_request(static_cast<ConnectRequest *>(ev));
        break;
    case GameEventTypes::evUpdateServer:
        on_update_server(static_cast<UpdateServer *>(ev));
        break;
    case GameEventTypes::evMapAddrRequest:
        on_map_req(static_cast<MapServerAddrRequest *>(ev));
        break;
    case GameEventTypes::evDeleteCharacter:
        on_delete_character(static_cast<DeleteCharacter *>(ev));
        break;
    case GameEventTypes::evUpdateCharacter:
        on_update_character(static_cast<UpdateCharacter *>(ev));
        break;
    case GameEventTypes::evUnknownEvent:
        on_unknown_link_event(static_cast<GameUnknownRequest *>(ev));
        break;
    // DB -> Server messages
    case GameDBEventTypes::evGameDbError:
        on_game_db_error(static_cast<GameDbErrorMessage *>(ev));
        break;
    case GameDBEventTypes::evGameAccountResponse:
        on_account_data(static_cast<GameAccountResponse *>(ev));
        break;
    case GameDBEventTypes::evRemoveCharacterResponse:
        on_character_deleted(static_cast<RemoveCharacterResponse *>(ev));
        break;
    default:
        assert(!"Unknown event encountered in dispatch.");
    }
}
void GameHandler::on_game_db_error(GameDbErrorMessage *ev)
{
    GameSession &session = m_session_store.session_from_event(ev);
    session.link()->putq(new GameEntryError(this,"Unauthorized !"));
}
///
/// \brief This handler is called when we get the account information for a
/// \param ev
///
void GameHandler::on_account_data(GameAccountResponse *ev)
{
    GameSession &session(m_session_store.session_from_event(ev));
    // we've got db answer, session is ok again
    m_session_store.locked_unmark_session_for_reaping(&session);
    session.m_game_account = ev->m_data;
    // Inform auth server about succesful client connection
    EventProcessor *tgt      = HandlerLocator::getAuth_Handler();
    tgt->putq(new ClientConnectedMessage({ev->session_token(),m_server->getId(),0 }));

    m_session_store.add_to_active_sessions(&session);
    CharacterSlots *slots_event=new CharacterSlots;
    slots_event->set_account_data(&session.m_game_account);
    session.link()->putq(slots_event);
}
void GameHandler::on_connection_request(ConnectRequest *ev)
{
    // TODO: disallow connects if server is overloaded
    qDebug("Client-side CRUDP Level: %d \n\t Tick Count: %d", ev->m_version, ev->m_tickcount);
    ev->src()->putq(new ConnectResponse);
}
void GameHandler::on_update_server(UpdateServer *ev)
{
    if(ev->m_build_date!=supported_version)
    {
        ev->src()->putq(new GameEntryError(this,"We are very sorry but your client version is not supported."));
        qDebug("GameEntryError: Client version %u not supported!", ev->m_build_date);
        return;
    }
    uint64_t expecting_session_token = m_session_store.connected_client(ev->authCookie);
    if(expecting_session_token==~0U)
    {
        ev->src()->putq(new GameEntryError(this,"Unauthorized !"));
        qDebug("GameEntryError: Unauthorized!");
        return;
    }
    GameSession &session(m_session_store.session_from_token(expecting_session_token));
    session.link((GameLink *)ev->src());
    session.m_direction = GameSession::EXITING_TO_LOGIN;
    session.link()->session_token(expecting_session_token);
    EventProcessor *game_db = HandlerLocator::getGame_DB_Handler(m_server->getId());
    game_db->putq(new GameAccountRequest({session.auth_id(), m_server->getMaxCharacterSlots(), true},
                                         expecting_session_token,this));

    // here we will wait for db response, so here we're going to put the session on the read-to-reap list
    // in case db does not respond in sane time frame, the session is going to be removed.
    m_session_store.locked_mark_session_for_reaping(&session,expecting_session_token);
    // if things work ok, than GameHandler::on_account_data will get all it needs.
}
void GameHandler::on_update_character(UpdateCharacter *ev)
{
    auto lnk = (GameLink *)ev->src();
    assert(lnk->session_token());
    GameSession &session = m_session_store.session_from_event(ev);
    assert(session.m_game_account.valid());

    ev->src()->putq(new CharacterResponse(this,ev->m_index,&session.m_game_account));

    // TODO: Do we update database here? issue #271
}
void GameHandler::on_idle(IdleEvent *ev)
{
    // idle for idle 'strategy'
//    GameLink * lnk = (GameLink *)ev->src();
//    lnk->putq(new IdleEvent);
}
void GameHandler::on_check_links()
{
    // walk the active sessions
    for(const auto &entry : m_session_store)
    {
        GameLink * client_link = entry->link();
        if(!client_link)
        {
            // don't send to disconnected clients :)
            continue;
        }
        // Send at least one packet within maximum_time_without_packets
        if(client_link->last_sent_packets()>maximum_time_without_packets)
            client_link->putq(new IdleEvent); // Threading trouble, last_sent_packets will not get updated until the packet is actually sent.
        else if(client_link->client_packets_waiting_for_ack()>MinPacketsToAck)
            client_link->putq(new IdleEvent); // Threading trouble, last_sent_packets will not get updated until the packet is actually sent.
    }
}
void GameHandler::report_service_status()
{
    postGlobalEvent(new GameServerStatusMessage({m_server->getAddress(),QDateTime::currentDateTime(),
                                                 uint16_t(m_session_store.num_sessions()),
                                                 m_server->getMaxPlayers(),m_server->getId(),true}));
}

void GameHandler::on_timeout(TimerEvent *ev)
{
    // TODO: This should send 'ping' packets on all client links to which we didn't send
    // anything in the last time quantum
    // 1. Find all links that have inactivity_time() > ping_time && <disconnect_time
    // For each found link
    //   If there is no ping_pending on this link, add a ping event to queue
    // 2. Find all links with inactivity_time() >= disconnect_time
    //   Disconnect given link.

    intptr_t timer_id = (intptr_t)ev->data();
    switch (timer_id) {
        case Link_Idle_Timer:
            on_check_links();
        break;
        case Session_Reaper_Timer:
            reap_stale_links();
        break;
        case Service_Status_Timer:
            report_service_status();
        break;
    }
}

void GameHandler::on_disconnect(DisconnectRequest *ev)
{
    GameLink * lnk = (GameLink *)ev->src();
    GameSession &session = m_session_store.session_from_event(ev);
    if(session.is_connected_to_map_server_id==0)
    {
        if(session.m_direction==GameSession::EXITING_TO_MAP)
        {
            SessionStore::MTGuard guard(m_session_store.reap_lock());
            m_session_store.mark_session_for_reaping(&session,lnk->session_token());
            m_session_store.session_link_lost(lnk->session_token());
        }
        else
        {
            EventProcessor * tgt = HandlerLocator::getAuth_Handler();
            tgt->putq(new ClientDisconnectedMessage({lnk->session_token()}));
            m_session_store.session_link_lost(lnk->session_token());
            m_session_store.remove_by_token(lnk->session_token(), session.auth_id());
        }
    }
    else
        m_session_store.session_link_lost(lnk->session_token());
    lnk->putq(new DisconnectResponse);
    // Post disconnect event to link, will close it's processing loop, after it sends the response
    lnk->putq(new DisconnectEvent(lnk->session_token())); // this should work, event if different threads try to do it in parallel
}
void GameHandler::on_link_lost(SEGSEvent *ev)
{
    GameLink * lnk = (GameLink *)ev->src();
    GameSession &session = m_session_store.session_from_event(ev);

    if(session.is_connected_to_map_server_id==0)
    {
        if(session.m_direction==GameSession::EXITING_TO_MAP)
        {
            SessionStore::MTGuard guard(m_session_store.reap_lock());
            m_session_store.mark_session_for_reaping(&session,lnk->session_token());
            m_session_store.session_link_lost(lnk->session_token());
        }
        else
        {
            EventProcessor * tgt = HandlerLocator::getAuth_Handler();
            tgt->putq(new ClientDisconnectedMessage({lnk->session_token()}));
            m_session_store.session_link_lost(lnk->session_token());
            m_session_store.remove_by_token(lnk->session_token(), session.auth_id());
        }
    }
    else
        m_session_store.session_link_lost(lnk->session_token());
    // Post disconnect event to link, will close it's processing loop
    lnk->putq(new DisconnectEvent(lnk->session_token()));
}
void GameHandler::on_character_deleted(RemoveCharacterResponse *ev)
{
    GameSession &session = m_session_store.session_from_event(ev);
    auto chr(session.m_game_account.get_character(ev->m_data.slot_idx));
    chr.reset();
    session.link()->putq(new DeletionAcknowledged);
}
void GameHandler::on_delete_character(DeleteCharacter *ev)
{
    EventProcessor *game_db = HandlerLocator::getGame_DB_Handler(m_server->getId());
    GameSession &session = m_session_store.session_from_event(ev);
    GameLink * lnk = (GameLink *)ev->src();
    auto chr(session.m_game_account.get_character(ev->m_index));
    // check if character exists, and if it's name is the same as the one passed here
    if(chr.m_name.compare(ev->m_char_name)==0)
    {
        game_db->putq(new RemoveCharacterRequest({session.m_game_account.m_game_server_acc_id, chr.index},
                                                 lnk->session_token(),this));
    }
    else
    {
        lnk->putq(new GameEntryError(this,"Given name was not the same as character name\n. Character was not deleted."));
    }
}
void GameHandler::on_client_expected(ExpectMapClientResponse *ev)
{
    GameSession &session = m_session_store.session_from_event(ev);
    GameLink *lnk = session.link();
    MapServerAddrResponse *r_ev=new MapServerAddrResponse;
    r_ev->m_map_cookie  = ev->m_data.cookie;
    r_ev->m_address     = ev->m_data.m_connection_addr;
    lnk->putq(r_ev);

}
void GameHandler::on_map_req(MapServerAddrRequest *ev)
{
    GameLink * lnk = (GameLink *)ev->src();
    GameSession &session = m_session_store.session_from_event(ev);
    if (!session.m_game_account.valid())
        return; // TODO:  return some kind of error.

    GameAccountResponseCharacterData *selected_slot = &session.m_game_account.get_character(ev->m_character_index);
    CharacterData cd;
    serializeFromQString(cd,selected_slot->m_serialized_chardata);
    QString map_path = cd.m_mapName;
    switch(ev->m_mapnumber)
    {
        case 0:
        if(map_path.isEmpty())
            map_path = "maps/city_zones/city_00_01/city_00_01.txt";
        break;
        case 1: // atlas park
            map_path = "maps/city_zones/city_01_01/city_01_01.txt";
        break;
        case 29:
            map_path = "maps/city_zones/city_01_03/city_01_03.txt";
        break;
    }

    if(selected_slot->isEmpty())
        selected_slot = nullptr; // passing a null to map server to indicate a new character is being created.

    EventProcessor *map_handler=HandlerLocator::getMap_Handler(m_server->getId());
    if(nullptr == map_handler)
    {
        lnk->putq(new GameEntryError(this,"No Map Servers are running."));
        return;
    }
    //TODO: this should handle multiple map servers, for now it doesn't care and always connects to the first one.
    assert(map_handler);
    if(selected_slot )
    {
        ACE_ASSERT(selected_slot->m_name == ev->m_char_name || !"Server-Client character synchronization failure!");
    }
    ExpectMapClientRequest *expect_client =
        new ExpectMapClientRequest({session.m_auth_account_id, session.m_access_level, lnk->peer_addr(),
                                    selected_slot, ev->m_character_index, ev->m_char_name, map_path,
                                    uint16_t(session.m_game_account.m_max_slots)},
                                   lnk->session_token());
    fprintf(stderr, " Telling map server to expect a client with character %s,%d\n", qPrintable(ev->m_char_name),
            ev->m_character_index);
    session.m_direction = GameSession::EXITING_TO_MAP;
    map_handler->putq(expect_client);
}
void GameHandler::on_unknown_link_event(GameUnknownRequest *)
{
        ACE_DEBUG((LM_WARNING,ACE_TEXT("Unknown GameHandler link event.\n")));
}

void GameHandler::on_expect_client( ExpectClientRequest *ev )
{
    GameSession *sess = m_session_store.create_or_reuse_session_for(ev->session_token());
    uint32_t cookie = m_session_store.expect_client_session(ev->session_token(),ev->m_data.m_from_addr,ev->m_data.m_client_id);
    sess->m_auth_account_id = ev->m_data.m_client_id;
    sess->m_access_level = ev->m_data.m_access_level;
    HandlerLocator::getAuth_Handler()->putq(new ExpectClientResponse({ev->m_data.m_client_id,cookie,m_server->getId()},ev->session_token()));
}

void GameHandler::on_client_connected_to_other_server(ClientConnectedMessage *ev)
{
    assert(ev->m_data.m_server_id);
    assert(ev->m_data.m_sub_server_id);
    GameSession &session(m_session_store.session_from_token(ev->m_data.m_session));
    {
        SessionStore::MTGuard guard(m_session_store.reap_lock());
        // check if this session perhaps is in ready for reaping set
        m_session_store.unmark_session_for_reaping(&session);
    }
    session.is_connected_to_map_server_id = ev->m_data.m_server_id;
    session.is_connected_to_map_instance_id = ev->m_data.m_sub_server_id;
}
void GameHandler::on_client_disconnected_from_other_server(ClientDisconnectedMessage *ev)
{
    GameSession &session(m_session_store.session_from_token(ev->m_data.m_session));
    session.is_connected_to_map_server_id = 0;
    session.is_connected_to_map_instance_id = 0;
    {
        SessionStore::MTGuard guard(m_session_store.reap_lock());
        m_session_store.mark_session_for_reaping(&session,ev->m_data.m_session);
    }
}
void GameHandler::reap_stale_links()
{
    SessionStore::MTGuard guard(m_session_store.reap_lock());
    EventProcessor *            tgt      = HandlerLocator::getAuth_Handler();
    m_session_store.reap_stale_links("GameInstance", link_is_stale_if_disconnected_for,
                                     [tgt](uint64_t tok) {
                                         tgt->putq(new ClientDisconnectedMessage({tok}));
                                     });
}
