/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup GameServer Projects/CoX/Servers/GameServer
 * @{
 */

#include "GameHandler.h"

#include "Common/Servers/InternalEvents.h"
#include "GameData/chardata_serializers.h"
#include "GameData/entitydata_serializers.h"
#include "GameData/map_definitions.h"
#include "Messages/Game/GameEvents.h"
#include "GameLink.h"
#include "GameServer.h"
#include "GameData/Character.h"
#include "Components/SEGSTimer.h"
#include "Servers/HandlerLocator.h"
#include "Servers/MessageBus.h"
#include "Components/serialization_common.h"
#include "Components/serialization_types.h"
#include "Components/TimeEvent.h"

using namespace SEGSEvents;

static const uint32_t supported_version=20040422;
namespace {
const ACE_Time_Value link_update_interval(0,500*1000);
const ACE_Time_Value service_update_interval(5,0);
const ACE_Time_Value session_reaping_interval(1,0);
const CRUDLink::duration maximum_time_without_packets(1000);
const ACE_Time_Value link_is_stale_if_disconnected_for(5,0);
const constexpr int MinPacketsToAck=5;
}

GameHandler::GameHandler()
{
    m_server = nullptr;
}
GameHandler::~GameHandler() = default;

void GameHandler::start()
{
    m_registered_timers.clear(); // remove all timer instances

    // TODO: This should send 'ping' packets on all client links to which we didn't send
    // anything in the last time quantum
    // 1. Find all links that have inactivity_time() > ping_time && <disconnect_time
    // For each found link
    //   If there is no ping_pending on this link, add a ping event to queue
    // 2. Find all links with inactivity_time() >= disconnect_time
    //   Disconnect given link.

    m_link_checker = addTimer(link_update_interval);
    m_service_status_timer = addTimer(service_update_interval);
    m_session_reaper_timer = addTimer(session_reaping_interval);

    startTimer(m_link_checker,&GameHandler::on_check_links);
    startTimer(m_service_status_timer,[this](const ACE_Time_Value &) {report_service_status();});
    startTimer(m_session_reaper_timer,[this](const ACE_Time_Value &) {reap_stale_links();});
}

void GameHandler::dispatch( Event *ev )
{
    assert(ev);
    switch(ev->type())
    {
    case evDisconnect: // link layer tells us that a link is not responsive/dead
        on_link_lost(ev);
        break;
    // Server <-> Server messages
    case evExpectClientRequest:
        on_expect_client(static_cast<ExpectClientRequest *>(ev));
        break;
    case evExpectMapClientResponse:
        on_client_expected(static_cast<ExpectMapClientResponse *>(ev));
        break;
    case evClientConnectedMessage:
        on_client_connected_to_other_server(static_cast<ClientConnectedMessage *>(ev));
        break;
    case evClientDisconnectedMessage:
        on_client_disconnected_from_other_server(static_cast<ClientDisconnectedMessage *>(ev));
        break;
    case GameEventTypes::evServerReconfigured:
        // reconfiguring service forces status broadcast
        report_service_status();
        break;
    // Client -> Server messages
    case evIdle:
        on_idle(static_cast<Idle*>(ev));
        break;
    case evDisconnectRequest:
        on_disconnect(static_cast<DisconnectRequest *>(ev));
        break;
    case evConnectRequest:
        on_connection_request(static_cast<ConnectRequest *>(ev));
        break;
    case GameEventTypes::evUpdateServer:
        on_update_server(static_cast<UpdateServer *>(ev));
        break;
    case GameEventTypes::evMapServerAddrRequest:
        on_map_req(static_cast<MapServerAddrRequest *>(ev));
        break;
    case GameEventTypes::evDeleteCharacter:
        on_delete_character(static_cast<DeleteCharacter *>(ev));
        break;
    case GameEventTypes::evUpdateCharacter:
        on_update_character(static_cast<UpdateCharacter *>(ev));
        break;
    case evUnknownEvent:
        on_unknown_link_event(static_cast<UnknownEvent *>(ev));
        break;
    // DB -> Server messages
    case evGameDbErrorMessage:
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

    tgt->putq(new ClientConnectedMessage({ev->session_token(),m_server->getId(),
                                          ev->m_data.m_game_server_acc_id, ev->m_data.m_game_server_acc_id},0));

    m_session_store.add_to_active_sessions(&session);
    //Create character objects.

    CharacterSlots *slots_event=new CharacterSlots();
    slots_event->m_data = std::move(ev->m_data);
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
    m_session_store.locked_mark_session_for_reaping(&session,expecting_session_token,"GameHander: Awaiting DB response");
    // if things work ok, than GameHandler::on_account_data will get all it needs.
}

void GameHandler::on_update_character(UpdateCharacter *ev)
{
    auto lnk = (GameLink *)ev->src();
    assert(lnk->session_token());
    GameSession &session = m_session_store.session_from_event(ev);
    assert(session.m_game_account.valid());

    ev->src()->putq(new CharacterResponse(this,ev->m_index, session.m_game_account));
}

void GameHandler::on_idle(Idle * /*ev*/)
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
            client_link->putq(new Idle); // Threading trouble, last_sent_packets will not get updated until the packet is actually sent.
        else if(client_link->client_packets_waiting_for_ack()>MinPacketsToAck)
            client_link->putq(new Idle); // Threading trouble, last_sent_packets will not get updated until the packet is actually sent.
    }
}

