/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

#include "MapHandler.h"
#include "MapClient.h"
#include "ServerManager.h"
#include "MapServer.h"
#include "MapInstance.h"
#include "MapManager.h"
#include "MapTemplate.h"
#include "Entity.h"
#include "SEGSTimer.h"

MapCommHandler::MapCommHandler()
{
}


void MapCommHandler::dispatch(SEGSEvent *ev)
{
    assert(ev);
    switch(ev->type())
    {
    case SEGS_EventTypes::evTimeout:
        on_timeout(static_cast<TimerEvent *>(ev));
        break;
    case Internal_EventTypes::evExpectClient:
        on_expect_client(static_cast<ExpectMapClient *>(ev));
        break;
    case MapEventTypes::evIdle:
        on_idle(static_cast<IdleEvent *>(ev));
        break;
    case MapEventTypes::evDisconnectRequest:
        on_disconnect(static_cast<DisconnectRequest *>(ev));
        break;
    case MapEventTypes::evConnectRequest:
        on_connection_request(static_cast<ConnectRequest *>(ev));
        break;
    case MapEventTypes::evEntityEnteringMap:
        on_create_map_entity(static_cast<NewEntity *>(ev));
        break;
    case MapEventTypes::evShortcutsRequest:
        on_shortcuts_request(static_cast<ShortcutsRequest *>(ev));
        break;
    case MapEventTypes::evSceneRequest:
    case MapEventTypes::evEntitiesRequest:
        on_instance_event(ev);
        break;
    case MapEventTypes::evUnknownEvent:
        break;
    }
}
void MapCommHandler::on_idle(IdleEvent *ev)
{
    MapLink * lnk = (MapLink *)ev->src();
    // TODO: put idle sending on timer, which is reset each time some other packet is sent ?
    lnk->putq(new IdleEvent);
}
void MapCommHandler::on_disconnect(DisconnectRequest *ev)
{
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *client = lnk->client_data();
    if(client)
    {
        lnk->client_data(0);
        m_clients.removeById(client->account_info().account_server_id());
    }
    lnk->putq(new DisconnectResponse);
    lnk->putq(new DisconnectEvent(this)); // this should work, event if different threads try to do it in parallel
}
void MapCommHandler::on_connection_request(ConnectRequest *ev)
{
    ev->src()->putq(new ConnectResponse);
}
void MapCommHandler::on_expect_client( ExpectMapClient *ev )
{
    u32 cookie = 0; // name in use
    // TODO: handle contention while creating 2 character with the same name from different clients
    // TODO: SELECT account_id from characters where name=ev->m_character_name
    MapTemplate *tpl=m_server->map_manager().get_template(ev->m_map_id);
    if(0==tpl)
    {
        cookie = 1;
    }
    else if(true) // check if (character does not exist || character exists and is owned by this client )
    {

        cookie    = 2+m_clients.ExpectClient(ev->m_from_addr,ev->m_client_id,ev->m_access_level);
        // 0 name already taken
        // 1 problem in database system
        MapClient *cl = m_clients.getExpectedByCookie(cookie-2);
        cl->name(ev->m_character_name);
        cl->current_map(tpl->get_instance());
    }
    ev->src()->putq(new ClientExpected(this,ev->m_client_id,cookie,m_server->getAddress()));
}
void MapCommHandler::on_create_map_entity(NewEntity *ev)
{
    //TODO: At this point we should pre-process the NewEntity packet and let the proper CoXMapInstance handle the rest of processing

    MapLink * lnk = (MapLink *)ev->src();
    MapClient *cl = m_clients.getExpectedByCookie(ev->m_cookie-2);

    assert(cl);
    assert(ev->m_ent);

    cl->entity(ev->m_ent);
    cl->link_state().link(lnk);
    if(ev->m_new_character)
    {
        cl->db_create();
//        start_idle_timer(cl);
        //cl->start_idle_timer();
    }
    cl->current_map()->create_entity(cl->char_entity());
    lnk->client_data(cl);
    lnk->putq(new MapInstanceConnected(this,1,""));
}
//! Routing events to their proper client's instance
void MapCommHandler::on_instance_event(SEGSEvent *ev)
{
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *cl =  lnk->client_data();
    assert(0!=cl);
    // forced dispatch
    cl->current_map()->dispatch(ev);
}
void MapCommHandler::on_shortcuts_request(ShortcutsRequest *ev)
{
    // TODO: expend this to properly access the data from :
    // Shortcuts are part of UserData and that should be a part of Client entity which is a part of InstanceData
    // TODO: use the access level and send proper commands
    MapLink * lnk  = (MapLink *)ev->src();
    Shortcuts *res = new Shortcuts;
    res->m_client  = lnk->client_data();
    lnk->putq(res);
}


void MapCommHandler::on_timeout( TimerEvent *ev )
{
    // TODO: This should send 'ping' packets on all client links to which we didn't send
    // anything in the last time quantum
    // 1. Find all links that have inactivity_time() > ping_time && <disconnect_time
    // For each found link
    //   If there is no ping_pending on this link, add a ping event to queue
    // 2. Find all links with inactivity_time() >= disconnect_time
    //   Disconnect given link.
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *cl =lnk->client_data();
}

SEGSEvent * MapCommHandler::dispatch_sync( SEGSEvent * )
{
    assert(!"NO SYNC HANDLING");
    return 0;
}

