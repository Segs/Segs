/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *

 */
#include "MapInstance.h"

#include "AdminServer.h"
#include "AdminServer/AccountInfo.h"
#include "version.h"
#include "DataHelpers.h"
#include "MapEvents.h"
#include "MapClient.h"
#include "MapServerData.h"
#include "MapManager.h"
#include "MapTemplate.h"
#include "MapServer.h"
#include "SEGSTimer.h"
#include "Entity.h"
#include "WorldSimulation.h"
#include "InternalEvents.h"


#include <QtCore/QDebug>


namespace {
enum {
    World_Update_Timer   = 1,
    State_Transmit_Timer = 2
};

ACE_Time_Value world_update_interval(0,1000*1000/WORLD_UPDATE_TICKS_PER_SECOND);
ACE_Time_Value resend_interval(0,250*1000);
}

using namespace std;
MapInstance::MapInstance(const string &name) : m_name(name), m_world_update_timer(nullptr)
{
    m_world = new World(m_entities);
    m_scripting_interface.reset(new ScriptingEngine);
}
void MapInstance::start()
{
    assert(m_world_update_timer==nullptr);
    m_scripting_interface->registerTypes();
    m_world_update_timer = new SEGSTimer(this,(void *)World_Update_Timer,world_update_interval,false); // world simulation ticks
    m_resend_timer = new SEGSTimer(this,(void *)State_Transmit_Timer,resend_interval,false); // state broadcast ticks
}

size_t MapInstance::num_active_clients()
{
    return m_clients.num_active_clients();
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
        case SEGS_EventTypes::evDisconnect:
            on_link_lost(ev);
            break;
        case MapEventTypes::evIdle:
            on_idle(static_cast<IdleEvent *>(ev));
            break;
        case MapEventTypes::evConnectRequest:
            on_connection_request(static_cast<ConnectRequest *>(ev));
            break;
        case MapEventTypes::evSceneRequest:
            on_scene_request(static_cast<SceneRequest *>(ev));
            break;
        case Internal_EventTypes::evExpectClient:
            on_expect_client(static_cast<ExpectMapClient *>(ev));
            break;
        case MapEventTypes::evDisconnectRequest:
            on_disconnect(static_cast<DisconnectRequest *>(ev));
            break;
        case MapEventTypes::evEntityEnteringMap:
            on_create_map_entity(static_cast<NewEntity *>(ev));
            break;
        case MapEventTypes::evClientQuit:
            on_client_quit(static_cast<ClientQuit*>(ev));
            break;
        case MapEventTypes::evEntitiesRequest:
            on_entities_request(static_cast<EntitiesRequest *>(ev));
            break;
        case MapEventTypes::evShortcutsRequest:
            on_shortcuts_request(static_cast<ShortcutsRequest *>(ev));
            break;
        case MapEventTypes::evInputState:
            on_input_state(static_cast<InputState *>(ev));
            break;
        case MapEventTypes::evCookieRequest:
            on_cookie_confirm(static_cast<CookieRequest *>(ev));
            break;
        case MapEventTypes::evEnterDoor:
            on_enter_door(static_cast<EnterDoor *>(ev));
            break;
        case MapEventTypes::evSetDestination:
            on_set_destination(static_cast<SetDestination *>(ev));
            break;
        case MapEventTypes::evWindowState:
            on_window_state(static_cast<WindowState *>(ev));
            break;
        case MapEventTypes::evInspirationDockMode:
            on_inspiration_dockmode(static_cast<InspirationDockMode *>(ev));
            break;
        case MapEventTypes::evAbortQueuedPower:
            on_abort_queued_power(static_cast<AbortQueuedPower *>(ev));
            break;
        case MapEventTypes::evConsoleCommand:
            on_console_command(static_cast<ConsoleCommand *>(ev));
            break;
        case MapEventTypes::evChatDividerMoved:
            on_command_chat_divider_moved(static_cast<ChatDividerMoved *>(ev));
            break;
        case MapEventTypes::evClientResumedRendering:
            on_client_resumed(static_cast<ClientResumedRendering *>(ev));
            break;
        case MapEventTypes::evMiniMapState:
            on_minimap_state(static_cast<MiniMapState *>(ev));
            break;
        case MapEventTypes::evEntityInfoRequest:
            on_entity_info_request(static_cast<EntityInfoRequest *>(ev));
            break;
        case MapEventTypes::evLocationVisited:
            on_location_visited(static_cast<LocationVisited *>(ev));
            break;
        case MapEventTypes::evPlaqueVisited:
            on_plaque_visited(static_cast<PlaqueVisited *>(ev));
            break;
        case MapEventTypes::evClientSettings:
            on_client_settings(static_cast<ClientSettings *>(ev));
            break;
        case MapEventTypes::evDescriptionAndBattleCry:
            on_description_and_battlecry(static_cast<DescriptionAndBattleCry *>(ev));
            break;
        default:
            fprintf(stderr,"Unhandled MapEventTypes %zu\n",ev->type()-MapEventTypes::base);
            //ACE_DEBUG ((LM_WARNING,ACE_TEXT ("Unhandled event type %d\n"),ev->type()));
    }
}

