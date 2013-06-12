/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *

 */
#include "MapInstance.h"
#include "version.h"
#include "MapEvents.h"
#include "MapClient.h"
#include "SEGSTimer.h"
#include "Entity.h"
#include "WorldSimulation.h"
namespace {
enum {
    World_Update_Timer   = 1,
    State_Transmit_Timer = 2
};
ACE_Time_Value world_update_interval(0,33*1000);
ACE_Time_Value resend_interval(0,250*1000);
}

using namespace std;
MapInstance::MapInstance( const string &name ) :m_name(name),m_world_update_timer(nullptr)
{
    m_world = new World(m_entities);
}
void MapInstance::start() {
    assert(m_world_update_timer==nullptr);
    m_world_update_timer = new SEGSTimer(this,(void *)World_Update_Timer,world_update_interval,false); // world simulation ticks
    m_resend_timer = new SEGSTimer(this,(void *)State_Transmit_Timer,resend_interval,false); // state broadcast ticks
}
MapInstance::~MapInstance() {
    delete m_world;
}
void MapInstance::enqueue_client(MapClient *clnt)
{
    m_world->addPlayer(clnt->char_entity());
    //m_queued_clients.push_back(clnt); // enter this client on the waiting list
}
// Here we would add the handler call in case we get evCombineRequest :)
void MapInstance::dispatch( SEGSEvent *ev )
{
    assert(ev);
    switch(ev->type())
    {
        case SEGS_EventTypes::evTimeout:
            on_timeout(static_cast<TimerEvent *>(ev));
            break;
        case MapEventTypes::evSceneRequest:
            on_scene_request(static_cast<SceneRequest *>(ev));
            break;
        case MapEventTypes::evEntitiesRequest:
            on_entities_request(static_cast<EntitiesRequest *>(ev));
            break;
        case MapEventTypes::evInputState:
            on_input_state(static_cast<InputState *>(ev));
            break;
        default:
            fprintf(stderr,"Unhandled event type %d\n",ev->type());
            //ACE_DEBUG ((LM_WARNING,ACE_TEXT ("Unhandled event type %d\n"),ev->type()));
    }
}

SEGSEvent * MapInstance::dispatch_sync( SEGSEvent * )
{
    assert(!"No sync dipatchable events here");
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
    //"maps/City_Zones/City_00_01/City_00_01.txt";
    res->m_map_desc="maps/City_Zones/City_01_01/City_01_01.txt";
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
    srand(time(0));
    MapClient *cl = lnk->client_data();
    // this sends the initial  'world', but without this client

    EntitiesResponse *res=new EntitiesResponse(cl); // initial world update -> current state
    res->is_incremental(false); //redundant
    res->entReceiveUpdate=true; //false;
    res->abs_time = m_world->timecount;
    res->finalize();
    lnk->putq(res);
    m_clients.push_back(cl); // add to the list of clients interested in world updates
}
//! Handle instance-wide timers
void MapInstance::on_timeout(TimerEvent *ev)
{
    int timer_id = (intptr_t)ev->data();
    switch (timer_id) {
        case World_Update_Timer:
            m_world->update(ev->arrival_time());
            break;
        case State_Transmit_Timer:
            sendState();
            break;
    }
}
void MapInstance::sendState() {

    if(m_clients.empty())
        return;
    MapClient *cl;
    vector<MapClient *>::iterator iter=m_clients.begin();
    vector<MapClient *>::iterator end=m_clients.end();
    static bool only_first=true;
    static int resendtxt=0;
    resendtxt++;
    for(;iter!=end; ++iter)
    {
        cl=*iter;
        EntitiesResponse *res=new EntitiesResponse(cl);
        if(cl->char_entity()->m_create==true) {
            res->is_incremental(false); // incremental world update = op 3
        }
        else {
            res->is_incremental(true); // incremental world update = op 2
        }
        res->entReceiveUpdate = true;
        res->abs_time = m_world->timecount;
        res->finalize();
        cl->link()->putq(res);
        if(resendtxt==10){

            std::string welcome_msg = std::string("Welcome to SEGS ") + VersionInfo::getAuthVersion();
            ChatMessage *msg = ChatMessage::adminMessage(welcome_msg.c_str() );
            cl->link()->putq(msg);
        }
        if(cl->char_entity()->m_create==true)
            cl->char_entity()->m_create=false;
    }
    only_first=false;
    if(resendtxt==10)
    {
        resendtxt=0;
    }
    // This is handling instance-wide timers

    //TODO: Move timer processing to per-client EventHandler ?
    //1. Find the client that this timer corresponds to.
    //2. Call appropriate method ( keep-alive, Entities update etc .. )
    //3. Maybe use one timer for all links ?

}
void MapInstance::on_combine_boosts(CombineRequest */*req*/)
{
    //TODO: do something here !
}
void MapInstance::on_input_state(InputState *st)
{
    MapLink * lnk = (MapLink *)st->src();
    MapClient *cl = lnk->client_data();
    Entity *ent = cl->char_entity();
    if(st->m_data.has_input_commit_guess)
        ent->m_input_ack = st->m_data.someOtherbits;
    ent->inp_state=st->m_data;
    //TODO: do something here !
}
