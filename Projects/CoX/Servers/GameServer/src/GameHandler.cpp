#include "GameHandler.h"
#include "GameEvents.h"
#include "AdminServerInterface.h"
#include "ServerManager.h"
#include "GameLink.h"
#include "GameEvents.h"
#include "CharacterClient.h"
#include "GameServer.h"
#include "Character.h"
static const u32 supported_version=20040422;

void GameHandler::dispatch( SEGSEvent *ev )
{
    assert(ev);
    ACE_DEBUG((LM_WARNING,ACE_TEXT("GameHandler link event %d\n"),ev->type()));
        switch(ev->type())
        {
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
    case Internal_EventTypes::evExpectClient:
        on_expect_client(static_cast<ExpectClient *>(ev));
        break;
        case Internal_EventTypes::evClientExpected:
                on_client_expected(static_cast<ClientExpected *>(ev));
                break;
        case SEGS_EventTypes::evConnect:
        break;
    case SEGS_EventTypes::evDisconnect:
        break;
        default:
                assert(!"Unknown event encountered in dispatch.");
        }
}
SEGSEvent * GameHandler::dispatch_sync( SEGSEvent *ev )
{
    switch(ev->type())
    {
    case Internal_EventTypes::evClientConnectionQuery:
            SEGSEvent *r=on_connection_query((ClientConnectionQuery *)ev);
            ev->release();
            return r;
    }
    return 0;
}

void GameHandler::on_connection_request(ConnectRequest *ev)
{
    // TODO: disallow connects if server is overloaded
    ev->src()->putq(new ConnectResponse);
}
void GameHandler::on_update_server(UpdateServer *ev)
{
    CharacterClient *cl=m_clients.getExpectedByCookie(ev->authCookie);
    if(cl==0)
    {
        ev->src()->putq(new GameEntryError(this,"Unauthorized !"));
        return;
    }
    if(ev->m_build_date!=supported_version)
    {
        ev->src()->putq(new GameEntryError(this,"We are very sorry but your client version is not supported."));
        return;
    }
    if(!cl->getCharsFromDb())
    {
        ev->src()->putq(new GameEntryError(this,"DB error encountered!"));
        return;
    }
    m_clients.connectedClient(ev->authCookie);
    ((GameLink *)ev->src())->client_data(cl); // store client object in link
        cl->link_state().link((GameLink *)ev->src()); // store link in client
    CharacterSlots *slots_event=new CharacterSlots;
    slots_event->set_client(cl); // at this point pointer to the client is held in event, if it's destroyed somewhere else KA-BOOM!!
    ev->src()->putq(slots_event);
}
void GameHandler::on_update_character(UpdateCharacter *ev)
{
    GameLink * lnk = (GameLink *)ev->src();
    CharacterClient *client = lnk->client_data();
    assert(client);
    ev->src()->putq(new CharacterResponse(this,ev->m_index,client));
}
void GameHandler::on_idle(IdleEvent *ev)
{
    // idle for idle 'strategy'
    GameLink * lnk = (GameLink *)ev->src();
    lnk->putq(new IdleEvent);
}
void GameHandler::on_disconnect(DisconnectRequest *ev)
{
    GameLink * lnk = (GameLink *)ev->src();
    CharacterClient *client = lnk->client_data();
    if(client)
    {
        lnk->client_data(0);
        m_clients.removeById(client->account_info().account_server_id());
    }
    lnk->putq(new DisconnectResponse);
    lnk->putq(new DisconnectEvent(this)); // this should work, event if different threads try to do it in parallel
}
void GameHandler::on_delete_character(DeleteCharacter *ev)
{
    GameLink * lnk = (GameLink *)ev->src();
    CharacterClient *client = lnk->client_data();
    Character *chr = client->getCharacter(ev->m_index);
    // check if character exists, and if it's name is the same as the one passed here
    if(chr && chr->getName().compare(ev->m_char_name)==0)
    {
        // if it is delete the character ( or maybe not, just mark it deleted ? )
        client->deleteCharacter(chr);
        lnk->putq(new DeletionAcknowledged);
    }
    else
    {
        lnk->putq(new GameEntryError(this,"Given name was not the same as character name\n. Character was not deleted."));
    }
}
void GameHandler::on_client_expected(ClientExpected *ev)
{
    // this is the case when we cannot use ev->src(), because it is not GameLink, but a MapHandler
    // we need to get a link from client_id
        CharacterClient *cl=m_clients.getById(ev->client_id);
        GameLink *lnk = (GameLink *)cl->link_state().link();
        MapServerAddrResponse *r_ev=new MapServerAddrResponse;
        r_ev->m_map_cookie  = ev->cookie;
        r_ev->m_address		= ev->m_connection_addr;
        lnk->putq(r_ev);

}
void GameHandler::on_map_req(MapServerAddrRequest *ev)
{
    GameLink * lnk = (GameLink *)ev->src();
    CharacterClient *client = lnk->client_data();

    if(!client)
        return; // TODO:  return some kind of error.

    if(ServerManager::instance()->MapServerCount()<=0)
    {
        lnk->putq(new GameEntryError(this,"There are no available Map Servers."));
        return;
    }
    EventProcessor *map_handler=ServerManager::instance()->GetMapServer(0)->event_target(); //TODO: this should handle multiple map servers
    AccountInfo &acc_inf(client->account_info());
        map_handler->putq(new ExpectMapClient(this,acc_inf.account_server_id(),acc_inf.access_level(),lnk->peer_addr(),ev->m_char_name,ev->m_mapnumber));
}
void GameHandler::on_unknown_link_event(GameUnknownRequest *)
{
        ACE_DEBUG((LM_WARNING,ACE_TEXT("Unknown GameHandler link event.\n")));
}