SEGSEvent * MapInstance::dispatch_sync( SEGSEvent * )
{
    assert(!"No sync dipatchable events here");

    return nullptr;
}
void MapInstance::on_idle(IdleEvent *ev)
{
    MapLink * lnk = (MapLink *)ev->src();
    // TODO: put idle sending on timer, which is reset each time some other packet is sent ?
    lnk->putq(new IdleEvent);
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
    MapLink * lnk  = (MapLink *)ev->src();
    Shortcuts *res = new Shortcuts;
    res->m_client  = lnk->client_data();
    lnk->putq(res);
}
void MapInstance::on_client_quit(ClientQuit*ev)
{
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *client = lnk->client_data();
    // process client removal -> sending delete event to all clients etc.
    assert(client && client->char_entity());
    if(ev->abort_disconnect)
        abortLogout(client->char_entity());
    else
        client->char_entity()->beginLogout(10);

}
void MapInstance::on_link_lost(SEGSEvent *ev)
{
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *client = lnk->client_data();
    if(client)
    {
        Entity *ent = client->char_entity();
        assert(ent);
        //todo: notify all clients about entity removal
        m_entities.removeEntityFromActiveList(ent);
        lnk->set_client_data(nullptr);
        m_clients.removeById(client->account_info().account_server_id());
    }
    lnk->putq(new DisconnectEvent(this));
}
void MapInstance::on_disconnect(DisconnectRequest *ev)
{
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *client = lnk->client_data();
    if(client)
    {
        Entity *ent = client->char_entity();
        assert(ent);
        //todo: notify all clients about entity removal
        m_entities.removeEntityFromActiveList(ent);
        lnk->set_client_data(nullptr);
        m_clients.removeById(client->account_info().account_server_id());
    }
    lnk->putq(new DisconnectResponse);
    lnk->putq(new DisconnectEvent(this)); // this should work, event if different threads try to do it in parallel
}
void MapInstance::on_expect_client( ExpectMapClient *ev )
{
    // TODO: handle contention while creating 2 characters with the same name from different clients
    // TODO: SELECT account_id from characters where name=ev->m_character_name
    uint32_t cookie = 0; // name in use
    MapTemplate *tpl=m_server->map_manager().get_template(ev->m_map_id);
    MapClient *cl = nullptr;
    if(nullptr==tpl)
    {
        ev->src()->putq(new ClientExpected(this,ev->m_client_id,1,m_server->getAddress()));
        return;
    }
    CharacterDatabase * char_db = AdminServer::instance()->character_db();
    if(!ev->char_from_db)
    {
        // attempt to create a new character, let's see if the name is taken
        if(char_db->named_character_exists(ev->m_character_name))
        {
            // name is taken, inform by setting cookie to 0.
            ev->src()->putq(new ClientExpected(this,ev->m_client_id,0,m_server->getAddress()));
            return;
        }
    }
    cookie    = 2+m_clients.ExpectClient(ev->m_from_addr,ev->m_client_id,ev->m_access_level);
    cl = m_clients.getExpectedByCookie(cookie-2);
    cl->name(ev->m_character_name);
    cl->current_map(tpl->get_instance());
    if(ev->char_from_db)
    {
        Entity *ent = m_entities.CreatePlayer();
        ent->fillFromCharacter(ev->char_from_db);
        ent->m_origin_idx = getEntityOriginIndex(true, ev->char_from_db->getOrigin());
        ent->m_class_idx = getEntityClassIndex(true, ev->char_from_db->getClass());
        cl->char_entity(ent);
    }
    ev->src()->putq(new ClientExpected(this,ev->m_client_id,cookie,m_server->getAddress()));
    //    else if(true) // check if (character does not exist || character exists and is owned by this client )
    //    {

    //        cookie    = 2+m_clients.ExpectClient(ev->m_from_addr,ev->m_client_id,ev->m_access_level);
    //        MapClient *cl = m_clients.getExpectedByCookie(cookie-2);
    //        cl->name(ev->m_character_name);
    //        cl->current_map(tpl->get_instance());
    //        ev->src()->putq(new ClientExpected(this,ev->m_client_id,cookie,m_server->getAddress()));
    //    }
}
void MapInstance::on_create_map_entity(NewEntity *ev)
{
    //TODO: At this point we should pre-process the NewEntity packet and let the proper CoXMapInstance handle the rest of processing

    MapLink * lnk = (MapLink *)ev->src();
    MapClient *cl = m_clients.getExpectedByCookie(ev->m_cookie-2);

    assert(cl);
    cl->link_state().link(lnk);
    if(ev->m_new_character)
    {
        Entity *e = m_entities.CreatePlayer();
        fillEntityFromNewCharData(*e,ev->m_character_data,g_GlobalMapServer->runtimeData().getPacker());
        e->m_origin_idx = getEntityOriginIndex(true, e->m_char.getOrigin());
        e->m_class_idx = getEntityClassIndex(true, e->m_char.getOrigin());
        cl->entity(e);
        cl->db_create();
        //        start_idle_timer(cl);
        //cl->start_idle_timer();
    }
    assert(cl->char_entity());
    cl->current_map()->enqueue_client(cl);
    lnk->set_client_data(cl);
    lnk->putq(new MapInstanceConnected(this,1,""));
}
void MapInstance::on_scene_request(SceneRequest *ev)
{
    MapLink * lnk = (MapLink *)ev->src();
    SceneEvent *res=new SceneEvent;
    res->undos_PP=0;
    res->var_14=true;
    res->m_outdoor_mission_map=false;
    res->m_map_number=1;
    //"maps/City_Zones/City_00_01/City_00_01.txt";
    res->m_map_desc="maps/City_Zones/City_01_01/City_01_01.txt";
    res->current_map_flags=true; //off 1
    res->unkn1=1;
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%d - %d - %d\n"),res->unkn1,res->undos_PP,res->current_map_flags));
    res->unkn2=true;
    lnk->putq(res);
}
void MapInstance::on_entities_request(EntitiesRequest *ev)
{
    // this packet should start the per-client send-world-state-update timer
    // actually I think the best place for this timer would be the map instance.
    // so this method should call MapInstace->initial_update(MapClient *);
    MapLink * lnk = (MapLink *)ev->src();
    srand(time(nullptr));
    MapClient *cl = lnk->client_data();
    assert(cl);
    m_clients.addToActiveClients(cl); // add to the list of clients interested in world updates
}
//! Handle instance-wide timers
void MapInstance::on_timeout(TimerEvent *ev)
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
        case World_Update_Timer:
            m_world->update(ev->arrival_time());
            break;
        case State_Transmit_Timer:
            sendState();
            break;
    }
}
void MapInstance::sendState() {

    if(num_active_clients()==0)
        return;

    ClientStore<MapClient>::ivClients iter=m_clients.begin();
    ClientStore<MapClient>::ivClients end=m_clients.end();
    static bool only_first=true;
    static int resendtxt=0;
    resendtxt++;

    for(;iter!=end; ++iter)
    {
        MapClient *cl = *iter;
        EntitiesResponse *res=new EntitiesResponse(cl);
        res->m_map_time_of_day = m_world->time_of_day();

        if(cl->m_in_map==false) // send full updates until client `resumes`
        {
            res->is_incremental(false); // full world update = op 3
        }
        else
        {
            res->is_incremental(true); // incremental world update = op 2
        }
        res->ent_major_update = true;
        res->abs_time = 30*100*(m_world->sim_frame_time/1000.0f);
        cl->link()->putq(res);
    }
    only_first=false;
    if(resendtxt==15)
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
    MapLink *  lnk = (MapLink *)st->src();
    MapClient *cl  = lnk->client_data();
    if(cl==nullptr)
    {
        qWarning() << "Bogus on_input_state, client is null";
        return;
    }
    Entity *   ent = cl->char_entity();
    if (st->m_data.has_input_commit_guess)
        ent->m_input_ack = st->m_data.send_id;
    ent->inp_state = st->m_data;

    // Input state messages can be followed by multiple commands.
    assert(st->m_user_commands.GetReadableBits()<32*1024*8); // simple sanity check ?
    // here we will try to extract all of them and put them on our processing queue
    while(st->m_user_commands.GetReadableBits()>1)
    {
        MapLinkEvent *ev = MapEventFactory::CommandEventFromStream(st->m_user_commands);
        if(!ev)
            break;
        ev->serializefrom(st->m_user_commands);
        // copy source packet seq number to created command
        ev->m_seq_number = st->m_seq_number;
        ev->src(st->src());
        // post the event to ourselves for dispatch
        putq(ev);
    }
    if(st->m_user_commands.GetReadableBits()!=0)
        qDebug() << "Not all bits were consumed";

    //TODO: do something here !
}
void MapInstance::on_cookie_confirm(CookieRequest * ev){
    printf("Received cookie confirm %x - %x\n",ev->cookie,ev->console);
}
void MapInstance::on_window_state(WindowState * ev){
    printf("Received window state %d - %d\n",ev->window_idx,ev->wnd.field_24);

}
static bool isChatMessage(const QString &msg)
{
    return msg.startsWith("l ") || msg.startsWith("local ") ||
            msg.startsWith("b ") || msg.startsWith("broadcast ");
}
static ChatMessage::eChatTypes getKindOfChatMessage(const QStringRef &msg)
{
    if(msg=="l" || msg=="local")
        return ChatMessage::CHAT_Local;
    if(msg=="b" || msg=="broadcast")
        return ChatMessage::CHAT_Broadcast;
    // unknown chat types are processed as local chat
    return ChatMessage::CHAT_Local;
}

