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
#include "GameData/ClientStates.h"
#include "GameData/playerdata_definitions.h"
#include "GameData/power_definitions.h"
#include "NetStructures/EntityHelpers.h"
#include "NetStructures/CharacterHelpers.h"
#include "NetStructures/Character.h"
#include "NetStructures/Contact.h"
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

// toggleLFG must be here because it relies on sendInfoMessage()
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
        sendTeamLooking(*e.m_client);
    }
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

bool isFriendOnline(MapClientSession &sess, uint32_t db_id)
{
    // TODO: sess is needed for mapinstance
    return getEntityByDBID(sess.m_current_map, db_id) != nullptr;
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


/*
 * SendUpdate Wrappers
 */
void sendInfoMessage(MessageChannel ch, QString msg, MapClientSession &tgt)
{
    uint32_t idx = getIdx(*tgt.m_ent);

    tgt.addCommand<InfoMessageCmd>(ch, idx, msg);

    qCDebug(logInfoMsg).noquote() << "InfoMessage:"
             << "\n  Channel:" << static_cast<int>(ch)
             << "\n  Target:" << idx
             << "\n  Message:" << msg;
}

void sendTimeStateLog(MapClientSession &sess, uint32_t control_log)
{
    qCDebug(logSlashCommand, "Sending TimeStateLog %d", control_log);
    sess.addCommand<AddTimeStateLog>(control_log);
}

void sendTimeUpdate(MapClientSession &sess, int32_t sec_since_jan_1_2000)
{
    qCDebug(logSlashCommand, "Sending TimeUpdate %d", sec_since_jan_1_2000);
    sess.addCommand<TimeUpdate>(sec_since_jan_1_2000);
}

void sendClientState(MapClientSession &sess, ClientStates client_state)
{
    qCDebug(logSlashCommand) << "Sending ClientState:" << uint8_t(client_state);
    sess.addCommand<SetClientState>(client_state);
}

void showMapXferList(MapClientSession &sess, bool has_location, glm::vec3 &location, QString &name)
{
    qCDebug(logSlashCommand) << "Showing MapXferList:" << name;
    sess.addCommand<MapXferList>(has_location, location, name);
}

void sendFloatingInfo(MapClientSession &sess, QString &msg, FloatingInfoStyle style, float delay)
{
    qCDebug(logSlashCommand) << "Sending FloatingInfo:" << msg;
    sess.addCommand<FloatingInfo>(sess.m_ent->m_idx, msg, style, delay);
}

void sendFloatingNumbers(MapClientSession &sess, uint32_t tgt_idx, int32_t amount)
{
    qCDebug(logSlashCommand, "Sending %d FloatingNumbers from %d to %d", amount, sess.m_ent->m_idx, tgt_idx);
    sess.addCommand<FloatingDamage>(sess.m_ent->m_idx, tgt_idx, amount);
}

void sendLevelUp(MapClientSession &sess)
{
    //qCDebug(logSlashCommand) << "Sending LevelUp:" << src.m_idx;
    sess.addCommand<LevelUp>();
}

void sendEnhanceCombineResponse(MapClientSession &sess, bool success, bool destroy)
{
    //qCDebug(logSlashCommand) << "Sending CombineEnhanceResponse:" << sess.m_ent->m_idx;
    sess.addCommand<CombineEnhanceResponse>(success, destroy);
}

void sendChangeTitle(MapClientSession &sess, bool select_origin)
{
    //qCDebug(logSlashCommand) << "Sending ChangeTitle Dialog:" << sess.m_ent->m_idx << "select_origin:" << select_origin;
    sess.addCommand<ChangeTitle>(select_origin);
}

void sendTrayAdd(MapClientSession &sess, uint32_t pset_idx, uint32_t pow_idx)
{
    qCDebug(logSlashCommand) << "Sending TrayAdd:" << sess.m_ent->m_idx << pset_idx << pow_idx;
    sess.addCommand<TrayAdd>(pset_idx, pow_idx);
}

void sendFriendsListUpdate(MapClientSession &sess, const FriendsList &friends_list)
{
    qCDebug(logFriends) << "Sending FriendsList Update.";
    sess.addCommand<FriendsListUpdate>(friends_list);
}

void sendSidekickOffer(MapClientSession &sess, uint32_t src_db_id)
{
    qCDebug(logTeams) << "Sending Sidekick Offer" << sess.m_ent->name() << "from" << src_db_id;
    sess.addCommand<SidekickOffer>(src_db_id);
}

void sendTeamLooking(MapClientSession &sess)
{
    std::vector<LFGMember> list = g_lfg_list;

    qCDebug(logLFG) << "Sending Team Looking to" << sess.m_ent->name();
    sess.addCommand<TeamLooking>(list);
}

void sendTeamOffer(MapClientSession &src, MapClientSession &tgt)
{
    QString name        = src.m_ent->name();
    uint32_t db_id      = tgt.m_ent->m_db_id;
    TeamOfferType type  = TeamOfferType::NoMission;

    // Check for mission, send appropriate TeamOfferType
    if(src.m_ent->m_has_team && src.m_ent->m_team != nullptr)
        if(src.m_ent->m_team->m_team_has_mission)
            type = TeamOfferType::WithMission; // TODO: Check for invalid missions to send `LeaveMission` instead

    qCDebug(logTeams) << "Sending Teamup Offer" << db_id << name << static_cast<uint8_t>(type);
    tgt.addCommand<TeamOffer>(db_id, name, type);
}

void sendFaceEntity(MapClientSession &sess, int32_t tgt_idx)
{
    qCDebug(logOrientation) << QString("Sending Face Entity to %1").arg(tgt_idx);
    sess.addCommand<FaceEntity>(tgt_idx);
}

void sendFaceLocation(MapClientSession &sess, glm::vec3 &loc)
{
    qCDebug(logOrientation) << QString("Sending Face Location to x: %1 y: %2 z: %3").arg(loc.x).arg(loc.y).arg(loc.z);
    sess.addCommand<FaceLocation>(loc);
}

void sendDoorMessage(MapClientSession &sess, uint32_t delay_status, QString &msg)
{
    qCDebug(logMapXfers) << QString("Sending Door Message; delay: %1 msg: %2").arg(delay_status).arg(msg);
    sess.addCommand<DoorMessage>(DoorMessageStatus(delay_status), msg);
}

void sendBrowser(MapClientSession &sess, QString &content)
{
    qCDebug(logMapEvents) << QString("Sending Browser");
    sess.addCommand<Browser>(content);
}

void sendTradeOffer(MapClientSession &tgt, const QString &name)
{
    const uint32_t db_id = tgt.m_ent->m_db_id;

    qCDebug(logTrades) << "Sending Trade Offer" << db_id << name;
    tgt.addCommand<TradeOffer>(db_id, name);
}

void sendTradeInit(MapClientSession &src, MapClientSession &tgt)
{
    const uint32_t src_db_id = src.m_ent->m_db_id;
    const uint32_t tgt_db_id = tgt.m_ent->m_db_id;

    qCDebug(logTrades) << "Sending Trade Init" << src_db_id << tgt_db_id;
    tgt.addCommand<TradeInit>(src_db_id);
    src.addCommand<TradeInit>(tgt_db_id);
}

void sendTradeCancel(MapClientSession &sess, const QString &msg)
{
    qCDebug(logTrades) << "Sending Trade Cancel" << msg;
    sess.addCommand<TradeCancel>(msg);
}

void sendTradeUpdate(MapClientSession &src, MapClientSession &tgt, const TradeMember& trade_src, const TradeMember& trade_tgt)
{
    qCDebug(logTrades) << "Sending Trade Update";
    src.addCommand<TradeUpdate>(trade_src, trade_tgt, *tgt.m_ent);
    tgt.addCommand<TradeUpdate>(trade_tgt, trade_src, *src.m_ent);
}

void sendTradeSuccess(MapClientSession &src, MapClientSession &tgt)
{
    const QString msg_src = "Trade with " + tgt.m_ent->name() + " was a success.";
    const QString msg_tgt = "Trade with " + src.m_ent->name() + " was a success.";

    qCDebug(logTrades) << "Sending Trade Success";
    src.addCommand<TradeSuccess>(msg_src);
    tgt.addCommand<TradeSuccess>(msg_tgt);
}

void sendContactDialog(MapClientSession &sess, QString msg_body, std::vector<ContactEntry> active_contacts)
{
    qCDebug(logSlashCommand) << "Sending ContactDialog:" << msg_body;
    sess.addCommand<ContactDialog>(msg_body, active_contacts);
}

void sendContactDialogYesNoOk(MapClientSession &sess, QString msg_body, bool has_yesno)
{
    qCDebug(logSlashCommand) << "Sending ContactDialogYesNo:" << has_yesno << msg_body;

    if(has_yesno)
        sess.addCommand<ContactDialogYesNo>(msg_body);
    else
        sess.addCommand<ContactDialogOk>(msg_body);
}

void sendContactDialogClose(MapClientSession &sess)
{
    qCDebug(logSlashCommand) << "Sending ContactDialogClose";
    sess.addCommand<ContactDialogClose>();
}

void sendWaypoint(MapClientSession &sess, int point_idx, glm::vec3 location)
{
    qCDebug(logSlashCommand) << QString("Sending SendWaypoint: %1 <%2, %3, %4>")
                                .arg(point_idx)
                                .arg(location.x, 0, 'f', 1)
                                .arg(location.y, 0, 'f', 1)
                                .arg(location.z, 0, 'f', 1);

    sess.addCommand<SendWaypoint>(point_idx, location);
}

void sendStance(MapClientSession &sess, PowerStance stance)
{
    qCDebug(logSlashCommand) << "Sending new PowerStance";
    sess.addCommand<SendStance>(stance);
}

void sendDeadNoGurney(MapClientSession &sess)
{
    qCDebug(logSlashCommand) << "Sending new PowerStance";
    sess.addCommand<DeadNoGurney>();
}


/*
 * usePower and increaseLevel here to provide access to
 * both Entity and sendInfoMessage
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
                sendInfoMessage(MessageChannel::COMBAT, QString("You cannot target allies with this power."), *ent.m_client);
                return;
            }
        }
        else
        {
            if(target_ent->m_is_villian)
            {
                sendInfoMessage(MessageChannel::COMBAT, QString("You must target allies with this power."), *ent.m_client);
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
        sendFaceEntity(*ent.m_client, tgt_idx);
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
        addTriggeredMove(ent, bits, powtpl.m_AttackFrames, bits);
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
            if (powtpl.pAttribMod[i].name.compare("Damage", Qt::CaseInsensitive) == 0)
            {
                // Deal Damage
                sendFloatingNumbers(*ent.m_client, tgt_idx, powtpl.pAttribMod[i].Magnitude);
                setHP(*target_ent->m_char, getHP(*target_ent->m_char)-powtpl.pAttribMod[i].Magnitude);

                // Build target specific messages
                from_msg = QString("You deal %1 damage with %2")
                        .arg(powtpl.pAttribMod[i].Magnitude)
                        .arg(powtpl.m_Name);

                to_msg   = QString("%1 hit you for %2 damage with %3")
                        .arg(ent.name())
                        .arg(powtpl.pAttribMod[i].Magnitude)
                        .arg(powtpl.m_Name);
            }
            else if (powtpl.pAttribMod[i].name.compare("Healing", Qt::CaseInsensitive) == 0)
            {
                // Do Healing
                sendFloatingNumbers(*ent.m_client, tgt_idx, powtpl.pAttribMod[i].Magnitude);
                setHP(*target_ent->m_char, getHP(*target_ent->m_char)+powtpl.pAttribMod[i].Magnitude);

                // Build target specific messages
                from_msg = QString("You heal %1 for %2 with %3")
                        .arg(target_ent->name())
                        .arg(powtpl.pAttribMod[i].Magnitude)
                        .arg(powtpl.m_Name);

                to_msg   = QString("%1 heals you for %2 with %3")
                        .arg(target_ent->name())
                        .arg(powtpl.pAttribMod[i].Magnitude)
                        .arg(powtpl.m_Name);
            }
            else
            {
                // TODO: buffs, debuffs, CC, summons, etc
                to_msg = QString("%1 %2 from %3")
                        .arg(powtpl.pAttribMod[i].Magnitude)
                        .arg(powtpl.pAttribMod[i].name)
                        .arg(powtpl.m_Name);

                if (powtpl.pAttribMod[i].Duration > 0)
                    to_msg.append(" for a duration of %1").arg(powtpl.pAttribMod[i].Duration);

                // Build target specific messages
                from_msg = QString("You cause ").append(to_msg);
                to_msg.prepend("You receive ");
            }

            // Send message to source combat window
            sendInfoMessage(MessageChannel::COMBAT, from_msg, *ent.m_client);

            // Send message to another player target
            if(target_ent->m_type == EntType::PLAYER && tgt_idx != ent.m_idx)
                sendInfoMessage(MessageChannel::DAMAGE, to_msg, *target_ent->m_client);
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
    sendInfoMessage(MessageChannel::COMBAT, console_msg, *ent.m_client);

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
    sendInfoMessage(MessageChannel::COMBAT, console_msg, *target_ent->m_client);
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
 * Lua Functions
 */
//

void addNpc(MapClientSession &cl, const char* name, glm::vec3 *loc, int variation)
{
    const NPCStorage & npc_store(getGameData().getNPCDefinitions());
    const QString name_string = QString::fromUtf8(name);
    QStringRef npc_name = QStringRef(&name_string);

    const Parse_NPC * npc_def = npc_store.npc_by_name(npc_name);
    if(!npc_def)
    {
        sendInfoMessage(MessageChannel::USER_ERROR, "No NPC definition for:"+npc_name, cl);
        return;
    }
    int idx = npc_store.npc_idx(npc_def);
    Entity *e = cl.m_current_map->m_entities.CreateNpc(getGameData(),*npc_def,idx, variation);

    forcePosition(*e, *loc);
    e->m_velocity = {0,0,0};
    sendInfoMessage(MessageChannel::DEBUG_INFO, QString("Created npc with ent idx:%1 at location x: %2 y: %3 z: %4").arg(e->m_idx).arg(loc->x).arg(loc->y).arg(loc->z), cl);
}

void giveEnhancement(MapClientSession *cl, const char* e_name, int e_level)
{
    CharacterData &cd = cl->m_ent->m_char->m_char_data;
    QString name = QString::fromUtf8(e_name);
    uint32_t level = e_level;
    QString msg = "You do not have room for any more enhancements!";

    if(getNumberEnhancements(cd) < 10)
    {
        msg = "Awarding Enhancement '" + name + "' to " + cl->m_ent->name();
        addEnhancementByName(cd, name, level);

        QString floating_msg = FloatingInfoMsg.find(FloatingMsg_FoundEnhancement).value();
        sendFloatingInfo(*cl, floating_msg, FloatingInfoStyle::FloatingInfo_Attention, 4.0);
    }
    qCDebug(logScripts()).noquote() << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, *cl);
}