void GameHandler::report_service_status()
{
    postGlobalEvent(new GameServerStatusMessage({m_server->getAddress(),QDateTime::currentDateTime(),
                                                 uint16_t(m_session_store.num_sessions()),
                                                 m_server->getMaxPlayers(),m_server->getId(),true},0));
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
            m_session_store.mark_session_for_reaping(&session,lnk->session_token(),"GameHander: Disconnect/Exiting map");
            m_session_store.session_link_lost(lnk->session_token(),"GameHandler: exited while joining map");
        }
        else
        {
            EventProcessor * tgt = HandlerLocator::getAuth_Handler();
            tgt->putq(
                new ClientDisconnectedMessage({lnk->session_token(), session.m_game_account.m_game_server_acc_id}, 0));
            m_session_store.session_link_lost(lnk->session_token(),"GameHandler: exited to login screen");
            m_session_store.remove_by_token(lnk->session_token(), session.auth_id());
        }
    }
    else
        m_session_store.session_link_lost(lnk->session_token(),"GameHandler: disconnected, but in a map");
    lnk->putq(new DisconnectResponse);
    // Post disconnect event to link, will close it's processing loop, after it sends the response
    lnk->putq(
        new Disconnect(lnk->session_token())); // this should work, event if different threads try to do it in parallel
}

void GameHandler::on_link_lost(Event *ev)
{
    GameLink * lnk = (GameLink *)ev->src();
    GameSession &session = m_session_store.session_from_event(ev);

    if(session.is_connected_to_map_server_id==0)
    {
        if(session.m_direction==GameSession::EXITING_TO_MAP)
        {
            SessionStore::MTGuard guard(m_session_store.reap_lock());
            m_session_store.mark_session_for_reaping(&session,lnk->session_token(),"GameHander: LinkLost/Exiting map");
            m_session_store.session_link_lost(lnk->session_token(),"GameHandler: link lost while joining map");
        }
        else
        {
            EventProcessor * tgt = HandlerLocator::getAuth_Handler();
            tgt->putq(new ClientDisconnectedMessage({lnk->session_token(), session.m_game_account.m_game_server_acc_id}, 0));
            m_session_store.session_link_lost(lnk->session_token(),"GameHandler: link lost on the way to login screen");
            m_session_store.remove_by_token(lnk->session_token(), session.auth_id());
        }
    }
    else
        m_session_store.session_link_lost(lnk->session_token(),"GameHandler: link lost, but in a map");
    // Post disconnect event to link, will close it's processing loop
    lnk->putq(new Disconnect(lnk->session_token()));
}

void GameHandler::on_character_deleted(RemoveCharacterResponse *ev)
{
    GameSession &session = m_session_store.session_from_event(ev);
    GameAccountResponseCharacterData& selected_slot = session.m_game_account.get_character(ev->m_data.slot_idx);
    selected_slot.reset();
    session.link()->putq(new DeleteAcknowledged);

    // change all emails where sender_id or recipient_id == char_id to 0
    EventProcessor *game_db = HandlerLocator::getGame_DB_Handler(m_server->getId());
    game_db->putq(new EmailUpdateOnCharDeleteMessage({selected_slot.m_db_id}, uint64_t(1)));

    // game_db->putq()
}