void MapInstance::process_chat(MapClient *sender,const QString &msg_text)
{
    QString sender_char_name;
    int first_space = msg_text.indexOf(' ');
    QStringRef cmd_str(msg_text.midRef(0,first_space));
    QStringRef msg_content(msg_text.midRef(first_space+1,msg_text.lastIndexOf("\n")));
    ChatMessage::eChatTypes kind = getKindOfChatMessage(cmd_str);
    std::vector<MapClient *> recipients;
    if(sender && sender->char_entity())
        sender_char_name = sender->char_entity()->name();

    switch(kind)
    {
        case ChatMessage::CHAT_Local:
        {
            // send only to clients within range
            glm::vec3 senderpos = sender->char_entity()->pos;
            for(MapClient *cl : m_clients)
            {
                glm::vec3 recpos = cl->char_entity()->pos;
                float range = 100.0f; // range of "hearing"
                float dist = glm::distance(senderpos,recpos);
                /*
                printf("senderpos: %f %f %f\n", senderpos.x, senderpos.y, senderpos.z);
                printf("recpos: %f %f %f\n", recpos.x, recpos.y, recpos.z);
                printf("sphere: %f\n", range);
                printf("dist: %f\n", dist);
                */
                if(dist<=range)
                    recipients.push_back(cl);
            }
            QString prepared_chat_message = QString("[Local]%1: %2").arg(sender_char_name,msg_content.toString());
            for(MapClient * cl : recipients)
            {
                ChatMessage *msg = ChatMessage::localMessage(prepared_chat_message,sender->char_entity());
                cl->addCommandToSendNextUpdate(std::unique_ptr<ChatMessage>(msg));
            }
            break;
        }
        case ChatMessage::CHAT_Broadcast:
            // send the message to everyone on this map
            std::copy(m_clients.begin(),m_clients.end(),std::back_insert_iterator<std::vector<MapClient *>>(recipients));
            QString prepared_chat_message = QString("%1: %2").arg(sender_char_name,msg_content.toString());
            for(MapClient * cl : recipients)
            {
                ChatMessage *msg = ChatMessage::broadcastMessage(prepared_chat_message,sender->char_entity());
                cl->addCommandToSendNextUpdate(std::unique_ptr<ChatMessage>(msg));
            }
            break;
    }

}
void MapInstance::on_console_command(ConsoleCommand * ev)
{
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *src = lnk->client_data();
    printf("Console command received %s\n",qPrintable(ev->contents));
    if(ev->contents.startsWith("script "))
    {
        //TODO: restrict scripting access to GM's and such
        QString code = ev->contents.mid(7,ev->contents.size()-7);
        m_scripting_interface->runScript(src,code,"user provided script");
        return;
    }
    else if(isChatMessage(ev->contents))
    {
        process_chat(src,ev->contents);
    }
    else if(ev->contents.startsWith("dlg ")) {
        StandardDialogCmd *dlg = new StandardDialogCmd(ev->contents.mid(4));
        src->addCommandToSendNextUpdate(std::unique_ptr<StandardDialogCmd>(dlg));
    }
}
void MapInstance::on_command_chat_divider_moved(ChatDividerMoved *ev)
{
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *src = lnk->client_data();
    qDebug() << "Chat divider moved to "<<ev->m_position << " for player" << src;
}
void MapInstance::on_minimap_state(MiniMapState *ev)
{
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *src = lnk->client_data();
    qDebug() << "MiniMapState tile "<<ev->tile_idx << " for player" << src;
}

