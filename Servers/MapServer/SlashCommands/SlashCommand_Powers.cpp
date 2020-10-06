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

#include "SlashCommand_Powers.h"

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
void cmdHandler_AddEntirePowerSet(const QStringList &params, MapClientSession &sess)
{
    if(params.size() < 2)
    {
        qCDebug(logSlashCommand) << "Bad invocation:" << params.join(" ");
        sendInfoMessage(MessageChannel::USER_ERROR, "Bad invocation:" + params.join(" "), sess);
        return;
    }

    CharacterData &cd = sess.m_ent->m_char->m_char_data;
    QString floating_msg = FloatingInfoMsg.find(FloatingMsg_FoundClue).value();

    uint32_t v1 = params.value(0).toUInt();
    uint32_t v2 = params.value(1).toUInt();

    QString msg = QString("Granting Entire PowerSet <%1, %2> to %3").arg(v1).arg(v2).arg(sess.m_ent->name());

    PowerPool_Info ppool;
    ppool.m_pcat_idx = v1;
    ppool.m_pset_idx = v2;

    addEntirePowerSet(cd, ppool);

    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
    sendFloatingInfo(sess, floating_msg, FloatingInfoStyle::FloatingInfo_Attention, 4.0);
}

void cmdHandler_AddPower(const QStringList &params, MapClientSession &sess)
{
    if(params.size() < 3)
    {
        qCDebug(logSlashCommand) << "Bad invocation:" << params.join(" ");
        sendInfoMessage(MessageChannel::USER_ERROR, "Bad invocation:" + params.join(" "), sess);
        return;
    }

    CharacterData &cd = sess.m_ent->m_char->m_char_data;
    QString floating_msg = FloatingInfoMsg.find(FloatingMsg_FoundClue).value();

    uint32_t v1 = params.value(0).toUInt();
    uint32_t v2 = params.value(1).toUInt();
    uint32_t v3 = params.value(2).toUInt();

    QString msg = QString("Granting Power <%1, %2, %3> to %4").arg(v1).arg(v2).arg(v3).arg(sess.m_ent->name());

    PowerPool_Info ppool;
    ppool.m_pcat_idx = v1;
    ppool.m_pset_idx = v2;
    ppool.m_pow_idx = v3;

    addPower(cd, ppool);

    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
    sendFloatingInfo(sess, floating_msg, FloatingInfoStyle::FloatingInfo_Attention, 4.0);
}

void cmdHandler_AddInspiration(const QStringList &params, MapClientSession &sess)
{
    QString val = params.join(" ");
    giveInsp(sess, val);
}

void cmdHandler_AddEnhancement(const QStringList &params, MapClientSession &sess)
{
    if(params.isEmpty())
    {
        qCDebug(logSlashCommand) << "Bad invocation:" << params.join(" ") << " requires the enhancement name";
        sendInfoMessage(MessageChannel::USER_ERROR, "Bad invocation:" + params.join(" ") + " requires the enhancement name", sess);
        return;
    }

    // second param is optional level
    QString name = params.value(0);
    uint32_t level = getLevel(*sess.m_ent->m_char);
    if(params.size() > 1)
        level = params.value(1).toUInt() -1;

    giveEnhancement(sess, name, level);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Access Level 1 Commands
void cmdHandler_ReSpec(const QStringList &/*params*/, MapClientSession &sess)
{
    CharacterData &cd = sess.m_ent->m_char->m_char_data;

    if(sess.m_ent->m_char->isEmpty())
        return;

    QString msg = "No powersets found for player " + sess.m_ent->name();

    if(cd.m_powersets.size() > 1)
    {
        msg = "Removing all powers for player " + sess.m_ent->name();
        cd.m_reset_powersets = true;
        cd.m_has_updated_powers = true;
    }

    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
    qCDebug(logSlashCommand).noquote() << msg;
}


//! @}
