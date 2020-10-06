/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup SlashCommands Projects/CoX/Servers/MapServer/SlashCommands
 * @{
 */

#include "SlashCommand_Debug.h"

#include "DataHelpers.h"
#include "GameData/Character.h"
#include "GameData/CharacterHelpers.h"
#include "GameData/EntityHelpers.h"
#include "Components/Logging.h"
#include "MapInstance.h"
#include "Messages/Map/Browser.h"
#include "Messages/Map/DoorMessage.h"
#include "Messages/Map/StandardDialogCmd.h"
#include "MessageHelpers.h"

#include <QtCore/QString>
#include <QtCore/QDebug>

using namespace SEGSEvents;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Access Level 9 Commands (GMs)
void cmdHandler_Dialog(const QStringList &params, MapClientSession &sess)
{
    sess.addCommandToSendNextUpdate(std::make_unique<StandardDialogCmd>(params.join(" ")));
}

void cmdHandler_InfoMessage(const QStringList &params, MapClientSession &sess)
{
    QString msg;
    int cmdType = int(MessageChannel::USER_ERROR);

    if(params.length() < 2)
        msg = "The /imsg command takes two arguments, a <b>number</b> and a <b>string</b>";
    else
    {
        bool ok = true;
        cmdType = params.value(0).toInt(&ok);
        if(!ok || cmdType<1 || cmdType>21)
        {
            msg = "The first /imsg argument must be a <b>number</b> between 1 and 21";
            cmdType = int(MessageChannel::USER_ERROR);
        }
        else
            msg = params.at(1);
    }
    sendInfoMessage(static_cast<MessageChannel>(cmdType), msg, sess);
}

void cmdHandler_UpdateChar(const QStringList &/*params*/, MapClientSession &sess)
{
    markEntityForDbStore(sess.m_ent, DbStoreFlags::Full);

    QString msg = "Updating Character in Database: " + sess.m_ent->name();
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_DebugChar(const QStringList &/*params*/, MapClientSession &sess)
{
    const Character &chardata(*sess.m_ent->m_char);
    QString msg = "DebugChar: " + sess.m_ent->name()
            + "\n  " + chardata.m_char_data.m_origin_name
            + "\n  " + chardata.m_char_data.m_class_name
            + "\n  map: " + getEntityDisplayMapName(sess.m_ent->m_entity_data)
            + "\n  db_id: " + QString::number(sess.m_ent->m_db_id) + ":" + QString::number(chardata.m_db_id)
            + "\n  idx: " + QString::number(sess.m_ent->m_idx)
            + "\n  access: " + QString::number(sess.m_ent->m_entity_data.m_access_level)
            + "\n  acct: " + QString::number(chardata.m_account_id)
            + "\n  lvl/clvl: " + QString::number(chardata.m_char_data.m_level+1) + "/" + QString::number(chardata.m_char_data.m_combat_level+1)
            + "\n  inf: " + QString::number(chardata.m_char_data.m_influence)
            + "\n  xp/debt: " + QString::number(chardata.m_char_data.m_experience_points) + "/" + QString::number(chardata.m_char_data.m_experience_debt)
            + "\n  lfg: " + QString::number(chardata.m_char_data.m_lfg)
            + "\n  afk: " + QString::number(chardata.m_char_data.m_afk)
            + "\n  tgt_idx: " + QString::number(getTargetIdx(*sess.m_ent));
    sess.m_ent->dump();
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_SendFloatingNumbers(const QStringList &params, MapClientSession &sess)
{
    Entity *tgt = nullptr;

    QString msg; // result messages
    bool ok1 = true;
    bool ok2 = true;
    uint32_t runtimes   = params.value(0).toUInt(&ok1);
    float amount        = params.value(1).toFloat(&ok2);
    QString name        = params.value(2);

    // reign in the insanity
    runtimes = std::clamp(runtimes, uint32_t(0), uint32_t(5));

    if(!ok1 || !ok2 || name.isEmpty())
    {
        msg = "FloatingNumbers takes three arguments: `/damage <number_times_to_run> <damage_amount> <target_name>`";
        qCDebug(logSlashCommand) << msg;
        sendInfoMessage(MessageChannel::USER_ERROR, msg, sess);
        return;
    }

    tgt = getEntity(&sess,name); // get Entity by name

    if(tgt == nullptr)
    {
        msg = "FloatingNumbers target " + name + " cannot be found.";
        qCDebug(logSlashCommand) << msg;
        sendInfoMessage(MessageChannel::USER_ERROR, msg, sess);
        return;
    }

    for(uint32_t i = 0; i<runtimes; i++)
    {
        sendFloatingNumbers(sess, tgt->m_idx, int(amount));

        setHP(*tgt->m_char, getHP(*tgt->m_char) - amount); // deal dmg

        if(amount >= 0) // damage
        {
            msg = QString("%1 deals %2 points of damage to %3.").arg(sess.m_ent->name()).arg(amount).arg(name);
            qCDebug(logSlashCommand) << msg;

            msg = QString("You deal %1 points of damage to %2.").arg(QString::number(amount), name);
            sendInfoMessage(MessageChannel::DAMAGE, msg, sess);
            msg = QString("%1 has dealt you %2 points of damage!").arg(sess.m_ent->name()).arg(amount);
            sendInfoMessage(MessageChannel::DAMAGE, msg, *tgt->m_client);
        }
        else
        {
            msg = QString("%1 heals %2 points of damage from %3.").arg(sess.m_ent->name()).arg(-amount).arg(name);
            qCDebug(logSlashCommand) << msg;

            msg = QString("You heal %1 points of damage from %2.").arg(-amount).arg(name);
            sendInfoMessage(MessageChannel::TEAM, msg, sess); // TEAM for green
            msg = QString("%1 has healed %2 points of damage from you!").arg(sess.m_ent->name()).arg(-amount);
            sendInfoMessage(MessageChannel::TEAM, msg, *tgt->m_client); // TEAM for green
        }
    }
}

void cmdHandler_SetSequence(const QStringList &params, MapClientSession &sess)
{
    bool        update  = params.value(0).toUInt();
    uint32_t    idx     = params.value(1).toUInt();
    uint8_t     time    = params.value(2).toUInt();

    QString msg = "Setting Sequence " + QString::number(idx) + " for " + QString::number(time);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);

    sess.m_ent->m_seq_update = update;
    sess.m_ent->m_seq_move_idx = idx;
    sess.m_ent->m_seq_move_change_time = time;
}

void cmdHandler_AddTriggeredMove(const QStringList &params, MapClientSession &sess)
{
    uint32_t move_idx, delay, fx_idx;
    move_idx    = params.value(0).toUInt();
    delay       = params.value(1).toUInt();
    fx_idx      = params.value(2).toUInt();

    addTriggeredMove(*sess.m_ent, move_idx, delay, fx_idx);

    QString msg = QString("Setting TriggeredMove: idx %1;  ticks: %2;  fx_idx: %3").arg(move_idx).arg(delay).arg(fx_idx);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_AddTimeStateLog(const QStringList &params, MapClientSession &sess)
{
    int val = params.value(0).toUInt();

    if(val == 0)
        val = ::time(nullptr);

    sendTimeStateLog(sess, val);

    QString msg = "Set TimeStateLog to: " + QString::number(val);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_SetClientState(const QStringList &params, MapClientSession &sess)
{
    int val = params.value(0).toUInt();

    sendClientState(sess, ClientStates(val));

    QString msg = "Setting ClientState to: " + QString::number(val);
    //qCDebug(logSlashCommand) << msg; // we're already sending a debug msg elsewhere
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_LevelUpXp(const QStringList &params, MapClientSession &sess)
{
    // start with our current level + 1, unless provided with one
    uint32_t level = getLevel(*sess.m_ent->m_char) + 1;
    if(!params.isEmpty())
        level = params.value(0).toUInt();

    GameDataStore &data(getGameData());
    // must adjust level for 0-index array, capped at 49
    uint32_t max_level = data.expMaxLevel();
    level = std::max(uint32_t(0), std::min(level, max_level));

    // XP must be high enough for the level you're advancing to
    // since this slash command is forcing a levelup, let's
    // increase xp accordingly
    if(getXP(*sess.m_ent->m_char) < data.expForLevel(level))
        setXP(*sess.m_ent->m_char, data.expForLevel(level));
    else
        return;

    qCDebug(logPowers) << "LEVELUP" << sess.m_ent->name() << "to" << level + 1
                       << "NumPowers:" << countAllOwnedPowers(sess.m_ent->m_char->m_char_data, false) // no temps
                       << "NumPowersAtLevel:" << data.countForLevel(level, data.m_pi_schedule.m_Power);

    // send levelup pkt to client
    sess.m_ent->m_char->m_client_window_state = ClientWindowState::Training; // flag character so we can handle dialog response
    sendLevelUp(sess);
}

void cmdHandler_TestDeadNoGurney(const QStringList &/*params*/, MapClientSession &sess)
{
    qCDebug(logSlashCommand) << "Sending DeadNoGurney";
    sendDeadNoGurney(sess);
}

void cmdHandler_DoorMessage(const QStringList &params, MapClientSession &sess)
{
    if(params.size() < 2)
    {
        qCDebug(logSlashCommand) << "Bad invocation:" << params.join(" ");
        sendInfoMessage(MessageChannel::USER_ERROR, "Bad invocation:" + params.join(" "), sess);
        return;
    }

    bool ok = true;
    uint32_t delay_status = params.at(0).toUInt(&ok);

    if(!ok || delay_status > 2)
    {
        qCDebug(logSlashCommand) << "First argument must be 0, 1, or 2;";
        sendInfoMessage(MessageChannel::USER_ERROR, "First argument must be 0, 1, or 2;", sess);
        return;
    }

    // Combine params after int and use those as door message
    QString msg = params.mid(1).join(" ");
    sendDoorMessage(sess, DoorMessageStatus(delay_status), msg);
}

void cmdHandler_Browser(const QStringList &params, MapClientSession &sess)
{
    QString content = params.join(" ");
    sendBrowser(sess, content);
}

void cmdHandler_SendTimeUpdate(const QStringList &/*params*/, MapClientSession &sess)
{
    // client expects PostgresEpoch of Jan 1 2000
    QDateTime base_date(QDate(2000,1,1));
    int32_t time_in_sec = static_cast<int32_t>(base_date.secsTo(QDateTime::currentDateTime()));

    sendTimeUpdate(sess, time_in_sec);
}

void cmdHandler_SendWaypoint(const QStringList &params, MapClientSession &sess)
{
    if(params.size() < 3)
    {
        qCDebug(logSlashCommand) << "Bad invocation: " << params.join(" ");
        sendInfoMessage(MessageChannel::USER_ERROR, "Bad invocation:" + params.join(" "), sess);
        return;
    }

    Destination cur_dest = getCurrentDestination(*sess.m_ent);
    int idx = cur_dest.point_idx; // client will only change waypoint if idx == client_side_idx

    glm::vec3 loc {
      params.at(0).toFloat(),
      params.at(1).toFloat(),
      params.at(2).toFloat()
    };

    QString msg = QString("Sending SendWaypoint: %1 <%2, %3, %4>")
                  .arg(idx)
                  .arg(loc.x, 0, 'f', 1)
                  .arg(loc.y, 0, 'f', 1)
                  .arg(loc.z, 0, 'f', 1);


    sendWaypoint(sess, idx, loc);
    setCurrentDestination(*sess.m_ent, idx, loc);
    sendInfoMessage(MessageChannel::USER_ERROR, msg, sess);
}

void cmdHandler_SetStateMode(const QStringList &params, MapClientSession &sess)
{
    uint32_t val = params.value(0).toUInt();

    sess.m_ent->m_state_mode = static_cast<ClientStates>(val);
    sess.m_ent->m_entity_update_flags.setFlag(sess.m_ent->UpdateFlag::STATEMODE);

    QString msg = "Set StateMode to: " + QString::number(val);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_Revive(const QStringList &params, MapClientSession &sess)
{
    Entity *tgt = nullptr;
    QString msg = "Revive format is '/revive {lvl} {optional: target_name}'";
    if(params.size() < 1)
    {
        qCDebug(logSlashCommand) << msg;
        sendInfoMessage(MessageChannel::USER_ERROR, msg, sess);
        return;
    }

    int revive_lvl = params.at(0).toUInt();
    if(params.size() > 1)
    {
        QString name = params.at(1);
        tgt = getEntity(&sess, name); // get Entity by name
        if(tgt == nullptr)
        {
            msg = QString("Revive target %1 cannot be found. Targeting Self.").arg(name);
            qCDebug(logSlashCommand) << msg;
            sendInfoMessage(MessageChannel::USER_ERROR, msg, sess);
            tgt = sess.m_ent;
        }
    }
    else
        tgt = sess.m_ent;

    revivePlayer(*tgt, static_cast<ReviveLevel>(revive_lvl));

    msg = "Reviving " + tgt->name();
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_AddCostumeSlot(const QStringList &/*params*/, MapClientSession &sess)
{
    sess.m_ent->m_char->addCostumeSlot();
    markEntityForDbStore(sess.m_ent, DbStoreFlags::Full);

    QString msg = "Adding Costume Slot to " + sess.m_ent->name();
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_ForceLogout(const QStringList &params, MapClientSession &sess)
{
    if(params.size() < 2)
    {
        QString msg = "ForceLogout requires a logout message. /forceLogout <HeroName> <Message!>";
        qCDebug(logSlashCommand) << msg;
        sendInfoMessage(MessageChannel::USER_ERROR, msg, sess);
        return;
    }

    QString name = params.at(0);
    QString message = params.at(1);
    sendForceLogout(sess, name, message);
}

void cmdHandler_SendLocations(const QStringList &/*params*/, MapClientSession &sess)
{
    VisitLocation visitlocation;
    visitlocation.m_location_name = "Test1";
    visitlocation.m_pos = glm::vec3(-44, 0, 261);

    sendLocation(sess, visitlocation);
}

void cmdHandler_SendConsoleOutput(const QStringList &params, MapClientSession &sess)
{
    if(params.size() < 1)
    {
        qCDebug(logSlashCommand) << "SendConsoleOutput. Bad invocation:  " << params.join(" ");
        sendInfoMessage(MessageChannel::USER_ERROR, "ConsoleOutput format is '/consoleOutput <Message>'", sess);
        return;
    }

    QString msg = params.join(" ");
    sendDeveloperConsoleOutput(sess, msg);
}

void cmdHandler_SendConsolePrint(const QStringList &params, MapClientSession &sess)
{
    if(params.size() < 1)
    {
        qCDebug(logSlashCommand) << "SendConsolePrintF. Bad invocation:  " << params.join(" ");
        sendInfoMessage(MessageChannel::USER_ERROR, "ConsolePrintF format is '/consolePrintF <Message>'", sess);
        return;
    }

    QString msg = params.join(" ");
    sendClientConsoleOutput(sess, msg);
}

void cmdHandler_ClearTarget(const QStringList &params, MapClientSession &sess)
{
    if(params.size() != 1)
    {
        qCDebug(logSlashCommand) << "ClearTarget. Bad invocation:  " << params.join(" ");
        sendInfoMessage(MessageChannel::USER_ERROR, "ClearTarget '/clearTarget <targetIdx>'", sess);
        return;
    }

    int idx = params.at(0).toInt();
    setTarget(*sess.m_ent, idx);
}

void cmdHandler_StartTimer(const QStringList &params, MapClientSession &sess)
{
    if(params.size() != 2)
    {
        qCDebug(logSlashCommand) << "StartTimer. Bad invocation:  " << params.join(" ");
        sendInfoMessage(MessageChannel::USER_ERROR, "StartTimer '/StartTimer <timerName> <seconds>'", sess);
        return;
    }
    QString message = params.at(0);
    float time = params.at(1).toFloat();

    sendMissionObjectiveTimer(sess, message, time);
}

// Slash commands for setting bit values
void cmdHandler_SetU1(const QStringList &params, MapClientSession &sess)
{
    uint32_t val = params.value(0).toUInt();

    setu1(*sess.m_ent, val);

    QString msg = "Set u1 to: " + QString::number(val);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

//! @}
