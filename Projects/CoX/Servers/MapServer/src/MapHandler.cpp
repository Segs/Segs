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

MapCommHandler::MapCommHandler()
{
}


void MapCommHandler::dispatch(SEGSEvent *ev)
{
    ACE_ASSERT(ev);
    switch(ev->type())
    {
    case SEGSEvent::evTimeout:
        on_timeout(static_cast<TimerEvent *>(ev));
        break;
    case Internal_EventTypes::evExpectClient:
        on_expect_client(static_cast<ExpectMapClient *>(ev));
        break;
    case MapEventTypes::evIdle:
        on_idle((IdleEvent<MapLinkEvent> *)ev);
        break;
    case MapEventTypes::evDisconnectRequest:
        on_disconnect((DisconnectRequest<MapLinkEvent> *)ev);
        break;
    case MapEventTypes::evConnectRequest:
        on_connection_request((ConnectRequest<MapLinkEvent> *)ev);
        break;
    case MapEventTypes::evEntityEnteringMap:
        on_create_map_entity(static_cast<NewEntity *>(ev));
        break;
    case MapEventTypes::evShortcutsRequest:
        on_shortcuts_request(static_cast<ShortcutsRequest *>(ev));
        break;
    case MapEventTypes::evSceneRequest:
        on_scene_request(static_cast<SceneRequest *>(ev));
        break;
    case MapEventTypes::evEntitiesRequest:
        on_entities_request(static_cast<EntitiesRequest *>(ev));
        break;
    case MapEventTypes::evUnknownEvent:
        break;
    }
}
void MapCommHandler::on_idle(IdleEvent<MapLinkEvent> *ev)
{
    MapLink * lnk = (MapLink *)ev->src();
    // TODO: put idle sending on timer, which is reset each time some other packet is sent ?
    lnk->putq(new IdleEvent<MapLinkEvent>);
}
void MapCommHandler::on_disconnect(DisconnectRequest<MapLinkEvent> *ev)
{
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *client = (MapClient *)lnk->client_data();
    if(client)
    {
        lnk->client_data(0);
        m_clients.removeById(client->account_info().account_server_id());
    }
    lnk->putq(new DisconnectResponse<MapLinkEvent>);
    lnk->putq(new DisconnectEvent(this)); // this should work, event if different threads try to do it in parallel
}
void MapCommHandler::on_connection_request(ConnectRequest<MapLinkEvent> *ev)
{
    ev->src()->putq(new ConnectResponse<MapLinkEvent>);
}
void MapCommHandler::on_expect_client( ExpectMapClient *ev )
{
    u32 cookie = 0; // name in use
    // TODO: handle contention while creating 2 character with the same from different clients
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
    ACE_ASSERT(cl);
    cl->entity(ev->m_ent);
    cl->link_state().link(lnk);
    if(ev->m_new_character)
    {
        cl->db_create();
//        start_idle_timer(cl);
        //cl->start_idle_timer();
    }
    lnk->client_data((void *)cl);
    lnk->putq(new MapInstanceConnected(this,1,""));
}

void MapCommHandler::on_shortcuts_request(ShortcutsRequest *ev)
{
    // TODO: expend this to properly access the data from :
    // Shortcuts are part of UserData and that should be a part of Client entity which is a part of InstanceData
    // TODO: use the access level and send proper commands
    MapLink * lnk = (MapLink *)ev->src();
    Shortcuts *res=new Shortcuts;
    res->m_client=(MapClient *)lnk->client_data();
    lnk->putq(res);
}
void MapCommHandler::on_scene_request(SceneRequest *ev)
{
    // TODO: Pull this up to CoXMapInstance level
    MapLink * lnk = (MapLink *)ev->src();
    SceneEvent *res=new SceneEvent;
    res->undos_PP=0;
    res->var_14=1;
    res->m_outdoor_map=1;//0;
    res->m_map_number=1;
    res->m_map_desc="maps/City_Zones/City_00_01/City_00_01.txt";
    res->current_map_flags=1; //off 1
    res->unkn1=1;
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%d - %d - %d\n"),res->unkn1,res->undos_PP,res->current_map_flags));
    res->unkn2=1;
    lnk->putq(res);
}
void MapCommHandler::on_entities_request(EntitiesRequest *ev)
{
    //TODO: Pull this up to CoXMapInstance
    // this packet should start the per-client send-world-state-update timer
    // actually I think the best place for this timer would be the map instance.
    // so this method should call MapInstace->initial_update(MapClient *);
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *cl =(MapClient *)lnk->client_data();
//    SEGSTimer tmr;
    // start map timer on this event
//    start_entity_state_update();
}

void MapCommHandler::on_timeout( TimerEvent *ev )
{
    // TODO: This should send 'ping' packets on all client links to which we didn't send anything in the last time quantum
    //
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *cl =(MapClient *)lnk->client_data();
    //TODO: Move timer processing to per-client EventHandler ?
    //1. Find the client that this timer corresponds to.
    //2. Call appropriate method ( keep-alive, Entities update etc .. )
    //3. Maybe use one timer for all links ?
}

SEGSEvent * MapCommHandler::dispatch_sync( SEGSEvent *ev )
{
    ACE_ASSERT(!"NO SYNC HANDLING");
    return 0;
}

