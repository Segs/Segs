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

#include "SlashCommand_SuperGroup.h"

#include "GameData/Character.h"
#include "GameData/EntityHelpers.h"
#include "Logging.h"
#include "MapInstance.h"
#include "Messages/Map/SuperGroupEvents.h"
#include "MessageHelpers.h"

#include <QtCore/QString>
#include <QtCore/QDebug>

using namespace SEGSEvents;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Access Level 9 Commands (GMs)
void cmdHandler_SetSuperGroup(const QStringList &params, MapClientSession &sess)
{
    int sg_id       = params.value(0).toInt();
    QString sg_name = params.value(1);
    //int sg_rank     = params.value(2).toInt();

    setSuperGroup(*sess.m_ent, bool(sg_id), sg_name);

    QString msg = QString("Set SuperGroup:  sg_id: %1  name: %2")
            .arg(sg_id)
            .arg(sg_name);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_RegisterSuperGroup(const QStringList &params, MapClientSession &sess)
{
    QString val = params.value(0);

    sendRegisterSuperGroup(sess, val);

    QString msg = QString("Register SuperGroup: %1").arg(val);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_SuperGroupCostume(const QStringList &/*params*/, MapClientSession &sess)
{
    SuperGroupStats *sgs = &sess.m_ent->m_char->m_char_data.m_supergroup;
    if(!sgs->m_has_supergroup && !sgs->m_has_sg_costume)
        return;

    sendSuperGroupCostume(sess, sgs->m_sg_costume);

    QString msg = QString("Sending SuperGroup Costume");
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Access Level 1 Commands
void cmdHandler_SuperGroupStats(const QStringList &/*params*/, MapClientSession &/*sess*/)
{
    // TODO: things =D
}

void cmdHandler_SuperGroupLeave(const QStringList &/*params*/, MapClientSession &sess)
{
    leaveSG(*sess.m_ent);
    QString msg = "Leaving SuperGroup";
    qCDebug(logSlashCommand).noquote() << msg;
    sendInfoMessage(MessageChannel::SUPERGROUP, msg, sess);
}

void cmdHandler_SuperGroupInvite(const QStringList &params, MapClientSession &sess)
{
    QString msg;
    const Entity* tgt = getEntity(&sess, params.join(" "));
    if(tgt == nullptr)
        return;

    qCDebug(logSuperGroups) << sess.m_ent->name() << sess.m_ent->m_db_id << tgt->m_db_id << tgt->name();

    if(!tgt->m_char->m_char_data.m_supergroup.m_has_supergroup)
    {
        QString from_name = sess.m_ent->name();
        tgt->m_client->addCommand<SuperGroupOffer>(sess.m_ent->m_db_id, from_name);
        msg = "Inviting " + tgt->name() + " to your supergroup.";
    }
    else
        msg = tgt->name() + " is already a member of a SuperGroup!";

    qCDebug(logSuperGroups) << sess.m_ent->name() << msg;
    sendInfoMessage(MessageChannel::SUPERGROUP, msg, sess);
}

void cmdHandler_SuperGroupKick(const QStringList &params, MapClientSession &sess)
{
    QString msg;
    Entity* tgt = getEntity(&sess, params.join(" "));
    if(tgt == nullptr)
        return;

    msg = kickSG(*sess.m_ent, *tgt);
    qCDebug(logSuperGroups) << sess.m_ent->name() << msg;
    sendInfoMessage(MessageChannel::SUPERGROUP, msg, sess);
}

void cmdHandler_SuperGroupMOTD(const QStringList &params, MapClientSession &sess)
{
    QString motd = params.join(" ");
    QString msg = setSGMOTD(*sess.m_ent, motd);

    qCDebug(logSlashCommand).noquote() << msg;
    sendInfoMessage(MessageChannel::SUPERGROUP, msg, sess);
}

void cmdHandler_SuperGroupMotto(const QStringList &params, MapClientSession &sess)
{
    QString motto = params.join(" ");
    QString msg = setSGMotto(*sess.m_ent, motto);

    qCDebug(logSlashCommand).noquote() << msg;
    sendInfoMessage(MessageChannel::SUPERGROUP, msg, sess);
}

void cmdHandler_SuperGroupMode(const QStringList &/*params*/, MapClientSession &sess)
{
    bool result = toggleSGMode(*sess.m_ent);

    QString msg = "Toggling SuperGroup Mode to " + result;
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::SUPERGROUP, msg, sess);
}

void cmdHandler_SuperGroupPromote(const QStringList &params, MapClientSession &sess)
{
    QString msg;
    Entity* tgt = getEntity(&sess, params.join(" "));
    if(tgt == nullptr)
        return;

    msg = modifySGRank(*sess.m_ent, *tgt, 1);
    qCDebug(logSuperGroups) << sess.m_ent->name() << msg;
    sendInfoMessage(MessageChannel::SUPERGROUP, msg, sess);
}

void cmdHandler_SuperGroupDemote(const QStringList &params, MapClientSession &sess)
{
    QString msg;
    Entity* tgt = getEntity(&sess, params.join(" "));
    if(tgt == nullptr)
        return;

    msg = modifySGRank(*sess.m_ent, *tgt, -1);
    qCDebug(logSuperGroups) << sess.m_ent->name() << msg;
    sendInfoMessage(MessageChannel::SUPERGROUP, msg, sess);
}

void cmdHandler_SuperGroupTitles(const QStringList &params, MapClientSession &sess)
{
    int idx = 0;
    QString msg = "Failed to change SG Titles.";

    if(params.at(0).startsWith("nameLeader ", Qt::CaseInsensitive))
        idx = 0;
    else if(params.at(0).startsWith("nameCaptain ", Qt::CaseInsensitive))
        idx = 1;
    else if(params.at(0).startsWith("nameMember ", Qt::CaseInsensitive))
        idx = 2;
    else
        idx = 3; // all titles

    if(idx == 3)
    {
        msg.clear();

        for(int i = 0; i<3; ++i)
            msg += setSGTitle(*sess.m_ent, i, params.value(i+1)) + "\n";
    }
    else
        msg = setSGTitle(*sess.m_ent, idx, params.value(1)); // take first arg only

    qCDebug(logSuperGroups) << sess.m_ent->name() << msg;
    sendInfoMessage(MessageChannel::SUPERGROUP, msg, sess);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Access Level 0 Commands
void cmdHandler_SuperGroupAccept(const QStringList &params, MapClientSession &sess)
{
    // game command: "sg_accept \"From\" to_db_id to_db_id \"To\""
    QString from_name       = params.value(1);
    uint32_t src_db_id      = params.value(2).toUInt();
    uint32_t tgt_db_id      = params.value(3).toUInt(); // always the same?
    QString tgt_name        = params.value(4);

    if(tgt_name != sess.m_ent->name())
    {
        qCDebug(logSlashCommand) << "SuperGroupAccept name does not match target.";
        return;
    }

    qCDebug(logSuperGroups) << from_name << src_db_id << tgt_db_id << tgt_name;

    Entity *from_ent = getEntity(&sess, from_name);
    if(from_ent == nullptr)
        return;

    SGResponse response = inviteSG(*from_ent, *sess.m_ent);

    qCDebug(logSlashCommand).noquote() << response.msgfrom;
    sendInfoMessage(MessageChannel::SUPERGROUP, response.msgfrom, *from_ent->m_client);

    if(response.is_success)
        sendInfoMessage(MessageChannel::SUPERGROUP, response.msgtgt, sess);
}

void cmdHandler_SuperGroupDecline(const QStringList &params, MapClientSession &sess)
{
    // game command: "sg_decline \"From\" to_db_id \"To\""
    QString msg;
    QString from_name   = params.value(1);
    uint32_t tgt_db_id  = params.value(2).toUInt();
    QString tgt_name    = params.value(3);

    Entity *from_ent = getEntity(&sess, from_name);
    if(from_ent == nullptr)
        return;

    msg = tgt_name + " declined a SuperGroup invite from " + from_name + " db_id: " + QString::number(tgt_db_id);
    qCDebug(logSlashCommand).noquote() << msg;

    msg = tgt_name + " declined your SuperGroup invite."; // to sender
    sendInfoMessage(MessageChannel::SUPERGROUP, msg, *from_ent->m_client);
    msg = "You declined the SuperGroup invite from " + from_name; // to target
    sendInfoMessage(MessageChannel::SUPERGROUP, msg, sess);
}

//! @}