void giveDebt(MapClientSession *cl, int debt)
{
    uint32_t current_debt = getDebt(*cl->m_ent->m_char);
    uint32_t debt_to_give = current_debt + debt;
    setDebt(*cl->m_ent->m_char, debt_to_give);
    QString msg = "Setting Debt to " + QString::number(debt_to_give);
    qCDebug(logScripts) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, *cl);
}

void giveEnd(MapClientSession *cl, float end)
{
    float current_end = getEnd(*cl->m_ent->m_char);
    float end_to_set = current_end + end;
    setEnd(*cl->m_ent->m_char, end_to_set);
    QString msg = QString("Setting Endurance to: %1").arg(end_to_set);
    qCDebug(logScripts) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, *cl);
}

void giveHp(MapClientSession *cl, float hp)
{
    float current_hp = getHP(*cl->m_ent->m_char);
    float hp_to_set = current_hp + hp;
    setHP(*cl->m_ent->m_char, hp_to_set);
    QString msg = QString("Setting HP to: %1").arg(hp_to_set);
    qCDebug(logScripts) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, *cl);
}

void giveInf(MapClientSession *cl, int inf)
{
    uint32_t current_inf = getInf(*cl->m_ent->m_char);
    uint32_t inf_to_set = current_inf + inf;
    setInf(*cl->m_ent->m_char, inf_to_set);
    sendInfoMessage(MessageChannel::DEBUG_INFO, QString("Setting inf to %1").arg(inf_to_set), *cl);
}

