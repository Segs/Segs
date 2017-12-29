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

#include <stdlib.h>

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
    Entity *ent = src->char_entity(); // user entity
    InfoMessageCmd *info; // leverage InfoMessageCmd
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
        int first_space=ev->contents.indexOf(' ');
        int second_space =ev->contents.indexOf(' ',first_space+1);
        InfoMessageCmd *info;
        if(second_space==-1) {
            info = new InfoMessageCmd(InfoType::USER_ERROR,
                                           "The /imsg command takes two arguments, a <b>number</b> and a <b>string</b>"
                                           );
        }
        else {
            bool ok=true;
            int cmdType=ev->contents.midRef(first_space+1,second_space-(first_space+1)).toInt(&ok);
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
    QString lowerContents = ev->contents.toLower();                             // ERICEDIT: Make the contents all lowercase for case-insensitivity.
    QString msg;                                                                // Initialize the variable to hold the debug message.
    if(lowerContents == "fly")                                                  // If the user enters "/fly":
    {
        ent->toggleFly(ent);                                                    // Call toggleFly() from Entity.cpp.
        if(ent->m_is_flying)                                                    // If the entity is now flying:
            msg = "Enabling " + lowerContents;                                  // Set the message to reflect that.
        else                                                                    // Else:
            msg = "Disabling " + lowerContents;                                 // Set the message to reflect disabling fly.

    }
    else if(lowerContents.startsWith("em") || lowerContents.startsWith("e")
            || lowerContents.startsWith("me"))                                  // ERICEDIT: This encompasses all emotes.
    {
        if(lowerContents.startsWith("em") || lowerContents.startsWith("me"))    // This if-else removes the prefix of the command for conciseness.
            lowerContents.replace(0, 3, "");
        else                                                                    // Requires a different argument for the "e" command.
            lowerContents.replace(0, 2, "");
                                                                                // Normal Emotes
        if(lowerContents == "afraid" || lowerContents == "cower"
                || lowerContents == "fear" || lowerContents == "scared")        // Afraid: Cower in fear, hold stance.
        {
            if(ent->m_is_flying)                                                // Different versions when flying and on the ground.
                msg = "Unhandled flying Afraid emote";
            else
                msg = "Unhandled ground Afraid emote";
        }
        else if((lowerContents == "akimbo" || lowerContents == "wings")         // Akimbo: Stands with fists on hips looking forward, hold stance.
                && !ent->m_is_flying)                                           // Not allowed when flying.
            msg = "Unhandled Akimbo emote";
        else if(lowerContents == "angry")                                       // Angry: Fists on hips and slouches forward, as if glaring or grumbling, hold stance.
            msg = "Unhandled Angry emote";
        else if(lowerContents == "atease")                                      // AtEase: Stands in the 'at ease' military position (legs spread out slightly, hands behind back) stance, hold stance.
            msg = "Unhandled AtEase emote";
        else if(lowerContents == "attack")                                      // Attack: Gives a charge! type point, fists on hips stance.
            msg = "Unhandled Attack emote";
        else if(lowerContents == "batsmash")                                    // BatSmash: Hit someone or something with a bat, repeat.
            msg = "Unhandled BatSmash emote";
        else if(lowerContents == "batsmashreact")                               // BatSmashReact: React as if getting hit with a bat, often used in duo with BatSmash.
            msg = "Unhandled BatSmashReact emote";
        else if(lowerContents == "bigwave" || lowerContents == "overhere")      // BigWave: Waves over the head, fists on hips stance.
            msg = "Unhandled BigWave emote";
        else if((lowerContents == "boombox" || lowerContents == "bb"            // BoomBox (has sound): Summons forth a boombox (it just appears) and leans over to turn it on, stands up and does a sort of dance. A random track will play.
                || lowerContents == "dropboombox") && !ent->m_is_flying)        // Not allowed when flying.
        {
            int rSong = rand() % 25 + 1;                                        // Randomly pick a song.
            switch(rSong)
            {
                case 1:                                                         // 1: BBAltitude
                {
                    msg = "1: Unhandled \"BBAltitude\" BoomBox emote";
                    break;
                }
                case 2:                                                         // 2: BBBeat
                {
                    msg = "2: Unhandled \"BBBeat\" BoomBox emote";
                    break;
                }
                case 3:                                                         // 3: BBCatchMe
                {
                    msg = "3: Unhandled \"BBCatchMe\" BoomBox emote";
                    break;
                }
                case 4:                                                         // 4: BBDance
                {
                    msg = "4: Unhandled \"BBDance\" BoomBox emote";
                    break;
                }
                case 5:                                                         // 5: BBDiscoFreak
                {
                    msg = "5: Unhandled \"BBDiscoFreak\" BoomBox emote";
                    break;
                }
                case 6:                                                         // 6: BBDogWalk
                {
                    msg = "6: Unhandled \"BBDogWalk\" BoomBox emote";
                    break;
                }
                case 7:                                                         // 7: BBElectroVibe
                {
                    msg = "7: Unhandled \"BBElectroVibe\" BoomBox emote";
                    break;
                }
                case 8:                                                         // 8: BBHeavyDude
                {
                    msg = "8: Unhandled \"BBHeavyDude\" BoomBox emote";
                    break;
                }
                case 9:                                                         // 9: BBInfoOverload
                {
                    msg = "9: Unhandled \"BBInfoOverload\" BoomBox emote";
                    break;
                }
                case 10:                                                        // 10: BBJumpy
                {
                    msg = "10: Unhandled \"BBJumpy\" BoomBox emote";
                    break;
                }
                case 11:                                                        // 11: BBKickIt
                {
                    msg = "11: Unhandled \"BBKickIt\" BoomBox emote";
                    break;
                }
                case 12:                                                        // 12: BBLooker
                {
                    msg = "12: Unhandled \"BBLooker\" BoomBox emote";
                    break;
                }
                case 13:                                                        // 13: BBMeaty
                {
                    msg = "13: Unhandled \"BBMeaty\" BoomBox emote";
                    break;
                }
                case 14:                                                        // 14: BBMoveOn
                {
                    msg = "14: Unhandled \"BBMoveOn\" BoomBox emote";
                    break;
                }
                case 15:                                                        // 15: BBNotorious
                {
                    msg = "15: Unhandled \"BBNotorious\" BoomBox emote";
                    break;
                }
                case 16:                                                        // 16: BBPeace
                {
                    msg = "16: Unhandled \"BBPeace\" BoomBox emote";
                    break;
                }
                case 17:                                                        // 17: BBQuickie
                {
                    msg = "17: Unhandled \"BBQuickie\" BoomBox emote";
                    break;
                }
                case 18:                                                        // 18: BBRaver
                {
                    msg = "18: Unhandled \"BBRaver\" BoomBox emote";
                    break;
                }
                case 19:                                                        // 19: BBShuffle
                {
                    msg = "19: Unhandled \"BBShuffle\" BoomBox emote";
                    break;
                }
                case 20:                                                        // 20: BBSpaz
                {
                    msg = "20: Unhandled \"BBSpaz\" BoomBox emote";
                    break;
                }
                case 21:                                                        // 21: BBTechnoid
                {
                    msg = "21: Unhandled \"BBTechnoid\" BoomBox emote";
                    break;
                }
                case 22:                                                        // 22: BBVenus
                {
                    msg = "22: Unhandled \"BBVenus\" BoomBox emote";
                    break;
                }
                case 23:                                                        // 23: BBWindItUp
                {
                    msg = "23: Unhandled \"BBWindItUp\" BoomBox emote";
                    break;
                }
                case 24:                                                        // 24: BBWahWah
                {
                    msg = "24: Unhandled \"BBWahWah\" BoomBox emote";
                    break;
                }
                case 25:                                                        // 25: BBYellow
                {
                    msg = "25: Unhandled \"BBYellow\" BoomBox emote";
                }
            }
        }
        else if((lowerContents == "bow" || lowerContents == "sorry")            // Bow: Chinese/Japanese style bow with palms together, returns to normal stance.
                && !ent->m_is_flying)                                           // Not allowed when flying.
            msg = "Unhandled Bow emote";
        else if(lowerContents == "bowdown" || lowerContents == "down")          // BowDown: Thrusts hands forward, then points down, as if ordering someone else to bow before you.
            msg = "Unhandled BowDown emote";
        else if(lowerContents == "burp" && !ent->m_is_flying)                   // Burp (has sound): A raunchy belch, wipes mouth with arm afterward, ape-like stance.
            msg = "Unhandled Burp emote";                                       // Not allowed when flying.
        else if(lowerContents == "cheer")                                       // Cheer: Randomly does one of 3 cheers, 1 fist raised, 2 fists raised or 2 fists lowered, repeats.
        {
            int rNum = rand() % 3 + 1;                                          // Randomly pick the cheer.
            switch(rNum)
            {
                case 1:                                                         // 1: 1 fist raised
                {
                    msg = "1: Unhandled \"1 fist raised\" Cheer emote";
                    break;
                }
                case 2:                                                         // 2: 2 fists raised
                {
                    msg = "2: Unhandled \"2 fists raised\" Cheer emote";
                    break;
                }
                case 3:                                                         // 3: 2 fists lowered
                {
                    msg = "3: Unhandled \"2 fists lowered\" Cheer emote";
                }
            }
        }
        else if(lowerContents == "clap")                                        // Clap (has sound): Claps hands several times, crossed arms stance.
            msg = "Unhandled Clap emote";
        else if(lowerContents == "coin" || lowerContents == "cointoss"
                || lowerContents == "flipcoin")                                 // Coin: Flips a coin, randomly displays heads or tails, and hold stance. Coin image remains until stance broken.
        {
            int rFlip = rand() % 2 + 1;                                         // Randomly pick heads or tails.
            switch(rFlip)
            {
                case 1:                                                         // 1: Heads
                {
                    msg = "1: Unhandled heads Coin emote";
                    break;
                }
                case 2:                                                         // 2: Tails
                {
                    msg = "2: Unhandled tails Coin emote";
                }
            }
        }
        else if(lowerContents == "crossarms" && !ent->m_is_flying)              // CrossArms: Crosses arms, stance (slightly different from most other crossed arm stances).
            msg = "Unhandled CrossArms emote";                                  // Not allowed when flying.
        else if(lowerContents == "dance")                                       // Dance: Randomly performs one of six dances.
        {
            int rDance = rand() % 6 + 1;                                        // Randomly pick the dance.
            switch(rDance)
            {
                case 1:                                                         // 1: Dances with elbows by hips.
                {
                    msg = "1: Unhandled \"Dances with elbows by hips\" Dance emote";
                    break;
                }
                case 2:                                                         // 2: Dances with fists raised.
                {
                    msg = "2: Unhandled \"Dances with fists raised\" Dance emote";
                    break;
                }
                case 3:                                                         // 3: Swaying hands by hips, aka "Really Bad" dancing.
                {
                    msg = "3: Unhandled \"Swaying hands by hips, aka 'Really Bad' dancing\" Dance emote";
                    break;
                }
                case 4:                                                         // 4: Swaying hands up in the air, like in a breeze.
                {
                    msg = "4: Unhandled \"Swaying hands up in the air, like in a breeze\" Dance emote";
                    break;
                }
                case 5:                                                         // 5: As Dance4, but jumping as well.
                {
                    msg = "5: Unhandled \"As Dance4, but jumping as well\" Dance emote";
                    break;
                }
                case 6:                                                         // 6: The monkey.
                {
                    msg = "6: Unhandled \"The monkey\" Dance emote";
                }
            }
        }
        else if(lowerContents == "dice" || lowerContents == "rolldice")         // Dice: Picks up, shakes and rolls a die, randomly displays the results (1-6), default stance. Die image quickly fades.
        {
            int rDice = rand() % 6 + 1;                                         // Randomly pick a die result.
            switch(rDice)
            {
                case 1:                                                         // 1: 1
                {
                    msg = "1: Unhandled \"1\" Dice emote";
                    break;
                }
                case 2:                                                         // 2: 2
                {
                    msg = "2: Unhandled \"2\" Dice emote";
                    break;
                }
                case 3:                                                         // 3: 3
                {
                    msg = "3: Unhandled \"3\" Dice emote";
                    break;
                }
                case 4:                                                         // 4: 4
                {
                    msg = "4: Unhandled \"4\" Dice emote";
                    break;
                }
                case 5:                                                         // 5: 5
                {
                    msg = "5: Unhandled \"5\" Dice emote";
                    break;
                }
                case 6:                                                         // 6: 6
                {
                    msg = "6: Unhandled \"6\" Dice emote";
                }
            }
        }
        else if(lowerContents == "dice1")                                       // Dice1: Picks up, shakes and rolls a die, displays a 1, default stance.
            msg = "Unhandled Dice1 emote";
        else if(lowerContents == "dice2")                                       // Dice2: Picks up, shakes and rolls a die, displays a 2, default stance.
            msg = "Unhandled Dice2 emote";
        else if(lowerContents == "dice3")                                       // Dice3: Picks up, shakes and rolls a die, displays a 3, default stance.
            msg = "Unhandled Dice3 emote";
        else if(lowerContents == "dice4")                                       // Dice4: Picks up, shakes and rolls a die, displays a 4, default stance.
            msg = "Unhandled Dice4 emote";
        else if(lowerContents == "dice5")                                       // Dice5: Picks up, shakes and rolls a die, displays a 5, default stance.
            msg = "Unhandled Dice5 emote";
        else if(lowerContents == "dice6")                                       // Dice6: Picks up, shakes and rolls a die, displays a 6, default stance.
            msg = "Unhandled Dice6 emote";
        else if(lowerContents == "disagree")                                    // Disagree: Shakes head, crosses hand in front, then offers an alternative, crossed arms stance.
            msg = "Unhandled Disagree emote";
        else if(lowerContents == "drat")                                        // Drat: Raises fists up, then down, stomping at the same time, same ending stance as Frustrated.
            msg = "Unhandled Drat emote";
        else if(lowerContents == "explain")                                     // Explain: Hold arms out in a "wait a minute" gesture, motion alternatives, then shrug.
            msg = "Unhandled Explain emote";
        else if(lowerContents == "evillaugh" || lowerContents == "elaugh"
                || lowerContents == "muahahaha" || lowerContents == "villainlaugh"
                || lowerContents == "villainouslaugh")                          // EvilLaugh: Extremely melodramatic, overacted evil laugh.
            msg = "Unhandled EvilLaugh emote";
        else if(lowerContents == "fancybow" || lowerContents == "elegantbow")   // FancyBow: A much more elegant, ball-room style bow, falls into neutral forward facing stance.
            msg = "Unhandled FancyBow emote";
        else if(lowerContents == "flex1" || lowerContents == "flexa")           // Flex1: Fists raised, flexing arms stance, hold stance. This is called a "double biceps" pose.
            msg = "Unhandled Flex1 emote";
        else if(lowerContents == "flex2" || lowerContents == "flex"
                || lowerContents == "flexb")                                    // Flex2: A side-stance flexing arms, hold stance. This is a sideways variation on the "most muscular" pose.
            msg = "Unhandled Flex2 emote";
        else if(lowerContents == "flex3" || lowerContents == "flexc")           // Flex3: Another side-stance, flexing arms, hold stance. This is an open variation on the "side chest" pose.
            msg = "Unhandled Flex3 emote";
        else if(lowerContents == "frustrated")                                  // Frustrated: Raises both fists and leans backwards, shaking fists and head, leads into a quick-breathing angry-looking stance.
            msg = "Unhandled Frustrated emote";
        else if(lowerContents == "grief")                                       // Grief: Falls to knees, hands on forehead, looks up and gestures a sort of "why me?" look with hands, goes into a sort of depressed slump while on knees, holds stance.
            msg = "Unhandled Grief emote";
        else if(lowerContents == "hi" || lowerContents == "wave")               // Hi: Simple greeting wave, fists on hips stance.
            msg = "Unhandled Hi emote";
        else if(lowerContents == "hmmm" || lowerContents == "plotting")         // Hmmm: Stare into the sky, rubbing chin, thinking.
            msg = "Unhandled Hmmm emote";
        else if(lowerContents == "jumpingjacks")                                // JumpingJacks (has sound): Does jumping jacks, repeats.
            msg = "Unhandled JumpingJacks emote";
        else if(lowerContents == "kneel")                                       // Kneel: Quickly kneels on both knees with hands on thighs (looks insanely uncomfortable), holds stance.
            msg = "Unhandled Kneel emote";
        else if(lowerContents == "laugh")                                       // Laugh: Fists on hips, tosses head back and laughs.
            msg = "Unhandled Laugh emote";
        else if(lowerContents == "laugh2" || lowerContents == "biglaugh"
                || lowerContents == "laughtoo")                                 // Laugh2: Another style of laugh.
            msg = "Unhandled Laugh2 emote";
        else if(lowerContents == "lecture")                                     // Lecture: Waves/shakes hands in different motions in a lengthy lecture, fists on hips stance.
            msg = "Unhandled Lecture emote";
        else if(lowerContents == "martialarts" || lowerContents == "kata")      // MartialArts (has sound): Warm up/practice punches and blocks.
            msg = "Unhandled MartialArts emote";
        else if(lowerContents == "militarysalute")                              // MilitarySalute: Stands in the military-style heads-high hand on forehead salute stance.
            msg = "Unhandled MilitarySalute emote";
        else if(lowerContents == "newspaper" || lowerContents == "afk")         // Newspaper: Materializes a newspaper and reads it.
            msg = "Unhandled Newspaper emote";
        else if(lowerContents == "no" || lowerContents == "dontattack")         // No: Shakes head and waves hands in front of character, crossed arms stance.
            msg = "Unhandled No emote";
        else if(lowerContents == "nod")                                         // Nod: Fists on hips, nod yes, hold stance.
            msg = "Unhandled Nod emote";
        else if(lowerContents == "none")                                        // None: Cancels the current emote, if any, and resumes default standing animation cycle.
            msg = "Unhandled None emote";
        else if(lowerContents == "paper")                                       // Paper: Plays rock/paper/scissors, picking paper (displays all three symbols for about 6 seconds, then displays and holds your choice until stance is broken).
            msg = "Unhandled Paper emote";
        else if(lowerContents == "plot" || lowerContents == "scheme")           // Plot: Rubs hands together while hunched over.
            msg = "Unhandled Plot emote";
        else if(lowerContents == "point")                                       // Point: Extends left arm and points in direction character is facing, hold stance.
            msg = "Unhandled Point emote";
        else if(lowerContents == "praise")                                      // Praise: Kneel prostrate and repeatedly bow in adoration.
            msg = "Unhandled Praise emote";
        else if(lowerContents == "protest")                                     // Protest: Hold hold up one of several randomly selected mostly unreadable protest signs.
            msg = "Unhandled Protest emote";
        else if(lowerContents == "roar" && !ent->m_is_flying)                   // Roar: Claws air, roaring, ape-like stance.
            msg = "Unhandled Roar emote";                                       // Not allowed when flying.
        else if(lowerContents == "rock")                                        // Rock: Plays rock/paper/scissors, picking rock (displays all three symbols for about 6 seconds, then displays and holds your choice until stance is broken).
            msg = "Unhandled Rock emote";
        else if(lowerContents == "salute")                                      // Salute: A hand-on-forehead salute, fists on hips stance.
            msg = "Unhandled Salute emote";
        else if(lowerContents == "scissors")                                    // Scissors: Plays rock/paper/scissors, picking scissors (displays all three symbols for about 6 seconds, then displays and holds your choice until stance is broken).
            msg = "Unhandled Scissors emote";
        else if(lowerContents == "score1")                                      // Score1: Holds a black on white scorecard up, displaying a 1, holds stance.
            msg = "Unhandled Score1 emote";
        else if(lowerContents == "score2")                                      // Score2: Holds a black on white scorecard up, displaying a 2, holds stance.
            msg = "Unhandled Score2 emote";
        else if(lowerContents == "score3")                                      // Score3: Holds a black on white scorecard up, displaying a 3, holds stance.
            msg = "Unhandled Score3 emote";
        else if(lowerContents == "score4")                                      // Score4: Holds a black on white scorecard up, displaying a 4, holds stance.
            msg = "Unhandled Score4 emote";
        else if(lowerContents == "score5")                                      // Score5: Holds a black on white scorecard up, displaying a 5, holds stance.
            msg = "Unhandled Score5 emote";
        else if(lowerContents == "score6")                                      // Score6: Holds a black on white scorecard up, displaying a 6, holds stance.
            msg = "Unhandled Score6 emote";
        else if(lowerContents == "score7")                                      // Score7: Holds a black on white scorecard up, displaying a 7, holds stance.
            msg = "Unhandled Score7 emote";
        else if(lowerContents == "score8")                                      // Score8: Holds a black on white scorecard up, displaying a 8, holds stance.
            msg = "Unhandled Score8 emote";
        else if(lowerContents == "score9")                                      // Score9: Holds a black on white scorecard up, displaying a 9, holds stance.
            msg = "Unhandled Score9 emote";
        else if(lowerContents == "score10")                                     // Score10: Holds a black on white scorecard up, displaying a 10, holds stance.
            msg = "Unhandled Score10 emote";
        else if(lowerContents == "shucks")                                      // Shucks: Swings fist and head dejectedly, neutral forward facing stance (not the default stance, same as huh/shrug).
            msg = "Unhandled Shucks emote";
        else if(lowerContents == "sit")                                         // Sit: Sits down, legs forward, with knees bent, elbows on knees, and slightly slumped over, stance.
            msg = "Unhandled Sit emote";
        else if(lowerContents == "smack")                                       // Smack: Backhand slap.
            msg = "Unhandled Smack emote";
        else if(lowerContents == "stop" || lowerContents == "raisehand")        // Stop: Raises your right hand above your head, hold stance.
            msg = "Unhandled Stop emote";
        else if(lowerContents == "tarzan" || lowerContents == "beatchest")      // Tarzan: Beats chest and howls, angry-looking stance.
        {
            if(ent->m_is_flying)                                                // Different versions when flying and on the ground.
                msg = "Unhandled flying Tarzan emote";
            else
                msg = "Unhandled ground Tarzan emote";
        }
        else if(lowerContents == "taunt1" || lowerContents == "taunta")         // Taunt1: Taunts, beckoning with one hand, then slaps fist into palm, repeating stance.

        {
            if(ent->m_is_flying)                                                // Different versions when flying and on the ground.
                msg = "Unhandled flying Taunt1 emote";
            else
                msg = "Unhandled ground Taunt1 emote";
        }
        else if(lowerContents == "taunt2" || lowerContents == "taunt"
                || lowerContents == "tauntb")                                   // Taunt2: Taunts, beckoning with both hands, combat stance.
        {
            if(ent->m_is_flying)                                                // Different versions when flying and on the ground.
                msg = "Unhandled flying Taunt2 emote";
            else
                msg = "Unhandled ground Taunt2 emote";
        }
        else if(lowerContents == "thanks" || lowerContents == "thankyou")       // Thanks: Gestures with hand, neutral forward facing stance.
            msg = "Unhandled Thanks emote";
        else if(lowerContents == "thewave")                                     // Thewave: Does the wave (as seen in stadiums at sporting events), neutral facing forward stance.
            msg = "Unhandled Thewave emote";
        else if(lowerContents == "victory")                                     // Victory: Raises hands excitedly, and then again less excitedly, and then a third time almost non-chalantly, falls into neutral forward facing stance.
            msg = "Unhandled Victory emote";
        else if(lowerContents == "wavefist" || lowerContents == "rooting")      // WaveFist (has sound): Waves fist, hoots and then claps (its a cheer), crossed arms stance.
            msg = "Unhandled WaveFist emote";
        else if(lowerContents == "welcome")                                     // Welcome: Open arms welcoming, fists on hips stance.
            msg = "Unhandled Welcome emote";
        else if(lowerContents == "whistle")                                     // Whistle (has sound): Whistles (sounds like a police whistle), ready-stance.
            msg = "Unhandled Whistle emote";
        else if(lowerContents == "winner" || lowerContents == "champion")       // Winner: Fist in fist cheer, right, and then left, neutral forward facing stance.
            msg = "Unhandled Winner emote";
        else if(lowerContents == "yourewelcome")                                // YoureWelcome: Bows head and gestures with hand, neutral forward facing stance.
            msg = "Unhandled YoureWelcome emote";
        else if(lowerContents == "yes" || lowerContents == "thumbsup")          // Yes: Big (literally) thumbs up and an affirmative nod, fists on hips stance.
            msg = "Unhandled Yes emote";
        else if(lowerContents == "yoga" || lowerContents == "lotus")            // Yoga: 	Sits down cross legged with hands on knees/legs, holds stance.
        {
            if(ent->m_is_flying)                                                // Different versions when flying and on the ground.
                msg = "Unhandled flying Yoga emote";
            else
                msg = "Unhandled ground Yoga emote";
        }
                                                                                // Boombox Emotes
        else if(lowerContents.startsWith("bb") && !ent->m_is_flying)            // Check if Boombox Emote.
        {                                                                       // Not allowed when flying.
            lowerContents.replace(0, 2, "");                                    // Remove the "BB" prefix for conciseness.
            if(lowerContents == "altitude")                                     // BBAltitude
                msg = "Unhandled BBAltitude emote";
            else if(lowerContents == "beat")                                    // BBBeat
                msg = "Unhandled BBBeat emote";
            else if(lowerContents == "catchme")                                 // BBCatchMe
                msg = "Unhandled BBCatchMe emote";
            else if(lowerContents == "dance")                                   // BBDance
                msg = "Unhandled BBDance emote";
            else if(lowerContents == "discofreak")                              // BBDiscoFreak
                msg = "Unhandled BBDiscoFreak emote";
            else if(lowerContents == "dogwalk")                                 // BBDogWalk
                msg = "Unhandled BBDogWalk emote";
            else if(lowerContents == "electrovibe")                             // BBElectroVibe
                msg = "Unhandled BBElectroVibe emote";
            else if(lowerContents == "heavydude")                               // BBHeavyDude
                msg = "Unhandled BBHeavyDude emote";
            else if(lowerContents == "infooverload")                            // BBInfoOverload
                msg = "Unhandled BBInfoOverload emote";
            else if(lowerContents == "jumpy")                                   // BBJumpy
                msg = "Unhandled BBJumpy emote";
            else if(lowerContents == "kickit")                                  // BBKickIt
                msg = "Unhandled BBKickIt emote";
            else if(lowerContents == "looker")                                  // BBLooker
                msg = "Unhandled BBLooker emote";
            else if(lowerContents == "meaty")                                   // BBMeaty
                msg = "Unhandled BBMeaty emote";
            else if(lowerContents == "moveon")                                  // BBMoveOn
                msg = "Unhandled BBMoveOn emote";
            else if(lowerContents == "notorious")                               // BBNotorious
                msg = "Unhandled BBNotorious emote";
            else if(lowerContents == "peace")                                   // BBPeace
                msg = "Unhandled BBPeace emote";
            else if(lowerContents == "quickie")                                 // BBQuickie
                msg = "Unhandled BBQuickie emote";
            else if(lowerContents == "raver")                                   // BBRaver
                msg = "Unhandled BBRaver emote";
            else if(lowerContents == "shuffle")                                 // BBShuffle
                msg = "Unhandled BBShuffle emote";
            else if(lowerContents == "spaz")                                    // BBSpaz
                msg = "Unhandled BBSpaz emote";
            else if(lowerContents == "technoid")                                // BBTechnoid
                msg = "Unhandled BBTechnoid emote";
            else if(lowerContents == "venus")                                   // BBVenus
                msg = "Unhandled BBVenus emote";
            else if(lowerContents == "winditup")                                // BBWindItUp
                msg = "Unhandled BBWindItUp emote";
            else if(lowerContents == "wahwah")                                  // BBWahWah
                msg = "Unhandled BBWahWah emote";
            else if(lowerContents == "yellow")                                  // BBYellow
                msg = "Unhandled BBYellow emote";
        }
                                                                                // Unlockable Emotes
                                                                                // TODO: Implement logic and variables for unlocking these emotes.
        else if(lowerContents == "dice7")                                       // Dice7: Picks up, shakes and rolls a die, displays a 7, default stance.
            msg = "Unhandled Dice7 emote";                                      // Unlocked by earning the Burkholder's Bane Badge (from the Ernesto Hess Task Force).
        else if(lowerContents == "listenpoliceband")                            // ListenPoliceBand: Listens in on the heroes' PPD police band radio.
            msg = "Unhandled ListenPoliceBand emote";                           // Heroes can use this without any unlock requirement. For villains, ListenStolenPoliceBand unlocks by earning the Outlaw Badge.
        else if(lowerContents == "snowflakes"
                || lowerContents == "throwsnowflakes")                          // Snowflakes: Throws snowflakes.
            if(ent->m_is_flying)                                                // Different versions when flying and on the ground.
                msg = "Unhandled flying Snowflakes emote";                      // Unlocked by purchasing from the Candy Keeper during the Winter Event.
            else
                msg = "Unhandled ground Snowflakes emote";
    }
    qDebug() << msg;                                                            // Print out the message to the server console.
    info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);                       // Create the message to send to the client.
    src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));     // Print the message to the client.
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
void MapInstance::on_switch_viewpoint(SwitchViewPoint *ev)
{
    qWarning() << "Unhandled switch viewpoint to"<<ev->new_viewpoint_is_firstperson;

}
void MapInstance::on_chat_reconfigured(ChatReconfigure *ev)
{
    qWarning() << "Unhandled chat channel mask setting"<<ev->m_chat_top_flags<<ev->m_chat_bottom_flags;
}