void MapInstance::on_client_resumed(ClientResumedRendering *ev)
{
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *cl = lnk->client_data();
    if(cl->m_in_map==false)
        cl->m_in_map = true;
    char buf[256];
    std::string welcome_msg = std::string("Welcome to SEGS ") + VersionInfo::getAuthVersion()+"\n";
    std::snprintf(buf, 256, "There are %zu active entites and %zu clients", m_entities.active_entities(),
                  num_active_clients());
    welcome_msg += buf;
    cl->addCommandToSendNextUpdate(std::unique_ptr<ChatMessage>(ChatMessage::adminMessage(welcome_msg.c_str())));
}
void MapInstance::on_location_visited(LocationVisited *ev)
{
    qWarning() << "Unhandled location visited event:" << ev->m_name <<
                  QString("(%1,%2,%3)").arg(ev->m_pos.x).arg(ev->m_pos.y).arg(ev->m_pos.z);
}

void MapInstance::on_plaque_visited(PlaqueVisited * ev)
{
    qWarning() << "Unhandled plaque visited event:" << ev->m_name <<
                  QString("(%1,%2,%3)").arg(ev->m_pos.x).arg(ev->m_pos.y).arg(ev->m_pos.z);

}

void MapInstance::on_inspiration_dockmode(InspirationDockMode *ev)
{
    qWarning() << "Unhandled inspiration dock mode:" << ev->dock_mode;
}