// This method is called by authentication service, to notify this GameServer that a client
// with given source ip/port,id and access_level has just logged in.
// If given client is not already logged in
//		This method will create a new CharacterClient object, put it in m_expected_clients collection, and return a key (u32)
//		that will be used by the client during connection
//		Also this will set m_expected_clients cleaning timer if it isn't set already
// If given client is logged in ( it can be found here, or any other GameServer )
//
// In return caller gets an unique client identifier. which is used later on to retrieve appropriate
// client object
void GameHandler::on_expect_client( ExpectClient *ev )
{
    u32 cookie = m_clients.ExpectClient(ev->m_from_addr,ev->m_client_id,ev->m_access_level);
    // let the client object know how can it access database
    m_clients.getExpectedByCookie(cookie)->setServer(this->m_server);
        ev->src()->putq(new ClientExpected(this,ev->m_client_id,cookie,m_server->getAddress()));
}
void GameHandler::checkClientConnection(u64 id)
{
    CharacterClient *client = m_clients.getById(id);
    if(client)
    {
        client->link_state().getState();
    }
    GameLink * lnk = (GameLink *)client->link_state().link();
    ACE_Time_Value inactive=lnk->inactivity_time();
    //	m_clients.getById(id)->;
    // empty for now, later on it will use client store to get the client, and then check it's packet backlog
    //
}
bool GameHandler::isClientConnected(u64 id)
{
    return m_clients.getById(id)!=NULL;
}
void GameHandler::disconnectClient( AccountInfo & cl )
{
    m_clients.removeById(cl.account_server_id()); // we're storing clients by their account server ids
}


SEGSEvent *GameHandler::on_connection_query(ClientConnectionQuery *ev)
{
    CharacterClient *cl=m_clients.getById(ev->m_id);
    if(cl==0)
        return new ClientConnectionResponse(this,ACE_Time_Value::max_time);
    // Client was not active for at least 15s. Warning this must check also map link!
    if(((GameLink *)cl->link_state().link())->inactivity_time()>ACE_Time_Value(15,0))
        disconnectClient(cl->account_info());
    return new ClientConnectionResponse(this,ACE_OS::gettimeofday());

}
