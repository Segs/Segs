/*
 * Super Entity Game Server
 * http://github.com/Segs
 * Copyright (c) 2006 - 2018 Super Entity Game Server Team (see Authors.txt)
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
#include "EntityStorage.h"
#include "WorldSimulation.h"
#include "InternalEvents.h"
#include "Database.h"
#include "SlashCommand.h"
#include "Common/GameData/CoHMath.h"
#include "Logging.h"

#include <QtCore/QDebug>
#include <QRegularExpression>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <stdlib.h>

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

    qInfo() << "Server running... awaiting client connections."; // best place for this?
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
        case Internal_EventTypes::evExpectClientRequest:
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
        case MapEventTypes::evSaveClientOptions:
            on_client_options(static_cast<SaveClientOptions *>(ev));
            break;
        case MapEventTypes::evDescriptionAndBattleCry:
            on_description_and_battlecry(static_cast<DescriptionAndBattleCry *>(ev));
            break;
        case MapEventTypes::evSetDefaultPowerSend:
            on_set_default_power_send(static_cast<SetDefaultPowerSend *>(ev));
            break;
        case MapEventTypes::evSetDefaultPower:
            on_set_default_power(static_cast<SetDefaultPower *>(ev));
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
        case MapEventTypes::evEntityInfoRequest:
            on_entity_info_request(static_cast<EntityInfoRequest *>(ev));
            break;
        case MapEventTypes::evSelectKeybindProfile:
            on_select_keybind_profile(static_cast<SelectKeybindProfile *>(ev));
            break;
        case MapEventTypes::evSetKeybind:
            on_set_keybind(static_cast<SetKeybind *>(ev));
            break;
        case MapEventTypes::evRemoveKeybind:
            on_remove_keybind(static_cast<RemoveKeybind *>(ev));
            break;
        case MapEventTypes::evResetKeybinds:
            on_reset_keybinds(static_cast<ResetKeybinds *>(ev));
            break;
        default:
            qCWarning(logMapEvents, "Unhandled MapEventTypes %zu\n", ev->type()-MapEventTypes::base);
    }
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
        ent->fillFromCharacter(*ev->char_from_db);
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
        cl->entity(e);
        cl->db_create();
        cl->char_entity(e);

        //        start_idle_timer(cl);
        //cl->start_idle_timer();
    }
    assert(cl->char_entity());

    // Now that we have an entity, fill it.
    CharacterDatabase *char_db = AdminServer::instance()->character_db();
    // TODO: Implement asynchronous database queries
    DbTransactionGuard grd(*char_db->getDb());
    if(false==char_db->fill(cl->char_entity()))
        return;
    grd.commit();

    if(logSpawn().isDebugEnabled())
        cl->char_entity()->dump();

    cl->current_map()->enqueue_client(cl);
    setMapName(*cl->char_entity()->m_char,name());
    setMapIdx(*cl->char_entity(),index());
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
        qCWarning(logMapEvents) << "Bogus on_input_state, client is null";
        return;
    }
    Entity *   ent = cl->char_entity();
    if (st->m_data.has_input_commit_guess)
        ent->m_input_ack = st->m_data.send_id;
    ent->inp_state = st->m_data;
    // Set Target
    ent->m_target_idx = st->m_target_idx;
    ent->m_assist_target_idx = st->m_assist_target_idx;
    // Set Orientation
    if(st->m_data.m_orientation_pyr.p || st->m_data.m_orientation_pyr.y || st->m_data.m_orientation_pyr.r) {
        ent->m_entity_data.m_orientation_pyr = st->m_data.m_orientation_pyr;
        ent->m_direction = fromCoHYpr(ent->m_entity_data.m_orientation_pyr);
    }

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
    {
        qCDebug(logMapEvents) << "bits: " << st->m_user_commands.GetReadableBits();
        qCWarning(logMapEvents) << "Not all bits were consumed";
        assert(false);
    }

    //TODO: do something here !
}
void MapInstance::on_cookie_confirm(CookieRequest * ev)
{
    qDebug("Received cookie confirm %x - %x\n", ev->cookie, ev->console);
}
void MapInstance::on_window_state(WindowState * ev)
{
    // Save GUISettings to character entity and entry in the database.
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *src = lnk->client_data();
    Entity *e = src->char_entity();

    int idx = ev->wnd.m_idx;
    e->m_char->m_gui.m_wnds.at(idx) = ev->wnd;

    qCDebug(logGUI) << "Received window state" << ev->wnd.m_idx << "-" << ev->wnd.m_mode;
    if(logGUI().isDebugEnabled())
        e->m_char->m_gui.m_wnds.at(idx).guiWindowDump();
}
QString process_replacement_strings(MapClient *sender,const QString &msg_text)
{
    /*
    // $$           - newline
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
        "\\$\\$",
        "\\$archetype",
        "\\$battlecry",
        "\\$level",
        "\\$name",
        "\\$origin",
        "\\$target"
    };

    const Character &c(*sender->char_entity()->m_char);

    QString  sender_class       = QString(getClass(c)).remove("Class_");
    QString  sender_battlecry   = getBattleCry(c);
    uint32_t sender_level       = getLevel(c);
    QString  sender_char_name   = c.getName();
    QString  sender_origin      = getOrigin(c);
    uint32_t target_idx         = getTargetIdx(*sender->char_entity());
    QString  target_char_name;

    qCDebug(logChat) << "src -> tgt: " << sender->char_entity()->m_idx  << "->" << target_idx;

    if(target_idx > 0)
    {
        Entity   *tgt    = getEntity(sender,target_idx);
        target_char_name = tgt->name();
    }
    else
        target_char_name = c.getName();

    foreach (const QString &str, replacements) {
        if(str == "\\$archetype")
            new_msg.replace(QRegularExpression(str), sender_class);
        else if(str == "\\$battlecry")
            new_msg.replace(QRegularExpression(str), sender_battlecry);
        else if(str == "\\$level")
            new_msg.replace(QRegularExpression(str), QString::number(sender_level));
        else if(str == "\\$name")
            new_msg.replace(QRegularExpression(str), sender_char_name);
        else if(str == "\\$origin")
            new_msg.replace(QRegularExpression(str), sender_origin);
        else if(str == "\\$target")
            new_msg.replace(QRegularExpression(str), target_char_name);
        else if(str == "\\$\\$")
        {
            if(new_msg.contains(str))
                qCDebug(logChat) << "need to send newline for" << str; // TODO: Need method for returning newline in str
        }
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
static MessageChannel getKindOfChatMessage(const QStringRef &msg)
{
    if(msg=="l" || msg=="local")                                                            // Aliases: local, l
        return MessageChannel::LOCAL;
    if(msg=="b" || msg=="broadcast" || msg=="y" || msg=="yell")                             // Aliases: broadcast, yell, b, y
        return MessageChannel::BROADCAST;
    if(msg=="g" || msg=="group" || msg=="team")                                             // Aliases: team, g, group
        return MessageChannel::TEAM;
    if(msg=="sg" || msg=="supergroup")                                                      // Aliases: sg, supergroup
        return MessageChannel::SUPERGROUP;
    if(msg=="req" || msg=="request" || msg=="auction" || msg=="sell")                       // Aliases: request, req, auction, sell
        return MessageChannel::REQUEST;
    if(msg=="f")                                                                            // Aliases: f
        return MessageChannel::FRIENDS;
    if(msg=="t" || msg=="tell" || msg=="w" || msg=="whisper" || msg=="p" || msg=="private") // Aliases: t, tell, whisper, w, private, p
        return MessageChannel::PRIVATE;
    // unknown chat types are processed as local chat
    return MessageChannel::LOCAL;
}

void MapInstance::process_chat(MapClient *sender,QString &msg_text)
{
    int first_space = msg_text.indexOf(QRegularExpression("\\s"), 0); // first whitespace, as the client sometimes sends tabs
    QString sender_char_name;
    QString prepared_chat_message;

    QStringRef cmd_str(msg_text.midRef(0,first_space));
    QStringRef msg_content(msg_text.midRef(first_space+1,msg_text.lastIndexOf("\n")));
    MessageChannel kind = getKindOfChatMessage(cmd_str);
    std::vector<MapClient *> recipients;

    if(sender && sender->char_entity())
        sender_char_name = sender->char_entity()->name();

    switch(kind)
    {
        case MessageChannel::LOCAL:
        {
            // send only to clients within range
            glm::vec3 senderpos = sender->char_entity()->m_entity_data.m_pos;
            for(MapClient *cl : m_clients)
            {
                glm::vec3 recpos = cl->char_entity()->m_entity_data.m_pos;
                float range = 50.0f; // range of "hearing". I assume this is in yards
                float dist = glm::distance(senderpos,recpos);

                qCDebug(logChat, "senderpos: %f %f %f", senderpos.x, senderpos.y, senderpos.z);
                qCDebug(logChat, "recpos: %f %f %f", recpos.x, recpos.y, recpos.z);
                qCDebug(logChat, "sphere: %f", range);
                qCDebug(logChat, "dist: %f", dist);

                if(dist<=range)
                    recipients.push_back(cl);
            }
            prepared_chat_message = QString("[Local] %1: %2").arg(sender_char_name,msg_content.toString());
            for(MapClient * cl : recipients)
            {
                sendChatMessage(MessageChannel::LOCAL,prepared_chat_message,sender,cl);
            }
            break;
        }
        case MessageChannel::BROADCAST:
        {
            // send the message to everyone on this map
            std::copy(m_clients.begin(),m_clients.end(),std::back_insert_iterator<std::vector<MapClient *>>(recipients));
            prepared_chat_message = QString(" %1: %2").arg(sender_char_name,msg_content.toString()); // where does [Broadcast] come from? The client?
            for(MapClient * cl : recipients)
            {
                sendChatMessage(MessageChannel::BROADCAST,prepared_chat_message,sender,cl);
            }
            break;
        }
        case MessageChannel::REQUEST:
        {
            // send the message to everyone on this map
            std::copy(m_clients.begin(),m_clients.end(),std::back_insert_iterator<std::vector<MapClient *>>(recipients));
            prepared_chat_message = QString(" %1: %2").arg(sender_char_name,msg_content.toString());
            for(MapClient * cl : recipients)
            {
                sendChatMessage(MessageChannel::REQUEST,prepared_chat_message,sender,cl);
            }
            break;
        }
        case MessageChannel::PRIVATE:
        {
            int first_comma = msg_text.indexOf(',');
            QStringRef target_name_ref(msg_text.midRef(first_space+1,(first_comma - first_space-1)));
            msg_content = msg_text.midRef(first_comma+1,msg_text.lastIndexOf("\n"));

            QString target_name = target_name_ref.toString();
            qCDebug(logChat) << "Private Chat:"
                             << "\n\t" << "target_name:" << target_name
                             << "\n\t" << "msg_text:" << msg_text;

            Entity *tgt = getEntity(sender,target_name);

            if(tgt == nullptr)
            {
                prepared_chat_message = QString("No player named \"%1\" currently online.").arg(target_name);
                sendInfoMessage(MessageChannel::USER_ERROR,prepared_chat_message,sender);
                break;
            }
            else
            {
                prepared_chat_message = QString(" -->%1: %2").arg(target_name,msg_content.toString());
                sendChatMessage(MessageChannel::PRIVATE,prepared_chat_message,sender,sender); // in this case, sender is target

                prepared_chat_message = QString(" %1: %2").arg(sender_char_name,msg_content.toString());
                sendChatMessage(MessageChannel::PRIVATE,prepared_chat_message,sender,tgt->m_client);
            }

            break;
        }
        case MessageChannel::TEAM:
        {
            if(!sender->char_entity()->m_has_team)
            {
                prepared_chat_message = "You are not a member of a Team.";
                sendInfoMessage(MessageChannel::USER_ERROR,prepared_chat_message,sender);
                break;
            }

            // Only send the message to characters on sender's team
            for(MapClient *cl : m_clients)
            {
                if(sender->char_entity()->m_team->m_team_idx == cl->char_entity()->m_team->m_team_idx)
                    recipients.push_back(cl);
            }
            prepared_chat_message = QString(" %1: %2").arg(sender_char_name,msg_content.toString());
            for(MapClient * cl : recipients)
            {
                sendChatMessage(MessageChannel::TEAM,prepared_chat_message,sender,cl);
            }
            break;
        }
        case MessageChannel::SUPERGROUP:
        {
            if(!sender->char_entity()->m_has_supergroup)
            {
                prepared_chat_message = "You are not a member of a SuperGroup.";
                sendInfoMessage(MessageChannel::USER_ERROR,prepared_chat_message,sender);
                break;
            }

            // Only send the message to characters in sender's supergroup
            for(MapClient *cl : m_clients)
            {
                if(sender->char_entity()->m_supergroup.m_SG_id == cl->char_entity()->m_supergroup.m_SG_id)
                    recipients.push_back(cl);
            }
            prepared_chat_message = QString(" %1: %2").arg(sender_char_name,msg_content.toString());
            for(MapClient * cl : recipients)
            {
                sendChatMessage(MessageChannel::SUPERGROUP,prepared_chat_message,sender,cl);
            }
            break;
        }
        case MessageChannel::FRIENDS:
        {
            FriendsList * fl = &sender->char_entity()->m_char->m_char_data.m_friendlist;
            if(!fl->m_has_friends || fl->m_friends_count == 0)
            {
                prepared_chat_message = "You don't have any friends to message.";
                sendInfoMessage(MessageChannel::USER_ERROR,prepared_chat_message,sender);
                break;
            }
            // Only send the message to characters in sender's friendslist
            prepared_chat_message = QString(" %1: %2").arg(sender_char_name,msg_content.toString());
            for(Friend &f : fl->m_friends)
            {
                if(f.m_online_status != true)
                    continue;

                Entity *tgt = getEntityByDBID(sender,f.m_db_id);
                if(tgt == nullptr) // In case we didn't toggle online_status.
                    continue;

                sendChatMessage(MessageChannel::FRIENDS,prepared_chat_message,sender,tgt->m_client);
            }
            sendChatMessage(MessageChannel::FRIENDS,prepared_chat_message,sender,sender);
            break;
        }
        default:
        {
            qCDebug(logChat) << "Unhandled MessageChannel type" << int(kind);
            break;
        }
    }
}
void MapInstance::on_console_command(ConsoleCommand * ev)
{
    QString contents = ev->contents.simplified();
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *src = lnk->client_data();
    Entity *ent = src->char_entity(); // user entity

    if(contents.contains("$")) // does it contain replacement strings?
        contents = process_replacement_strings(src, contents);

    //printf("Console command received %s\n",qPrintable(ev->contents));

    if(isChatMessage(contents))
    {
        process_chat(src,contents);
    }
    else if(contents.startsWith("em ",Qt::CaseInsensitive) || contents.startsWith("e ",Qt::CaseInsensitive)
            || contents.startsWith("me ",Qt::CaseInsensitive) || contents.startsWith("emote ",Qt::CaseInsensitive))                                  // ERICEDIT: This encompasses all emotes.
    {
        on_emote_command(contents, ent);
    }
    else {
        runCommand(contents,*ent);
    }
}
void MapInstance::on_emote_command(const QString &command, Entity *ent)
{
    QString msg;                                                                // Initialize the variable to hold the debug message.
    MapClient *src = ent->m_client;
    std::vector<MapClient *> recipients;

    QString cmd_str = command.section(QRegularExpression("\\s+"), 0, 0);
    QString emote_str = command.section(QRegularExpression("\\s+"), 1, -1);
                                                                                // Normal Emotes
    static const QStringList afraidCommands = {"afraid", "cower", "fear", "scared"};
    static const QStringList akimboCommands = {"akimbo", "wings"};
    static const QStringList bigWaveCommands = {"bigwave", "overhere"};
    static const QStringList boomBoxCommands = {"boombox", "bb", "dropboombox"};
    static const QStringList bowCommands = {"bow", "sorry"};
    static const QStringList bowDownCommands = {"bowdown", "down"};
    static const QStringList coinCommands = {"coin", "cointoss", "flipcoin"};
    static const QStringList diceCommands = {"dice", "rolldice"};
    static const QStringList evilLaughCommands = {"evillaugh", "elaugh", "muahahaha", "villainlaugh", "villainouslaugh"};
    static const QStringList fancyBowCommands = {"fancybow", "elegantbow"};
    static const QStringList flex1Commands = {"flex1", "flexa"};
    static const QStringList flex2Commands = {"flex2", "flex", "flexb"};
    static const QStringList flex3Commands = {"flex3", "flexc"};
    static const QStringList hiCommands = {"hi", "wave"};
    static const QStringList hmmCommands = {"hmmm", "plotting"};
    static const QStringList laugh2Commands = {"laugh2", "biglaugh", "laughtoo"};
    static const QStringList martialArtsCommands = {"martialarts", "kata"};
    static const QStringList newspaperCommands = {"newspaper", "afk"};
    static const QStringList noCommands = {"no", "dontattack"};
    static const QStringList plotCommands = {"plot", "scheme"};
    static const QStringList stopCommands = {"stop", "raisehand"};
    static const QStringList tarzanCommands = {"tarzan", "beatchest"};
    static const QStringList taunt1Commands = {"taunt1", "taunta"};
    static const QStringList taunt2Commands = {"taunt2", "taunt", "tauntb"};
    static const QStringList thanksCommands = {"thanks", "thankyou"};
    static const QStringList waveFistCommands = {"wavefist", "rooting"};
    static const QStringList winnerCommands = {"winner", "champion"};
    static const QStringList yesCommands = {"yes", "thumbsup"};
    static const QStringList yogaCommands = {"yoga", "lotus"};
    static const QStringList snowflakesCommands = {"snowflakes", "throwsnowflakes"};

    if(afraidCommands.contains(emote_str, Qt::CaseInsensitive))                                  // Afraid: Cower in fear, hold stance.
    {
        if(ent->m_is_flying)                                                    // Different versions when flying and on the ground.
            msg = "Unhandled flying Afraid emote";
        else
            msg = "Unhandled ground Afraid emote";
    }
    else if(akimboCommands.contains(emote_str, Qt::CaseInsensitive) && !ent->m_is_flying)        // Akimbo: Stands with fists on hips looking forward, hold stance.
        msg = "Unhandled Akimbo emote";                                         // Not allowed when flying.
    else if(emote_str.toLower() == "angry")                                           // Angry: Fists on hips and slouches forward, as if glaring or grumbling, hold stance.
        msg = "Unhandled Angry emote";
    else if(emote_str.toLower() == "atease")                                          // AtEase: Stands in the 'at ease' military position (legs spread out slightly, hands behind back) stance, hold stance.
        msg = "Unhandled AtEase emote";
    else if(emote_str.toLower() == "attack")                                          // Attack: Gives a charge! type point, fists on hips stance.
        msg = "Unhandled Attack emote";
    else if(emote_str.toLower() == "batsmash")                                        // BatSmash: Hit someone or something with a bat, repeat.
        msg = "Unhandled BatSmash emote";
    else if(emote_str.toLower() == "batsmashreact")                                   // BatSmashReact: React as if getting hit with a bat, often used in duo with BatSmash.
        msg = "Unhandled BatSmashReact emote";
    else if(bigWaveCommands.contains(emote_str, Qt::CaseInsensitive))                            // BigWave: Waves over the head, fists on hips stance.
        msg = "Unhandled BigWave emote";
    else if(boomBoxCommands.contains(emote_str, Qt::CaseInsensitive) && !ent->m_is_flying)       // BoomBox (has sound): Summons forth a boombox (it just appears) and leans over to turn it on, stands up and does a sort of dance. A random track will play.
    {                                                                           // Not allowed when flying.
        int rSong = rand() % 25 + 1;                                            // Randomly pick a song.
        switch(rSong)
        {
            case 1:                                                             // 1: BBAltitude
            {
                msg = "1: Unhandled \"BBAltitude\" BoomBox emote";
                break;
            }
            case 2:                                                             // 2: BBBeat
            {
                msg = "2: Unhandled \"BBBeat\" BoomBox emote";
                break;
            }
            case 3:                                                             // 3: BBCatchMe
            {
                msg = "3: Unhandled \"BBCatchMe\" BoomBox emote";
                break;
            }
            case 4:                                                             // 4: BBDance
            {
                msg = "4: Unhandled \"BBDance\" BoomBox emote";
                break;
            }
            case 5:                                                             // 5: BBDiscoFreak
            {
                msg = "5: Unhandled \"BBDiscoFreak\" BoomBox emote";
                break;
            }
            case 6:                                                             // 6: BBDogWalk
            {
                msg = "6: Unhandled \"BBDogWalk\" BoomBox emote";
                break;
            }
            case 7:                                                             // 7: BBElectroVibe
            {
                msg = "7: Unhandled \"BBElectroVibe\" BoomBox emote";
                break;
            }
            case 8:                                                             // 8: BBHeavyDude
            {
                msg = "8: Unhandled \"BBHeavyDude\" BoomBox emote";
                break;
            }
            case 9:                                                             // 9: BBInfoOverload
            {
                msg = "9: Unhandled \"BBInfoOverload\" BoomBox emote";
                break;
            }
            case 10:                                                            // 10: BBJumpy
            {
                msg = "10: Unhandled \"BBJumpy\" BoomBox emote";
                break;
            }
            case 11:                                                            // 11: BBKickIt
            {
                msg = "11: Unhandled \"BBKickIt\" BoomBox emote";
                break;
            }
            case 12:                                                            // 12: BBLooker
            {
                msg = "12: Unhandled \"BBLooker\" BoomBox emote";
                break;
            }
            case 13:                                                            // 13: BBMeaty
            {
                msg = "13: Unhandled \"BBMeaty\" BoomBox emote";
                break;
            }
            case 14:                                                            // 14: BBMoveOn
            {
                msg = "14: Unhandled \"BBMoveOn\" BoomBox emote";
                break;
            }
            case 15:                                                            // 15: BBNotorious
            {
                msg = "15: Unhandled \"BBNotorious\" BoomBox emote";
                break;
            }
            case 16:                                                            // 16: BBPeace
            {
                msg = "16: Unhandled \"BBPeace\" BoomBox emote";
                break;
            }
            case 17:                                                            // 17: BBQuickie
            {
                msg = "17: Unhandled \"BBQuickie\" BoomBox emote";
                break;
            }
            case 18:                                                            // 18: BBRaver
            {
                msg = "18: Unhandled \"BBRaver\" BoomBox emote";
                break;
            }
            case 19:                                                            // 19: BBShuffle
            {
                msg = "19: Unhandled \"BBShuffle\" BoomBox emote";
                break;
            }
            case 20:                                                            // 20: BBSpaz
            {
                msg = "20: Unhandled \"BBSpaz\" BoomBox emote";
                break;
            }
            case 21:                                                            // 21: BBTechnoid
            {
                msg = "21: Unhandled \"BBTechnoid\" BoomBox emote";
                break;
            }
            case 22:                                                            // 22: BBVenus
            {
                msg = "22: Unhandled \"BBVenus\" BoomBox emote";
                break;
            }
            case 23:                                                            // 23: BBWindItUp
            {
                msg = "23: Unhandled \"BBWindItUp\" BoomBox emote";
                break;
            }
            case 24:                                                            // 24: BBWahWah
            {
                msg = "24: Unhandled \"BBWahWah\" BoomBox emote";
                break;
            }
            case 25:                                                            // 25: BBYellow
            {
                msg = "25: Unhandled \"BBYellow\" BoomBox emote";
            }
        }
    }
    else if(bowCommands.contains(emote_str, Qt::CaseInsensitive) && !ent->m_is_flying)           // Bow: Chinese/Japanese style bow with palms together, returns to normal stance.
        msg = "Unhandled Bow emote";                                            // Not allowed when flying.
    else if(bowDownCommands.contains(emote_str, Qt::CaseInsensitive))                            // BowDown: Thrusts hands forward, then points down, as if ordering someone else to bow before you.
        msg = "Unhandled BowDown emote";
    else if(emote_str.toLower() == "burp" && !ent->m_is_flying)                       // Burp (has sound): A raunchy belch, wipes mouth with arm afterward, ape-like stance.
        msg = "Unhandled Burp emote";                                           // Not allowed when flying.
    else if(emote_str.toLower() == "cheer")                                           // Cheer: Randomly does one of 3 cheers, 1 fist raised, 2 fists raised or 2 fists lowered, repeats.
    {
        int rNum = rand() % 3 + 1;                                              // Randomly pick the cheer.
        switch(rNum)
        {
            case 1:                                                             // 1: 1 fist raised
            {
                msg = "1: Unhandled \"1 fist raised\" Cheer emote";
                break;
            }
            case 2:                                                             // 2: 2 fists raised
            {
                msg = "2: Unhandled \"2 fists raised\" Cheer emote";
                break;
            }
            case 3:                                                             // 3: 2 fists lowered
            {
                msg = "3: Unhandled \"2 fists lowered\" Cheer emote";
            }
        }
    }
    else if(emote_str.toLower() == "clap")                                            // Clap (has sound): Claps hands several times, crossed arms stance.
        msg = "Unhandled Clap emote";
    else if(coinCommands.contains(emote_str, Qt::CaseInsensitive))                               // Coin: Flips a coin, randomly displays heads or tails, and hold stance. Coin image remains until stance broken.
    {
        int rFlip = rand() % 2 + 1;                                             // Randomly pick heads or tails.
        switch(rFlip)
        {
            case 1:                                                             // 1: Heads
            {
                msg = "1: Unhandled heads Coin emote";
                break;
            }
            case 2:                                                             // 2: Tails
            {
                msg = "2: Unhandled tails Coin emote";
            }
        }
    }
    else if(emote_str.toLower() == "crossarms" && !ent->m_is_flying)                  // CrossArms: Crosses arms, stance (slightly different from most other crossed arm stances).
        msg = "Unhandled CrossArms emote";                                      // Not allowed when flying.
    else if(emote_str.toLower() == "dance")                                           // Dance: Randomly performs one of six dances.
    {
        int rDance = rand() % 6 + 1;                                            // Randomly pick the dance.
        switch(rDance)
        {
            case 1:                                                             // 1: Dances with elbows by hips.
            {
                msg = "1: Unhandled \"Dances with elbows by hips\" Dance emote";
                break;
            }
            case 2:                                                             // 2: Dances with fists raised.
            {
                msg = "2: Unhandled \"Dances with fists raised\" Dance emote";
                break;
            }
            case 3:                                                             // 3: Swaying hands by hips, aka "Really Bad" dancing.
            {
                msg = "3: Unhandled \"Swaying hands by hips, aka 'Really Bad' dancing\" Dance emote";
                break;
            }
            case 4:                                                             // 4: Swaying hands up in the air, like in a breeze.
            {
                msg = "4: Unhandled \"Swaying hands up in the air, like in a breeze\" Dance emote";
                break;
            }
            case 5:                                                             // 5: As Dance4, but jumping as well.
            {
                msg = "5: Unhandled \"As Dance4, but jumping as well\" Dance emote";
                break;
            }
            case 6:                                                             // 6: The monkey.
            {
                msg = "6: Unhandled \"The monkey\" Dance emote";
            }
        }
    }
    else if(diceCommands.contains(emote_str, Qt::CaseInsensitive))                               // Dice: Picks up, shakes and rolls a die, randomly displays the results (1-6), default stance. Die image quickly fades.
    {
        int rDice = rand() % 6 + 1;                                             // Randomly pick a die result.
        switch(rDice)
        {
            case 1:                                                             // 1: 1
            {
                msg = "1: Unhandled \"1\" Dice emote";
                break;
            }
            case 2:                                                             // 2: 2
            {
                msg = "2: Unhandled \"2\" Dice emote";
                break;
            }
            case 3:                                                             // 3: 3
            {
                msg = "3: Unhandled \"3\" Dice emote";
                break;
            }
            case 4:                                                             // 4: 4
            {
                msg = "4: Unhandled \"4\" Dice emote";
                break;
            }
            case 5:                                                             // 5: 5
            {
                msg = "5: Unhandled \"5\" Dice emote";
                break;
            }
            case 6:                                                             // 6: 6
            {
                msg = "6: Unhandled \"6\" Dice emote";
            }
        }
    }
    else if(emote_str.toLower() == "dice1")                                           // Dice1: Picks up, shakes and rolls a die, displays a 1, default stance.
        msg = "Unhandled Dice1 emote";
    else if(emote_str.toLower() == "dice2")                                           // Dice2: Picks up, shakes and rolls a die, displays a 2, default stance.
        msg = "Unhandled Dice2 emote";
    else if(emote_str.toLower() == "dice3")                                           // Dice3: Picks up, shakes and rolls a die, displays a 3, default stance.
        msg = "Unhandled Dice3 emote";
    else if(emote_str.toLower() == "dice4")                                           // Dice4: Picks up, shakes and rolls a die, displays a 4, default stance.
        msg = "Unhandled Dice4 emote";
    else if(emote_str.toLower() == "dice5")                                           // Dice5: Picks up, shakes and rolls a die, displays a 5, default stance.
        msg = "Unhandled Dice5 emote";
    else if(emote_str.toLower() == "dice6")                                           // Dice6: Picks up, shakes and rolls a die, displays a 6, default stance.
        msg = "Unhandled Dice6 emote";
    else if(emote_str.toLower() == "disagree")                                        // Disagree: Shakes head, crosses hand in front, then offers an alternative, crossed arms stance.
        msg = "Unhandled Disagree emote";
    else if(emote_str.toLower() == "drat")                                            // Drat: Raises fists up, then down, stomping at the same time, same ending stance as Frustrated.
        msg = "Unhandled Drat emote";
    else if(emote_str.toLower() == "explain")                                         // Explain: Hold arms out in a "wait a minute" gesture, motion alternatives, then shrug.
        msg = "Unhandled Explain emote";
    else if(evilLaughCommands.contains(emote_str, Qt::CaseInsensitive))                          // EvilLaugh: Extremely melodramatic, overacted evil laugh.
        msg = "Unhandled EvilLaugh emote";
    else if(fancyBowCommands.contains(emote_str, Qt::CaseInsensitive))                           // FancyBow: A much more elegant, ball-room style bow, falls into neutral forward facing stance.
        msg = "Unhandled FancyBow emote";
    else if(flex1Commands.contains(emote_str, Qt::CaseInsensitive))                              // Flex1: Fists raised, flexing arms stance, hold stance. This is called a "double biceps" pose.
        msg = "Unhandled Flex1 emote";
    else if(flex2Commands.contains(emote_str, Qt::CaseInsensitive))                              // Flex2: A side-stance flexing arms, hold stance. This is a sideways variation on the "most muscular" pose.
        msg = "Unhandled Flex2 emote";
    else if(flex3Commands.contains(emote_str, Qt::CaseInsensitive))                              // Flex3: Another side-stance, flexing arms, hold stance. This is an open variation on the "side chest" pose.
        msg = "Unhandled Flex3 emote";
    else if(emote_str.toLower() == "frustrated")                                      // Frustrated: Raises both fists and leans backwards, shaking fists and head, leads into a quick-breathing angry-looking stance.
        msg = "Unhandled Frustrated emote";
    else if(emote_str.toLower() == "grief")                                           // Grief: Falls to knees, hands on forehead, looks up and gestures a sort of "why me?" look with hands, goes into a sort of depressed slump while on knees, holds stance.
        msg = "Unhandled Grief emote";
    else if(hiCommands.contains(emote_str, Qt::CaseInsensitive))                                 // Hi: Simple greeting wave, fists on hips stance.
        msg = "Unhandled Hi emote";
    else if(hmmCommands.contains(emote_str, Qt::CaseInsensitive))                                // Hmmm: Stare into the sky, rubbing chin, thinking.
        msg = "Unhandled Hmmm emote";
    else if(emote_str.toLower() == "jumpingjacks")                                    // JumpingJacks (has sound): Does jumping jacks, repeats.
        msg = "Unhandled JumpingJacks emote";
    else if(emote_str.toLower() == "kneel")                                           // Kneel: Quickly kneels on both knees with hands on thighs (looks insanely uncomfortable), holds stance.
        msg = "Unhandled Kneel emote";
    else if(emote_str.toLower() == "laugh")                                           // Laugh: Fists on hips, tosses head back and laughs.
        msg = "Unhandled Laugh emote";
    else if(laugh2Commands.contains(emote_str, Qt::CaseInsensitive))                             // Laugh2: Another style of laugh.
        msg = "Unhandled Laugh2 emote";
    else if(emote_str.toLower() == "lecture")                                         // Lecture: Waves/shakes hands in different motions in a lengthy lecture, fists on hips stance.
        msg = "Unhandled Lecture emote";
    else if(martialArtsCommands.contains(emote_str, Qt::CaseInsensitive))                        // MartialArts (has sound): Warm up/practice punches and blocks.
        msg = "Unhandled MartialArts emote";
    else if(emote_str.toLower() == "militarysalute")                                  // MilitarySalute: Stands in the military-style heads-high hand on forehead salute stance.
        msg = "Unhandled MilitarySalute emote";
    else if(newspaperCommands.contains(emote_str, Qt::CaseInsensitive))                          // Newspaper: Materializes a newspaper and reads it.
        msg = "Unhandled Newspaper emote";
    else if(noCommands.contains(emote_str, Qt::CaseInsensitive))                                 // No: Shakes head and waves hands in front of character, crossed arms stance.
        msg = "Unhandled No emote";
    else if(emote_str.toLower() == "nod")                                             // Nod: Fists on hips, nod yes, hold stance.
        msg = "Unhandled Nod emote";
    else if(emote_str.toLower() == "none")                                            // None: Cancels the current emote, if any, and resumes default standing animation cycle.
        msg = "Unhandled None emote";
    else if(emote_str.toLower() == "paper")                                           // Paper: Plays rock/paper/scissors, picking paper (displays all three symbols for about 6 seconds, then displays and holds your choice until stance is broken).
        msg = "Unhandled Paper emote";
    else if(plotCommands.contains(emote_str, Qt::CaseInsensitive))                               // Plot: Rubs hands together while hunched over.
        msg = "Unhandled Plot emote";
    else if(emote_str.toLower() == "point")                                           // Point: Extends left arm and points in direction character is facing, hold stance.
        msg = "Unhandled Point emote";
    else if(emote_str.toLower() == "praise")                                          // Praise: Kneel prostrate and repeatedly bow in adoration.
        msg = "Unhandled Praise emote";
    else if(emote_str.toLower() == "protest")                                         // Protest: Hold hold up one of several randomly selected mostly unreadable protest signs.
        msg = "Unhandled Protest emote";
    else if(emote_str.toLower() == "roar" && !ent->m_is_flying)                       // Roar: Claws air, roaring, ape-like stance.
        msg = "Unhandled Roar emote";                                           // Not allowed when flying.
    else if(emote_str.toLower() == "rock")                                            // Rock: Plays rock/paper/scissors, picking rock (displays all three symbols for about 6 seconds, then displays and holds your choice until stance is broken).
        msg = "Unhandled Rock emote";
    else if(emote_str.toLower() == "salute")                                          // Salute: A hand-on-forehead salute, fists on hips stance.
        msg = "Unhandled Salute emote";
    else if(emote_str.toLower() == "scissors")                                        // Scissors: Plays rock/paper/scissors, picking scissors (displays all three symbols for about 6 seconds, then displays and holds your choice until stance is broken).
        msg = "Unhandled Scissors emote";
    else if(emote_str.toLower() == "score1")                                          // Score1: Holds a black on white scorecard up, displaying a 1, holds stance.
        msg = "Unhandled Score1 emote";
    else if(emote_str.toLower() == "score2")                                          // Score2: Holds a black on white scorecard up, displaying a 2, holds stance.
        msg = "Unhandled Score2 emote";
    else if(emote_str.toLower() == "score3")                                          // Score3: Holds a black on white scorecard up, displaying a 3, holds stance.
        msg = "Unhandled Score3 emote";
    else if(emote_str.toLower() == "score4")                                          // Score4: Holds a black on white scorecard up, displaying a 4, holds stance.
        msg = "Unhandled Score4 emote";
    else if(emote_str.toLower() == "score5")                                          // Score5: Holds a black on white scorecard up, displaying a 5, holds stance.
        msg = "Unhandled Score5 emote";
    else if(emote_str.toLower() == "score6")                                          // Score6: Holds a black on white scorecard up, displaying a 6, holds stance.
        msg = "Unhandled Score6 emote";
    else if(emote_str.toLower() == "score7")                                          // Score7: Holds a black on white scorecard up, displaying a 7, holds stance.
        msg = "Unhandled Score7 emote";
    else if(emote_str.toLower() == "score8")                                          // Score8: Holds a black on white scorecard up, displaying a 8, holds stance.
        msg = "Unhandled Score8 emote";
    else if(emote_str.toLower() == "score9")                                          // Score9: Holds a black on white scorecard up, displaying a 9, holds stance.
        msg = "Unhandled Score9 emote";
    else if(emote_str.toLower() == "score10")                                         // Score10: Holds a black on white scorecard up, displaying a 10, holds stance.
        msg = "Unhandled Score10 emote";
    else if(emote_str.toLower() == "shucks")                                          // Shucks: Swings fist and head dejectedly, neutral forward facing stance (not the default stance, same as huh/shrug).
        msg = "Unhandled Shucks emote";
    else if(emote_str.toLower() == "sit")                                             // Sit: Sits down, legs forward, with knees bent, elbows on knees, and slightly slumped over, stance.
        msg = "Unhandled Sit emote";
    else if(emote_str.toLower() == "smack")                                           // Smack: Backhand slap.
        msg = "Unhandled Smack emote";
    else if(stopCommands.contains(emote_str, Qt::CaseInsensitive))                               // Stop: Raises your right hand above your head, hold stance.
        msg = "Unhandled Stop emote";
    else if(tarzanCommands.contains(emote_str, Qt::CaseInsensitive))                             // Tarzan: Beats chest and howls, angry-looking stance.
    {
        if(ent->m_is_flying)                                                    // Different versions when flying and on the ground.
            msg = "Unhandled flying Tarzan emote";
        else
            msg = "Unhandled ground Tarzan emote";
    }
    else if(taunt1Commands.contains(emote_str, Qt::CaseInsensitive))                             // Taunt1: Taunts, beckoning with one hand, then slaps fist into palm, repeating stance.

    {
        if(ent->m_is_flying)                                                    // Different versions when flying and on the ground.
            msg = "Unhandled flying Taunt1 emote";
        else
            msg = "Unhandled ground Taunt1 emote";
    }
    else if(taunt2Commands.contains(emote_str, Qt::CaseInsensitive))                             // Taunt2: Taunts, beckoning with both hands, combat stance.
    {
        if(ent->m_is_flying)                                                    // Different versions when flying and on the ground.
            msg = "Unhandled flying Taunt2 emote";
        else
            msg = "Unhandled ground Taunt2 emote";
    }
    else if(thanksCommands.contains(emote_str, Qt::CaseInsensitive))                             // Thanks: Gestures with hand, neutral forward facing stance.
        msg = "Unhandled Thanks emote";
    else if(emote_str.toLower() == "thewave")                                         // Thewave: Does the wave (as seen in stadiums at sporting events), neutral facing forward stance.
        msg = "Unhandled Thewave emote";
    else if(emote_str.toLower() == "victory")                                         // Victory: Raises hands excitedly, and then again less excitedly, and then a third time almost non-chalantly, falls into neutral forward facing stance.
        msg = "Unhandled Victory emote";
    else if(waveFistCommands.contains(emote_str, Qt::CaseInsensitive))                           // WaveFist (has sound): Waves fist, hoots and then claps (its a cheer), crossed arms stance.
        msg = "Unhandled WaveFist emote";
    else if(emote_str.toLower() == "welcome")                                         // Welcome: Open arms welcoming, fists on hips stance.
        msg = "Unhandled Welcome emote";
    else if(emote_str.toLower() == "whistle")                                         // Whistle (has sound): Whistles (sounds like a police whistle), ready-stance.
        msg = "Unhandled Whistle emote";
    else if(winnerCommands.contains(emote_str, Qt::CaseInsensitive))                             // Winner: Fist in fist cheer, right, and then left, neutral forward facing stance.
        msg = "Unhandled Winner emote";
    else if(emote_str.toLower() == "yourewelcome")                                    // YoureWelcome: Bows head and gestures with hand, neutral forward facing stance.
        msg = "Unhandled YoureWelcome emote";
    else if(yesCommands.contains(emote_str, Qt::CaseInsensitive))                                // Yes: Big (literally) thumbs up and an affirmative nod, fists on hips stance.
        msg = "Unhandled Yes emote";
    else if(yogaCommands.contains(emote_str, Qt::CaseInsensitive))                               // Yoga: Sits down cross legged with hands on knees/legs, holds stance.
    {
        if(ent->m_is_flying)                                                    // Different versions when flying and on the ground.
            msg = "Unhandled flying Yoga emote";
        else
            msg = "Unhandled ground Yoga emote";
    }
                                                                                // Boombox Emotes
    else if(emote_str.startsWith("bb") && !ent->m_is_flying)                // Check if Boombox Emote.
    {                                                                           // Not allowed when flying.
        emote_str.replace(0, 2, "");                                        // Remove the "BB" prefix for conciseness.
        if(emote_str.toLower() == "altitude")                                         // BBAltitude
            msg = "Unhandled BBAltitude emote";
        else if(emote_str.toLower() == "beat")                                        // BBBeat
            msg = "Unhandled BBBeat emote";
        else if(emote_str.toLower() == "catchme")                                     // BBCatchMe
            msg = "Unhandled BBCatchMe emote";
        else if(emote_str.toLower() == "dance")                                       // BBDance
            msg = "Unhandled BBDance emote";
        else if(emote_str.toLower() == "discofreak")                                  // BBDiscoFreak
            msg = "Unhandled BBDiscoFreak emote";
        else if(emote_str.toLower() == "dogwalk")                                     // BBDogWalk
            msg = "Unhandled BBDogWalk emote";
        else if(emote_str.toLower() == "electrovibe")                                 // BBElectroVibe
            msg = "Unhandled BBElectroVibe emote";
        else if(emote_str.toLower() == "heavydude")                                   // BBHeavyDude
            msg = "Unhandled BBHeavyDude emote";
        else if(emote_str.toLower() == "infooverload")                                // BBInfoOverload
            msg = "Unhandled BBInfoOverload emote";
        else if(emote_str.toLower() == "jumpy")                                       // BBJumpy
            msg = "Unhandled BBJumpy emote";
        else if(emote_str.toLower() == "kickit")                                      // BBKickIt
            msg = "Unhandled BBKickIt emote";
        else if(emote_str.toLower() == "looker")                                      // BBLooker
            msg = "Unhandled BBLooker emote";
        else if(emote_str.toLower() == "meaty")                                       // BBMeaty
            msg = "Unhandled BBMeaty emote";
        else if(emote_str.toLower() == "moveon")                                      // BBMoveOn
            msg = "Unhandled BBMoveOn emote";
        else if(emote_str.toLower() == "notorious")                                   // BBNotorious
            msg = "Unhandled BBNotorious emote";
        else if(emote_str.toLower() == "peace")                                       // BBPeace
            msg = "Unhandled BBPeace emote";
        else if(emote_str.toLower() == "quickie")                                     // BBQuickie
            msg = "Unhandled BBQuickie emote";
        else if(emote_str.toLower() == "raver")                                       // BBRaver
            msg = "Unhandled BBRaver emote";
        else if(emote_str.toLower() == "shuffle")                                     // BBShuffle
            msg = "Unhandled BBShuffle emote";
        else if(emote_str.toLower() == "spaz")                                        // BBSpaz
            msg = "Unhandled BBSpaz emote";
        else if(emote_str.toLower() == "technoid")                                    // BBTechnoid
            msg = "Unhandled BBTechnoid emote";
        else if(emote_str.toLower() == "venus")                                       // BBVenus
            msg = "Unhandled BBVenus emote";
        else if(emote_str.toLower() == "winditup")                                    // BBWindItUp
            msg = "Unhandled BBWindItUp emote";
        else if(emote_str.toLower() == "wahwah")                                      // BBWahWah
            msg = "Unhandled BBWahWah emote";
        else if(emote_str.toLower() == "yellow")                                      // BBYellow
            msg = "Unhandled BBYellow emote";
    }
                                                                                // Unlockable Emotes
                                                                                // TODO: Implement logic and variables for unlocking these emotes.
    else if(emote_str.toLower() == "dice7")                                           // Dice7: Picks up, shakes and rolls a die, displays a 7, default stance.
        msg = "Unhandled Dice7 emote";                                          // Unlocked by earning the Burkholder's Bane Badge (from the Ernesto Hess Task Force).
    else if(emote_str.toLower() == "listenpoliceband")                                // ListenPoliceBand: Listens in on the heroes' PPD police band radio.
        msg = "Unhandled ListenPoliceBand emote";                               // Heroes can use this without any unlock requirement. For villains, ListenStolenPoliceBand unlocks by earning the Outlaw Badge.
    else if(snowflakesCommands.contains(emote_str, Qt::CaseInsensitive))                         // Snowflakes: Throws snowflakes.
    {
        if(ent->m_is_flying)                                                    // Different versions when flying and on the ground.
            msg = "Unhandled flying Snowflakes emote";                          // Unlocked by purchasing from the Candy Keeper during the Winter Event.
        else
            msg = "Unhandled ground Snowflakes emote";
    }
    else                                                                        // If not specific command, output EMOTE message.
    {
        // "CharacterName {emote message}"
        msg = QString("%1 %2").arg(ent->name(),emote_str);
    }

    // send only to clients within range
    glm::vec3 senderpos = src->char_entity()->m_entity_data.m_pos;
    for(MapClient *cl : m_clients)
    {
        glm::vec3 recpos = cl->char_entity()->m_entity_data.m_pos;
        float range = 50.0f; // range of "hearing". I assume this is in yards
        float dist = glm::distance(senderpos,recpos);

        qCDebug(logEmotes, "senderpos: %f %f %f", senderpos.x, senderpos.y, senderpos.z);
        qCDebug(logEmotes, "recpos: %f %f %f", recpos.x, recpos.y, recpos.z);
        qCDebug(logEmotes, "sphere: %f", range);
        qCDebug(logEmotes, "dist: %f", dist);

        if(dist<=range)
            recipients.push_back(cl);
    }
    for(MapClient * cl : recipients)
    {
        sendChatMessage(MessageChannel::EMOTE,msg,src,cl);
        qCDebug(logEmotes) << msg;
    }
}
void MapInstance::on_command_chat_divider_moved(ChatDividerMoved *ev)
{
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *src = lnk->client_data();
    Entity *ent = src->char_entity();

    ent->m_char->m_gui.m_chat_divider_pos = ev->m_position;
    qCDebug(logMapEvents) << "Chat divider moved to " << ev->m_position << " for player" << ent->name();
}
void MapInstance::on_minimap_state(MiniMapState *ev)
{
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *src = lnk->client_data();
    Entity *ent = src->char_entity();

    qCDebug(logMiniMap) << "MiniMapState tile "<<ev->tile_idx << " for player" << ent->name();
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
    sendInfoMessage(MessageChannel::SERVER,QString::fromStdString(welcome_msg),cl);

    sendServerMOTD(cl->char_entity());
}
void MapInstance::on_location_visited(LocationVisited *ev)
{
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *cl = lnk->client_data();
    qCDebug(logMapEvents) << "Attempting a call to script location_visited with:"<<ev->m_name<<qHash(ev->m_name);
    auto val = m_scripting_interface->callFuncWithClientContext(cl,"location_visited",qHash(ev->m_name));
    sendInfoMessage(MessageChannel::DEBUG_INFO,QString::fromStdString(val),cl);

    qCWarning(logMapEvents) << "Unhandled location visited event:" << ev->m_name <<
                  QString("(%1,%2,%3)").arg(ev->m_pos.x).arg(ev->m_pos.y).arg(ev->m_pos.z);
}

void MapInstance::on_plaque_visited(PlaqueVisited * ev)
{
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *cl = lnk->client_data();
    qCDebug(logMapEvents) << "Attempting a call to script plaque_visited with:"<<ev->m_name<<qHash(ev->m_name);
    auto val = m_scripting_interface->callFuncWithClientContext(cl,"plaque_visited",qHash(ev->m_name));
    qCWarning(logMapEvents) << "Unhandled plaque visited event:" << ev->m_name <<
                  QString("(%1,%2,%3)").arg(ev->m_pos.x).arg(ev->m_pos.y).arg(ev->m_pos.z);
}

void MapInstance::on_inspiration_dockmode(InspirationDockMode *ev)
{
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *src = lnk->client_data();
    Entity *ent = src->char_entity();

    ent->m_char->m_gui.m_insps_tray_mode = ev->dock_mode;
    qCDebug(logMapEvents) << "Saving inspirations dock mode to GUISettings:" << ev->dock_mode;
}

void MapInstance::on_enter_door(EnterDoor *ev)
{
    qCWarning(logMapEvents).noquote() << "Unhandled door entry request to:" << ev->name;
    if(ev->unspecified_location)
        qCWarning(logMapEvents).noquote() << "    no location provided";
    else
        qCWarning(logMapEvents).noquote() << ev->location.x<< ev->location.y<< ev->location.z;
    //pseudocode:
    //  auto door = get_door(ev->name,ev->location);
    //  if(door and player_can_enter(door)
    //    process_map_transfer(player,door->targetMap);
}

void MapInstance::on_change_stance(ChangeStance * ev)
{
    qCWarning(logMapEvents) << "Unhandled change stance request";
    if(ev->enter_stance)
        qCWarning(logMapEvents) << "  enter stance" <<ev->powerset_index<<ev->power_index;
    else
        qCWarning(logMapEvents) << "  exit stance";
}

void MapInstance::on_set_destination(SetDestination * ev)
{
    qCWarning(logMapEvents) << "Unhandled set destination request"
               << "\n\t" << "index" << ev->point_index
               << "loc" << ev->destination.x << ev->destination.y << ev->destination.z;
}

void MapInstance::on_abort_queued_power(AbortQueuedPower * ev)
{
    qCWarning(logMapEvents) << "Unhandled abort queued power request";
}

void MapInstance::on_description_and_battlecry(DescriptionAndBattleCry * ev)
{
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *src = lnk->client_data();
    Character &c(*src->char_entity()->m_char);

    setBattleCry(c,ev->battlecry);
    setDescription(c,ev->description);
    qCDebug(logDescription) << "Saving description and battlecry:" << ev->description << ev->battlecry;
}

void MapInstance::on_entity_info_request(EntityInfoRequest * ev)
{
    // Return Description
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *src = lnk->client_data();

    Entity *tgt = getEntity(src,ev->entity_idx);
    if(tgt == nullptr)
    {
        qCDebug(logMapEvents) << "No target active, doing nothing";
        return;
    }

    QString description = getDescription(*tgt->m_char);

    src->addCommandToSendNextUpdate(std::unique_ptr<EntityInfoResponse>(new EntityInfoResponse(description)));
    qCDebug(logDescription) << "Entity info requested" << ev->entity_idx << description;
}

void MapInstance::on_client_options(SaveClientOptions * ev)
{
    // Save options/keybinds to character entity and entry in the database.
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *src = lnk->client_data();
    Entity *e = src->char_entity();

    e->m_char->m_options = ev->data;
    charUpdateOptions(e); // Update database with opts/kbds

    qCDebug(logMapEvents) << "Client options saved to database.";
}

void MapInstance::on_switch_viewpoint(SwitchViewPoint *ev)
{
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *src = lnk->client_data();
    Entity *ent = src->char_entity();

    ent->m_char->m_options.m_first_person_view = ev->new_viewpoint_is_firstperson;
    qCDebug(logMapEvents) << "Saving viewpoint mode to ClientOptions" << ev->new_viewpoint_is_firstperson;
}

void MapInstance::on_chat_reconfigured(ChatReconfigure *ev)
{
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *src = lnk->client_data();
    Entity *ent = src->char_entity();

    ent->m_char->m_gui.m_chat_top_flags = ev->m_chat_top_flags;
    ent->m_char->m_gui.m_chat_bottom_flags = ev->m_chat_bottom_flags;

    qCDebug(logMapEvents) << "Saving chat channel mask settings to GUISettings" << ev->m_chat_top_flags << ev->m_chat_bottom_flags;
}

void MapInstance::on_set_default_power_send(SetDefaultPowerSend *ev)
{
    qCWarning(logMapEvents) << "Unhandled Set Default Power Send request:" << ev->powerset_idx << ev->power_idx;
}

void MapInstance::on_set_default_power(SetDefaultPower *ev)
{
    qCWarning(logMapEvents) << "Unhandled Set Default Power request.";
}

void MapInstance::on_unqueue_all(UnqueueAll *ev)
{
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *src = lnk->client_data();
    Entity *ent = src->char_entity();

    // What else could go here?
    ent->m_target_idx = 0;
    ent->m_assist_target_idx = 0;
    // cancelAttack(ent);

    qCWarning(logMapEvents) << "Incomplete Unqueue all request. Setting Target and Assist Target to 0";
}

void MapInstance::on_target_chat_channel_selected(TargetChatChannelSelected *ev)
{
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *src = lnk->client_data();
    Entity *ent = src->char_entity();

    qCDebug(logMapEvents) << "Saving chat channel type to GUISettings:" << ev->m_chat_type;
    ent->m_char->m_gui.m_cur_chat_channel = ev->m_chat_type;
}

void MapInstance::on_activate_inspiration(ActivateInspiration *ev)
{
    qCWarning(logMapEvents) << "Unhandled use inspiration request." << ev->row_idx << ev->slot_idx;
    // TODO: not sure what the client expects from the server here
}

void MapInstance::on_powers_dockmode(PowersDockMode *ev)
{
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *src = lnk->client_data();
    Entity *ent = src->char_entity();

    ent->m_char->m_gui.m_powers_tray_mode = ev->toggle_secondary_tray;
    qCDebug(logMapEvents) << "Saving powers tray dock mode to GUISettings:" << ev->toggle_secondary_tray;
}

void MapInstance::on_switch_tray(SwitchTray *ev)
{
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *src = lnk->client_data();
    Entity *ent = src->char_entity();

    ent->m_char->m_gui.m_tray1_number = ev->tray1_num;
    ent->m_char->m_gui.m_tray2_number = ev->tray2_num;
    ent->m_char->m_gui.m_tray3_number = ev->tray_unk1;
    qCDebug(logMapEvents) << "Saving Tray States to GUISettings. Tray1:" << ev->tray1_num+1 << "Tray2:" << ev->tray2_num+1 << "Unk1:" << ev->tray_unk1;
    // TODO: need to load powers for new tray.
    qCWarning(logMapEvents) << "TODO: Need to load powers for new trays";
}

void MapInstance::on_set_keybind(SetKeybind *ev)
{
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *src = lnk->client_data();
    Entity *ent = src->char_entity();

    KeyName key = static_cast<KeyName>(ev->key);
    ModKeys mod = static_cast<ModKeys>(ev->mods);

    ent->m_char->m_keybinds.setKeybind(ev->profile, key, mod, ev->command, ev->is_secondary);
    //qCDebug(logMapEvents) << "Setting keybind: " << ev->profile << QString::number(ev->key) << QString::number(ev->mods) << ev->command << ev->is_secondary;
}

void MapInstance::on_remove_keybind(RemoveKeybind *ev)
{
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *src = lnk->client_data();
    Entity *ent = src->char_entity();

    ent->m_char->m_keybinds.removeKeybind(ev->profile,(KeyName &)ev->key,(ModKeys &)ev->mods);
    //qCWarning(logMapEvents) << "Clearing Keybind: " << ev->profile << QString::number(ev->key) << QString::number(ev->mods);
}

void MapInstance::on_reset_keybinds(ResetKeybinds *ev)
{
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *src = lnk->client_data();
    Entity *ent = src->char_entity();

    ent->m_char->m_keybinds.resetKeybinds();
    qCDebug(logMapEvents) << "Resetting Keybinds to defaults.";
}

void MapInstance::on_select_keybind_profile(SelectKeybindProfile *ev)
{
    MapLink * lnk = (MapLink *)ev->src();
    MapClient *src = lnk->client_data();
    Entity *ent = src->char_entity();

    ent->m_char->m_keybinds.setKeybindProfile(ev->profile);
    qCDebug(logMapEvents) << "Saving currently selected Keybind Profile. Profile name: " << ev->profile;
}