void MapInstance::on_enter_door(EnterDoor *ev)
{
    qWarning().noquote() << "Unhandled door entry request to:" << ev->name;
    if(ev->unspecified_location)
        qWarning().noquote() << "    no location provided";
    else
        qWarning().noquote() << ev->location.x<< ev->location.y<< ev->location.z;
    //pseudocode:
    //  auto door = get_door(ev->name,ev->location);
    //  if(door and player_can_enter(door)
    //    process_map_transfer(player,door->targetMap);
}

void MapInstance::on_change_stance(ChangeStance * ev)
{
    qWarning() << "Unhandled change stance request";
    if(ev->enter_stance)
        qWarning() << "  enter stance" <<ev->powerset_index<<ev->power_index;
    else
        qWarning() << "  exit stance";
}

void MapInstance::on_set_destination(SetDestination * ev)
{
    qWarning() << "Unhandled set destination request";
    qWarning() << "  index" <<ev->point_index<< "loc"<<ev->destination.x<<ev->destination.y<<ev->destination.z;
}

void MapInstance::on_abort_queued_power(AbortQueuedPower * ev)
{
    qWarning() << "Unhandled abort queued power request";
}

void MapInstance::on_description_and_battlecry(DescriptionAndBattleCry * ev)
{
    qWarning() << "Unhandled description and battlecry request" << ev->description<<ev->battlecry;
    // TODO: client expects us to force it to fill description and battlecry fields in UI?
}

void MapInstance::on_entity_info_request(EntityInfoRequest * ev)
{
    qWarning() << "Unhandled entity info requested" << ev->entity_idx;
    // TODO: send something to the client, question is, what kind of packet should be used here ??

}

void MapInstance::on_client_settings(ClientSettings * ev)
{
    qWarning() << "Unhandled client settings";
    //TODO: serialize settings to client entry in the database.
}
