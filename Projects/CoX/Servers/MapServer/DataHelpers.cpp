/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup MapServer Projects/CoX/Servers/MapServer
 * @{
 */

#include "DataHelpers.h"

#include "MapServer.h"
#include "MapInstance.h"
#include "MessageHelpers.h"
#include "GameData/GameDataStore.h"
#include "GameData/ClientStates.h"
#include "GameData/map_definitions.h"
#include "GameData/playerdata_definitions.h"
#include "GameData/power_definitions.h"
#include "Common/GameData/CharacterHelpers.h"
#include "Common/GameData/Character.h"
#include "Common/GameData/Team.h"
#include "Common/GameData/LFG.h"
#include "Common/Messages/Map/ContactList.h"
#include "Common/Messages/Map/EmailHeaders.h"
#include "Common/Messages/Map/EmailRead.h"
#include "Common/Messages/EmailService/EmailEvents.h"
#include "Common/Messages/Map/MapEvents.h"
#include "Logging.h"

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QDebug>
#include <random>

using namespace SEGSEvents;
/*
 * Entity Methods
 */
// Getters
uint32_t    getIdx(const Entity &e) { return e.m_idx; }
uint32_t    getDbId(const Entity &e) { return e.m_db_id; }
uint32_t    getAccessLevel(const Entity &e) { return e.m_entity_data.m_access_level; }
uint32_t    getTargetIdx(const Entity &e) { return e.m_target_idx; }
uint32_t    getAssistTargetIdx(const Entity &e) { return e.m_assist_target_idx; }
glm::vec3   getSpeed(const Entity &e) { return e.m_motion_state.m_speed; }
float       getBackupSpd(const Entity &e) { return e.m_motion_state.m_backup_spd; }
float       getJumpHeight(const Entity &e) { return e.m_motion_state.m_jump_height; }
uint8_t     getUpdateId(const Entity &e) { return e.m_update_id; }
Destination     getCurrentDestination(const Entity &e) { return e.m_cur_destination; }
ClientStates    getStateMode(const Entity &e) { return e.m_state_mode; }

// Setters
void    setDbId(Entity &e, uint8_t val) { e.m_char->m_db_id = val; e.m_db_id = val; }
void    setMapIdx(Entity &e, uint32_t val) { e.m_entity_data.m_map_idx = val; }
void    setSpeed(Entity &e, float v1, float v2, float v3)
{
    // TODO: we really shouldn't modify surface-mods here. But currently this appears
    // to be the only way to adjust speed. This is a work-around until a fix can be found.
    e.m_motion_state.m_surf_mods->max_speed = std::max({v1, v2, v3});
    e.m_motion_state.m_speed = {v1,v2,v3};
}
void    setBackupSpd(Entity &e, float val) { e.m_motion_state.m_backup_spd = val; }
void    setJumpHeight(Entity &e, float val) { e.m_motion_state.m_jump_height = val; }
void    setUpdateID(Entity &e, uint8_t val) { e.m_update_id = val;}

void    setTeamID(Entity &e, uint8_t team_id)
{
    e.m_has_team = team_id != 0;
    if(team_id == 0)
    {
        delete e.m_team;
        e.m_team = nullptr;
    }

    if(!e.m_team)
        return;

    qDebug().noquote() << "Team Info:"
                       << "\n  Has Team:" << e.m_has_team
                       << "\n  ID:" << e.m_team->m_team_idx
                       << "\n  Size:" << e.m_team->m_team_members.size()
                       << "\n  Members:" << e.m_team->m_team_members.data();
}

void    setSuperGroup(Entity &e, int sg_id, QString sg_name, uint32_t sg_rank)
{
    // TODO: provide method for updating SuperGroup Colors
    if(sg_id == 0)
    {
        e.m_has_supergroup          = false;
        e.m_supergroup.m_SG_id      = 0;
        e.m_supergroup.m_SG_name    = "";
        e.m_supergroup.m_SG_color1  = 0x996633FF;
        e.m_supergroup.m_SG_color2  = 0x336699FF;
        e.m_supergroup.m_SG_rank    = 0;
    }
    else
    {
        e.m_has_supergroup          = true;
        e.m_supergroup.m_SG_id      = sg_id;
        e.m_supergroup.m_SG_name    = sg_name;
        e.m_supergroup.m_SG_color1  = 0xAA3366FF;
        e.m_supergroup.m_SG_color2  = 0x66AA33FF;
        e.m_supergroup.m_SG_rank    = sg_rank;
    }
    qDebug().noquote() << "SG Info:"
             << "\n  Has Team:" << e.m_has_supergroup
             << "\n  ID:" << e.m_supergroup.m_SG_id
             << "\n  Name:" << e.m_supergroup.m_SG_name
             << "\n  Color1:" << e.m_supergroup.m_SG_color1
             << "\n  Color2:" << e.m_supergroup.m_SG_color2
             << "\n  Rank:" << e.m_supergroup.m_SG_rank;
}

void setTarget(Entity &e, uint32_t target_idx)
{
    e.m_target_idx = target_idx;
    setAssistTarget(e);
}

void setAssistTarget(Entity &e)
{
    if(getTargetIdx(e) == 0)
    {
        e.m_assist_target_idx = 0;
        return;
    }

    Entity *target_ent = getEntity(e.m_client, getTargetIdx(e));
    if(target_ent == nullptr)
    {
        e.m_assist_target_idx = 0;
        return;
    }

    // TODO: are there any entity types that are invalid assist targets?
    e.m_assist_target_idx = getTargetIdx(*target_ent);

    qCDebug(logTarget) << "Assist Target is:" << getAssistTargetIdx(e);
}

void setCurrentDestination(Entity &e, int point_idx, glm::vec3 location)
{
    e.m_cur_destination.point_idx = point_idx;
    e.m_cur_destination.location = location;
}

// For live debugging
void    setu1(Entity &e, int val) { e.u1 = val; }

// Toggles
void    toggleFlying(Entity &e) { e.m_motion_state.m_is_flying = !e.m_motion_state.m_is_flying; }
void    toggleFalling(Entity &e) { e.m_motion_state.m_is_falling = !e.m_motion_state.m_is_falling; }
void    toggleJumping(Entity &e) { e.m_motion_state.m_is_jumping = !e.m_motion_state.m_is_jumping; }
void    toggleSliding(Entity &e) { e.m_motion_state.m_is_sliding = !e.m_motion_state.m_is_sliding; }

void toggleStunned(Entity &e)
{
    e.m_motion_state.m_is_stunned = !e.m_motion_state.m_is_stunned;
    // TODO: toggle stunned FX above head
}

void toggleJumppack(Entity &e)
{
    e.m_motion_state.m_has_jumppack = !e.m_motion_state.m_has_jumppack;
    // TODO: toggle costume part for jetpack back item.
}

void    toggleControlsDisabled(Entity &e) { e.m_motion_state.m_controls_disabled = !e.m_motion_state.m_controls_disabled; }
void    toggleFullUpdate(Entity &e) { e.m_full_update = !e.m_full_update; }
void    toggleControlId(Entity &e) { e.m_has_control_id = !e.m_has_control_id; }
void    toggleInterp(Entity &e) { e.m_has_interp = !e.m_has_interp; }
void    toggleMoveInstantly(Entity &e) { e.m_move_instantly = !e.m_move_instantly; }
void    toggleTeamBuffs(PlayerData &c) { c.m_gui.m_team_buffs = !c.m_gui.m_team_buffs; }

void toggleLFG(Entity &e)
{
    CharacterData *cd = &e.m_char->m_char_data;

    if(e.m_has_team)
    {
        QString errormsg = "You're already on a team! You cannot toggle LFG.";
        sendInfoMessage(MessageChannel::USER_ERROR, errormsg, *e.m_client);
        errormsg = e.name() + "is already on a team and cannot toggle LFG.";
        qCDebug(logTeams) << errormsg;
        removeLFG(e); // just in-case
        return;
    }

    if(cd->m_lfg)
        removeLFG(e);
    else
    {
        addLFG(e);
        sendTeamLooking(&e);
    }
}

void toggleCollision(Entity &e)
{
    e.m_motion_state.m_no_collision = !e.m_motion_state.m_no_collision;

    if (e.m_motion_state.m_no_collision)
        e.m_move_type |= MoveType::MOVETYPE_NOCOLL;
    else
        e.m_move_type &= ~MoveType::MOVETYPE_NOCOLL;

    qDebug() << "Collision =" << QString::number(e.m_move_type, 2) << e.m_motion_state.m_no_collision;
}

void toggleMovementAuthority(Entity &e)
{
    toggleFullUpdate(e);
    toggleControlId(e);
}


// Misc Methods
void charUpdateDB(Entity *e)
{
    markEntityForDbStore(e,DbStoreFlags::Full);
}


// Poll EntityManager to return Entity by Name or IDX
Entity * getEntity(MapClientSession *src, const QString &name)
{
    MapInstance *mi = src->m_current_map;
    EntityManager &em(mi->m_entities);
    QString errormsg;

    // Iterate through all active entities and return entity by name
    for (Entity* pEnt : em.m_live_entlist)
    {
        if (pEnt->name() == name)
            return pEnt;
    }

    errormsg = "Entity " + name + " does not exist, or is not currently online.";
    qWarning() << errormsg;
    sendInfoMessage(MessageChannel::USER_ERROR, errormsg, *src);
    return nullptr;
}

Entity * getEntity(MapClientSession *src, uint32_t idx)
{
    MapInstance *mi = src->m_current_map;
    EntityManager &em(mi->m_entities);
    QString errormsg;

    if(idx!=0) // Entity idx 0 is special case, so we can't return it
    {
        // Iterate through all active entities and return entity by idx
        for (Entity* pEnt : em.m_live_entlist)
        {
            if (pEnt->m_idx == idx)
                return pEnt;
        }
    }
    errormsg = "Entity " + QString::number(idx) + " does not exist, or is not currently online.";
    qWarning() << errormsg;
    sendInfoMessage(MessageChannel::USER_ERROR, errormsg, *src);
    return nullptr;
}

/**
 * @brief Finds the Entity in the MapInstance
 * @param mi map instance
 * @param db_id db id of the entity to find.
 * @return pointer to the entity or nullptr if it does not exist.
 */
Entity *getEntityByDBID(MapInstance *mi,uint32_t db_id)
{
    EntityManager &em(mi->m_entities);
    QString        errormsg;

    if (db_id == 0)
        return nullptr;
    // TODO: Iterate through all entities in Database and return entity by db_id
    for (Entity *pEnt : em.m_live_entlist)
    {
        if (pEnt->m_db_id == db_id)
            return pEnt;
    }
    return nullptr;
}

void sendServerMOTD(MapClientSession *tgt)
{
    qDebug().noquote() << "Sending Server MOTD to" << tgt->m_ent->m_char->getName();

    QString fileName("scripts/motd.smlx");
    QFile file(fileName);
    if(file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString contents(file.readAll());
        tgt->addCommand<StandardDialogCmd>(contents);
    }
    else {
        QString errormsg = "Failed to load MOTD file. \'" + file.fileName() + "\' not found.";
        qDebug() << errormsg;
        sendInfoMessage(MessageChannel::DEBUG_INFO, errormsg, *tgt);
    }
}

void positionTest(MapClientSession *tgt)
{
    if(tgt->m_ent->m_type != EntType::PLAYER)
        return;

    QString output = "==== Position Test =======================\n";

    output += QString("Move Time: %1\n")
            .arg(tgt->m_ent->m_states.current()->m_move_time, 0, 'f', 1);

    output += QString("Prev Pos <%1, %2, %3>\n")
            .arg(tgt->m_ent->m_motion_state.m_last_pos.x, 0, 'f', 1)
            .arg(tgt->m_ent->m_motion_state.m_last_pos.y, 0, 'f', 1)
            .arg(tgt->m_ent->m_motion_state.m_last_pos.z, 0, 'f', 1);

    output += QString("Server Pos <%1, %2, %3>\n")
            .arg(tgt->m_ent->m_entity_data.m_pos.x, 0, 'f', 1)
            .arg(tgt->m_ent->m_entity_data.m_pos.y, 0, 'f', 1)
            .arg(tgt->m_ent->m_entity_data.m_pos.z, 0, 'f', 1);

    FixedPointValue fpvx(tgt->m_ent->m_entity_data.m_pos.x);
    FixedPointValue fpvy(tgt->m_ent->m_entity_data.m_pos.y);
    FixedPointValue fpvz(tgt->m_ent->m_entity_data.m_pos.z);
    output += QString("Client Pos <%1, %2, %3>\n")
            .arg(fpvx.store)
            .arg(fpvy.store)
            .arg(fpvz.store);

    output += QString("Velocity <%1, %2, %3> @ %4\n")
            .arg(tgt->m_ent->m_motion_state.m_velocity.x, 0, 'f', 1)
            .arg(tgt->m_ent->m_motion_state.m_velocity.y, 0, 'f', 1)
            .arg(tgt->m_ent->m_motion_state.m_velocity.z, 0, 'f', 1)
            .arg(tgt->m_ent->m_motion_state.m_velocity_scale/255, 0, 'f', 1);

    qDebug().noquote() << output;
    sendInfoMessage(MessageChannel::DEBUG_INFO, output, *tgt);
}


/*
 * sendEmail Wrappers for providing access to Email Database
 */
void sendEmailHeaders(MapClientSession &sess)
{
    if(!sess.m_ent->m_client)
    {
        qWarning() << "m_client does not yet exist!";
        return;
    }

    // later on the email id should be auto-incremented from DB
    EmailHeaderRequest* msgToHandler = new EmailHeaderRequest({
                                        sess.m_ent->m_char->m_db_id,
                                        sess.m_ent->m_char->getName(),
                                        "TEST", 576956720},
                sess.link()->session_token());
    EventProcessor* tgt = HandlerLocator::getEmail_Handler();
    tgt->putq(msgToHandler);
}

void sendEmail(MapClientSession& sess, QString recipient_name, QString subject, QString message)
{
    if(!sess.m_ent->m_client)
    {
        qWarning() << "m_client does not yet exist!";
        return;
    }

    uint32_t timestamp = 0;

    EmailSendMessage* msgToHandler = new EmailSendMessage({
                sess.m_ent->m_char->m_db_id,
                sess.m_ent->m_char->getName(),    // -> sender
                recipient_name,
                subject,
                message,
                timestamp},
                sess.link()->session_token());

    HandlerLocator::getEmail_Handler()->putq(msgToHandler);
}

void readEmailMessage(MapClientSession& sess, const uint32_t email_id)
{
    if(!sess.m_ent->m_client)
    {
        qWarning() << "m_client does not yet exist!";
        return;
    }

    EmailReadRequest* msgToHandler = new EmailReadRequest({email_id}, sess.link()->session_token());
    HandlerLocator::getEmail_Handler()->putq(msgToHandler);
}

void deleteEmailHeaders(MapClientSession& sess, const uint32_t email_id)
{
    if(!sess.m_ent->m_client)
    {
        qWarning() << "m_client does not yet exist!";
        return;
    }

    EmailDeleteMessage* msgToHandler = new EmailDeleteMessage({email_id}, sess.link()->session_token());
    HandlerLocator::getEmail_Handler()->putq(msgToHandler);
}

bool isFriendOnline(Entity &src, uint32_t db_id)
{
    // TODO: src is needed for mapclient
    return getEntityByDBID(src.m_client->m_current_map, db_id) != nullptr;
}

void setInterpolationSettings(MapClientSession *sess, const bool active, const uint8_t level, const uint8_t bits)
{
    g_interpolating = active;
    g_interpolation_level = level;
    g_interpolation_bits = bits;
     QString output = QString("Setting Interpolation Settings (active, level, bits): %1, %2, %3")
            .arg(g_interpolating)
            .arg(g_interpolation_level)
            .arg(g_interpolation_bits);
     sendInfoMessage(MessageChannel::DEBUG_INFO, output, *sess);
    qCDebug(logPosition) << output;
}

QString createMapMenu() // TODO: compileMonorailMenu() as well
{
    QString msg_body = "<linkhoverbg #118866aa><link white><linkhover white><table>";
    for (auto &map_data : getAllMapData())
    {
        uint32_t map_idx = map_data.m_map_idx;
        QString map_name = getDisplayMapName(map_idx);
        msg_body.append(QString("<a href=\"cmd:enterdoorvolume %1\"><tr><td>%2</td></tr></a>").arg(map_idx).arg(map_name));
    }
    msg_body.append("</table>");

    return msg_body;
}


/*
 * Titles -- TODO: get titles from texts/English/titles_def
 */
static const QStringList g_generic_titles =
{
    "NULL",
    "Awesome",
    "Bold",
    "Courageous",
    "Daring",
    "Extraordinary",
    "Famous",
    "Gallant",
    "Heroic",
    "Incomparable",
    "Legendary",
    "Magnificent",
    "Outstanding",
    "Powerful",
    "Remarkable",
    "Startling",
    "Terrific",
    "Ultimate",
    "Valiant",
    "Wonderful",
};

// TODO: get titles from texts/English/titles_def
static const QStringList g_origin_titles =
{
    "NULL",
    "Adept",
    "Bright",
    "Curious",
    "Deductiv",
    "Exceptional",
    "Far Seeing",
    "Glorious",
    "Honorable",
    "Indescribable",
    "Lucky",
    "Majestic",
    "Otherworldly",
    "Phenomenal",
    "Redoubtable",
    "Stupendous",
    "Thoughtful",
    "Unearthly",
    "Venturous",
    "Watchful",
};

const QString &getGenericTitle(uint32_t val)
{
    return g_generic_titles.at(val);
}

const QString &getOriginTitle(uint32_t val)
{
    return g_origin_titles.at(val);
}

/*
 * sendInfoMessage wrapper to provide access to NetStructures
 */
void messageOutput(MessageChannel ch, const QString &msg, Entity &tgt)
{
    sendInfoMessage(ch, msg, *tgt.m_client);
}

/*
 * SendUpdate Wrappers to provide access to NetStructures
 */
void sendTimeStateLog(MapClientSession &src, uint32_t control_log)
{
    qCDebug(logSlashCommand, "Sending TimeStateLog %d", control_log);
    src.addCommand<AddTimeStateLog>(control_log);
}

void sendTimeUpdate(MapClientSession &src, int32_t sec_since_jan_1_2000)
{
    qCDebug(logSlashCommand, "Sending TimeUpdate %d", sec_since_jan_1_2000);
    src.addCommand<TimeUpdate>(sec_since_jan_1_2000);
}

void sendClientState(MapClientSession &sess, ClientStates client_state)
{
    qCDebug(logSlashCommand) << "Sending ClientState:" << uint8_t(client_state);
    sess.addCommand<SetClientState>(client_state);
}

void showMapXferList(MapClientSession &sess, bool has_location, glm::vec3 &location, QString &name)
{
    sess.m_ent->m_is_using_mapmenu = true;
    qCDebug(logSlashCommand) << "Showing MapXferList:" << name;
    sess.addCommand<MapXferList>(has_location, location, name);
}

void sendFloatingInfo(MapClientSession &tgt, QString &msg, FloatingInfoStyle style, float delay)
{
    qCDebug(logSlashCommand) << "Sending FloatingInfo:" << msg;
    tgt.addCommand<FloatingInfo>(tgt.m_ent->m_idx, msg, style, delay);
}

void sendFloatingNumbers(MapClientSession &src, uint32_t tgt_idx, int32_t amount)
{
    qCDebug(logSlashCommand, "Sending %d FloatingNumbers from %d to %d", amount, src.m_ent->m_idx, tgt_idx);
    src.addCommand<FloatingDamage>(src.m_ent->m_idx, tgt_idx, amount);
}

void sendLevelUp(MapClientSession &src)
{
    //qCDebug(logSlashCommand) << "Sending LevelUp:" << src.m_idx;
    src.addCommand<LevelUp>();
}

void sendEnhanceCombineResponse(Entity *tgt, bool success, bool destroy)
{
    //qCDebug(logSlashCommand) << "Sending CombineEnhanceResponse:" << tgt->m_idx;
    tgt->m_client->addCommandToSendNextUpdate(std::unique_ptr<CombineEnhanceResponse>(new CombineEnhanceResponse(success, destroy)));
}

void sendChangeTitle(Entity *tgt, bool select_origin)
{
    //qCDebug(logSlashCommand) << "Sending ChangeTitle Dialog:" << tgt->m_idx << "select_origin:" << select_origin;
    tgt->m_client->addCommandToSendNextUpdate(std::unique_ptr<ChangeTitle>(new ChangeTitle(select_origin)));
}

void sendTrayAdd(Entity *tgt, uint32_t pset_idx, uint32_t pow_idx)
{
    qCDebug(logSlashCommand) << "Sending TrayAdd:" << tgt->m_idx << pset_idx << pow_idx;
    tgt->m_client->addCommandToSendNextUpdate(std::unique_ptr<TrayAdd>(new TrayAdd(pset_idx, pow_idx)));
}

void sendFriendsListUpdate(Entity *src, const FriendsList &friends_list)
{
    qCDebug(logFriends) << "Sending FriendsList Update.";
    src->m_client->addCommandToSendNextUpdate(std::unique_ptr<FriendsListUpdate>(new FriendsListUpdate(friends_list)));
}

void sendSidekickOffer(Entity *tgt, uint32_t src_db_id)
{
    qCDebug(logTeams) << "Sending Sidekick Offer" << tgt->name() << "from" << src_db_id;
    tgt->m_client->addCommandToSendNextUpdate(std::unique_ptr<SidekickOffer>(new SidekickOffer(src_db_id)));
}

void sendTeamLooking(Entity *tgt)
{
    std::vector<LFGMember> list = g_lfg_list;

    qCDebug(logLFG) << "Sending Team Looking to" << tgt->name();
    tgt->m_client->addCommandToSendNextUpdate(std::unique_ptr<TeamLooking>(new TeamLooking(list)));
}

void sendTeamOffer(Entity *src, Entity *tgt)
{
    QString name        = src->name();
    uint32_t db_id      = tgt->m_db_id;
    TeamOfferType type  = NoMission;

    // Check for mission, send appropriate TeamOfferType
    if(src->m_has_team && src->m_team != nullptr)
        if(src->m_team->m_team_has_mission)
            type = WithMission; // TODO: Check for invalid missions to send `LeaveMission` instead

    qCDebug(logTeams) << "Sending Teamup Offer" << db_id << name << type;
    tgt->m_client->addCommandToSendNextUpdate(std::unique_ptr<TeamOffer>(new TeamOffer(db_id, name, type)));
}

void sendFaceEntity(Entity &src, int32_t tgt_idx)
{
    qCDebug(logOrientation) << QString("Sending Face Entity to %1").arg(tgt_idx);
    src.m_client->addCommandToSendNextUpdate(std::unique_ptr<FaceEntity>(new FaceEntity(tgt_idx)));
}

void sendFaceLocation(Entity &src, glm::vec3 &loc)
{
    qCDebug(logOrientation) << QString("Sending Face Location to x: %1 y: %2 z: %3").arg(loc.x).arg(loc.y).arg(loc.z);
    src.m_client->addCommandToSendNextUpdate(std::unique_ptr<FaceLocation>(new FaceLocation(loc)));
}

void sendDoorMessage(MapClientSession &tgt, uint32_t delay_status, QString &msg)
{
    qCDebug(logMapXfers).noquote() << QString("Sending Door Message; delay: %1; msg: %2").arg(delay_status).arg(msg);
    tgt.addCommand<DoorMessage>(DoorMessageStatus(delay_status), msg);
}

void sendBrowser(MapClientSession &tgt, QString &content)
{
    qCDebug(logMapEvents) << QString("Sending Browser");
    tgt.addCommand<Browser>(content);
}

void sendTradeOffer(const Entity& src, Entity& tgt)
{
    const QString &name = src.name();
    const uint32_t db_id = tgt.m_db_id;

    qCDebug(logTrades) << "Sending Trade Offer" << db_id << name;
    tgt.m_client->addCommandToSendNextUpdate(std::make_unique<SEGSEvents::TradeOffer>(db_id, name));
}

void sendTradeInit(Entity& src, Entity& tgt)
{
    const uint32_t src_db_id = src.m_db_id;
    const uint32_t tgt_db_id = tgt.m_db_id;

    qCDebug(logTrades) << "Sending Trade Init" << src_db_id << tgt_db_id;
    tgt.m_client->addCommandToSendNextUpdate(std::make_unique<SEGSEvents::TradeInit>(src_db_id));
    src.m_client->addCommandToSendNextUpdate(std::make_unique<SEGSEvents::TradeInit>(tgt_db_id));
}

void sendTradeCancel(Entity& ent, const QString& msg)
{
    qCDebug(logTrades) << "Sending Trade Cancel" << msg;
    ent.m_client->addCommandToSendNextUpdate(std::make_unique<SEGSEvents::TradeCancel>(msg));
}

void sendTradeUpdate(Entity& src, Entity& tgt, const TradeMember& trade_src, const TradeMember& trade_tgt)
{
    qCDebug(logTrades) << "Sending Trade Update";
    src.m_client->addCommandToSendNextUpdate(std::make_unique<SEGSEvents::TradeUpdate>(trade_src, trade_tgt, tgt));
    tgt.m_client->addCommandToSendNextUpdate(std::make_unique<SEGSEvents::TradeUpdate>(trade_tgt, trade_src, src));
}

void sendTradeSuccess(Entity& src, Entity& tgt)
{
    const QString msg_src = "Trade with " + tgt.name() + " was a success.";
    const QString msg_tgt = "Trade with " + src.name() + " was a success.";

    qCDebug(logTrades) << "Sending Trade Success";
    src.m_client->addCommandToSendNextUpdate(std::make_unique<SEGSEvents::TradeSuccess>(msg_src));
    tgt.m_client->addCommandToSendNextUpdate(std::make_unique<SEGSEvents::TradeSuccess>(msg_tgt));
}

void sendContactDialog(MapClientSession &src, QString msg_body, std::vector<ContactEntry> active_contacts)
{
    qCDebug(logSlashCommand) << "Sending ContactDialog:" << msg_body;
    src.addCommand<ContactDialog>(msg_body, active_contacts);
}

void sendContactDialogYesNoOk(MapClientSession &src, QString msg_body, bool has_yesno)
{
    qCDebug(logSlashCommand) << "Sending ContactDialogYesNo:" << has_yesno << msg_body;

    if(has_yesno)
        src.addCommand<ContactDialogYesNo>(msg_body);
    else
        src.addCommand<ContactDialogOk>(msg_body);
}

void sendContactDialogClose(MapClientSession &src)
{
    qCDebug(logSlashCommand) << "Sending ContactDialogClose";
    src.addCommand<ContactDialogClose>();
}

void updateContactStatusList(MapClientSession &src, const Contact &updated_contact_data)
{
    vContactList contacts = src.m_ent->m_char->m_char_data.m_contacts;
    //find contact
    bool found = false;

    for (Contact & contact : contacts)
    {
        if(contact.m_npc_id == updated_contact_data.m_npc_id)
        {
            found = true;
            //contact already in list, update contact;
            contact = updated_contact_data;
            break;
        }
    }

    if(!found)
        contacts.push_back(updated_contact_data);

    //update database contactList
    src.m_ent->m_char->m_char_data.m_contacts = contacts;
    qCDebug(logScripts) << "Sending Character Contact Database updated";

    //Send contactList to client
    src.addCommandToSendNextUpdate(std::unique_ptr<ContactStatusList>(new ContactStatusList(contacts)));
    qCDebug(logScripts) << "Sending ContactStatusList";
}

void sendContactStatusList(MapClientSession &src)
{
    vContactList contacts = src.m_ent->m_char->m_char_data.m_contacts;
    //Send contactList to client
    src.addCommandToSendNextUpdate(std::unique_ptr<ContactStatusList>(new ContactStatusList(contacts)));
    qCDebug(logScripts) << "Sending ContactStatusList";
}

void sendWaypoint(MapClientSession &src, int point_idx, glm::vec3 &location)
{
    qCDebug(logSlashCommand) << QString("Sending SendWaypoint: %1 <%2, %3, %4>")
                                .arg(point_idx)
                                .arg(location.x, 0, 'f', 1)
                                .arg(location.y, 0, 'f', 1)
                                .arg(location.z, 0, 'f', 1);

    src.addCommand<SendWaypoint>(point_idx, location);
}

void sendStance(MapClientSession &src, PowerStance stance)
{
    qCDebug(logSlashCommand) << "Sending new PowerStance";
    src.addCommand<SendStance>(stance);
}

void sendDeadNoGurney(MapClientSession &sess)
{
    qCDebug(logSlashCommand) << "Sending new PowerStance";
    sess.addCommand<DeadNoGurney>();
}

void sendDoorAnimStart(MapClientSession &sess, glm::vec3 &entry_pos, glm::vec3 &target_pos, bool has_anims, QString &seq_state)
{
    qCDebug(logSlashCommand).noquote()
        << QString("Sending DoorAnimStart: entry<%1, %2, %3>  target<%4, %5, %6>  has_anims: %7  seq_state: %8")
               .arg(entry_pos.x, 0, 'f', 1)
               .arg(entry_pos.y, 0, 'f', 1)
               .arg(entry_pos.z, 0, 'f', 1)
               .arg(target_pos.x, 0, 'f', 1)
               .arg(target_pos.y, 0, 'f', 1)
               .arg(target_pos.z, 0, 'f', 1)
               .arg(has_anims)
               .arg(seq_state);

    sess.addCommand<DoorAnimStart>(entry_pos, target_pos, has_anims, seq_state);
}

void sendDoorAnimExit(MapClientSession &sess, bool force_move)
{
    qCDebug(logSlashCommand) << "Sending DoorAnimExit" << force_move;
    sess.addCommand<DoorAnimExit>(force_move);
}


/*
 * usePower here to provide access to messageOutput
 */
void usePower(Entity &ent, uint32_t pset_idx, uint32_t pow_idx, int32_t tgt_idx, int32_t tgt_id)
{
    QString from_msg, to_msg;
    CharacterPower * ppower = nullptr;
    ppower = getOwnedPowerByVecIdx(ent, pset_idx, pow_idx);
    const Power_Data powtpl = ppower->getPowerTemplate();

    if(ppower == nullptr || powtpl.m_Name.isEmpty())
        return;

    // Only dump powers if logPowers() is enabled
    //if(logPowers().isDebugEnabled())
        //dumpPower(*ppower);

    //treat toggle as clicks, ignore everything else for now
    if (powtpl.Type != PowerType::Toggle && powtpl.Type != PowerType::Click)
        return;

    // Target IDX of -1 is actually SELF
    if(tgt_idx == -1)
        tgt_idx = getIdx(ent);

    // Get target and check that it's valid
    Entity *target_ent = getEntity(ent.m_client, tgt_idx);
    if(target_ent == nullptr)
    {
        qCDebug(logPowers) << "Failed to find target:" << tgt_idx << tgt_id;
        return;
    }

    // TODO: Check for PVP flags
    // if(src.can_pvp && tgt.can_pvp) pvp_ok = true;

    // Check Range -- TODO: refactor as checkRange() and checkTarget()
    // self targeting doesn't need these checks
    // we can check EntsAffected for StoredEntsEnum::CASTER here
    if (powtpl.Range == float(0.0))
    {
        target_ent = &ent;
        tgt_idx = ent.m_idx;
    }
    else
    {
        // TODO: source should target last permitted target or
        // if on a team, target target_ent's target instead
        // of sending error messages.
        // if(!targetValid(target)) target = last_valid_target;

        // Consider if target is valid target or
        // if target isn't villian, but needs to be
        if(powtpl.Target == StoredEntEnum::Enemy
                || powtpl.Target == StoredEntEnum::Foe
                || powtpl.Target == StoredEntEnum::NPC)
        {
            if(!target_ent->m_is_villian /*&& !pvp_ok*/)
            {
                messageOutput(MessageChannel::COMBAT, QString("You cannot target allies with this power."), ent);
                return;
            }
        }
        else
        {
            if(target_ent->m_is_villian)
            {
                messageOutput(MessageChannel::COMBAT, QString("You must target allies with this power."), ent);
                return;
            }
        }

        // Check if the target is in range
        glm::vec3 senderpos = ent.m_entity_data.m_pos;
        glm::vec3 recpos = target_ent->m_entity_data.m_pos;

        if (glm::distance(senderpos,recpos) > powtpl.Range)
        {
            from_msg = FloatingInfoMsg.find(FloatingMsg_OutOfRange).value();
            sendFloatingInfo(*ent.m_client, from_msg, FloatingInfoStyle::FloatingInfo_Info, 0.0);
            return;
        }

        // Face towards your target
        sendFaceEntity(ent, tgt_idx);
    }

    // Send PowerStance to client
    PowerStance pstance;
    pstance.has_stance = true;
    pstance.pset_idx = pset_idx;
    pstance.pow_idx = pow_idx;
    ent.m_stance = pstance;
    sendStance(*ent.m_client, pstance);

    // Clear old moves and add TriggeredMove to queue
    ent.m_triggered_moves.clear();
    for(auto bits : powtpl.AttackBits)
    {
        // TODO: pull from stored FX name and lookup idx
        // for now, send bits again
        addTriggeredMove(ent, uint32_t(bits), powtpl.m_AttackFrames, uint32_t(bits));
    }

    // Check and set endurance based upon end cost
    // TODO: refactor as checkEnduranceCost()
    float endurance = getEnd(*ent.m_char);
    float end_cost = powtpl.EnduranceCost;

    qCDebug(logPowers) << "Endurance Cost" << end_cost << "/" << endurance;
    if(end_cost > endurance)
    {
        from_msg = FloatingInfoMsg.find(FloatingMsg_NotEnoughEndurance).value();
        sendFloatingInfo(*ent.m_client, from_msg, FloatingInfoStyle::FloatingInfo_Info, 0.0);
        return;
    }

    setEnd(*ent.m_char, endurance-end_cost);

    // Queue power and add to recharge queue if necessary
    // TODO: refactor as queuePower()
    QueuedPowers qpowers;
    qpowers.m_pow_idxs = {pset_idx, pow_idx};
    qpowers.m_active_state_change   = true;
    qpowers.m_activation_state      = true;
    qpowers.m_timer_updated         = true;
    qpowers.m_time_to_activate      = powtpl.TimeToActivate;
    qpowers.m_recharge_time         = powtpl.RechargeTime;
    qpowers.m_activate_period       = powtpl.ActivatePeriod;

    // Add to queues
    ent.m_queued_powers.push_back(qpowers); // Activation Queue
    ent.m_recharging_powers.push_back(qpowers); // Recharging Queue

    // TODO: Refactor this out
    QStringList fly_names = {
        "Combat_Flight",
        "Fly",
        "Group_Fly",
    };
    if(fly_names.contains(powtpl.m_Name, Qt::CaseInsensitive))
    {
        toggleFlying(ent);

        if(getSpeed(ent) == glm::vec3(1.0f, 1.0f, 1.0f))
            setSpeed(ent, 5.0f, 5.0f, 5.0f);
        else
            setSpeed(ent, 1.0f, 1.0f, 1.0f);
    }

    // TODO: Refactor this out
    if(powtpl.m_Name == "Super_Speed")
    {
        if(getSpeed(ent) == glm::vec3(1.0f, 1.0f, 1.0f))
            setSpeed(ent, 5.0f, 5.0f, 5.0f);
        else
            setSpeed(ent, 1.0f, 1.0f, 1.0f);
    }

    // If there are charges remaining, use them.
    if(ppower->m_is_limited && ppower->m_charges_remaining)
        --ppower->m_charges_remaining;

    // was power temporary with no charges remaining?
    // if so, remove it and update powers array
    if(ppower->m_is_limited && !ppower->m_charges_remaining)
    {
        ppower->m_erase_power = true; // mark for removal
        ent.m_char->m_char_data.m_reset_powersets = true; // powerset array has changed
    }

    // Update Powers to Client to show Recharging/Timers/Etc in UI
    ent.m_char->m_char_data.m_has_updated_powers = true; // this is really important!

    if(powtpl.Target == StoredEntEnum::Enemy || powtpl.Target == StoredEntEnum::Foe
            || powtpl.Target == StoredEntEnum::NPC)
    {
        //roll to hit
        int roll = rand()%100;
        int chance = int(powtpl.Accuracy * 75);

        qCDebug(logPowers) << "Power hit chance: " << roll << " / " << chance;
        if (roll > chance)
        {
            from_msg = "miss";
            sendFloatingInfo(*ent.m_client, from_msg, FloatingInfoStyle::FloatingInfo_Info, 0.0);
            return;
        }

    }

    // TODO: check for auto hit EntsAutoHit for StoredEntsEnum::CASTER
    //   if (powtpl.EffectArea == StoredAffectArea::Character)//single target so do the following once
    //   else                                                // AOE has to check all valid targets, and do the following

    if(!powtpl.pAttribMod.empty())
    {
        // effects done here
        for(uint32_t i = 0; i<powtpl.pAttribMod.size(); i++)
        {
            QByteArray lower_name = powtpl.pAttribMod[i].name.toLower();
            if (lower_name == "Damage")
            {
                // Deal Damage
                sendFloatingNumbers(*ent.m_client, tgt_idx, powtpl.pAttribMod[i].Magnitude);
                setHP(*target_ent->m_char, getHP(*target_ent->m_char)-powtpl.pAttribMod[i].Magnitude);

                // Build target specific messages
                from_msg = QString("You deal %1 damage with ")
                        .arg(powtpl.pAttribMod[i].Magnitude) + powtpl.m_Name;

                to_msg   = QString("%1 hit you for %2 damage with ")
                        .arg(ent.name())
                        .arg(powtpl.pAttribMod[i].Magnitude)  + powtpl.m_Name;
            }
            else if ("Healing" == lower_name)
            {
                // Do Healing
                sendFloatingNumbers(*ent.m_client, tgt_idx, powtpl.pAttribMod[i].Magnitude);
                setHP(*target_ent->m_char, getHP(*target_ent->m_char)+powtpl.pAttribMod[i].Magnitude);

                // Build target specific messages
                from_msg = QString("You heal %1 for %2 with ")
                        .arg(target_ent->name())
                        .arg(powtpl.pAttribMod[i].Magnitude)
                        + powtpl.m_Name;

                to_msg   = QString("%1 heals you for %2 with ")
                        .arg(target_ent->name())
                        .arg(powtpl.pAttribMod[i].Magnitude)
                        + powtpl.m_Name;
            }
            else
            {
                // TODO: buffs, debuffs, CC, summons, etc
                to_msg = QString("%1 %2 from %3")
                        .arg(powtpl.pAttribMod[i].Magnitude)
                        .arg(QString(powtpl.pAttribMod[i].name))
                        .arg(QString(powtpl.m_Name));

                if (powtpl.pAttribMod[i].Duration > 0)
                    to_msg.append(QString(" for a duration of %1").arg(powtpl.pAttribMod[i].Duration));

                // Build target specific messages
                from_msg = QString("You cause ").append(to_msg);
                to_msg.prepend("You receive ");
            }

            // Send message to source combat window
            messageOutput(MessageChannel::COMBAT, from_msg, ent);

            // Send message to another player target
            if(target_ent->m_type == EntType::PLAYER && tgt_idx != ent.m_idx)
                messageOutput(MessageChannel::DAMAGE, to_msg, *target_ent);
        }
    }

    // TODO: Do actual power animations. For now, show silly message to source.
    QStringList batman_kerpow{"AIEEE!", "ARRRGH!", "AWKKKKKK!", "BAM!", "BANG!", "BAP!",
                     "BIFF!", "BLOOP!", "BLURP!", "BOFF!", "BONK!", "CLANK!",
                     "CLASH!", "CLUNK!", "CRAAACK!", "CRASH!", "CRUNCH!", "EEE-YOW!",
                     "FLRBBBBB!", "GLIPP!", "GLURPP!", "KAPOW!", "KER-PLOP!", "KLONK!",
                     "KRUNCH!", "OOOFF!", "OUCH!", "OWWW!", "PAM!", "PLOP!",
                     "POW!", "POWIE!", "QUNCKKK!", "RAKKK!", "RIP!", "SLOSH!",
                     "SOCK!", "SPLAAT!", "SWAAP!", "SWISH!", "SWOOSH!", "THUNK!",
                     "THWACK!", "THWAPP!", "TOUCHÉ!", "UGGH!", "URKK!", "VRONK!",
                     "WHACK!", "WHAMM!", "WHAP!", "ZAM!", "ZAP!", "ZGRUPPP!",
                     "ZLONK!", "ZLOPP!", "ZLOTT!", "ZOK!", "ZOWIE!", "ZWAPP!"};

    std::random_device rng;
    std::mt19937 urng(rng());
    std::shuffle(batman_kerpow.begin(), batman_kerpow.end(), urng);
    QString floating_msg = batman_kerpow.first();
    QString console_msg;
    assert(ent.m_client);
    sendFloatingInfo(*ent.m_client, floating_msg, FloatingInfoStyle::FloatingInfo_Attention, 4.0);

    // Handle powers without any attrib mod data
    // If we've manually assigned power info to these powers, skip the rest.
    // TODO: Everything below will go away when we have actual power behaviors in Lua scripts
    if(!powtpl.pAttribMod.empty() || tgt_idx == ent.m_idx)
        return;

    // calculate damage
    float damage = 1.0f;

    // Send message to source
    console_msg = floating_msg + " You hit " + target_ent->name() + " for " + QString::number(damage) + " damage!";
    messageOutput(MessageChannel::COMBAT, console_msg, ent);

    // Deal Damage
    sendFloatingNumbers(*ent.m_client, tgt_idx, damage);
    setHP(*target_ent->m_char, getHP(*target_ent->m_char)-damage);

    // If not a player, skip the rest
    if(target_ent->m_type != EntType::PLAYER)
        return;

    // Send message to target
    assert(target_ent->m_client);

    sendFloatingInfo(*target_ent->m_client, floating_msg, FloatingInfoStyle::FloatingInfo_Attention, 4.0);
    console_msg = floating_msg + " You were hit by " + ent.name() + " for " + QString::number(damage) + " damage!";
    messageOutput(MessageChannel::COMBAT, console_msg, *target_ent);
}

void increaseLevel(Entity &ent)
{
    setLevel(*ent.m_char, getLevel(*ent.m_char)+1);
    // increase security level every time we visit a trainer and level up
    ++ent.m_char->m_char_data.m_security_threat;
    ent.m_char->m_in_training = false; // we're done training

    QString contents = FloatingInfoMsg.find(FloatingMsg_Leveled).value();
    sendFloatingInfo(*ent.m_client, contents, FloatingInfoStyle::FloatingInfo_Attention, 4.0);
}


/*
 * Team related helpers
 */
void findTeamMember(Entity &tgt)
{
    sendTeamLooking(&tgt);
}

/*
 * Lua Functions
 */
//

void addNpc(MapClientSession &sess, QString &name, glm::vec3 &loc, int variation)
{
    const NPCStorage & npc_store(getGameData().getNPCDefinitions());
    const Parse_NPC * npc_def = npc_store.npc_by_name(&name);
    if(!npc_def)
    {
        sendInfoMessage(MessageChannel::USER_ERROR, "No NPC definition for: " + name, sess);
        return;
    }

    int idx = npc_store.npc_idx(npc_def);
    Entity *e = sess.m_current_map->m_entities.CreateNpc(getGameData(), *npc_def, idx, variation);

    forcePosition(*e, loc);
    sendInfoMessage(MessageChannel::DEBUG_INFO, QString("Created npc with ent idx:%1 at location x: %2 y: %3 z: %4").arg(e->m_idx).arg(loc.x).arg(loc.y).arg(loc.z), sess);
}

void addNpcWithOrientation(MapClientSession &sess, QString &name, glm::vec3 &loc, int variation, glm::vec3 &ori)
{
    const NPCStorage & npc_store(getGameData().getNPCDefinitions());
    const Parse_NPC * npc_def = npc_store.npc_by_name(&name);
    if(!npc_def)
    {
        sendInfoMessage(MessageChannel::USER_ERROR, "No NPC definition for: " + name, sess);
        return;
    }

    int idx = npc_store.npc_idx(npc_def);
    Entity *e = sess.m_current_map->m_entities.CreateNpc(getGameData(), *npc_def, idx, variation);

    forcePosition(*e, loc);
    forceOrientation(*e, ori);
    sendInfoMessage(MessageChannel::DEBUG_INFO, QString("Created npc with ent idx:%1 at location x: %2 y: %3 z: %4").arg(e->m_idx).arg(loc.x).arg(loc.y).arg(loc.z), sess);
}

void giveEnhancement(MapClientSession &sess, QString &name, int e_level)
{
    CharacterData &cd = sess.m_ent->m_char->m_char_data;
    uint32_t level = e_level;
    QString msg = "You do not have room for any more enhancements!";

    if(getNumberEnhancements(cd) < 10)
    {
        msg = "Awarding Enhancement '" + name + "' to " + sess.m_ent->name();
        addEnhancementByName(cd, name, level);

        QString floating_msg = FloatingInfoMsg.find(FloatingMsg_FoundEnhancement).value();
        sendFloatingInfo(sess, floating_msg, FloatingInfoStyle::FloatingInfo_Attention, 4.0);
    }
    qCDebug(logScripts()).noquote() << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void giveDebt(MapClientSession &sess, int debt)
{
    uint32_t current_debt = getDebt(*sess.m_ent->m_char);
    uint32_t debt_to_give = current_debt + debt;
    setDebt(*sess.m_ent->m_char, debt_to_give);
    QString msg = "Setting Debt to " + QString::number(debt_to_give);
    qCDebug(logScripts) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void giveEnd(MapClientSession &sess, float end)
{
    float current_end = getEnd(*sess.m_ent->m_char);
    float end_to_set = current_end + end;
    setEnd(*sess.m_ent->m_char, end_to_set);
    QString msg = QString("Setting Endurance to: %1").arg(end_to_set);
    qCDebug(logScripts) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void giveHp(MapClientSession &sess, float hp)
{
    float current_hp = getHP(*sess.m_ent->m_char);
    float hp_to_set = current_hp + hp;
    setHP(*sess.m_ent->m_char, hp_to_set);
    QString msg = QString("Setting HP to: %1").arg(hp_to_set);
    qCDebug(logScripts) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void giveInf(MapClientSession &sess, int inf)
{
    uint32_t current_inf = getInf(*sess.m_ent->m_char);
    uint32_t inf_to_set = current_inf + inf;
    setInf(*sess.m_ent->m_char, inf_to_set);
    sendInfoMessage(MessageChannel::DEBUG_INFO, QString("Setting inf to %1").arg(inf_to_set), sess);
}

void giveInsp(MapClientSession &sess, QString &name)
{
    CharacterData &cd = sess.m_ent->m_char->m_char_data;
    QString msg = "You do not have room for any more inspirations!";

    if(getNumberInspirations(cd) < getMaxNumberInspirations(cd))
    {
        msg = "Awarding Inspiration '" + name + "' to " + sess.m_ent->name();

        addInspirationByName(cd, name);

        // NOTE: floating message shows no message here, but plays the awarding insp sound!
        QString floating_msg = FloatingInfoMsg.find(FloatingMsg_FoundInspiration).value();
        sendFloatingInfo(sess, floating_msg, FloatingInfoStyle::FloatingInfo_Attention, 4.0);
    }

    qCDebug(logScripts).noquote() << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void giveXp(MapClientSession &sess, int xp)
{
    uint32_t lvl = getLevel(*sess.m_ent->m_char);
    uint32_t current_xp = getXP(*sess.m_ent->m_char);

    // Calculate XP - Debt difference by server settings?

    uint32_t current_debt = getDebt(*sess.m_ent->m_char);
    if(current_debt > 0)
    {
        uint32_t debt_to_pay = 0;
        uint32_t half_xp = xp / 2;
        if(current_debt > half_xp)
        {
            debt_to_pay = half_xp; //Half to debt
            xp = half_xp;
        }
        else
        {
            debt_to_pay = current_debt;
            xp = xp - current_debt;
        }
        uint32_t newDebt = current_debt - debt_to_pay;
        setDebt(*sess.m_ent->m_char, newDebt);
        sendInfoMessage(MessageChannel::DEBUG_INFO, QString("You paid %1 to your debt").arg(debt_to_pay), sess);
    }
    uint32_t xp_to_give = current_xp + xp;
    setXP(*sess.m_ent->m_char, xp_to_give);
    sendInfoMessage(MessageChannel::DEBUG_INFO, QString("You were awarded %1 XP").arg(xp), sess);
    QString msg = "Setting XP to " + QString::number(xp_to_give);
    uint32_t new_lvl = getLevel(*sess.m_ent->m_char);
    sendInfoMessage(MessageChannel::DEBUG_INFO, QString("You were lvl %1 now %2").arg(lvl).arg(new_lvl), sess);

    //This check doesn't show level change
    if(new_lvl != lvl)
    {
        sess.addCommand<FloatingInfo>(sess.m_ent->m_idx, FloatingInfoMsg.find(FloatingMsg_Leveled).value(), FloatingInfo_Attention , 4.0);
        msg += " and LVL to " + QString::number(new_lvl);
    }
    qCDebug(logScripts) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}


//! @}
