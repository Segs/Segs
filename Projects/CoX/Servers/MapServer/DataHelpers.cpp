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
#include "GameData/GameDataStore.h"
#include "GameData/playerdata_definitions.h"
#include "GameData/map_definitions.h"
#include "NetStructures/CharacterHelpers.h"
#include "NetStructures/Character.h"
#include "NetStructures/Team.h"
#include "NetStructures/LFG.h"
#include "Events/EmailHeaders.h"
#include "Events/EmailRead.h"
#include "Servers/GameServer/EmailEvents.h"
#include "MapEvents.h"
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
glm::vec3   getSpeed(const Entity &e) { return e.m_spd; }
float       getBackupSpd(const Entity &e) { return e.m_backup_spd; }
float       getJumpHeight(const Entity &e) { return e.m_jump_height; }
uint8_t     getUpdateId(const Entity &e) { return e.m_update_id; }

// Setters
void    setDbId(Entity &e, uint8_t val) { e.m_char->m_db_id = val; e.m_db_id = val; }
void    setMapIdx(Entity &e, uint32_t val) { e.m_entity_data.m_map_idx = val; }
void    setSpeed(Entity &e, float v1, float v2, float v3) { e.m_spd = {v1,v2,v3}; }
void    setBackupSpd(Entity &e, float val) { e.m_backup_spd = val; }
void    setJumpHeight(Entity &e, float val) { e.m_jump_height = val; }
void    setUpdateID(Entity &e, uint8_t val) { e.m_update_id = val;}

void    setTeamID(Entity &e, uint8_t team_id)
{
    if(team_id == 0)
    {
        e.m_has_team            = false;
        delete e.m_team;
        e.m_team = nullptr;
    }
    else
        e.m_has_team            = true;

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

// For live debugging
void    setu1(Entity &e, int val) { e.u1 = val; }

// Toggles
void    toggleFlying(Entity &e) { e.m_is_flying = !e.m_is_flying; }
void    toggleFalling(Entity &e) { e.m_is_falling = !e.m_is_falling; }
void    toggleJumping(Entity &e) { e.m_is_jumping = !e.m_is_jumping; }
void    toggleSliding(Entity &e) { e.m_is_sliding = !e.m_is_sliding; }

void toggleStunned(Entity &e)
{
    e.m_is_stunned = !e.m_is_stunned;
    // TODO: toggle stunned FX above head
}

void toggleJumppack(Entity &e)
{
    e.m_has_jumppack = !e.m_has_jumppack;
    // TODO: toggle costume part for jetpack back item.
}

void    toggleControlsDisabled(Entity &e) { e.m_controls_disabled = !e.m_controls_disabled; }
void    toggleFullUpdate(Entity &e) { e.m_full_update = !e.m_full_update; }
void    toggleControlId(Entity &e) { e.m_has_control_id = !e.m_has_control_id; }
void    toggleExtraInfo(Entity &e) { e.m_extra_info = !e.m_extra_info; }
void    toggleMoveInstantly(Entity &e) { e.m_move_instantly = !e.m_move_instantly; }

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
        StandardDialogCmd *dlg = new StandardDialogCmd(contents);
        tgt->addCommandToSendNextUpdate(std::unique_ptr<StandardDialogCmd>(dlg));
    }
    else {
        QString errormsg = "Failed to load MOTD file. \'" + file.fileName() + "\' not found.";
        qDebug() << errormsg;
        sendInfoMessage(MessageChannel::DEBUG_INFO, errormsg, *tgt);
    }
}

void on_awaiting_dead_no_gurney_test(MapClientSession &session)
{
    session.m_ent->m_client->addCommandToSendNextUpdate(std::unique_ptr<DeadNoGurney>(new DeadNoGurney()));
}

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

