#include "GameHandler.h"

#include "GameEvents.h"
#include "Servers/HandlerLocator.h"
#include "AdminServerInterface.h"
#include "CharacterDatabase.h"
#include "ServerManager.h"
#include "GameLink.h"
#include "GameEvents.h"
#include "GameServer.h"
#include "Character.h"
#include "SEGSTimer.h"

static const uint32_t supported_version=20040422;
namespace {
enum {
    Link_Idle_Timer   = 1
};
const ACE_Time_Value link_update_interval(0,500*1000);
const ACE_Time_Value maximum_time_without_packets(0,1000*1000);
const constexpr int MinPacketsToAck=5;
}

void GameHandler::start() {
    ACE_ASSERT(m_link_checker==nullptr);
    m_link_checker = new SEGSTimer(this,(void *)Link_Idle_Timer,link_update_interval,false);
}

void GameHandler::dispatch( SEGSEvent *ev )
{
    assert(ev);
    switch(ev->type())
    {
    case SEGS_EventTypes::evTimeout:
        on_timeout(static_cast<TimerEvent *>(ev));
        break;
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
    // Server <-> Server messages
    case Internal_EventTypes::evExpectClientRequest:
        on_expect_client(static_cast<ExpectClientRequest *>(ev));
        break;
    case Internal_EventTypes::evExpectMapClientResponse:
        on_client_expected(static_cast<ExpectMapClientResponse *>(ev));
        break;
    case SEGS_EventTypes::evConnect:
        break;
    case SEGS_EventTypes::evDisconnect: // link layer tells us that a link is not responsive/dead
        on_link_lost(ev);
        break;
    default:
        assert(!"Unknown event encountered in dispatch.");
    }
}
SEGSEvent * GameHandler::dispatchSync( SEGSEvent *ev )
{
    switch(ev->type())
    {
    case Internal_EventTypes::evClientConnectionRequest:
            SEGSEvent *r=on_connection_query((ClientConnectionRequest *)ev);
            ev->release();
            return r;
    }
    return nullptr;
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
    uint64_t expecting_ssession_token = m_session_store.connectedClient(ev->authCookie);
    if(expecting_ssession_token==~0U)
    {
        ev->src()->putq(new GameEntryError(this,"Unauthorized !"));
        qDebug("GameEntryError: Unauthorized!");
        return;
    }
    GameSession &session(m_session_store.sessionFromToken(expecting_ssession_token));
    session.m_link = (GameLink *)ev->src();
    session.m_link->session_token(expecting_ssession_token);
    if(!fillGameAccountData(session.m_account.m_acc_server_acc_id,session.m_game_account))
    {
        ev->src()->putq(new GameEntryError(this,"DB error encountered!"));
        qDebug("GameEntryError: getCharsFromDb error!");
        return;
    }
    m_session_store.addToActiveSessions(&session);
    CharacterSlots *slots_event=new CharacterSlots;
    slots_event->set_account_data(&session.m_game_account);
    ev->src()->putq(slots_event);
}
void GameHandler::on_update_character(UpdateCharacter *ev)
{
    auto lnk = (GameLink *)ev->src();
    assert(lnk->session_token());
    GameSession &session = m_session_store.sessionFromEvent(ev);
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
        GameLink * client_link = entry->m_link;
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
    }
}
void GameHandler::on_disconnect(DisconnectRequest *ev)
{
    GameSession &session = m_session_store.sessionFromEvent(ev);
    GameLink * lnk = (GameLink *)ev->src();
    //TODO: removing session from the GameHandler's store, the client can be connected to MapServer
    // consider not removing session here, should help with map hand-over ?
    m_session_store.removeByToken(lnk->session_token());
    lnk->putq(new DisconnectResponse);
    // Post disconnect event to link, will close it's processing loop, after it sends the response
    lnk->putq(new DisconnectEvent(this)); // this should work, event if different threads try to do it in parallel
}
void GameHandler::on_link_lost(SEGSEvent *ev)
{
    GameSession &session = m_session_store.sessionFromEvent(ev);
    GameLink * lnk = (GameLink *)ev->src();
    //TODO: removing session from the GameHandler's store, the client can be connected to MapServer
    // consider not removing session here, should help with map hand-over ?
    m_session_store.removeByToken(lnk->session_token());
    // Post disconnect event to link, will close it's processing loop
    lnk->putq(new DisconnectEvent(this));
}
void GameHandler::on_delete_character(DeleteCharacter *ev)
{
    GameSession &session = m_session_store.sessionFromEvent(ev);
    GameLink * lnk = (GameLink *)ev->src();
    auto chr(session.m_game_account.get_character(ev->m_index));
    // check if character exists, and if it's name is the same as the one passed here
    if(chr.m_name.compare(ev->m_char_name)==0)
    {
        // if it is delete the character ( or maybe not, just mark it deleted ? )
        bool res = removeCharacter(session.m_game_account.m_game_server_acc_id,chr.index);
        if(res)
            chr.reset();
        lnk->putq(new DeletionAcknowledged);
    }
    else
    {
        lnk->putq(new GameEntryError(this,"Given name was not the same as character name\n. Character was not deleted."));
    }
}
void GameHandler::on_client_expected(ExpectMapClientResponse *ev)
{
    // this is the case when we cannot use ev->src(), because it is not GameLink, but a MapHandler
    // we need to get a link based on the session token
    GameSession &session = m_session_store.sessionFromEvent(ev);
    GameLink *lnk = session.m_link;
    MapServerAddrResponse *r_ev=new MapServerAddrResponse;
    r_ev->m_map_cookie  = ev->m_data.cookie;
    r_ev->m_address     = ev->m_data.m_connection_addr;
    lnk->putq(r_ev);

}
void GameHandler::on_map_req(MapServerAddrRequest *ev)
{
    GameLink * lnk = (GameLink *)ev->src();
    GameSession &session = m_session_store.sessionFromEvent(ev);
    if (!session.m_game_account.valid())
        return; // TODO:  return some kind of error.

    GameAccountResponseCharacterData *selected_slot = &session.m_game_account.get_character(ev->m_character_index);
    if(selected_slot->isEmpty())
        selected_slot = nullptr; // passing a null to map server to indicate a new character is being created.
    if(ServerManager::instance()->MapServerCount()<=0)
    {
        lnk->putq(new GameEntryError(this,"There are no available Map Servers."));
        return;
    }
    //TODO: this should handle multiple map servers, for now it doesn't care and always connects to the first one.
    EventProcessor *map_handler=ServerManager::instance()->GetMapServer(0)->event_target();
    AuthAccountData &acc_inf(session.m_account);
    if(selected_slot )
    {
        ACE_ASSERT(selected_slot->m_name == ev->m_char_name || !"Server-Client character synchronization failure!");
    }
    ExpectMapClientRequest *expect_client =
        new ExpectMapClientRequest({acc_inf.m_acc_server_acc_id, acc_inf.m_access_level, lnk->peer_addr(),
                                    selected_slot, ev->m_character_index, ev->m_char_name, ev->m_mapnumber},
                                   lnk->session_token());
    fprintf(stderr, " Telling map server to expect a client with character %s,%d\n", qPrintable(ev->m_char_name),
            ev->m_character_index);
    map_handler->putq(expect_client);
}
void GameHandler::on_unknown_link_event(GameUnknownRequest *)
{
        ACE_DEBUG((LM_WARNING,ACE_TEXT("Unknown GameHandler link event.\n")));
}

