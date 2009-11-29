/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: MapHandler.cpp 319 2007-01-26 17:03:18Z nemerle $
 */

#include "MapHandler.h"
#include "MapClient.h"
#include "ServerManager.h"
#include "MapServer.h"
#include "CoXMap.h"
#include "Entity.h"
#include "CoXMap.h"

MapCommHandler::MapCommHandler()
{
    m_handled_worlds[0]=new CoXMap("City_00_01");
    m_handled_worlds[0]->activate(THR_NEW_LWP|THR_JOINABLE|THR_INHERIT_SCHED,1);
}


void MapCommHandler::dispatch(SEGSEvent *ev)
{
	ACE_ASSERT(ev);
	switch(ev->type())
	{
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
        on_create_map_entity((NewEntity *)ev);
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
    if(m_handled_worlds.find(ev->m_map_id)==m_handled_worlds.end())
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
        cl->current_map(m_handled_worlds[ev->m_map_id]);
    }
	ev->src()->putq(new ClientExpected(this,ev->m_client_id,cookie,m_server->getAddress()));
}
void MapCommHandler::on_create_map_entity(NewEntity *ev)
{
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *cl = m_clients.getExpectedByCookie(ev->m_cookie-2);
    ACE_ASSERT(cl);
    cl->entity(ev->m_ent);
    cl->link_state().link(lnk);    
    if(ev->m_new_character)
    {
        cl->db_create();
    }
    lnk->client_data((void *)cl);
    lnk->putq(new MapInstanceConnected(this,1,""));
}

void MapCommHandler::on_shortcuts_request(ShortcutsRequest *ev)
{
    MapLink * lnk = (MapLink *)ev->src();
    Shortcuts *res=new Shortcuts;
    res->m_client=(MapClient *)lnk->client_data();
    // TODO: use the access level and send proper commands
    lnk->putq(res);
}
void MapCommHandler::on_scene_request(SceneRequest *ev)
{
    MapLink * lnk = (MapLink *)ev->src();
    Scene *res=new Scene;
    res->undos_PP=0;
	res->var_14=1;
	res->m_outdoor_map=1;//0;
	res->m_map_number=1;
	res->m_map_desc="maps/City_Zones/City_00_01/City_00_01.txt";
	res->current_map_flags=1; //off 1
	Vector3 positions[]=
	{
		Vector3(0,0,0),Vector3(0,0,0),Vector3(0,0,0),Vector3(0,0,0),
		Vector3(0,0,0),Vector3(0,0,0),Vector3(0,0,0),Vector3(0,0,0),
		Vector3(0,0,0),Vector3(0,0,0),Vector3(0,0,0),Vector3(0,0,0),
		Vector3(0,0,0),Vector3(0,0,0),
	};
	string names[]={
		"grpa2228","grp2544",
		"object_library/city_templates/map_2d_blackpoly/map_city_00_01","grp2555",
		"grp2561","grp2575",
		"grp2578","grp2582",
		"grpsound2583","grpsound2584",
		"grp2585","grp2586",
		"grp2587","grp2588",
	};
	/*
	for(int i=0; i<13; i++)
	res->m_refs.push_back(MapRef(ids[i],names[i],positions[i],Vector3(0.0,0.0,0.0)));
	res->m_crc.resize(res->m_trays.size());
	Matrix4x3 mat;
	for(size_t j=0; j<sizeof(Matrix4x3)/4; j++)
	{
		((float *)&mat.row1)[j]=0.0f;
	}
	mat.row1.vals.x = mat.row2.vals.y = mat.row3.vals.z= 1.0;
	res->ref_crc=0;
	res->ref_count=11;
	*/
	res->unkn1=1;
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%d - %d - %d\n"),res->unkn1,res->undos_PP,res->current_map_flags));
	res->unkn2=1;
    lnk->putq(res);
}
void MapCommHandler::on_entities_request(EntitiesRequest *ev)
{
    // start map timer on this event
}

SEGSEvent * MapCommHandler::dispatch_sync( SEGSEvent *ev )
{
    ACE_ASSERT(!"NO SYNC HANDLING");
    return 0;
}
