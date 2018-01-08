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
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>


namespace {
enum {
    World_Update_Timer   = 1,
    State_Transmit_Timer = 2
};

ACE_Time_Value world_update_interval(0,1000*1000/WORLD_UPDATE_TICKS_PER_SECOND);
ACE_Time_Value resend_interval(0,250*1000);
void loadAndRunLua(std::unique_ptr<ScriptingEngine> &lua,const QString &locations_scriptname)
{
    if(QFile::exists(locations_scriptname))
    {
        lua->loadAndRunFile(locations_scriptname);
    }
    else
    {
        qWarning().noquote() << locations_scriptname <<"is missing";
    }
}
}

using namespace std;
MapInstance::MapInstance(const QString &name) : m_name(name), m_world_update_timer(nullptr)
{
    m_world = new World(m_entities);
    m_scripting_interface.reset(new ScriptingEngine);
}
void MapInstance::start()
{
    assert(m_world_update_timer==nullptr);
    m_scripting_interface->registerTypes();
    QFileInfo mapDataDirInfo("MapInstances/"+m_name);
    if(mapDataDirInfo.exists() && mapDataDirInfo.isDir())
    {
        qInfo() << "Loading map instance data...";
        QString locations_scriptname="MapInstances/"+m_name+'/'+"locations.lua";
        QString plaques_scriptname="MapInstances/"+m_name+'/'+"plaques.lua";

        loadAndRunLua(m_scripting_interface,locations_scriptname);
        loadAndRunLua(m_scripting_interface,plaques_scriptname);
    }
    else
    {
        QDir::current().mkpath("MapInstances/"+m_name);
        qWarning() << "FAILED to load map instance data. Check to see if file exists:"<< "MapInstances/"+m_name;
    }
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
        case MapEventTypes::evPowersDockMode:
            on_powers_dockmode(static_cast<PowersDockMode *>(ev));
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
        case MapEventTypes::evChatReconfigure:
            on_chat_reconfigured(static_cast<ChatReconfigure *>(ev));
            break;
        case MapEventTypes::evPlaqueVisited:
            on_plaque_visited(static_cast<PlaqueVisited *>(ev));
            break;
        case MapEventTypes::evSwitchViewPoint:
            on_switch_viewpoint(static_cast<SwitchViewPoint *>(ev));
            break;
        case MapEventTypes::evClientSettings:
            on_client_settings(static_cast<ClientSettings *>(ev));
            break;
        case MapEventTypes::evDescriptionAndBattleCry:
            on_description_and_battlecry(static_cast<DescriptionAndBattleCry *>(ev));
            break;
        case MapEventTypes::evUnqueueAll:
            on_unqueue_all(static_cast<UnqueueAll *>(ev));
            break;
        case MapEventTypes::evActivateInspiration:
            on_activate_inspiration(static_cast<ActivateInspiration *>(ev));
            break;
        case MapEventTypes::evSwitchTray:
            on_switch_tray(static_cast<SwitchTray *>(ev));
            break;
        case MapEventTypes::evTargetChatChannelSelected:
            on_target_chat_channel_selected(static_cast<TargetChatChannelSelected *>(ev));
            break;
        default:
            fprintf(stderr,"Unhandled MapEventTypes %zu\n",ev->type()-MapEventTypes::base);
            //ACE_DEBUG ((LM_WARNING,ACE_TEXT ("Unhandled event type %d\n"),ev->type()));
    }
}