void sendEmail(MapClientSession& sess, uint32_t recipient_id, QString subject, QString message)
{
    if(!sess.m_ent->m_client)
    {
        qWarning() << "m_client does not yet exist!";
        return;
    }

    EmailSendMessage* msgToHandler = new EmailSendMessage(
                EmailSendData({sess.m_ent->m_char->m_db_id,
                               recipient_id,
                               sess.m_ent->m_char->getName(),    // -> sender
                               subject, message, 0}),
                sess.link()->session_token());

    HandlerLocator::getEmail_Handler()->putq(msgToHandler);
}

void readEmailMessage(MapClientSession& sess, const int id)
{
    if(!sess.m_ent->m_client)
    {
        qWarning() << "m_client does not yet exist!";
        return;
    }

    QString message = "Email ID \n" + QString::number(id);

    EmailRead *emailRead = new EmailRead(
                id,
                message,
                sess.m_ent->m_client->m_name);

    sess.m_ent->m_client->addCommandToSendNextUpdate(std::unique_ptr<EmailRead>(emailRead));

    /*
    EmailReadMessage* msgToHandler = new EmailReadMessage(
                EmailReadData({e->m_client, id}));
    HandlerLocator::getEmail_Handler()->putq(msgToHandler);
    */
}

void deleteEmailHeaders(MapClientSession& sess, const int id)
{
    if(!sess.m_ent->m_client)
    {
        qWarning() << "m_client does not yet exist!";
        return;
    }
}

/*
 * Character Methods
 */
// TODO: get titles from texts/English/titles_def
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

QString getMapPath(const EntityData &ed)
{
    return getMapPath(ed.m_map_idx);
}

QString getEntityDisplayMapName(const EntityData &ed)
{
    return getDisplayMapName(ed.m_map_idx);
}

const QString &getFriendDisplayMapName(const Friend &f)
{
    static const QString offline(QStringLiteral("OFFLINE"));
    if (!f.m_online_status)
        return offline;
    return getDisplayMapName(f.m_map_idx);
}

void toggleTeamBuffs(PlayerData &c) { c.m_gui.m_team_buffs = !c.m_gui.m_team_buffs; }

/*
 * Looking For Group
 */
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

/*
 * getMapServerData Wrapper to provide access to NetStructures
 */
GameDataStore *getMapServerData()
{
    return &g_GlobalMapServer->runtimeData();
}

/*
 * sendInfoMessage wrapper to provide access to NetStructures
 */
void messageOutput(MessageChannel ch, QString &msg, Entity &tgt)
{
    sendInfoMessage(ch, msg, *tgt.m_client);
}

/*
 * SendUpdate Wrappers to provide access to NetStructures
 */
void sendClientState(MapClientSession &ent, ClientStates client_state)
{
    qCDebug(logSlashCommand) << "Sending ClientState:" << QString::number(client_state);
    ent.addCommand<SetClientState>(client_state);
}

