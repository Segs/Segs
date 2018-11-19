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
#include "Common/GameData/EntityHelpers.h"
#include "Common/GameData/Team.h"
#include "Common/GameData/LFG.h"
#include "Common/Messages/Map/ClueList.h"
#include "Common/Messages/Map/ContactList.h"
#include "Common/Messages/Map/EmailHeaders.h"
#include "Common/Messages/Map/EmailRead.h"
#include "Common/Messages/EmailService/EmailEvents.h"
#include "Common/Messages/Map/MapEvents.h"
#include "Common/Messages/Map/Tasks.h"
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
        if(pEnt->name() == name)
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
            if(pEnt->m_idx == idx)
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

    if(db_id == 0)
        return nullptr;
    // TODO: Iterate through all entities in Database and return entity by db_id
    for (Entity *pEnt : em.m_live_entlist)
    {
        if(pEnt->m_db_id == db_id)
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
    sess.m_ent->m_is_using_mapmenu = true;
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
    sess.m_ent->m_rare_update = true; // titles have changed, resend them
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
    {
        if(src.m_ent->m_team->m_has_taskforce)
            type = TeamOfferType::WithMission; // TODO: Check for invalid missions to send `LeaveMission` instead
    }

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
    qCDebug(logMapXfers).noquote() << QString("Sending Door Message; delay: %1; msg: %2").arg(delay_status).arg(msg);
    sess.addCommand<DoorMessage>(DoorMessageStatus(delay_status), msg);
}

void sendBrowser(MapClientSession &sess, QString &content)
{
    qCDebug(logMapEvents) << QString("Sending Browser");
    sess.addCommand<Browser>(content);
}

