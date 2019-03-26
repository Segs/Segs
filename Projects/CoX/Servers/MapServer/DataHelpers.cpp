/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
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
#include "TimeHelpers.h"
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
#include "Common/Messages/Map/ConsoleMessages.h"
#include "Common/Messages/Map/EmailHeaders.h"
#include "Common/Messages/Map/EmailRead.h"
#include "Common/Messages/Map/ForceLogout.h"
#include "Common/Messages/Map/SendVisitLocation.h"
#include "Common/Messages/EmailService/EmailEvents.h"
#include "Common/Messages/Map/MapEvents.h"
#include "Common/Messages/Map/StoresEvents.h"
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

Entity * getEntity(MapInstance* mi, uint32_t idx)
{
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
    //sendInfoMessage(MessageChannel::USER_ERROR, errormsg, *src);
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

void sendMissionObjectiveTimer(MapClientSession &sess, QString &message, float time)
{
    sess.addCommand<MissionObjectiveTimer>(message, time);
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

QString createKioskMessage(Entity* player)
{
    //Can we add hrefs to trigger dialog buttons with a script callback to navigate to other screen/pages?
    QString msg_body = "<linkhoverbg #118866aa><link white><linkhover white><table>";
    auto index = player->m_char->m_char_data.m_class_name.indexOf('_');
    msg_body.append(QString("<tr><td>%1</td><td>Level: %2 %3</td></tr>").arg(player->name()).arg(player->m_char->m_char_data.m_level + 1).arg(player->m_char->m_char_data.m_class_name.mid(index + 1)));
    msg_body.append(QString("<tr><td>%1</td><td>%2</td></tr>").arg("Hide and seek").arg(player->m_player->m_player_statistics.m_hide_seek.m_found_count));

    msg_body.append(QString("<tr></tr>"));// line break

    if(player->m_player->m_player_statistics.m_relay_races.size() > 0){
        msg_body.append(QString("<tr>Relay Race Results</tr>"));
    }

    for(const RelayRaceResult &r: player->m_player->m_player_statistics.m_relay_races)
    {
        QString lastTime = QDateTime::fromTime_t(r.m_last_time).toUTC().toString("hh:mm:ss");
        QString bestTime = QDateTime::fromTime_t(r.m_best_time).toUTC().toString("hh:mm:ss");
        msg_body.append(QString("<tr><td>Box #%1</td><td>Last Time: %2</td><td>Best Time: %3</td></tr>").arg(r.m_segment).arg(lastTime).arg(bestTime));
    }

    msg_body.append("</table>");

    return msg_body;
}


/*
 * sendEmail Wrappers for providing access to Email Database
 */

void getEmailHeaders(MapClientSession& sess)
{   
    if(!sess.m_ent->m_client)
    {
        qWarning() << "m_client does not yet exist!";
        return;
    }

    HandlerLocator::getEmail_Handler()->putq(new EmailHeaderRequest(
        {sess.m_ent->m_char->m_db_id}, sess.link()->session_token()));    
}

void sendEmail(MapClientSession& sess, QString recipient_name, QString subject, QString message)
{
    if(!sess.m_ent->m_client)
    {
        qWarning() << "m_client does not yet exist!";
        return;
    }

    uint32_t timestamp = getSecsSince2000Epoch();

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
    vContactList contacts = sess.m_ent->m_player->m_contacts;
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
    sess.m_ent->m_player->m_contacts = contacts;
    markEntityForDbStore(sess.m_ent, DbStoreFlags::Full);
    qCDebug(logSlashCommand) << "Sending Character Contact Database updated";

    //Send contactList to client
    qCDebug(logSlashCommand) << "Sending ContactStatusList";
    sess.addCommand<ContactStatusList>(contacts);
}

void sendContactStatusList(MapClientSession &sess)
{
    vContactList contacts = sess.m_ent->m_player->m_contacts;
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
    vClueList clue_list = sess.m_ent->m_player->m_clues;
    sess.addCommand<ClueList>(clue_list);
}

void sendSouvenirList(MapClientSession &sess)
{
    vSouvenirList souvenir_list = sess.m_ent->m_player->m_souvenirs;
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
    if(powtpl.Type != PowerType::Toggle && powtpl.Type != PowerType::Click)
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
    if(powtpl.Range == float(0.0))
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
        if(roll > chance)
        {
            from_msg = "miss";
            sendFloatingInfo(*ent.m_client, from_msg, FloatingInfoStyle::FloatingInfo_Info, 0.0);
            return;
        }
    }

    // TODO: check for auto hit EntsAutoHit for StoredEntsEnum::CASTER
    //   if(powtpl.EffectArea == StoredAffectArea::Character)//single target so do the following once
    //   else                                                // AOE has to check all valid targets, and do the following

    if(!powtpl.pAttribMod.empty())
    {
        // effects done here
        for(uint32_t i = 0; i<powtpl.pAttribMod.size(); i++)
        {
            QByteArray lower_name = powtpl.pAttribMod[i].name.toLower();
            if(lower_name == "Damage")
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
            else if("Healing" == lower_name)
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

                if(powtpl.pAttribMod[i].Duration > 0)
                    to_msg.append(QString(" for a duration of %1").arg(powtpl.pAttribMod[i].Duration));

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
    QStringList batman_kerpow{"BAM!", "BANG!", "BONK!", "CLANK!", "CLASH!",
                              "CRAAACK!", "CRASH!", "CRUNCH!", "EEE-YOW!",
                              "KAPOW!", "KER-PLOP!", "OUCH!", "POW!", "TOUCHÉ!",
                              "UGGH!", "WHACK!", "WHAMM!", "ZAM!", "ZAP!"};

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
    float damage = 9.1f;

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

void addNpcWithOrientation(MapClientSession &sess, QString &name, glm::vec3 &loc, int variation, glm::vec3 &ori, QString &npc_name)
{
    addNpcWithOrientation(*sess.m_current_map, name, loc, variation, ori, npc_name);
}

void addNpcWithOrientation(MapInstance &mi, QString &name, glm::vec3 &loc, int variation, glm::vec3 &ori, QString &npc_name)
{
    const NPCStorage & npc_store(getGameData().getNPCDefinitions());
    const Parse_NPC * npc_def = npc_store.npc_by_name(&name);
    if(!npc_def)
    {
        qCDebug(logScripts()) << "No NPC definition for: " + name;
        //sendInfoMessage(MessageChannel::USER_ERROR, "No NPC definition for: " + name, sess);
        return;
    }

    int idx = npc_store.npc_idx(npc_def);
    Entity *e = mi.m_entities.CreateNpc(getGameData(), *npc_def, idx, variation);
    e->m_char->setName(npc_name);

    forcePosition(*e, loc);
    forceOrientation(*e, ori);
    qCDebug(logScripts()) << QString("Created npc with ent idx:%1 at location x: %2 y: %3 z: %4").arg(e->m_idx).arg(loc.x).arg(loc.y).arg(loc.z);
    //sendInfoMessage(MessageChannel::DEBUG_INFO, QString("Created npc with ent idx:%1 at location x: %2 y: %3 z: %4").arg(e->m_idx).arg(loc.x).arg(loc.y).arg(loc.z), sess);

    mi.m_scripting_interface->callFuncWithMapInstance(&mi, "npc_added", e->m_idx);
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

    GameDataStore &data(getGameData());
    if (data.m_uses_xp_mod && 
        data.m_xp_mod_startdate <= QDateTime::currentDateTime() && 
        data.m_xp_mod_enddate >= QDateTime::currentDateTime())
    {
        xp *= data.m_xp_mod_multiplier;
    }

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

void giveTempPower(MapClientSession *cl, const char* power)
{
    CharacterData &cd = cl->m_ent->m_char->m_char_data;
    PowerPool_Info ppool;
    QString temp_power_set = "Temporary_Powers";

    ppool.m_pcat_idx = getPowerCatByName(temp_power_set);
    ppool.m_pset_idx = getPowerSetByName(temp_power_set, ppool.m_pcat_idx);

    Parse_PowerSet pset = getGameData().get_powerset(ppool.m_pcat_idx, ppool.m_pset_idx);

    for(size_t i = 0; i < pset.Available.size(); ++i)
    {
        qCDebug(logScripts) << "giveTempPower Name: " << pset.m_Powers[i].m_Name;
        if(pset.m_Powers[i].m_Name == power)
        {
            qCDebug(logScripts) << "giveTempPower PowerFound";
            ppool.m_pow_idx = uint32_t(i);
            addPower(cd, ppool);
            break;
        }
    }
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
    cl->m_ent->m_player->m_tasks_entry_list = task_entry_list;

    cl->addCommand<TaskStatusList>(task_entry_list);
    qCDebug(logScripts) << "Sending New TaskStatusList";
}

void sendUpdateTaskStatusList(MapClientSession &src, Task task)
{
    vTaskEntryList task_entry_list = src.m_ent->m_player->m_tasks_entry_list;
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
                //Tast already in list, update task;
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
    src.m_ent->m_player->m_tasks_entry_list = task_entry_list;
    qCDebug(logScripts) << "SendUpdateTaskStatusList DB Task list updated";

    //Send Task list to client
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
    vTaskEntryList task_entry_list = src.m_ent->m_player->m_tasks_entry_list;

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
    vTaskEntryList task_entry_list = src.m_ent->m_player->m_tasks_entry_list;
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
        src.m_ent->m_player->m_tasks_entry_list = task_entry_list;

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

void sendKiosk(MapClientSession &cl)
{
    QString msg = createKioskMessage(cl.m_ent);
    sendBrowser(cl, msg);
}

void setTitle(MapClientSession &sess, QString &title)
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

void addClue(MapClientSession &cl, Clue clue)
{
    vClueList clue_list = cl.m_ent->m_player->m_clues;
    clue_list.push_back(clue);
    cl.m_ent->m_player->m_clues = clue_list;
    markEntityForDbStore(cl.m_ent, DbStoreFlags::Full);
    cl.addCommand<ClueList>(clue_list);

}

void removeClue(MapClientSession &cl, Clue clue)
{
    vClueList clue_list = cl.m_ent->m_player->m_clues;
    int count = 0;
    bool found = false;
    for (const Clue &c: clue_list)
    {
        if(c.m_name == clue.m_name)
        {
            found = true;
            break;
        }
        ++count;
    }

    if(found)
    {
        clue_list.erase(clue_list.begin() + count);
        cl.m_ent->m_player->m_clues = clue_list;
        markEntityForDbStore(cl.m_ent, DbStoreFlags::Full);
        cl.addCommand<ClueList>(clue_list);
    }
    else
    {
        qCDebug(logScripts) << "Clue: " << clue.m_name << " not found.";
    }
}

void addSouvenir(MapClientSession &cl, Souvenir souvenir)
{
    vSouvenirList souvenir_list = cl.m_ent->m_player->m_souvenirs;
    if(souvenir_list.size() > 0)
        souvenir.m_idx = souvenir_list.size(); // Server sets the idx

    qCDebug(logScripts) << "Souvenir m_idx: " << souvenir.m_idx << " about to be added";
    souvenir_list.push_back(souvenir);
    cl.m_ent->m_player->m_souvenirs = souvenir_list;
    markEntityForDbStore(cl.m_ent, DbStoreFlags::Full);
    cl.addCommand<SouvenirListHeaders>(souvenir_list);
}

void removeSouvenir(MapClientSession &cl, Souvenir souvenir)
{
    vSouvenirList souvenir_list = cl.m_ent->m_player->m_souvenirs;
    int count = 0;
    bool found = false;
    for (const Souvenir &s: souvenir_list)
    {
        if(s.m_name == souvenir.m_name)
        {
            found = true;
            break;
        }
        ++count;
    }

    if(found)
    {
        souvenir_list.erase(souvenir_list.begin() + count);
        cl.m_ent->m_player->m_souvenirs = souvenir_list;
        markEntityForDbStore(cl.m_ent, DbStoreFlags::Full);
        cl.addCommand<SouvenirListHeaders>(souvenir_list);
    }
    else
    {
        qCDebug(logScripts) << "Souvenir: " << souvenir.m_name << " not found.";
    }

}

void removeContact(MapClientSession &sess, Contact contact)
{
    vContactList contacts = sess.m_ent->m_player->m_contacts;

    bool found = false;
    int count = 0;
    for (Contact & c : contacts)
    {
        if(c.m_npc_id == contact.m_npc_id)
        {
            found = true;
            break;
        }
        ++count;
    }

    if(!found)
    {
        qCDebug(logScripts) << "Contact " << contact.m_name << " not found";
    }
    else
    {
        contacts.erase(contacts.begin() + count);
        sess.m_ent->m_player->m_contacts = contacts;
        markEntityForDbStore(sess.m_ent, DbStoreFlags::Full);
        sess.addCommand<ContactStatusList>(contacts);
    }

}

void revive(MapClientSession *cl, int revive_lvl)
{
    setStateMode(*cl->m_ent, ClientStates::RESURRECT);
    revivePlayer(*cl->m_ent, static_cast<ReviveLevel>(revive_lvl));
}

void logSpawnLocations(MapClientSession &cl, const char* spawn_type)
{
    QString spawn_name = QString::fromUtf8(spawn_type);
    auto spawners = cl.m_current_map->getSpawners();

    if(spawners.empty())
    {
        qCDebug(logScripts) << "spawners empty";
        return;
    }
    int count = 0;
    auto spawn_list = spawners.values(spawn_name);

    for(auto const &s: spawn_list)
    {
        glm::mat4 mat4 = s;
        glm::vec3 loc = glm::vec3(mat4[3]);
        qCDebug(logScripts) << "Spawn: " << spawn_name << " loc x: " << loc.x << " y: " << loc.y << " z: " << loc.z ;
        ++count;
    }
}

void respawn(MapClientSession &cl, const char* spawn_type)
{
    qCDebug(logScripts) << "respawn Called";

    QString spawn_name = QString::fromUtf8(spawn_type);
    Entity *e = cl.m_ent;

    // Spawn player position and PYR
    glm::vec3 spawn_pos = glm::vec3(128.0f,16.0f,-198.0f);
    glm::vec3 spawn_pyr = glm::vec3(0.0f, 0.0f, 0.0f);

    auto spawners = cl.m_current_map->getSpawners();

    if(!spawners.empty())
    {
        glm::mat4 v = glm::mat4(1.0f);
        auto spawn_list = spawners.values(spawn_name);
        if(!spawn_list.empty())
        {
            v = spawn_list[rand() % spawn_list.size()];

            // Position
            spawn_pos = glm::vec3(v[3]);

            // Orientation
            auto valquat = glm::quat_cast(v);
            spawn_pyr = toCoH_YPR(valquat);

            forcePosition(*e, spawn_pos);
            forceOrientation(*e, spawn_pyr);
        }
        else
        {
            qCDebug(logScripts) << "spawn_list for " << spawn_name << " is empty.";
        }
    }
    else
    {
        qCDebug(logScripts) << "spawners empty";
    }
}


void openStore(MapClientSession &sess, int entity_idx)
{
    // Future: Look up stores owned by NPC in NPC Bin files
    Store store;
    store.m_npc_idx = entity_idx; // Set for testing
    vStoreItems store_items;

    store_items.emplace_back(QStringLiteral("All_SL9_CL2"), 100);
    store_items.emplace_back(QStringLiteral("FBSA"), 100);
    store.m_store_Items = store_items;

    qCDebug(logStores) << "Sending OpenStore...";
    sess.addCommand<StoreOpen>(store);
}

void modifyInf(MapClientSession &sess, int amount)
{
    int32_t inf = getInf(*sess.m_ent->m_char);
    // amount can be negitive to lower influence
    inf = inf + amount;
    if(inf < 0)
        inf = 0;

    setInf(*sess.m_ent->m_char, inf);
}

void sendForceLogout(MapClientSession &cl, QString &player_name, QString &logout_message)
{
    Entity* e = getEntity(&cl, player_name);

    if(e == nullptr)
    {
        qCDebug(logSlashCommand) << "Entity: " << player_name << " not found";
        return;
    }

    MapClientSession *tgt = e->m_client;
    qCDebug(logScripts) << "SendForceLogout. Mesage: " << logout_message;
    tgt->link()->putq(new ForceLogout(logout_message));
}

void sendLocation(MapClientSession &cl, VisitLocation location)
{
    qWarning("SendLocation, unknown affect on client");
    qCDebug(logSlashCommand) << "sendLocation: " << location.m_location_name;
    vLocationList location_list;

    //Test locations in Atlas Park
    location_list.push_back(VisitLocation("Statue1", glm::vec3(126.5,27.4315,-300)));
    location_list.push_back(VisitLocation("Patrol_Easy_1_City_01_01", glm::vec3(461.0,0,778)));
    location_list.push_back(VisitLocation("Patrol_Easy_5_City_01_01", glm::vec3(1099.5,0,124.5)));

    cl.addCommand<SendLocations>(location_list);
}

void sendDeveloperConsoleOutput(MapClientSession &cl, QString &message)
{
    cl.addCommand<ConsoleOutput>(message);
}

void sendClientConsoleOutput(MapClientSession &cl, QString &message)
{
    cl.addCommand<ConsolePrint>(message);
}

void npcSendMessage(MapClientSession &cl, QString& channel, int entityIdx, QString& message)
{
    QStringRef ch(&channel,0,1); //Get first char of channel name. Channel will default to local if unknown
    QString formated = ch + ' '+ message;
    Entity *e = getEntity(&cl, entityIdx);

    if(e != nullptr)
        cl.m_current_map->add_chat_message(e, formated);
}

void npcSendMessage(MapInstance &mi, QString& channel, int entityIdx, QString& message)
{
    QStringRef ch(&channel,0,1); //Get first char of channel name. Channel will default to local if unknown
    QString formated = ch + ' '+ message;
    Entity *e = getEntity(&mi, entityIdx);

    if(e != nullptr)
        mi.add_chat_message(e, formated);
}

void addRelayRaceResult(MapClientSession &cl, RelayRaceResult &raceResult)
{
    vRelayRace races = cl.m_ent->m_player->m_player_statistics.m_relay_races;
    int count = 0;
    bool found = false;
    for(const RelayRaceResult &r: races)
    {
        if(r.m_segment == raceResult.m_segment)
        {
            found = true;
            if(r.m_best_time > raceResult.m_last_time || r.m_best_time == 0)
            {
                races[count] = raceResult;
                races[count].m_best_time = raceResult.m_last_time;
            }
            else
                races[count].m_last_time = raceResult.m_last_time;

            break;
        }
        ++count;
    }
    if(!found)
    {
        raceResult.m_best_time = raceResult.m_last_time;
        races.push_back(raceResult);
    }
    cl.m_ent->m_player->m_player_statistics.m_relay_races = races;
    markEntityForDbStore(cl.m_ent, DbStoreFlags::Full);
}

void addHideAndSeekResult(MapClientSession &cl, int points)
{
    HideAndSeek hideAndSeek = cl.m_ent->m_player->m_player_statistics.m_hide_seek;

    if(hideAndSeek.m_found_count == 0)
        hideAndSeek.m_found_count = points;
    else
        hideAndSeek.m_found_count += points;

    cl.m_ent->m_player->m_player_statistics.m_hide_seek = hideAndSeek;
    markEntityForDbStore(cl.m_ent, DbStoreFlags::Full);
}

RelayRaceResult getRelayRaceResult(MapClientSession &cl, int segment)
{
    vRelayRace results = cl.m_ent->m_player->m_player_statistics.m_relay_races;
    RelayRaceResult result;
    for (const RelayRaceResult &r: results)
    {
        if(r.m_segment == segment)
        {
            result = r;
            break;
        }
    }

    return result;
}


void addEnemy(MapInstance &mi, QString &name, glm::vec3 &loc, int variation, glm::vec3 &ori, QString &npc_name, int level, QString &faction_name, int f_rank)
{
    const NPCStorage & npc_store(getGameData().getNPCDefinitions());
    const Parse_NPC * npc_def = npc_store.npc_by_name(&name);
    if(!npc_def)
    {
        qCDebug(logNpcSpawn) << "No NPC definition for: " + name;
        //sendInfoMessage(MessageChannel::USER_ERROR, "No NPC definition for: " + name, sess);
        return;
    }

    int idx = npc_store.npc_idx(npc_def);
    Entity *e = mi.m_entities.CreateCritter(getGameData(), *npc_def, idx, variation, level);
    e->m_char->setName(npc_name);

    //Sets target info menu faction. Skull, Hellions, Freakshow, etc
    e->m_faction_data.m_faction_name = faction_name;

    forcePosition(*e, loc);
    forceOrientation(*e, ori);
    qCDebug(logNpcSpawn) << QString("Created Enemy with ent idx:%1 at location x: %2 y: %3 z: %4").arg(e->m_idx).arg(loc.x).arg(loc.y).arg(loc.z);
    //sendInfoMessage(MessageChannel::DEBUG_INFO, QString("Created npc with ent idx:%1 at location x: %2 y: %3 z: %4").arg(e->m_idx).arg(loc.x).arg(loc.y).arg(loc.z), sess);
}

void addVictim(MapInstance &mi, QString &name, glm::vec3 &loc, int variation, glm::vec3 &ori, QString &npc_name)
{
    const NPCStorage & npc_store(getGameData().getNPCDefinitions());
    const Parse_NPC * npc_def = npc_store.npc_by_name(&name);
    if(!npc_def)
    {
        qCDebug(logNpcSpawn) << "No NPC definition for: " + name;
        //sendInfoMessage(MessageChannel::USER_ERROR, "No NPC definition for: " + name, sess);
        return;
    }

    int idx = npc_store.npc_idx(npc_def);
    Entity *e = mi.m_entities.CreateGeneric(getGameData(), *npc_def, idx, variation, EntType::CRITTER);
    e->m_char->setName(npc_name);
    e->m_is_hero = true;
    e->m_is_villian = false;

    //Should these be predefined by DB/Json/Loaded by script or something else?
    e->m_char->m_char_data.m_combat_level = 1;
    e->m_char->m_char_data.m_level = 1;
    e->m_char->m_char_data.m_security_threat = 1;

    e->m_char->m_max_attribs.m_HitPoints = 100;
    e->m_char->m_max_attribs.m_Endurance = 100;
    e->m_char->m_char_data.m_current_attribs.m_HitPoints = 100;
    e->m_char->m_char_data.m_current_attribs.m_Endurance = 100;

    //Sets target info menu faction. Skull, Hellions, Freakshow, etc
    e->m_faction_data.m_faction_name = "Citizen";

    //Required to send changes to clients
    e->m_pchar_things = true;

    forcePosition(*e, loc);
    forceOrientation(*e, ori);
    qCDebug(logNpcSpawn) << QString("Created Victim with ent idx:%1 at location x: %2 y: %3 z: %4").arg(e->m_idx).arg(loc.x).arg(loc.y).arg(loc.z);
    //sendInfoMessage(MessageChannel::DEBUG_INFO, QString("Created npc with ent idx:%1 at location x: %2 y: %3 z: %4").arg(e->m_idx).arg(loc.x).arg(loc.y).arg(loc.z), sess);

}

//! @}

