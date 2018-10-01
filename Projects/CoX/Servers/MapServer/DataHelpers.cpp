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
#include "NetStructures/CharacterHelpers.h"
#include "NetStructures/Character.h"
#include "NetStructures/Contact.h"
#include "NetStructures/Team.h"
#include "NetStructures/LFG.h"
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
Destination getCurrentDestination(const Entity &e) { return e.m_cur_destination; }

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

void setCurrentDestination(Entity &e, int point_idx, glm::vec3 location)
{
    e.m_cur_destination.point_idx = point_idx;
    e.m_cur_destination.location = location;
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

void on_awaiting_dead_no_gurney_test(MapClientSession &session)
{
    session.m_ent->m_client->addCommandToSendNextUpdate(std::unique_ptr<DeadNoGurney>(new DeadNoGurney()));
}

bool isFriendOnline(Entity &src, uint32_t db_id)
{
    // TODO: src is needed for mapclient
    return getEntityByDBID(src.m_client->m_current_map, db_id) != nullptr;
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

void sendBrowser(MapClientSession &tgt, QString &content)
{
    qCDebug(logMapEvents) << QString("Sending Browser");
    tgt.addCommand<Browser>(content);
}

void sendTradeOffer(const Entity& src, Entity& tgt)
{
    const QString name = src.name();
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

void sendWaypoint(MapClientSession &src, int point_idx, glm::vec3 location)
{
    qCDebug(logSlashCommand) << QString("Sending SendWaypoint: %1 <%2, %3, %4>")
                                .arg(point_idx)
                                .arg(location.x, 0, 'f', 1)
                                .arg(location.y, 0, 'f', 1)
                                .arg(location.z, 0, 'f', 1);

    src.addCommand<SendWaypoint>(point_idx, location);
}


/*
 * sendEmail Wrappers for providing access to Email Database
 */
void sendEmailHeaders(Entity *e)
{
    if(!e->m_client)
    {
        qWarning() << "m_client does not yet exist!";
        return;
    }
    MapClientSession *src = e->m_client;

    EmailHeaders *header = new EmailHeaders(152, "TestSender ", "TEST", 576956720);
    src->addCommandToSendNextUpdate(std::unique_ptr<EmailHeaders>(header));
}

void readEmailMessage(Entity *e, const int id){
    if(!e->m_client)
    {
        qWarning() << "m_client does not yet exist!";
        return;
    }
    MapClientSession *src = e->m_client;

    EmailRead *msg = new EmailRead(id, "https://youtu.be/PsCKnxe8hGY\\nhttps://youtu.be/dQw4w9WgXcQ", "TestSender");
    src->addCommandToSendNextUpdate(std::unique_ptr<EmailRead>(msg));
}


/*
 * usePower here to provide access to messageOutput
 */
void usePower(Entity &ent, uint32_t pset_idx, uint32_t pow_idx, uint32_t tgt_idx, uint32_t tgt_id)
{
    // Add to activepowers queue
    CharacterPower * ppower = nullptr;
    ppower = getOwnedPower(ent, pset_idx, pow_idx);
    Power_Data pdata = ppower->getPowerTemplate();

    if(ppower != nullptr && !pdata.m_Name.isEmpty())
        ent.m_queued_powers.push_back(ppower);

    dumpPower(*ppower);
    QString msg;

    if (pdata.Type != PowerType::Toggle && pdata.Type != PowerType::Click)      //treat toggle as clicks, ignore everything else for now
        return;
    Entity *target_ent = getEntity(ent.m_client, tgt_idx);

    if (pdata.Range != float(0.0)) {               // self targetting doesn't need these checks
        if(target_ent == nullptr)
        {
            qCDebug(logPowers) << "Failed to find target:" << tgt_idx << tgt_id;
            return;
        }
        //consider if target is valid target

        if(pdata.Target == StoredEntEnum::Enemy || pdata.Target == StoredEntEnum::Foe
                || pdata.Target == StoredEntEnum::NPC)
        {
            if (!target_ent->m_is_villian){
                messageOutput(MessageChannel::COMBAT, QString("needs foe, targeting ally"), ent);
                return;
            }
        }
        else{
            if (target_ent->m_is_villian){
                messageOutput(MessageChannel::COMBAT, QString("needs ally, targeting foe"), ent);
                return;
                }
            }
        glm::vec3 senderpos = ent.m_entity_data.m_pos;          // Check if the target is in range
        glm::vec3 recpos = target_ent->m_entity_data.m_pos;

        if (glm::distance(senderpos,recpos) > pdata.Range)
        {
            msg = FloatingInfoMsg.find(FloatingMsg_OutOfRange).value();
            sendFloatingInfo(*ent.m_client, msg, FloatingInfoStyle::FloatingInfo_Info, 0.0);
            return;
        }
    }

    float endurance = getEnd(*ent.m_char);
    float end_cost = std::max(pdata.EnduranceCost, 1.0f);

    qCDebug(logPowers) << "Endurance Cost" << end_cost << "/" << endurance;
    if(end_cost > endurance)                                // Check for endurance
    {
        msg = FloatingInfoMsg.find(FloatingMsg_NotEnoughEndurance).value();
        sendFloatingInfo(*ent.m_client, msg, FloatingInfoStyle::FloatingInfo_Info, 0.0);
        return;
    }
    setEnd(*ent.m_char, endurance-end_cost);

    if(pdata.Target == StoredEntEnum::Enemy || pdata.Target == StoredEntEnum::Foe
            || pdata.Target == StoredEntEnum::NPC)
    {                                                   //roll to hit
        int roll = rand()%100;
        int chance = int(pdata.Accuracy * 75);
        messageOutput(MessageChannel::COMBAT, QString("roll " + QString::number(roll)
                                                      + "/"+ QString::number(chance)), ent);
        if (roll > chance)
        {
            QString msg = "miss";
            sendFloatingInfo(*ent.m_client, msg, FloatingInfoStyle::FloatingInfo_Info, 0.0);
        return;
        }

    }    //todo check for auto hit
 //   if (pdata.EffectArea == StoredAffectArea::Character)//single target so do the following once
 //   else                                                // AOE has to check all valid targets, and do the following
    if (!pdata.pAttribMod.empty())
    {                       // effects done here
        for(uint32_t i = 0; i<pdata.pAttribMod.size(); i++)
        {
            if (pdata.pAttribMod[i].name == "Damage")
            {      // Deal Damage
                sendFloatingNumbers(*ent.m_client, tgt_idx, pdata.pAttribMod[i].Magnitude);
                setHP(*target_ent->m_char, getHP(*target_ent->m_char)-pdata.pAttribMod[i].Magnitude);
                msg = QString::number(pdata.pAttribMod[i].Magnitude) + " damage with " + pdata.m_Name;
            }
            else if (pdata.pAttribMod[i].name == "Healing")
            {
                sendFloatingNumbers(*ent.m_client, tgt_idx, pdata.pAttribMod[i].Magnitude);
                setHP(*target_ent->m_char, getHP(*target_ent->m_char)+pdata.pAttribMod[i].Magnitude);
                msg = QString::number(pdata.pAttribMod[i].Magnitude) + " healing with " + pdata.m_Name;
            }
            else
            {                                  // TODO: buffs, debuffs, CC, summons, etc
                msg = "magnitude "+ QString::number(pdata.pAttribMod[i].Magnitude) + " "
                        + pdata.pAttribMod[i].name + " from " + pdata.m_Name;
                if (pdata.pAttribMod[i].Duration > 0)
                    msg += " for a duration of " + QString::number(pdata.pAttribMod[i].Duration);
            }
            messageOutput(MessageChannel::DAMAGE, "You cause "+msg, ent);
            if(target_ent->m_type == EntType::PLAYER && tgt_idx != ent.m_idx)  //send them the message too
                messageOutput(MessageChannel::DAMAGE, "You recieve "+msg, *target_ent);

        }
        return;
    }
    else
    {
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

    // calculate damage
    float damage = 1.0f;

    // Send message to source
    console_msg = floating_msg + " You hit " + target_ent->name() + " for " + QString::number(damage) + " damage!";
    messageOutput(MessageChannel::COMBAT, console_msg, ent);

    if(target_ent->m_type != EntType::PLAYER)
        return;
    if(tgt_idx == ent.m_idx) // Skip the rest if targeting self.
        return;

    // Send message to target
    assert(target_ent->m_client);

    sendFloatingInfo(*target_ent->m_client, floating_msg, FloatingInfoStyle::FloatingInfo_Attention, 4.0);
    console_msg = floating_msg + " You were hit by " + ent.name() + " for " + QString::number(damage) + " damage!";
    messageOutput(MessageChannel::COMBAT, console_msg, *target_ent);
    }
}
void findTeamMember(Entity &tgt)
{
    sendTeamLooking(&tgt);
}
//! @}