void showMapXferList(MapClientSession &ent, bool has_location, glm::vec3 &location, QString &name)
{
    qCDebug(logSlashCommand) << "Showing MapXferList:" << name;
    ent.addCommand<MapXferList>(has_location, location, name);
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

void sendLevelUp(Entity *tgt)
{
    //qCDebug(logSlashCommand) << "Sending LevelUp:" << tgt->m_idx;
    tgt->m_client->addCommandToSendNextUpdate(std::unique_ptr<LevelUp>(new LevelUp()));
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

void sendFaceEntity(Entity *src, uint8_t tgt_idx)
{
    qCDebug(logOrientation) << QString("Sending Face Entity to %1").arg(tgt_idx);
    src->m_client->addCommandToSendNextUpdate(std::unique_ptr<FaceEntity>(new FaceEntity(tgt_idx)));
}

void sendFaceLocation(Entity *src, glm::vec3 &loc)
{
    qCDebug(logOrientation) << QString("Sending Face Location to x: %1 y: %2 z: %3").arg(loc.x).arg(loc.y).arg(loc.z);
    src->m_client->addCommandToSendNextUpdate(std::unique_ptr<FaceLocation>(new FaceLocation(loc)));
}

void sendDoorMessage(MapClientSession &tgt, uint32_t delay_status, QString &msg)
{
    qCDebug(logMapXfers) << QString("Sending Door Message; delay: %1 msg: %2").arg(delay_status).arg(msg);
    tgt.addCommand<DoorMessage>(DoorMessageStatus(delay_status), msg);
}

void usePower(Entity &ent, uint32_t pset_idx, uint32_t pow_idx, uint32_t tgt_idx, uint32_t tgt_id)
{
    // Add to activepowers queue
    CharacterPower * ppower = nullptr;
    ppower = getOwnedPower(ent, pset_idx, pow_idx);
    if(ppower != nullptr && !ppower->m_name.isEmpty())
        ent.m_queued_powers.push_back(ppower);

    float endurance = getEnd(*ent.m_char);
    float end_cost = std::max(ppower->m_power_tpl.EnduranceCost, 1.0f);

    qCDebug(logPowers) << "Endurance Cost" << end_cost << "/" << endurance;
    if(end_cost > endurance)
    {
        QString msg = "Not enough endurance to use power" + ppower->m_name;
        messageOutput(MessageChannel::DEBUG_INFO, msg, ent);
        return;
    }

    setEnd(*ent.m_char, endurance-end_cost);

    // TODO: Do actual power stuff. For now, be silly.
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

    if(tgt_idx == ent.m_idx) // Skip the rest if targeting self.
        return;

    Entity *target_ent = getEntity(ent.m_client, tgt_idx);
    if(target_ent == nullptr)
    {
        qCDebug(logPowers) << "Failed to find target:" << tgt_idx << tgt_id;
        return;
    }

    // calculate damage
    float damage = 1.0f;

    // Send message to source
    console_msg = floating_msg + " You hit " + target_ent->name() + " for " + QString::number(damage) + " damage!";
    messageOutput(MessageChannel::COMBAT, console_msg, ent);

    if(target_ent->m_type != EntType::PLAYER)
        return;

    // Send message to target
    assert(target_ent->m_client);

    sendFloatingInfo(*target_ent->m_client, floating_msg, FloatingInfoStyle::FloatingInfo_Attention, 4.0);
    console_msg = floating_msg + " You were hit by " + ent.name() + " for " + QString::number(damage) + " damage!";
    messageOutput(MessageChannel::COMBAT, console_msg, *target_ent);

    // Deal Damage
    sendFloatingNumbers(*ent.m_client, tgt_idx, damage);
    setHP(*target_ent->m_char, getHP(*target_ent->m_char)-damage);
}
void addFriend(Entity &src, Entity &tgt)
{
    QString msg;
    FriendsList &src_data(src.m_char->m_char_data.m_friendlist);

    if(src_data.m_friends_count >= g_max_friends)
    {
        msg = "You cannot have more than " + QString::number(g_max_friends) + " friends.";
        qCDebug(logFriends).noquote() << msg;
        messageOutput(MessageChannel::USER_ERROR, msg, src);
        return; // break early
    }

    src_data.m_has_friends = true;
    src_data.m_friends_count++;

    Friend f;
    f.m_online_status   = (tgt.m_client != nullptr); // need some other method for this.
    f.m_db_id           = tgt.m_db_id;
    f.m_name            = tgt.name();
    f.m_class_idx       = tgt.m_entity_data.m_class_idx;
    f.m_origin_idx      = tgt.m_entity_data.m_origin_idx;
    f.m_map_idx         = tgt.m_entity_data.m_map_idx;
    f.m_mapname         = getEntityDisplayMapName(tgt.m_entity_data);

    // add to friendlist
    src_data.m_friends.emplace_back(f);
    qCDebug(logFriends) << "friendslist size:" << src_data.m_friends_count << src_data.m_friends.size();

    msg = "Adding " + tgt.name() + " to your friendlist.";
    qCDebug(logFriends).noquote() << msg;
    messageOutput(MessageChannel::FRIENDS, msg, src);

    if(logFriends().isDebugEnabled())
        dumpFriends(src);

    sendFriendsListUpdate(&src, src_data); // Send FriendsListUpdate
}

void removeFriend(Entity &src, QString friend_name)
{
    QString msg;
    FriendsList &src_data(src.m_char->m_char_data.m_friendlist);

    qCDebug(logFriends) << "Searching for friend" << friend_name << "to remove them.";

    QString lower_name = friend_name.toLower();
    auto iter = std::find_if( src_data.m_friends.begin(), src_data.m_friends.end(),
                              [lower_name](const Friend& f)->bool {return lower_name==f.m_name.toLower();});

    if(iter!=src_data.m_friends.end())
    {
        msg = "Removing " + iter->m_name + " from your friends list.";
        iter = src_data.m_friends.erase(iter);

        qCDebug(logFriends) << msg;
        if(logFriends().isDebugEnabled())
            dumpFriends(src);
    }
    else
        msg = friend_name + " is not on your friends list.";

    if(src_data.m_friends.empty())
        src_data.m_has_friends = false;

    src_data.m_friends_count = src_data.m_friends.size();

    qCDebug(logFriends).noquote() << msg;
    messageOutput(MessageChannel::FRIENDS, msg, src);

    // Send FriendsListUpdate
    sendFriendsListUpdate(&src, src_data);
}

bool isFriendOnline(Entity &src, uint32_t db_id)
{
    // TODO: src is needed for mapclient
    return getEntityByDBID(src.m_client->m_current_map, db_id) != nullptr;
}


void findTeamMember(Entity &tgt)
{
    sendTeamLooking(&tgt);
}
bool inviteTeam(Entity &src, Entity &tgt)
{
    if(src.name() == tgt.name())
    {
        qCDebug(logTeams) << "You cannot invite yourself to a team.";
        return false;
    }

    if(!src.m_has_team)
    {
        qCDebug(logTeams) << src.name() << "is forming a team.";
        src.m_team = new Team;
        src.m_team->addTeamMember(&src);

        tgt.m_team = src.m_team;
        src.m_team->addTeamMember(&tgt);
        return true;
    }
    else if (src.m_has_team && src.m_team->isTeamLeader(&src))
    {
        src.m_team->addTeamMember(&tgt);
        return true;
    }
    else
    {
        qCDebug(logTeams) << src.name() << "is not team leader.";
        return false;
    }

    qCWarning(logTeams) << "How did we get here in inviteTeam?";
    return false;
}

bool kickTeam(Entity &tgt)
{
    if (!tgt.m_has_team)
        return false;

    removeTeamMember(*tgt.m_team,&tgt);
    return true;
}

void leaveTeam(Entity &e)
{
    if(!e.m_team)
    {
        qCWarning(logTeams) << "Trying to leave a team, but Entity has no team!?";
        return;
    }

    removeTeamMember(*e.m_team,&e);
}

/*
 * Sidekick Methods -- Sidekick system requires teaming.
 */
// TODO: expose these to config and fail-test
static const int g_max_sidekick_level_difference = 3;
static const int g_min_sidekick_mentor_level = 10;

bool isSidekickMentor(const Entity &e)
{
    return (e.m_char->m_char_data.m_sidekick.m_type == SidekickType::IsMentor);
}

void inviteSidekick(Entity &src, Entity &tgt)
{
    const QString possible_messages[] = {
        QStringLiteral("Unable to add sidekick."),
        QStringLiteral("To Mentor another player, you must be at least 3 levels higher than them."),
        QStringLiteral("To Mentor another player, you must be at least level 10."),
        QStringLiteral("You are already Mentoring someone."),
        tgt.name() + QStringLiteral("is already a sidekick."),
        QStringLiteral("To Mentor another player, you must be on the same team."),
    };

    QString     msg = possible_messages[0];
    Sidekick    &src_sk = src.m_char->m_char_data.m_sidekick;
    Sidekick    &tgt_sk = tgt.m_char->m_char_data.m_sidekick;
    uint32_t    src_lvl = getLevel(*src.m_char);
    uint32_t    tgt_lvl = getLevel(*tgt.m_char);

    // Only a mentor may invite a sidekick
    if(src_lvl < tgt_lvl+g_max_sidekick_level_difference)
        msg = possible_messages[1];
    else if(src_lvl < g_min_sidekick_mentor_level)
        msg = possible_messages[2];
    else if(src_sk.m_has_sidekick)
        msg = possible_messages[3];
    else if (tgt_sk.m_has_sidekick)
        msg = possible_messages[4];
    else if(!src.m_has_team || !tgt.m_has_team || src.m_team == nullptr || tgt.m_team == nullptr)
        msg = possible_messages[5];
    else if(src.m_team->m_team_idx != tgt.m_team->m_team_idx)
        msg = possible_messages[5];
    else
    {
        // Store this here now for sidekick_accept / decline
        tgt_sk.m_db_id = src.m_db_id;

        // sendSidekickOffer
        sendSidekickOffer(&tgt, src.m_db_id); // tgt gets dialog, src.db_id is named.
        return; // break early
    }

    qCDebug(logTeams).noquote() << msg;
    messageOutput(MessageChannel::USER_ERROR, msg, src);
}

void addSidekick(Entity &tgt, Entity &src)
{
    QString     msg;
    Sidekick    &src_sk = src.m_char->m_char_data.m_sidekick;
    Sidekick    &tgt_sk = tgt.m_char->m_char_data.m_sidekick;
    uint32_t    src_lvl = getLevel(*src.m_char);

    src_sk.m_has_sidekick = true;
    tgt_sk.m_has_sidekick = true;
    src_sk.m_db_id = tgt.m_db_id;
    tgt_sk.m_db_id = src.m_db_id;
    src_sk.m_type = SidekickType::IsMentor;
    tgt_sk.m_type = SidekickType::IsSidekick;
    setCombatLevel(*tgt.m_char, src_lvl - 1);
    // TODO: Implement 225 feet "leash" for sidekicks.

    msg = QString("%1 is now Mentoring %2.").arg(src.name(),tgt.name());
    qCDebug(logTeams).noquote() << msg;

    // Send message to each player
    msg = QString("You are now Mentoring %1.").arg(tgt.name()); // Customize for src.
    messageOutput(MessageChannel::TEAM, msg, src);
    msg = QString("%1 is now Mentoring you.").arg(src.name()); // Customize for src.
    messageOutput(MessageChannel::TEAM, msg, tgt);
}

void removeSidekick(Entity &src)
{
    QString     msg = "Unable to remove sidekick.";
    Sidekick    &src_sk = src.m_char->m_char_data.m_sidekick;

    if(!src_sk.m_has_sidekick || src_sk.m_db_id == 0)
    {
        msg = "You are not sidekicked with anyone.";
        qCDebug(logTeams).noquote() << msg;
        messageOutput(MessageChannel::USER_ERROR, msg, src);
        return; // break early
    }
    assert(false);
    //TODO: this function should actually post messages related to de-sidekicking to our target entity.
    Entity      *tgt            = nullptr; //getEntityByDBID(src_sk.m_db_id);
    Sidekick    &tgt_sk         = tgt->m_char->m_char_data.m_sidekick;

    if(tgt == nullptr)
    {
        msg = "Your sidekick is not currently online.";
        qCDebug(logTeams).noquote() << msg;

        // reset src Sidekick relationship
        src_sk.m_has_sidekick = false;
        src_sk.m_type         = SidekickType::NoSidekick;
        src_sk.m_db_id        = 0;
        setCombatLevel(*src.m_char,getLevel(*src.m_char)); // reset CombatLevel

        return; // break early
    }

    // Anyone can terminate a Sidekick relationship
    if(!tgt_sk.m_has_sidekick || (tgt_sk.m_db_id != src.m_db_id))
    {
        // tgt doesn't know it's sidekicked with src. So clear src sidekick info.
        src_sk.m_has_sidekick = false;
        src_sk.m_type         = SidekickType::NoSidekick;
        src_sk.m_db_id        = 0;
        setCombatLevel(*src.m_char,getLevel(*src.m_char)); // reset CombatLevel
        msg = QString("You are no longer sidekicked with anyone.");
    }
    else {

        // Send message to each player
        if(isSidekickMentor(src))
        {
            // src is mentor, tgt is sidekick
            msg = QString("You are no longer mentoring %1.").arg(tgt->name());
            messageOutput(MessageChannel::TEAM, msg, src);
            msg = QString("%1 is no longer mentoring you.").arg(src.name());
            messageOutput(MessageChannel::TEAM, msg, *tgt);
        }
        else
        {
            // src is sidekick, tgt is mentor
            msg = QString("You are no longer mentoring %1.").arg(src.name());
            messageOutput(MessageChannel::TEAM, msg, *tgt);
            msg = QString("%1 is no longer mentoring you.").arg(tgt->name());
            messageOutput(MessageChannel::TEAM, msg, src);
        }

        src_sk.m_has_sidekick = false;
        src_sk.m_type         = SidekickType::NoSidekick;
        src_sk.m_db_id        = 0;
        setCombatLevel(*src.m_char,getLevel(*src.m_char)); // reset CombatLevel

        tgt_sk.m_has_sidekick = false;
        tgt_sk.m_type         = SidekickType::NoSidekick;
        tgt_sk.m_db_id        = 0;
        setCombatLevel(*tgt->m_char,getLevel(*tgt->m_char)); // reset CombatLevel

        msg = QString("%1 and %2 are no longer sidekicked.").arg(src.name(),tgt->name());
        qCDebug(logTeams).noquote() << msg;

        return; // break early
    }

    qCDebug(logTeams).noquote() << msg;
    messageOutput(MessageChannel::USER_ERROR, msg, src);
}
void removeTeamMember(Team &self, Entity *e)
{
    qCDebug(logTeams) << "Searching team members for" << e->name() << "to remove them.";
    uint32_t id_to_find = e->m_db_id;
    auto iter = std::find_if( self.m_team_members.begin(), self.m_team_members.end(),
                              [id_to_find](const Team::TeamMember& t)->bool {return id_to_find==t.tm_idx;});
    if(iter!=self.m_team_members.end())
    {
        if(iter->tm_idx == self.m_team_leader_idx)
            self.m_team_leader_idx = self.m_team_members.front().tm_idx;

        iter = self.m_team_members.erase(iter);
        e->m_has_team = false;
        e->m_team = nullptr;

        if(e->m_char->m_char_data.m_sidekick.m_has_sidekick)
            removeSidekick(*e);

        qCDebug(logTeams) << "Removing" << iter->tm_name << "from team" << self.m_team_idx;
        if(logTeams().isDebugEnabled())
            self.listTeamMembers();
    }

    if(self.m_team_members.size() > 1)
        return;

    qCDebug(logTeams) << "One player left on team. Removing last entity and deleting team.";
    if(logTeams().isDebugEnabled())
        self.listTeamMembers();

    // int idx = self.m_team_members.front().tm_idx;

    assert(false);
    // TODO: this should post an Team-removal event to the target entity, since we can't access other server's
    // Entity lists
    Entity *tgt = nullptr; //getEntityByDBID(idx);
    if(tgt == nullptr)
        return;

    tgt->m_has_team = false;
    tgt->m_team = nullptr;
    self.m_team_members.clear();
    self.m_team_leader_idx = 0;

    qCDebug(logTeams) << "After removing all entities.";
    if(logTeams().isDebugEnabled())
        self.listTeamMembers();
}

bool isEntityOnMissionMap(const EntityData &ed)
{
    QString mapName = getMapName(ed.m_map_idx);
    // Hazard and Trial maps are considered as mission maps
    return mapName.contains("Hazard") || mapName.contains("Trial");
}

//! @}

