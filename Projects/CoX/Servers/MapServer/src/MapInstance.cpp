/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

#include "MapInstance.h"
#include "MapEvents.h"
#include "MapClient.h"
#include "SEGSTimer.h"
using namespace std;
MapInstance::MapInstance( const string &name ) :m_name(name)
{

}

void MapInstance::dispatch( SEGSEvent *ev )
{
    ACE_ASSERT(ev);
    switch(ev->type())
    {
    case SEGSEvent::evTimeout:
        on_timeout(static_cast<TimerEvent *>(ev));
        break;
    case MapEventTypes::evSceneRequest:
        on_scene_request(static_cast<SceneRequest *>(ev));
        break;
    case MapEventTypes::evEntitiesRequest:
        on_entities_request(static_cast<EntitiesRequest *>(ev));
        break;
    }
}

SEGSEvent * MapInstance::dispatch_sync( SEGSEvent * )
{
    ACE_ASSERT(!"No sync dipatchable events here");
    return 0;
}
void MapInstance::on_scene_request(SceneRequest *ev)
{
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
void MapInstance::on_entities_request(EntitiesRequest *ev)
{
    // this packet should start the per-client send-world-state-update timer
    // actually I think the best place for this timer would be the map instance.
    // so this method should call MapInstace->initial_update(MapClient *);
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *cl =(MapClient *)lnk->client_data();
    EntitiesResponse *res=new EntitiesResponse(cl,false); // initial world update -> current state
    m_clients.push_back(cl); // add to the list of clients interested in world updates
    if(m_world_update_timer==0)
    {
        m_world_update_timer = new SEGSTimer(this,0,ACE_Time_Value(0,100),false); // repeatable timer
    }
    (void)cl; //TODO: actually use the MapClient instance
    //    SEGSTimer tmr;
    // start map timer on this event
    //    start_entity_state_update();
    lnk->putq(res);
}
//! Handle instance-wide timers
void MapInstance::on_timeout(TimerEvent *ev)
{
    MapClient *cl;
    vector<MapClient *>::iterator iter=m_clients.begin();
    vector<MapClient *>::iterator end=m_clients.end();
    for(;iter!=end; ++iter)
    {
        cl=*iter;
        EntitiesResponse *res=new EntitiesResponse(cl,true); // incremental world update = op 2
        cl->link()->putq(res);
    }
    // This is handling instance-wide timers

    // simulation_engine->tick()
    //TODO: Move timer processing to per-client EventHandler ?
    //1. Find the client that this timer corresponds to.
    //2. Call appropriate method ( keep-alive, Entities update etc .. )
    //3. Maybe use one timer for all links ?
}