void sendTailorOpen(MapClientSession &sess)
{
    sess.m_ent->m_rare_update = false;
    qCDebug(logTailor) << QString("Sending TailorOpen");
    sess.addCommand<TailorOpen>();
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

void updateContactStatusList(MapClientSession &sess, const Contact &updated_contact_data)
{
    vContactList contacts = sess.m_ent->m_char->m_char_data.m_contacts;
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
    sess.m_ent->m_char->m_char_data.m_contacts = contacts;
    markEntityForDbStore(sess.m_ent, DbStoreFlags::Full);
    qCDebug(logSlashCommand) << "Sending Character Contact Database updated";

    //Send contactList to client
    qCDebug(logSlashCommand) << "Sending ContactStatusList";
    sess.addCommand<ContactStatusList>(contacts);
}

void sendContactStatusList(MapClientSession &sess)
{
    vContactList contacts = sess.m_ent->m_char->m_char_data.m_contacts;
    //Send contactList to client
    sess.addCommand<ContactStatusList>(contacts);
    qCDebug(logSlashCommand) << "Sending ContactStatusList";
}

void sendWaypoint(MapClientSession &sess, int point_idx, glm::vec3 &location)
{
    qCDebug(logSlashCommand) << QString("Sending SendWaypoint: %1 <%2, %3, %4>")
                                .arg(point_idx)
                                .arg(location.x, 0, 'f', 1)
                                .arg(location.y, 0, 'f', 1)
                                .arg(location.z, 0, 'f', 1);

    sess.addCommand<SendWaypoint>(point_idx, location);
}

void sendStance(MapClientSession &sess, PowerStance &stance)
{
    qCDebug(logSlashCommand) << "Sending new PowerStance";
    sess.addCommand<SendStance>(stance);
}

void sendClueList(MapClientSession &sess)
{
    vClueList clue_list = sess.m_ent->m_char->m_char_data.m_clue_souvenir_list.m_clue_list;
    sess.addCommand<ClueList>(clue_list);
}

void sendSouvenirList(MapClientSession &sess)
{
    vSouvenirList souvenir_list = sess.m_ent->m_char->m_char_data.m_clue_souvenir_list.m_souvenir_list;
    sess.addCommand<SouvenirListHeaders>(souvenir_list);
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
 * usePower and increaseLevel here to provide access to
 * both Entity and sendInfoMessage
 */
void usePower(Entity &ent, uint32_t pset_idx, uint32_t pow_idx, int32_t tgt_idx)//  ,int32_t tgt_id
{
    QString from_msg, to_msg;
    CharacterPower * ppower = nullptr;

    ppower = getOwnedPowerByVecIdx(ent, pset_idx, pow_idx);
    const Power_Data powtpl = ppower->getPowerTemplate();

    if(ent.m_char->getHealth() == 0.0f && powtpl.CastableAfterDeath == 0)   //Allows self rez
        return;                                                             // no error message
    if(ppower == nullptr || powtpl.m_Name.isEmpty())
        return;

    if (powtpl.Type == PowerType::Toggle && (ent.m_auto_powers.size() > 0))             //checks if there are active toggles
    {
        for(auto rpow_idx = ent.m_auto_powers.begin(); rpow_idx != ent.m_auto_powers.end();rpow_idx++)
        {
            if (rpow_idx->m_pow_idxs.m_pow_vec_idx == pow_idx && rpow_idx->m_pow_idxs.m_pset_vec_idx == pset_idx)// this toggle is already on
            {
                queueRecharge(ent, pset_idx, pow_idx, powtpl.RechargeTime);
                ent.m_auto_powers.erase(rpow_idx);
                return;
            }
        }
    }

    // Target IDX of -1 is actually SELF
    if(tgt_idx == -1)
        tgt_idx = getIdx(ent);

    // Get target and check that it's a valid entity
    Entity *target_ent = getEntity(ent.m_client, tgt_idx);
    if(target_ent == nullptr)
    {
        qCDebug(logPowers) << "Failed to find target:" << tgt_idx;
        return;
    }

    // Consider if target is valid target or not
    if(validTarget(ent, ent, powtpl.Target))                //check self targetting first
    {
        target_ent = &ent;
        tgt_idx = ent.m_idx;
    }
    else if(!validTarget(*target_ent, ent, powtpl.Target))      //if not self, and iof not select target...
    {
        if (ent.m_assist_target_idx != -1)              //try assist target
            tgt_idx = ent.m_assist_target_idx;
        else if (target_ent->m_target_idx != -1)        //try target of target
            tgt_idx = target_ent->m_target_idx;

        Entity *target_ent = getEntity(ent.m_client, tgt_idx);  //try getting a new entity

        if (target_ent == nullptr || !validTarget(*target_ent, ent, powtpl.Target))
        {
            from_msg = "Invalid target";
            sendFloatingInfo(*ent.m_client, from_msg, FloatingInfoStyle::FloatingInfo_Info, 0.0);
            return;
        }
    }                                               // at this point the target is valid

    // Check Range -- TODO: refactor as checkRange() and checkTarget()
    // Check if the power uses range first
    if(powtpl.Range != float(0.0))
    {
        glm::vec3 senderpos = ent.m_entity_data.m_pos;
        glm::vec3 recpos = target_ent->m_entity_data.m_pos;

        if(glm::distance(senderpos,recpos) > powtpl.Range)
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
    if(!ent.m_recharging_powers.empty())    //check if this is already recharging
    {
        for(const QueuedPowers & pow : ent.m_recharging_powers)
        {
            if (pow.m_pow_idxs.m_pow_vec_idx == pow_idx  && pow.m_pow_idxs.m_pset_vec_idx == pset_idx)
            {
                from_msg = FloatingInfoMsg.find(FloatingMsg_Recharging).value();
                sendFloatingInfo(*ent.m_client, from_msg, FloatingInfoStyle::FloatingInfo_Info, 0.0);
                return;
            }
        }
    }
    queuePower(ent, pset_idx, pow_idx, tgt_idx, powtpl.TimeToActivate);

    if (powtpl.Type == PowerType::Toggle && !powtpl.GroupMembership.empty())    //scan for toggles with the same group
    {
        for(auto rpow_idx = ent.m_auto_powers.begin(); rpow_idx != ent.m_auto_powers.end();)
        {
            ppower = getOwnedPowerByVecIdx(ent, rpow_idx->m_pow_idxs.m_pset_vec_idx, rpow_idx->m_pow_idxs.m_pow_vec_idx);
            const Power_Data temppow = ppower->getPowerTemplate();

            if (temppow.Type == PowerType::Toggle && !temppow.GroupMembership.empty()
                    && powtpl.GroupMembership[0] == temppow.GroupMembership[0])    //and shut those off
            {
                queueRecharge(ent, temppow.powerset_idx, temppow.power_index, temppow.RechargeTime);
                ent.m_auto_powers.erase(rpow_idx);                              //bug: doesn't turn off active state
            }
            else
                ++rpow_idx;
        }
    }
}
void queuePower(Entity &ent, uint32_t pset_idx, uint32_t pow_idx, int tgt_idx, float time)
{
    QueuedPowers qpowers;           // every check has passed, so queue the power, and wait for it to call the next part
    qpowers.m_pow_idxs = {pset_idx, pow_idx};
    qpowers.m_active_state_change   = true;
    qpowers.m_timer_updated         = true;
    qpowers.m_activation_state      = true;
    qpowers.m_tgt_idx               = tgt_idx;
    qpowers.m_recharge_time         = time;
    qpowers.m_activate_period       = time;
    qpowers.m_time_to_activate      = time + 0.2f;
    ent.m_queued_powers.push_back(qpowers);             // Activation Queue
    ent.m_char->m_char_data.m_has_updated_powers = true;
}
void queueRecharge(Entity &ent, uint32_t pset_idx, uint32_t pow_idx, float time)
{
    QueuedPowers qpowers;
    qpowers.m_pow_idxs = {pset_idx, pow_idx};
    qpowers.m_active_state_change   = true;
    qpowers.m_timer_updated         = true;
    qpowers.m_recharge_time         = time;
    qpowers.m_activation_state      = false;
    qpowers.m_time_to_activate      = 0;
    qpowers.m_activate_period       = 0;
    ent.m_queued_powers.push_back(qpowers);             // puts it back in activation queue, just to set the activation state to false
    ent.m_recharging_powers.push_back(qpowers);         // put this into recharge Queue
    ent.m_char->m_char_data.m_has_updated_powers = true;

}
void doPower(Entity &ent, QueuedPowers powerinput)
{
    QString from_msg, to_msg;
    int tgt_idx = powerinput.m_tgt_idx;
    if(tgt_idx == -1)
        tgt_idx = getIdx(ent);

    Entity *target_ent = getEntity(ent.m_client, tgt_idx);
    if(target_ent == nullptr)
    {
        qCDebug(logPowers) << "Failed to find target:" << tgt_idx;
        return;
    }
    CharacterPower * ppower = nullptr;
    ppower = getOwnedPowerByVecIdx(ent, powerinput.m_pow_idxs.m_pset_vec_idx, powerinput.m_pow_idxs.m_pow_vec_idx);
    const Power_Data powtpl = ppower->getPowerTemplate();

    setEnd(*ent.m_char, getEnd(*ent.m_char)-powtpl.EnduranceCost);

    // Queue power
    if (powtpl.Type == PowerType::Click)
    {
        queueRecharge(ent, powerinput.m_pow_idxs.m_pset_vec_idx, powerinput.m_pow_idxs.m_pow_vec_idx, powtpl.RechargeTime);
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

    if (powtpl.Radius != 0.0f)           // Only AoE have a radius
    {
        glm::vec3 center;
        if (powtpl.EffectArea == StoredAffectArea::Location)
            center = ent.m_target_loc;
        if (powtpl.EffectArea == StoredAffectArea::Cone)
            center = ent.m_entity_data.m_pos;
        if (powtpl.EffectArea == StoredAffectArea::Sphere)
            center = target_ent->m_entity_data.m_pos;
        MapInstance *mi = ent.m_client->m_current_map;
        EntityManager &em(mi->m_entities);

        for (Entity* pEnt : em.m_live_entlist)
        {
            if(validTargets(*pEnt, ent, powtpl.EntsAffected))
            {
                if (glm::distance(center, pEnt->m_entity_data.m_pos) < powtpl.Radius)
                    findAttrib(ent, pEnt, ppower);
            }
        }
    }
    else
         findAttrib(ent, target_ent, ppower);
}

void findAttrib(Entity &ent, Entity *target_ent, CharacterPower * ppower)
{
    QString from_msg;
    const Power_Data powtpl = ppower->getPowerTemplate();
    if (!validTargets(*target_ent, ent, powtpl.EntsAutoHit))//
       {
        //roll to hit
        int roll = rand()%100;
                                    //find defense values for attack types, find highest
        int chance = int(std::min(95.0f,(std::max(5.0f, (powtpl.Accuracy /* * enhancment accuracy */    //min 5%, max 95%
                         * (75 + ent.m_char->m_char_data.m_current_attribs.m_ToHit
                            - target_ent->m_char->m_char_data.m_current_attribs.m_Defense) /* and target.def(this type of dmg) */
                         * ent.m_char->m_char_data.m_current_attribs.m_Accuracy)))));

        qCDebug(logPowers) << "Power hit chance: " << roll << " / " << chance;
        if(roll > chance)
        {
            from_msg = "miss";
            sendFloatingInfo(*ent.m_client, from_msg, FloatingInfoStyle::FloatingInfo_Info, 0.0);
            return;
        }
    }

    if(!powtpl.pAttribMod.empty())
    {
        for(uint32_t i = 0; i<powtpl.pAttribMod.size(); i++)
        {
            if (rand()%100 > powtpl.pAttribMod[i].Chance )

            {
             qCDebug(logPowers) <<  "roll failed to surpass: " << powtpl.pAttribMod[i].Chance;

            }
             else
             {
                if (powtpl.pAttribMod[i].Target == AttribModTarget::Self)
                {
                    target_ent = &ent;
                }
                //   if (powtpl.pAttribMod[i].Delay != 0 || powtpl.pAttribMod[i].Period != 0) ;// queue on target to do effect after delay
                doEffect(ent, target_ent, ppower, powtpl.pAttribMod[i]);
            }
        }
        return;
    }
    else
        sendFloatingNumbers(*ent.m_client, target_ent->m_idx, 1);       //temp to show what hits
}
void doEffect(Entity &ent, Entity *target_ent, CharacterPower * ppower, StoredAttribMod mod)
{
    QString lower_name = mod.name.toLower();
    float scale = mod.Scale;
    if (lower_name == "damage")
    {
        //scale = scale * target_ent->m_char->m_max_attribs.m_DamageTypes[0];
        setHP(*target_ent->m_char, getHP(*target_ent->m_char)-scale);
    }
    else if (lower_name == "healing")
    {
        if (mod.Aspect == AttribMod_Aspect::Current)
            scale = target_ent->m_char->m_max_attribs.m_HitPoints * scale;
        setHP(*target_ent->m_char, getHP(*target_ent->m_char)+scale);
    }
    else if (lower_name == "endurancemod")  // Change endurance, can be positive or negative
    {
        if (mod.Aspect == AttribMod_Aspect::Current)
            scale = target_ent->m_char->m_max_attribs.m_Endurance * scale;
        setEnd(*target_ent->m_char, getEnd(*target_ent->m_char)+scale);
    }
    else if (lower_name == "knockback") //   KB isn't a buff like other CC
        ;
    else if (lower_name == "revive")
        revivePlayer(*target_ent, ReviveLevel::AWAKEN);
    else if (lower_name == "teleport")
    {
       forcePosition(*target_ent, ent.m_target_loc);
    }
    else if (lower_name == "spawn")
       ; //lots of work needed for this one
    else
    {
        if (mod.Duration > 0)
        {
            addBuff(*target_ent, ppower, mod, ent.m_idx);
        }
    }
    sendResult(ent, *target_ent, lower_name, scale);
}

// sends floating numbers and chat text to players, and later this can add aggro to NPCs
void sendResult(Entity &src,Entity &tgt, QString name, float value)
{
    if (src.m_entity == tgt.m_entity)
        return;
    if (src.m_type == EntType::PLAYER)
    {
        if (name == "damage")
        {
            sendFloatingNumbers(*src.m_client, tgt.m_idx, int(value));   // this sends damage without needing the FloatingInfo_Damage
            sendInfoMessage(MessageChannel::DAMAGE, QString("You do %1 damage to %2") .arg(value) .arg(QString(tgt.name())) , *src.m_client);
        }
        else    // show floating info for now so we know when something was successfull
        {
            sendFloatingInfo(*src.m_client, name , FloatingInfoStyle::FloatingInfo_Info, 0.0);
            sendInfoMessage(MessageChannel::COMBAT, QString("You cause %1 (%2 mag) to %3") .arg(name) .arg(value) .arg(QString(tgt.name())) , *src.m_client);
        }
    }//else add aggro to tgt, from src, by value * src.threatmodifier

    if (tgt.m_type == EntType::PLAYER)
    {
        if (name == "damage")
        {
           sendFloatingNumbers(*tgt.m_client, tgt.m_idx, int(value));   // this sends damage without needing the FloatingInfo_Damage
           sendInfoMessage(MessageChannel::DAMAGE, QString("You receive %1 damage from %2") .arg(value) .arg(tgt.name()) , *tgt.m_client);
        }
        else
        {
            sendFloatingInfo(*tgt.m_client, name , FloatingInfoStyle::FloatingInfo_Info, 0.0);
            sendInfoMessage(MessageChannel::COMBAT, QString("%1 causes you to receive (%2 mag) %3") .arg(src.name() .arg(value) .arg(name) ) , *tgt.m_client);
        }
    }// no aggro in this case
}

void removeBuff(Entity &ent, Buffs &buff)
{
    for(auto buff_idx = ent.m_buffs.begin(); buff_idx != ent.m_buffs.end();)
    {
        if (buff_idx->m_name == buff.m_name)
        {
            for (int i =0; i<buff.m_value_name.size();i++)
                modifyAttrib(ent, buff.m_value_name[i], -buff.m_value[i]);

            ent.m_buffs.erase(buff_idx);
            return;                                         // without this it removes all buffs of same name, if we need that?
        }
        else buff_idx++;
    }
}
void addBuff(Entity &ent, CharacterPower * ppower, StoredAttribMod attrib, uint entidx)
{
    const Power_Data powtpl = ppower->getPowerTemplate();

    if (attrib.StackType == AttribStackType::Replace)       // most are replace
    {
        for (Buffs & temp : ent.m_buffs)
       {
           if ((temp.m_name == powtpl.m_Name) && (temp.source_ent_idx == entidx))
           {
                for (int i =0; i<temp.m_value_name.size();i++)
                    if (temp.m_value_name[i] == attrib.name)
                    {
                        temp.m_duration = attrib.Duration;      //if the same buff is there, refresh the duration and return
                        return;
                    }

            temp.m_value.push_back(attrib.Scale);
            temp.m_value_name.push_back(attrib.name);
            modifyAttrib(ent, attrib.name, attrib.Scale);       //if the same power has buffed, but not this stat, add this stat
            return;
           }
       }
    }
    Buffs buff;                                                 //otherewise, make a new buff
    buff.m_name = powtpl.m_Name;
    buff.m_buff_info = ppower->m_power_info;
    buff.m_duration = attrib.Duration ;
    buff.m_value_name.push_back(attrib.name);
    buff.m_value.push_back(attrib.Scale);
    buff.source_ent_idx = entidx;
    ent.m_buffs.push_back(buff);
    ent.m_update_buffs = true;
    modifyAttrib(ent, attrib.name, attrib.Scale);
}

void applyInspirationEffect(Entity &ent, uint32_t col, uint32_t row)
{
    const CharacterInspiration *insp = getInspiration(ent, col, row);

    CharacterPower temp;
    temp.m_power_info = insp->m_insp_info;
    findAttrib(ent, &ent, &temp);
}

bool useInspiration(Entity &ent, uint32_t col, uint32_t row)
{
    CharacterData &cd = ent.m_char->m_char_data;
    const CharacterInspiration *insp = getInspiration(ent, col, row);

    if(insp == nullptr)
        return true;

    QStringList revive_names = {"Awaken","Bounce_Back","Restoration",};
    if (ent.m_char->getHealth() == 0.0f)                    // isdead()?
        if(!revive_names.contains(insp->m_name, Qt::CaseInsensitive))
            return false;                                   //only wakies can be used when dead, and can't be used any other time

    qCDebug(logPowers) << "Using inspiration from" << col << "x" << row;
    applyInspirationEffect(ent, col, row);
    removeInspiration(cd, col, row);
    return true;
}

void increaseLevel(Entity &ent)
{
    qCDebug(logMapEvents) << "Increasing Level";
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

/*void addNpc(MapClientSession &sess, QString &name, glm::vec3 &loc, int variation)
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
*/
void addNpc(MapClientSession &sess, QString &npc_name, glm::vec3 &loc, int variation, QString &name)
{
    const NPCStorage & npc_store(getGameData().getNPCDefinitions());
    const Parse_NPC * npc_def = npc_store.npc_by_name(&npc_name);
    if(!npc_def)
    {
        sendInfoMessage(MessageChannel::USER_ERROR, "No NPC definition for: " + name, sess);
        return;
    }

    int idx = npc_store.npc_idx(npc_def);
    Entity *e = sess.m_current_map->m_entities.CreateNpc(getGameData(), *npc_def, idx, variation);
    e->m_char->setName(name);

    forcePosition(*e, loc);
    sendInfoMessage(MessageChannel::DEBUG_INFO, QString("Created npc with ent idx:%1 at location x: %2 y: %3 z: %4").arg(e->m_idx).arg(loc.x).arg(loc.y).arg(loc.z), sess);

    auto val = sess.m_current_map->m_scripting_interface->callFuncWithClientContext(&sess, "npc_added", e->m_idx);
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

    // TODO: Calculate XP - Debt difference by server settings?
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

    GameDataStore &data(getGameData());
    if(xp_to_give >= data.expForLevel(lvl+1))
    {
        // If we've earned enough XP, give us Leveled Up floating text
        QString floating_msg = FloatingInfoMsg.find(FloatingMsg_Leveled).value();
        sendFloatingInfo(sess, floating_msg, FloatingInfoStyle::FloatingInfo_Attention, 4.0);
        sendInfoMessage(MessageChannel::DEBUG_INFO, QString("You are now ready to level up to %1. Please visit the nearest trainer to finish your level up.").arg(lvl+1), sess);
    }

    QString msg = "Setting XP to " + QString::number(xp_to_give);
    qCDebug(logScripts) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void addListOfTasks(MapClientSession *cl, vTaskList task_list)
{
    //Send contactList to client
    TaskEntry task_entry;
    task_entry.m_db_id = cl->m_ent->m_db_id;
    task_entry.m_reset_selected_task = true; // Expose to Lua?
    task_entry.m_task_list = task_list;
    vTaskEntryList task_entry_list;
    task_entry_list.push_back(task_entry);

    //update database task list
    cl->m_ent->m_char->m_char_data.m_tasks_entry_list = task_entry_list;

    cl->addCommand<TaskStatusList>(task_entry_list);
    qCDebug(logScripts) << "Sending New TaskStatusList";
}

void sendUpdateTaskStatusList(MapClientSession &src, Task task)
{
    vTaskEntryList task_entry_list = src.m_ent->m_char->m_char_data.m_tasks_entry_list;
    //find task
    bool found = false;

    for (uint32_t i = 0; i < task_entry_list.size(); ++i)
    {
        for(uint32_t t = 0; t < task_entry_list[i].m_task_list.size(); ++t)
        {
            if(task_entry_list[i].m_task_list[t].m_task_idx == task.m_task_idx) // maybe npcId instead?
            {
                found = true;
                qCDebug(logScripts) << "SendUpdateTaskStatusList Updating old task";
                //contact already in list, update task;
                task_entry_list[i].m_task_list.at(t) = task;
                break;
            }
        }

        if(found)
            break;
    }

    if(!found)
    {
        qCDebug(logScripts) << "SendUpdateTaskStatusList Creating new task";
        uint32_t listSize = task_entry_list.size();
        if(task_entry_list.size() > 0)
        {
            qCDebug(logScripts) << "SendUpdateTaskStatusList task list not empty";
            task_entry_list[listSize].m_task_list.push_back(task); // Just use last task entry, Should only be one.
        }
        else
        {
            qCDebug(logScripts) << "SendUpdateTaskStatusList task list empty";
            std::vector<Task> task_list;
            task_list.push_back(task);
            TaskEntry t_entry;
            t_entry.m_db_id = src.m_ent->m_db_id;
            t_entry.m_reset_selected_task = true;
            t_entry.m_task_list = task_list;
            task_entry_list.push_back(t_entry);
        }
    }

    //update database Task list
    src.m_ent->m_char->m_char_data.m_tasks_entry_list = task_entry_list;
    qCDebug(logScripts) << "SendUpdateTaskStatusList DB Task list updated";

    //Send contactList to client
    src.addCommand<TaskStatusList>(task_entry_list);
    qCDebug(logScripts) << "SendUpdateTaskStatusList List updated";
}

void selectTask(MapClientSession &src, Task task)
{
    src.addCommand<TaskSelect>(task);
    qCDebug(logScripts) << "SelectTask";
}

void sendTaskStatusList(MapClientSession &src)
{
    vTaskEntryList task_entry_list = src.m_ent->m_char->m_char_data.m_tasks_entry_list;

    //Send taskList to client
    src.addCommand<TaskStatusList>(task_entry_list);
    qCDebug(logScripts) << "SendTaskStatusList";
}

void updateTaskDetail(MapClientSession &src, Task task)
{
    //Send task detail to client
    src.addCommand<TaskDetail>(task.m_db_id, task.m_task_idx, task.m_detail);
    qCDebug(logScripts) << "Sending TaskDetail";
}

void removeTask(MapClientSession &src, Task task)
{
    vTaskEntryList task_entry_list = src.m_ent->m_char->m_char_data.m_tasks_entry_list;
    //find task
    bool found = false;

    for (uint32_t i = 0; i < task_entry_list.size(); ++i)
    {
        for(uint32_t t = 0; t < task_entry_list[i].m_task_list.size(); ++t)
        {
            if(task_entry_list[i].m_task_list[t].m_task_idx == task.m_task_idx) // maybe db_id
            {
                found = true;
                task_entry_list[i].m_task_list.erase(task_entry_list[i].m_task_list.begin() + t);
                break;
            }
        }

        if(found)
            break;
    }

    if(!found)
        qCDebug(logScripts) << "Remove Task. Task not found";
    else
    {
        //update database Task list
        src.m_ent->m_char->m_char_data.m_tasks_entry_list = task_entry_list;

        //Send contactList to client
        src.addCommand<TaskStatusList>(task_entry_list);
        qCDebug(logScripts) << "Remove Task. Sending updated TaskStatusList";
    }
}

void playerTrain(MapClientSession &sess)
{
    uint level = getLevel(*sess.m_ent->m_char) + 1; // must be +1
    GameDataStore &data(getGameData());
    if(level > data.expMaxLevel())
    {
        QString msg = "You are already at the max level!";
        qCDebug(logSlashCommand) << msg;
        sendInfoMessage(MessageChannel::USER_ERROR, msg, sess);
        return;
    }

    // XP must be high enough for the level you're advancing to
    if(getXP(*sess.m_ent->m_char) < data.expForLevel(level))
    {
        QString msg = "You do not have enough Experience Points to train to the next level!";
        qCDebug(logSlashCommand) << msg;
        sendInfoMessage(MessageChannel::USER_ERROR, msg, sess);
        return;
    }

    qCDebug(logPowers) << "LEVELUP" << sess.m_ent->name() << "to" << level+1
                       << "NumPowers:" << countAllOwnedPowers(sess.m_ent->m_char->m_char_data, false) // no temps
                       << "NumPowersAtLevel:" << data.countForLevel(level, data.m_pi_schedule.m_Power);

    // send levelup pkt to client
    sess.m_ent->m_char->m_in_training = true; // flag character so we can handle dialog response
    sendContactDialogClose(sess);
    sendLevelUp(sess);
}

void setTitle(MapClientSession &sess, QString title)
{
    bool select_origin = false;
    if(!title.isEmpty())
        select_origin = true;

    sendContactDialogClose(sess);
    sendChangeTitle(sess, select_origin);
}

void showMapMenu(MapClientSession &sess)
{
    // if has_location == true, then player cannot be more than 400
    // units away from pos or window will close
    bool has_location = false;
    glm::vec3 location = sess.m_ent->m_entity_data.m_pos;
    QString msg_body = createMapMenu();
    sendContactDialogClose(sess);
    showMapXferList(sess, has_location, location, msg_body);
}

void setAlignment(Entity &e, QString align)
{
    if (align.compare("hero", Qt::CaseInsensitive) == 0)
    {
        e.m_is_hero = true;
        e.m_is_villian = false;
    }
    else if (align.compare("villian", Qt::CaseInsensitive) == 0)
    {
        e.m_is_villian = true;
        e.m_is_hero = false;
    }
    else if (align.compare("both", Qt::CaseInsensitive) == 0)
    {
        e.m_is_villian = true;
        e.m_is_hero = true;
    }
    else if (align.compare("neither", Qt::CaseInsensitive) == 0  || align.compare("none", Qt::CaseInsensitive) == 0)
    {
        e.m_is_villian = false;
        e.m_is_hero = false;
    }
    else
    {
        qCDebug(logSlashCommand) << "invalid alignment: " << align;
        return;
    }
    e.m_char->m_char_data.m_alignment = align;
}

void addClue(MapClientSession &cl, Clue clue)
{
    vClueList clue_list = cl.m_ent->m_char->m_char_data.m_clue_souvenir_list.m_clue_list;
    clue_list.push_back(clue);
    cl.m_ent->m_char->m_char_data.m_clue_souvenir_list.m_clue_list = clue_list;
    cl.addCommand<ClueList>(clue_list);
}

void addSouvenir(MapClientSession &cl, Souvenir souvenir)
{
    vSouvenirList souvenir_list = cl.m_ent->m_char->m_char_data.m_clue_souvenir_list.m_souvenir_list;
    if(souvenir_list.size() > 0)
        souvenir.m_idx = souvenir_list.size(); // Server sets the idx

    qCDebug(logScripts) << "Souvenir m_idx: " << souvenir.m_idx << " about to be added";
    souvenir_list.push_back(souvenir);
    cl.m_ent->m_char->m_char_data.m_clue_souvenir_list.m_souvenir_list = souvenir_list;
    cl.addCommand<SouvenirListHeaders>(souvenir_list);
}
//! @}
