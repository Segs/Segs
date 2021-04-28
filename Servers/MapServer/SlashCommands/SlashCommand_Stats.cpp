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

#include "SlashCommand_Stats.h"

#include "DataHelpers.h"
#include "GameData/Character.h"
#include "GameData/CharacterHelpers.h"
#include "Components/Logging.h"
#include "MapInstance.h"
#include "Messages/Map/FloatingInfoStyles.h"
#include "MessageHelpers.h"
#include "Components/Settings.h"

#include <QtCore/QString>
#include <QtCore/QDebug>

using namespace SEGSEvents;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Access Level 9 Commands (GMs)
void cmdHandler_Falling(const QStringList &/*params*/, MapClientSession &sess)
{
    toggleFalling(*sess.m_ent);

    QString msg = "Toggling falling";
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_Sliding(const QStringList &/*params*/, MapClientSession &sess)
{
    toggleSliding(*sess.m_ent);

    QString msg = "Toggling sliding";
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_Jumping(const QStringList &/*params*/, MapClientSession &sess)
{
    toggleJumping(*sess.m_ent);

    QString msg = "Toggling jumping";
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_Stunned(const QStringList &/*params*/, MapClientSession &sess)
{
    toggleStunned(*sess.m_ent);

    QString msg = "Toggling stunned";
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_SetSpeed(const QStringList &params, MapClientSession &sess)
{
    float v1 = params.value(0).toFloat();
    float v2 = params.value(1).toFloat();
    float v3 = params.value(2).toFloat();
    setSpeed(*sess.m_ent, v1, v2, v3);

    QString msg = QString("Set Speed to: <%1,%2,%3>").arg(v1).arg(v2).arg(v3);

    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_SetBackupSpd(const QStringList &params, MapClientSession &sess)
{
    float val = params.value(0).toFloat();
    setBackupSpd(*sess.m_ent, val);

    QString msg = "Set BackupSpd to: " + QString::number(val);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_SetJumpHeight(const QStringList &params, MapClientSession &sess)
{
    float val = params.value(0).toFloat();
    setJumpHeight(*sess.m_ent, val);

    QString msg = "Set JumpHeight to: " + QString::number(val);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_SetHP(const QStringList &params, MapClientSession &sess)
{
    float attrib = params.value(0).toFloat();

    changeHP(*sess.m_ent, attrib);

    QString msg = QString("Setting HP to: %1 / %2")
            .arg(attrib).arg(getMaxHP(*sess.m_ent->m_char));
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_SetEnd(const QStringList &params, MapClientSession &sess)
{
    float attrib = params.value(0).toFloat();
    float maxattrib = sess.m_ent->m_char->m_max_attribs.m_Endurance;

    if(attrib > maxattrib)
        attrib = maxattrib;

    setEnd(*sess.m_ent->m_char,attrib);

    QString msg = QString("Setting Endurance to: %1 / %2").arg(attrib).arg(maxattrib);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_SetXP(const QStringList &params, MapClientSession &sess)
{
    uint32_t attrib = params.value(0).toUInt();
    uint32_t lvl = getLevel(*sess.m_ent->m_char);

    setXP(*sess.m_ent->m_char, attrib);
    QString msg = "Setting XP to " + QString::number(attrib);

    uint32_t newlvl = getLevel(*sess.m_ent->m_char);
    if(lvl != newlvl)
        msg += " and LVL to " + QString::number(newlvl);

    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_GiveXP(const QStringList &params, MapClientSession &sess)
{
    uint32_t attrib = params.value(0).toUInt();
    uint32_t lvl = getLevel(*sess.m_ent->m_char);

    giveXp(sess, attrib);
    QString msg = "Giving " + QString::number(attrib) + " XP";

    uint32_t newlvl = getLevel(*sess.m_ent->m_char);
    if(lvl != newlvl)
        msg += " and setting LVL to " + QString::number(newlvl);

    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_SetDebt(const QStringList &params, MapClientSession &sess)
{
    uint32_t attrib = params.value(0).toUInt();

    setDebt(*sess.m_ent->m_char, attrib);
    QString msg = QString("Setting XP Debt to %1").arg(attrib);

    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_SetInf(const QStringList &params, MapClientSession &sess)
{
    uint32_t attrib = params.value(0).toUInt();

    setInf(*sess.m_ent->m_char, attrib);

    QString msg = "Setting influence to: " + QString::number(attrib);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_SetLevel(const QStringList &params, MapClientSession &sess)
{
    uint32_t attrib = params.value(0).toUInt() - 1; // convert from 1-50 to 0-49

    setLevel(*sess.m_ent->m_char, attrib);

    QString contents = FloatingInfoMsg.find(FloatingMsg_Leveled).value();
    sendFloatingInfo(sess, contents, FloatingInfoStyle::FloatingInfo_Attention, 4.0);

    QString msg = "Setting Level to: " + QString::number(attrib + 1);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_SetCombatLevel(const QStringList &params, MapClientSession &sess)
{
    uint32_t attrib = params.value(0).toUInt() - 1; // convert from 1-50 to 0-49

    setCombatLevel(*sess.m_ent->m_char, attrib);

    QString msg = "Setting Combat Level to: " + QString::number(attrib+1);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Access Level 2 Commands
void cmdHandler_Alignment(const QStringList &params, MapClientSession &sess)
{
    if(params.size() == 1)
    {
        setAlignment(*sess.m_ent, params.at(0));
        sendInfoMessage(MessageChannel::DEBUG_INFO, "New alignment: " + params.at(0), sess);
        return;
    }
    QString msg = "Choose from hero, villain, both or none/neither: ";
    qCDebug(logSlashCommand) << msg << params.join(" ");
    sendInfoMessage(MessageChannel::USER_ERROR, msg + params.join(" "), sess);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Access Level 1 Commands
void cmdHandler_AFK(const QStringList &params, MapClientSession &sess)
{
    Entity* e = sess.m_ent;
    QString afk_msg = params.join(" ");
    toggleAFK(*e->m_char, afk_msg);

    QString msg = "Setting afk message to: " + afk_msg;
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::EMOTE, msg, sess);

    // the server regards writing on chat (including cmd commands) as an input
    // so specifically for afk, we treat it as a non-input in this way
    e->m_has_input_on_timeframe = false;
}

void cmdHandler_SetTitles(const QStringList &params, MapClientSession &sess)
{
    QString title = params.join(" ");
    setTitle(sess, title);
}

void cmdHandler_SetCustomTitles(const QStringList &params, MapClientSession &sess)
{
    bool        prefix;
    QString     msg, generic, origin, special;

    if(params.size() == 0)
    {
        setTitles(*sess.m_ent->m_char);
        msg = "Titles reset to nothing";
    }
    else
    {
        prefix  = !params.value(0).isEmpty();
        generic = params.value(1);
        origin  = params.value(2);
        special = params.value(3);
        setTitles(*sess.m_ent->m_char, prefix, generic, origin, special);
        msg = "Titles changed to: " + QString::number(prefix) + " " + generic + " " + origin + " " + special;
    }
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::USER_ERROR, msg, sess);
}

void cmdHandler_SetSpecialTitle(const QStringList &params, MapClientSession &sess)
{
    bool        prefix;
    QString     msg, generic, origin, special;
    prefix = sess.m_ent->m_char->m_char_data.m_has_the_prefix;
    generic = getGenericTitle(*sess.m_ent->m_char);
    origin = getOriginTitle(*sess.m_ent->m_char);
    special = params.join(" ");

    setTitles(*sess.m_ent->m_char, prefix, generic, origin, special);
    msg = "Titles changed to: " + QString::number(prefix) + " " + generic + " " + origin + " " + special;

    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::USER_ERROR, msg, sess);
}

void cmdHandler_SetAssistTarget(const QStringList &/*params*/, MapClientSession &sess)
{
    // it appears that `/assist` should target the target of your current target
    // but it also seems that what we call m_assist_target_idx is actually
    // the last friendly target you selected. We need to check target type
    // and set m_target_idx or m_assist_target_idx depending on result.

    Entity *target_ent = getTargetEntity(sess);
    if(target_ent == nullptr)
        return;

    uint32_t new_target = getTargetIdx(*target_ent);
    if(new_target == 0)
        return;

    if(target_ent->m_is_villain)
        setTarget(*sess.m_ent, new_target);
    else
        setAssistTarget(*sess.m_ent, new_target);

    QString msg = "Now targeting " + target_ent->name() + "'s target";
    sendInfoMessage(MessageChannel::TEAM, msg, sess);
    qCDebug(logSlashCommand).noquote() << msg;
}

//! @}