SEGSEvent * MapInstance::dispatch_sync( SEGSEvent * )
{
    assert(!"No sync dispatchable events here");

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
        e->m_class_idx = getEntityClassIndex(true, e->m_char.getClass());
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
QString process_replacement_strings(MapClient *sender,const QString &msg_text)
{
    /*
    // $archetype   - the archetype of your character
    // $battlecry   - your character's battlecry, as entered on your character ID screen
    // $level       - your character's current level
    // $name        - your character's name
    // $origin      - your character's origin
    // $target      - your currently selected target's name

    msg_text = msg_text.replace("$target",sender->char_entity()->target->name());
    */

    QString new_msg = msg_text;
    static const QStringList replacements = {
        "\\$archetype",
        "\\$battlecry",
        "\\$level",
        "\\$name",
        "\\$origin",
        "\\$target"
    };
    Character sender_char       = sender->char_entity()->m_char;

    QString  sender_class       = QString(getClass(sender_char)).remove("Class_");
    QString  sender_battlecry   = sender_char.m_battle_cry;
    uint32_t sender_level       = getLevel(sender_char);
    QString  sender_char_name   = sender_char.getName();
    QString  sender_origin      = getOrigin(sender_char);
    QString  target_idx         = sender->char_entity()->m_target_idx;
    Entity   tgt                = getEntityByIdx(target_idx);
    QString  target_char_name   = tgt.getName();

    foreach (const QString &str, replacements) {
        if(str == "\\$archetype")
            new_msg.replace(QRegExp(str), sender_class);
        else if(str == "\\$battlecry")
            new_msg.replace(QRegExp(str), sender_battlecry);
        else if(str == "\\$level")
            new_msg.replace(QRegExp(str), QString::number(sender_level));
        else if(str == "\\$name")
            new_msg.replace(QRegExp(str), sender_char_name);
        else if(str == "\\$origin")
            new_msg.replace(QRegExp(str), sender_origin);
        else if(str == "\\$target")
            new_msg.replace(QRegExp(str), target_char_name);
    }
    return new_msg;
}
static bool isChatMessage(const QString &msg)
{
    static const QStringList chat_prefixes = {
            "l", "local",
            "b", "broadcast", "y", "yell",
            "g", "group", "sg", "supergroup",
            "req", "request",
            "f",
            "t", "tell", "w", "whisper", "p", "private"
    };
    QString space(msg.mid(0,msg.indexOf(' ')));
    return chat_prefixes.contains(space);
}
static ChatMessage::eChatTypes getKindOfChatMessage(const QStringRef &msg)
{
    if(msg=="l" || msg=="local")                                                            // Aliases: local, l
        return ChatMessage::CHAT_Local;
    if(msg=="b" || msg=="broadcast" || msg=="y" || msg=="yell")                             // Aliases: broadcast, yell, b, y
        return ChatMessage::CHAT_Broadcast;
    if(msg=="g" || msg=="group" || msg=="team")                                             // Aliases: team, g, group
        return ChatMessage::CHAT_TEAM;
    if(msg=="sg" || msg=="supergroup")                                                      // Aliases: sg, supergroup
        return ChatMessage::CHAT_SuperGroup;
    if(msg=="req" || msg=="request" || msg=="auction" || msg=="sell")                       // Aliases: request, req, auction, sell
        return ChatMessage::CHAT_Request;
    if(msg=="f")                                                                            // Aliases: f
        return ChatMessage::CHAT_Friend;
    if(msg=="t" || msg=="tell" || msg=="w" || msg=="whisper" || msg=="p" || msg=="private") // Aliases: t, tell, whisper, w, private, p
        return ChatMessage::CHAT_PRIVATE;
    // unknown chat types are processed as local chat
    return ChatMessage::CHAT_Local;
}

void MapInstance::process_chat(MapClient *sender,QString &msg_text)
{
    int first_space = msg_text.indexOf(' ');
    QString sender_char_name;

    if(msg_text.contains("$")) // does it contain replacement strings?
        msg_text = process_replacement_strings(sender, msg_text);

    QStringRef cmd_str(msg_text.midRef(0,first_space));
    QStringRef msg_content(msg_text.midRef(first_space+1,msg_text.lastIndexOf("\n")));
    InfoMessageCmd *info;
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
                float range = 50.0f; // range of "hearing". I assume this is in yards
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
            QString prepared_chat_message = QString("[Local] %1: %2").arg(sender_char_name,msg_content.toString());
            for(MapClient * cl : recipients)
            {
                ChatMessage *msg = ChatMessage::localMessage(prepared_chat_message,sender->char_entity());
                cl->addCommandToSendNextUpdate(std::unique_ptr<ChatMessage>(msg));
            }
            break;
        }
        case ChatMessage::CHAT_Broadcast:
        {
            // send the message to everyone on this map
            std::copy(m_clients.begin(),m_clients.end(),std::back_insert_iterator<std::vector<MapClient *>>(recipients));
            QString prepared_chat_message = QString(" %1: %2").arg(sender_char_name,msg_content.toString()); // where does [Broadcast] come from? The client?
            for(MapClient * cl : recipients)
            {
                ChatMessage *msg = ChatMessage::broadcastMessage(prepared_chat_message,sender->char_entity());
                cl->addCommandToSendNextUpdate(std::unique_ptr<ChatMessage>(msg));
            }
            break;
        }
        case ChatMessage::CHAT_Request:
        {
            // send the message to everyone on this map
            std::copy(m_clients.begin(),m_clients.end(),std::back_insert_iterator<std::vector<MapClient *>>(recipients));
            QString prepared_chat_message = QString("[Request] %1: %2").arg(sender_char_name,msg_content.toString());
            for(MapClient * cl : recipients)
            {
                info = new InfoMessageCmd(InfoType::REQUEST_COM, prepared_chat_message);
                cl->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
            }
            break;
        }
        case ChatMessage::CHAT_PRIVATE:
        {
            int first_comma = msg_text.indexOf(',');
            QStringRef target_name(msg_text.midRef(first_space+1,msg_text.indexOf(',')-1)));
            msg_content = msg_text.midRef(first_comma+1,msg_text.lastIndexOf("\n"));

            Entity *tgt = getEntityByName(target_name);
            
            prepared_chat_message = QString("[Tell] -->%1: %2").arg(target_name,msg_content.toString());
            info = new InfoMessageCmd(InfoType::PRIVATE_COM, prepared_chat_message);
            src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
            
            prepared_chat_message = QString("[Tell] %1: %2").arg(sender_char_name,msg_content.toString());
            info = new InfoMessageCmd(InfoType::PRIVATE_COM, prepared_chat_message);
            tgt->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
            
            break;
        }
        case ChatMessage::CHAT_TEAM:
        {
            // Only send the message to characters on sender's team
            for(MapClient *cl : m_clients)
            {
                if(sender->char_entity()->m_group_name == cl->char_entity()->m_group_name)
                    recipients.push_back(cl);
            }
            QString prepared_chat_message = QString("[Team] %1: %2").arg(sender_char_name,msg_content.toString());
            for(MapClient * cl : recipients)
            {
                info = new InfoMessageCmd(InfoType::TEAM_COM, prepared_chat_message);
                cl->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
            }
            break;
        }
        case ChatMessage::CHAT_SuperGroup:
        {
            // Only send the message to characters in sender's supergroup
            for(MapClient *cl : m_clients)
            {
                if(sender->char_entity()->m_SG_id == cl->char_entity()->m_SG_id)
                    recipients.push_back(cl);
            }
            QString prepared_chat_message = QString("[SuperGroup] %1: %2").arg(sender_char_name,msg_content.toString());
            for(MapClient * cl : recipients)
            {
                info = new InfoMessageCmd(InfoType::SUPERGROUP_COM, prepared_chat_message);
                cl->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
            }
            break;
        }
    }
}
void MapInstance::on_console_command(ConsoleCommand * ev)
{
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *src = lnk->client_data();
    // user entity
    Entity *ent = src->char_entity();
    // leverage InfoMessageCmd
    InfoMessageCmd *info;
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
    else if(ev->contents.startsWith("imsg ")) {
        int first_space = ev->contents.indexOf(' ');
        int second_space = ev->contents.indexOf(' ',first_space+1);
        if(second_space==-1) {
            info = new InfoMessageCmd(InfoType::USER_ERROR,
                                           "The /imsg command takes two arguments, a <b>number</b> and a <b>string</b>"
                                           );
        }
        else {
            bool ok = true;
            int cmdType = ev->contents.midRef(first_space+1,second_space-(first_space+1)).toInt(&ok);
            if(!ok || cmdType<1 || cmdType>21) {
                info = new InfoMessageCmd(InfoType::USER_ERROR,
                                               "The first /imsg argument must be a <b>number</b> between 1 and 21"
                                               );
            }
            else {
                info = new InfoMessageCmd(InfoType(cmdType),ev->contents.mid(second_space+1));
            }

        }
        src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
    }
    else if(ev->contents.startsWith("smilex ")) {
        int space = ev->contents.indexOf(' ');
        QString fileName("scripts/" + ev->contents.mid(space+1));
        if(!fileName.endsWith(".smlx"))
                fileName.append(".smlx");
        QFile file(fileName);
        if(QFileInfo::exists(fileName) && file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QString contents(file.readAll());
            StandardDialogCmd *dlg = new StandardDialogCmd(contents);
            src->addCommandToSendNextUpdate(std::unique_ptr<StandardDialogCmd>(dlg));
        }
        else {
            QString errormsg = "Failed to load smilex file. \'" + file.fileName() + "\' not found.";
            qDebug() << errormsg;
            src->addCommandToSendNextUpdate(std::unique_ptr<ChatMessage>(ChatMessage::adminMessage(errormsg)));
        }
    }
    else if(ev->contents == "fly") {
        toggleFly(ent);

        QString msg = "Toggling " + ev->contents;
        qDebug() << msg;
        info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
        src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
    }
    else if(ev->contents == "stunned") {
        toggleStunned(ent);

        QString msg = "Toggling " + ev->contents;
        qDebug() << msg;
        info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
        src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
    }
    else if(ev->contents == "jumppack") {
        toggleJumppack(ent);

        QString msg = "Toggling " + ev->contents;
        qDebug() << msg;
        info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
        src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
    }
    else if(ev->contents.startsWith("setBackupSpd ")) {
        int space = ev->contents.indexOf(' ');
        float val = ev->contents.mid(space+1).toFloat();
        setBackupSpd(ent, val);

        QString msg = "Set BackupSpd to: " + QString::number(val);
        qDebug() << msg;
        info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
        src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
    }
    else if(ev->contents.startsWith("setJumpHeight ")) {
        int space = ev->contents.indexOf(' ');
        float val = ev->contents.mid(space+1).toFloat();
        setJumpHeight(ent, val);

        QString msg = "Set JumpHeight to: " + QString::number(val);
        qDebug() << msg;
        info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
        src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
    }
    else if(ev->contents.startsWith("afk ") || ev->contents == "afk") {
        int space = ev->contents.indexOf(' ');
        QString val = ev->contents.mid(space+1);
        toggleAFK(ent->m_char, val);

        QString msg = "Setting afk message to: " + val;
        qDebug() << msg;
        info = new InfoMessageCmd(InfoType::EMOTE, msg);
        src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
    }
    else if(ev->contents.startsWith("setHP ")) {
        int space = ev->contents.indexOf(' ');
        QString val = ev->contents.mid(space+1);
        float attrib = val.toFloat();
        float maxattrib = ent->m_char.m_max_attribs.m_HitPoints;

        if(attrib > maxattrib)
            attrib = maxattrib;

        ent->m_char.m_current_attribs.m_HitPoints = attrib;

        QString msg = "Setting HP to: " + QString::number(attrib) + "/" + QString::number(maxattrib);
        qDebug() << msg;
        info = new InfoMessageCmd(InfoType::REGULAR, msg);
        src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
    }
    else if(ev->contents.startsWith("setEnd ")) {
        int space = ev->contents.indexOf(' ');
        QString val = ev->contents.mid(space+1);
        float attrib = val.toFloat();
        float maxattrib = ent->m_char.m_max_attribs.m_Endurance;

        if(attrib > maxattrib)
            attrib = maxattrib;

        ent->m_char.m_current_attribs.m_Endurance = attrib;

        QString msg = "Setting Endurance to: " + QString::number(attrib) + "/" + QString::number(maxattrib);
        qDebug() << msg;
        info = new InfoMessageCmd(InfoType::REGULAR, msg);
        src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
    }
    else if(ev->contents.startsWith("setXP ")) {
        int space = ev->contents.indexOf(' ');
        QString val = ev->contents.mid(space+1);
        float attrib = val.toFloat();
        uint32_t lvl = getLevel(ent->m_char);
        uint32_t newlvl;

        setXP(ent->m_char, attrib);
        QString msg = "Setting XP to " + QString::number(attrib);

        newlvl = getLevel(ent->m_char);
        if(newlvl != lvl)
            msg += " and LVL to " + QString::number(newlvl+1);

        qDebug() << msg;
        info = new InfoMessageCmd(InfoType::REGULAR, msg);
        src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
    }
    else if(ev->contents.startsWith("setLevel ")) {
        int space = ev->contents.indexOf(' ');
        QString val = ev->contents.mid(space+1);
        uint32_t attrib = val.toUInt();

        setLevel(ent->m_char, attrib-1); // TODO: Why must this be -1?

        QString msg = "Setting Level to: " + QString::number(attrib);
        qDebug() << msg;
        info = new InfoMessageCmd(InfoType::REGULAR, msg);
        src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
    }
    else if(ev->contents == "whoall") {
        QString msg = "Players on this map:\n";

        for(MapClient *cl : m_clients)
        {
            QString name        = cl->char_entity()->name();
            QString lvl         = QString::number(getLevel(cl->char_entity()->m_char));
            QString clvl        = QString::number(cl->char_entity()->m_char.m_combat_level);
            QString origin      = getOrigin(cl->char_entity()->m_char);
            QString archetype   = QString(getClass(cl->char_entity()->m_char)).remove("Class_");

            // Format: character_name "lvl" level "clvl" combat_level origin archetype
            msg += name + " lvl " + lvl + " clvl " + clvl + " " + origin + " " + archetype + "\n";
        }

        qDebug().noquote() << msg;
        info = new InfoMessageCmd(InfoType::SVR_COM, msg);
        src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
    }
    else if(ev->contents == "setTitles" || (ev->contents.startsWith("setTitles ") || ev->contents.startsWith("title_change "))) {
        QString msg;
        bool prefix;
        QString generic;
        QString origin;
        QString special;

        if(ev->contents == "setTitles")
        {
            setTitles(ent->m_char);
            msg = "Titles reset to nothing";
        }
        else
        {
            int space1      = ev->contents.indexOf(' ');
            int space2      = ev->contents.indexOf(' ',space1+1);
            int space3      = ev->contents.indexOf(' ',space2+1);
            int space4      = ev->contents.indexOf(' ',space3+1);

            if(space2 == -1 || space3 == -1 || space4 == -1) {
                msg = "The /setTitle command takes four arguments, a boolean (true/false) and three strings. e.g. /setTitle 1 generic origin special";
            }
            else {
                prefix  = ev->contents.mid(space1+1,space2-(space1+1)).toInt();
                generic = ev->contents.mid(space2+1,space3-(space2+1));
                origin  = ev->contents.mid(space3+1,space4-(space3+1));
                special = ev->contents.mid(space4+1);
                setTitles(ent->m_char, prefix, generic, origin, special);
                msg = "Titles changed to: " + QString::number(prefix) + " " + generic + " " + origin + " " + special;
            }
        }
        qDebug() << msg;
        info = new InfoMessageCmd(InfoType::USER_ERROR, msg);
        src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
    }
    else if(ev->contents == "controls_disabled") {
        toggleControlsDisabled(ent);

        QString msg = "Toggling " + ev->contents;
        qDebug() << msg;
        info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
        src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
    }
    else if(ev->contents.startsWith("updateID ")) {
        int space = ev->contents.indexOf(' ');
        QString val = ev->contents.mid(space+1);
        uint8_t attrib = val.toUInt();

        setUpdateID(ent, attrib);

        QString msg = "Setting updateID to: " + QString::number(attrib);
        qDebug() << msg;
        info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
        src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
    }
    else if(ev->contents == "lfg") {
        toggleLFG(ent->m_char);

        QString msg = "Toggling " + ev->contents;
        qDebug() << msg;
        info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
        src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
    }
    else if(ev->contents.startsWith("setInf ")) {
        int space = ev->contents.indexOf(' ');
        QString val = ev->contents.mid(space+1);
        uint32_t attrib = val.toUInt();

        setInf(ent->m_char, attrib);

        QString msg = "Setting influence to: " + QString::number(attrib);
        qDebug() << msg;
        info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
        src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
    }
    // Slash commands for setting bit values
    else if(ev->contents.startsWith("setu1 ")) {
        int space = ev->contents.indexOf(' ');
        int val = ev->contents.mid(space+1).toInt();
        setu1(ent, val);

        QString msg = "Set u1 to: " + QString::number(val);
        qDebug() << msg;
        info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
        src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
        src->addCommandToSendNextUpdate(std::unique_ptr<ChatMessage>(ChatMessage::debugMessage(msg)));
    }
    else if(ev->contents.startsWith("setu2 ")) {
        int space = ev->contents.indexOf(' ');
        int val = ev->contents.mid(space+1).toInt();
        setu2(ent, val);

        QString msg = "Set u2 to: " + QString::number(val);
        qDebug() << msg;
        info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
        src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
        src->addCommandToSendNextUpdate(std::unique_ptr<ChatMessage>(ChatMessage::debugMessage(msg)));
    }
    else if(ev->contents.startsWith("setu3 ")) {
        int space = ev->contents.indexOf(' ');
        int val = ev->contents.mid(space+1).toInt();
        setu3(ent, val);

        QString msg = "Set u3 to: " + QString::number(val);
        qDebug() << msg;
        info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
        src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
        src->addCommandToSendNextUpdate(std::unique_ptr<ChatMessage>(ChatMessage::debugMessage(msg)));
    }
    else if(ev->contents.startsWith("setu4 ")) {
        int space = ev->contents.indexOf(' ');
        int val = ev->contents.mid(space+1).toInt();
        setu4(ent, val);

        QString msg = "Set u4 to: " + QString::number(val);
        qDebug() << msg;
        info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
        src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
        src->addCommandToSendNextUpdate(std::unique_ptr<ChatMessage>(ChatMessage::debugMessage(msg)));
    }
    else if(ev->contents.startsWith("setu5 ")) {
        int space = ev->contents.indexOf(' ');
        int val = ev->contents.mid(space+1).toInt();
        setu5(ent, val);

        QString msg = "Set u5 to: " + QString::number(val);
        qDebug() << msg;
        info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
        src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
        src->addCommandToSendNextUpdate(std::unique_ptr<ChatMessage>(ChatMessage::debugMessage(msg)));
    }
    else if(ev->contents.startsWith("setu6 ")) {
        int space = ev->contents.indexOf(' ');
        int val = ev->contents.mid(space+1).toInt();
        setu6(ent, val);

        QString msg = "Set u6 to: " + QString::number(val);
        qDebug() << msg;
        info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
        src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
        src->addCommandToSendNextUpdate(std::unique_ptr<ChatMessage>(ChatMessage::debugMessage(msg)));
    }
    else {
        qDebug() << "Unhandled game command:" << ev->contents;
    }
}
void MapInstance::on_command_chat_divider_moved(ChatDividerMoved *ev)
{
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *src = lnk->client_data();
    qDebug() << "Chat divider moved to " << ev->m_position << " for player" << src;
}
void MapInstance::on_minimap_state(MiniMapState *ev)
{
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *src = lnk->client_data();
    //qDebug() << "MiniMapState tile "<<ev->tile_idx << " for player" << src;
    // TODO: Save these tile #s to dbase and (presumably) load upon entering map to remove fog-of-war from map
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
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *cl = lnk->client_data();
    qDebug() << "Attempting a call to script location_visited with:"<<ev->m_name<<qHash(ev->m_name);
    auto val = m_scripting_interface->callFuncWithClientContext(cl,"location_visited",qHash(ev->m_name));
    cl->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(new InfoMessageCmd(InfoType::DEBUG_INFO,val.c_str())));

    qWarning() << "Unhandled location visited event:" << ev->m_name <<
                  QString("(%1,%2,%3)").arg(ev->m_pos.x).arg(ev->m_pos.y).arg(ev->m_pos.z);
}