void GameHandler::serialize_from(std::istream &/*is*/)
{
    assert(false);
}

void GameHandler::serialize_to(std::ostream &/*is*/)
{
    assert(false);
}

void GameHandler::on_delete_character(DeleteCharacter *ev)
{
    EventProcessor *game_db = HandlerLocator::getGame_DB_Handler(m_server->getId());
    GameSession &session = m_session_store.session_from_event(ev);
    GameLink * lnk = (GameLink *)ev->src();
    const GameAccountResponseCharacterData& selected_slot = session.m_game_account.get_character(ev->m_index);

    // check if character exists, and if it's name is the same as the one passed here
    if(selected_slot.m_name == ev->m_char_name)
    {
        game_db->putq(new RemoveCharacterRequest({session.m_game_account.m_game_server_acc_id, selected_slot.m_slot_idx},
                                                 lnk->session_token(), this));
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
    if(!session.m_game_account.valid())
        return; // TODO:  return some kind of error.

    GameAccountResponseCharacterData *selected_slot = &session.m_game_account.get_character(ev->m_character_index);
    EntityData ed;
    try
    {
        serializeFromQString(ed,selected_slot->m_serialized_entity_data);
    }
    catch(cereal::RapidJSONException &e)
    {
        qWarning() << e.what();
    }
    catch(std::exception &e)
    {
        qCritical() << e.what();
    }

    // will never be empty because we look based on m_map_idx, and integers cannot be null
    QString map_path = getMapPath(ed.m_map_idx).toLower();

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
        ACE_ASSERT(selected_slot->m_name == ev->m_char_name || !"Server-Client character synchronizatigon failure!");
    }
    QString chardata;
    if(selected_slot)
        serializeToQString(*selected_slot,chardata);
    ExpectMapClientRequest *expect_client =
        new ExpectMapClientRequest({session.m_auth_account_id, session.m_access_level, lnk->peer_addr(),
                                    chardata,
                                    ev->m_character_index, ev->m_char_name, map_path,
                                    uint16_t(session.m_game_account.m_max_slots)},
                                   lnk->session_token(),this);
    qInfo("Telling map server to expect a client with character %s, %d\n", qPrintable(ev->m_char_name),
            ev->m_character_index);

    session.m_direction = GameSession::EXITING_TO_MAP;
    map_handler->putq(expect_client);
}

void GameHandler::on_unknown_link_event(UnknownEvent *)
{
        qWarning() << "Unknown GameHandler link event";
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
    // remove this session from 'ready for reaping set'
    m_session_store.locked_unmark_session_for_reaping(&session);

    session.is_connected_to_map_server_id = ev->m_data.m_server_id;
    session.is_connected_to_map_instance_id = ev->m_data.m_sub_server_id;

    postGlobalEvent(new ClientConnectedMessage(
    {ev->m_data.m_session, ev->m_data.m_server_id, ev->m_data.m_sub_server_id, ev->m_data.m_char_db_id}, 0));
}

void GameHandler::on_client_disconnected_from_other_server(ClientDisconnectedMessage *ev)
{
    GameSession &session(m_session_store.session_from_token(ev->m_data.m_session));
    session.is_connected_to_map_server_id = 0;
    session.is_connected_to_map_instance_id = 0;
    m_session_store.locked_mark_session_for_reaping(&session, ev->m_data.m_session,
                                                    "GameHandler: disconnected from child server");

    postGlobalEvent(new ClientDisconnectedMessage({ev->m_data.m_session, ev->m_data.m_char_db_id}, 0));
}

void GameHandler::reap_stale_links()
{
    SessionStore::MTGuard guard(m_session_store.reap_lock());
    EventProcessor *            tgt      = HandlerLocator::getAuth_Handler();

    // Putting m_db_id as 0 to remove wmissing warning
    m_session_store.reap_stale_links("GameInstance", link_is_stale_if_disconnected_for,
                                     [tgt](uint64_t tok) {
                                         tgt->putq(new ClientDisconnectedMessage({tok, 0},0));
                                     });
}
//! @}