void giveInsp(MapClientSession *cl, const char *value)
{
    CharacterData &cd = cl->m_ent->m_char->m_char_data;
    QString val = QString::fromUtf8(value);
    QString msg = "You do not have room for any more inspirations!";

    if(getNumberInspirations(cd) < getMaxNumberInspirations(cd))
    {
        msg = "Awarding Inspiration '" + val + "' to " + cl->m_ent->name();

        addInspirationByName(cd, val);

        // NOTE: floating message shows no message here, but plays the awarding insp sound!
        QString floating_msg = FloatingInfoMsg.find(FloatingMsg_FoundInspiration).value();
        sendFloatingInfo(*cl, floating_msg, FloatingInfoStyle::FloatingInfo_Attention, 4.0);
    }

    qCDebug(logScripts).noquote() << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, *cl);
}

void giveXp(MapClientSession *cl, int xp)
{
    uint32_t lvl = getLevel(*cl->m_ent->m_char);
    uint32_t current_xp = getXP(*cl->m_ent->m_char);

    // Calculate XP - Debt difference by server settings?

    uint32_t current_debt = getDebt(*cl->m_ent->m_char);
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
        setDebt(*cl->m_ent->m_char, newDebt);
        sendInfoMessage(MessageChannel::DEBUG_INFO, QString("You paid %1 to your debt").arg(debt_to_pay), *cl);
    }
    uint32_t xp_to_give = current_xp + xp;
    setXP(*cl->m_ent->m_char, xp_to_give);
    sendInfoMessage(MessageChannel::DEBUG_INFO, QString("You were awarded %1 XP").arg(xp), *cl);
    QString msg = "Setting XP to " + QString::number(xp_to_give);
    uint32_t new_lvl = getLevel(*cl->m_ent->m_char);
    sendInfoMessage(MessageChannel::DEBUG_INFO, QString("You were lvl %1 now %2").arg(lvl).arg(new_lvl), *cl);

    //This check doesn't show level change

    if(new_lvl != lvl)
    {
        cl->addCommand<FloatingInfo>(cl->m_ent->m_idx, FloatingInfoMsg.find(FloatingMsg_Leveled).value(), FloatingInfo_Attention , 4.0);
        msg += " and LVL to " + QString::number(new_lvl);
    }
    qCDebug(logScripts) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, *cl);
}


//! @}