// This method is called by authentication service, to notify this GameServer that a client
// with given source ip/port,id and access_level has just logged in.
// If given client is not already logged in
//  This method will create a new CharacterClient object, put it in m_expected_clients collection, and return a key (uint32_t)
//  that will be used by the client during connection
//  Also this will set m_expected_clients cleaning timer if it isn't set already
// If given client is logged in ( it can be found here, or any other GameServer )
//
// In return caller gets an unique client identifier. which is used later on to retrieve appropriate
// client object
void GameHandler::on_expect_client( ExpectClientRequest *ev )
{
    GameSession &sess = m_session_store.createSession(ev->session_token());
    uint32_t cookie = m_session_store.ExpectClientSession(ev->session_token(),ev->m_data.m_from_addr,ev->m_data.m_client_id);
    sess.m_state = GameSession::CLIENT_EXPECTED;
    sess.m_account.m_acc_server_acc_id = ev->m_data.m_client_id;
    HandlerLocator::getAuth_Handler()->putq(new ExpectClientResponse({ev->m_data.m_client_id,cookie,m_server->getId()},ev->session_token()));
}
SEGSEvent *GameHandler::on_connection_query(ClientConnectionRequest *ev)
{
    if (!m_session_store.hasSessionFor(ev->session_token()))
    {
        // no session here
        qDebug() << __FUNCTION__ << "No session in store for" << ev->session_token();
        return new ClientConnectionResponse({ACE_Time_Value::max_time}, ev->session_token());
    }
    GameSession &session(m_session_store.sessionFromEvent(ev));


    if (session.m_link == nullptr)
{
        qDebug() << __FUNCTION__ << "No active link set in session for" << ev->session_token();
        return new ClientConnectionResponse({ACE_Time_Value::max_time},ev->session_token());
    }
    // Client was not active for at least 15s. Warning this must check also map link!
    if (session.m_link->client_last_seen_packets() > ACE_Time_Value(15, 0))
    {
        // we assume the session was either handed over to
        m_session_store.removeFromActiveSessions(&session);
    }
    return new ClientConnectionResponse({session.m_link->client_last_seen_packets()}, ev->session_token());

}