void MapInstance::on_plaque_visited(PlaqueVisited * ev)
{
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *cl = lnk->client_data();
    qDebug() << "Attempting a call to script plaque_visited with:"<<ev->m_name<<qHash(ev->m_name);
    auto val = m_scripting_interface->callFuncWithClientContext(cl,"plaque_visited",qHash(ev->m_name));
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
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *src = lnk->client_data();
    // user entity
    Entity *ent = src->char_entity();

    if(!ev->battlecry.isNull() && !ev->description.isNull())
    {
        qWarning() << "Attempted description and battlecry request:" << ev->description << ev->battlecry;
        ent->m_char.m_battle_cry = ev->battlecry;
        ent->m_char.m_character_description = ev->description;
    }
    else
        qWarning() << "Unhandled description and battlecry request" << ev->description<<ev->battlecry;
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

void MapInstance::on_switch_viewpoint(SwitchViewPoint *ev)
{
    qWarning() << "Unhandled switch viewpoint to" << ev->new_viewpoint_is_firstperson;

}

void MapInstance::on_chat_reconfigured(ChatReconfigure *ev)
{
    qWarning() << "Unhandled chat channel mask setting" << ev->m_chat_top_flags << ev->m_chat_bottom_flags;
}

void MapInstance::on_unqueue_all(UnqueueAll *ev)
{
    qWarning() << "Unhandled unqueue all request:" << ev->g_input_pak;
    // TODO: not sure what the client expects from the server here
    // and is it really unqueing everything? Is this named correctly?
}

void MapInstance::on_target_chat_channel_selected(TargetChatChannelSelected *ev)
{
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *src = lnk->client_data();
    // user entity
    Entity *ent = src->char_entity();

    qWarning() << "Unhandled change chat type request." << ev->m_chat_type;
    // TODO: not sure what the client expects the server to do here, but m_chat_type
    // corresponds to the InfoType in InfoMessageCmd and eChatTypes in ChatMessage

    // Passing cur_chat_channel to Entity in case we need it somewhere.
    ent->m_cur_chat_channel = ev->m_chat_type;
}

void MapInstance::on_activate_inspiration(ActivateInspiration *ev)
{
    qWarning() << "Unhandled use inspiration request." << ev->row_idx << ev->slot_idx;
    // TODO: not sure what the client expects from the server here
}

void MapInstance::on_powers_dockmode(PowersDockMode *ev)
{
    qWarning() << "Unhandled powers dock mode:" << ev->dock_mode << ev->toggle_secondary_tray;
}

void MapInstance::on_switch_tray(SwitchTray *ev)
{
    qWarning() << "Unhandled switch tray request. Tray1:" << ev->tray1_num+1 << "Tray2:" << ev->tray2_num+1 << "Unk1:" << ev->tray_unk1;
    // TODO: need to load powers for new tray.
}
