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

#include "SlashCommand_Teams.h"

#include "DataHelpers.h"
#include "GameData/Character.h"
#include "Components/Logging.h"
#include "MapInstance.h"
#include "MessageHelpers.h"
#include "Components/Settings.h"

#include <QtCore/QString>
#include <QtCore/QDebug>

using namespace SEGSEvents;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Access Level 9 Commands (GMs)
void cmdHandler_SetTeam(const QStringList &params, MapClientSession &sess)
{
    uint8_t val = params.value(0).toUInt();

    setTeamID(*sess.m_ent, val);

    QString msg = "Set Team ID to: " + QString::number(val);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_TeamDebug(const QStringList &/*params*/, MapClientSession &sess)
{
    QString msg = "Sending team debug to console output.";
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);

    sess.m_ent->m_team->dump(); // Send team debug info
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Access Level 1 Commands
void cmdHandler_Invite(const QStringList &params, MapClientSession &sess)
{
    Entity* tgt = getEntity(&sess, params.join(" "));
    if(tgt == nullptr)
        return;

    if(tgt->m_has_team)
    {
        const QString msg = tgt->name() + " is already on a team.";
        qCDebug(logTeams) << msg;
        sendInfoMessage(MessageChannel::SERVER, msg, sess);
        return;
    }

    if(tgt->name() == sess.m_name)
    {
        const QString msg = "You cannot invite yourself to a team.";
        qCDebug(logTeams) << msg;
        sendInfoMessage(MessageChannel::SERVER, msg, sess);
        return;
    }

    if(sess.m_ent->m_has_team && sess.m_ent->m_team != nullptr)
    {
        if(!sess.m_ent->m_team->isTeamLeader(sess.m_ent->m_db_id))
        {
            const QString msg = "Only the team leader can invite players to the team.";
            qCDebug(logTeams) << sess.m_ent->name() << msg;
            sendInfoMessage(MessageChannel::TEAM, msg, sess);
            return;
        }
    }

    sendTeamOffer(sess, *tgt->m_client);
}

void cmdHandler_Kick(const QStringList &params, MapClientSession &sess)
{
    Entity* tgt = getEntity(&sess, params.join(" "));
    if(tgt == nullptr)
        return;

    const QString name = tgt->name();
    QString msg;
    if(kickTeam(*tgt))
        msg = "Kicking " + name + " from team.";
    else
        msg = "Failed to kick " + name;

    qCDebug(logSlashCommand).noquote() << msg;
    sendInfoMessage(MessageChannel::TEAM, msg, sess);
}

void cmdHandler_LeaveTeam(const QStringList &/*params*/, MapClientSession &sess)
{
    leaveTeam(*sess.m_ent);
    QString msg = "Leaving Team";
    qCDebug(logSlashCommand).noquote() << msg;
    sendInfoMessage(MessageChannel::TEAM, msg, sess);
}

void cmdHandler_FindMember(const QStringList &/*params*/, MapClientSession &sess)
{
    sendTeamLooking(sess);
    QString msg = "Finding Team Member";
    qCDebug(logSlashCommand).noquote() << msg;
    sendInfoMessage(MessageChannel::CHAT_TEXT, msg, sess);
}

void cmdHandler_MakeLeader(const QStringList &params, MapClientSession &sess)
{
    Entity* tgt = getEntity(&sess, params.join(" "));
    if(tgt == nullptr)
        return;

    const QString name = tgt->name();
    QString msg;
    if(makeTeamLeader(*sess.m_ent,*tgt))
        msg = "Making " + name + " team leader.";
    else
        msg = "Failed to make " + name + " team leader.";

    qCDebug(logSlashCommand).noquote() << msg;
    sendInfoMessage(MessageChannel::TEAM, msg, sess);
}

void cmdHandler_TeamBuffs(const QStringList & /*params*/, MapClientSession &sess)
{
    toggleTeamBuffs(*sess.m_ent->m_player);

    QString msg = "Toggling Team Buffs display mode.";
    qCDebug(logSlashCommand).noquote() << msg;
}

// Sidekick Related
void cmdHandler_Sidekick(const QStringList &params, MapClientSession &sess)
{
    Entity* tgt = getEntity(&sess, params.join(" "));
    if(tgt == nullptr || sess.m_ent->m_char->isEmpty() || tgt->m_char->isEmpty())
        return;

    auto res=inviteSidekick(*sess.m_ent, *tgt);
    static const QLatin1Literal possible_messages[] = {
        QLatin1String("Unable to add sidekick."),
        QLatin1String("To Mentor another player, you must be at least 3 levels higher than them."),
        QLatin1String("To Mentor another player, you must be at least level 10."),
        QLatin1String("You are already Mentoring someone."),
        QLatin1String("Target is already a sidekick."),
        QLatin1String("To Mentor another player, you must be on the same team."),
    };
    if(res==SidekickChangeStatus::SUCCESS)
    {
        // sendSidekickOffer
        sendSidekickOffer(*tgt->m_client, sess.m_ent->m_db_id); // tgt gets dialog, src.db_id is named.
    }
    else
    {
        qCDebug(logTeams).noquote() << possible_messages[int(res)-1];
        sendInfoMessage(MessageChannel::USER_ERROR, possible_messages[int(res)-1], sess);
    }
}

void cmdHandler_UnSidekick(const QStringList &/*params*/, MapClientSession &sess)
{
    if(sess.m_ent->m_char->isEmpty())
        return;
    QString msg;

    uint32_t sidekick_id = getSidekickId(*sess.m_ent->m_char);
    auto res = removeSidekick(*sess.m_ent, sidekick_id);
    if(res==SidekickChangeStatus::GENERIC_FAILURE)
    {
        msg = "You are not sidekicked with anyone.";
        qCDebug(logTeams).noquote() << msg;
        sendInfoMessage(MessageChannel::USER_ERROR, msg, sess);
    }
    else if(res==SidekickChangeStatus::SUCCESS)
    {
        Entity *tgt = getEntityByDBID(sess.m_current_map, sidekick_id);
        QString tgt_name = tgt ? tgt->name() : "Unknown Player";
        if(isSidekickMentor(*sess.m_ent))
        {
            // src is mentor, tgt is sidekick
            msg = QString("You are no longer mentoring %1.").arg(tgt_name);
            sendInfoMessage(MessageChannel::TEAM, msg, sess);
            if(tgt)
            {
                msg = QString("%1 is no longer mentoring you.").arg(sess.m_ent->name());
                sendInfoMessage(MessageChannel::TEAM, msg, *tgt->m_client);
            }
        }
        else
        {
            // src is sidekick, tgt is mentor
            if(tgt)
            {
                msg = QString("You are no longer mentoring %1.").arg(sess.m_ent->name());
                sendInfoMessage(MessageChannel::TEAM, msg, *tgt->m_client);
            }
            msg = QString("%1 is no longer mentoring you.").arg(tgt_name);
            sendInfoMessage(MessageChannel::TEAM, msg, sess);
        }
    }
    else if(res==SidekickChangeStatus::NOT_SIDEKICKED_CURRENTLY)
    {
        msg = QString("You are no longer sidekicked with anyone.");
        sendInfoMessage(MessageChannel::USER_ERROR, msg, sess);
    }
}

// LFG Related
void cmdHandler_LFG(const QStringList &/*params*/, MapClientSession &sess)
{
    toggleLFG(*sess.m_ent);
    QString msg = "Toggling LFG";
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::SERVER, msg, sess);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Access Level 0 Commands
void cmdHandler_TeamAccept(const QStringList &params, MapClientSession &sess)
{
    // game command: "team_accept \"From\" to_db_id to_db_id \"To\""

    QString msgfrom = "Something went wrong with TeamAccept.";
    QString msgtgt = "Something went wrong with TeamAccept.";
    QString from_name       = params.value(0);
    uint32_t tgt_db_id      = params.value(1).toUInt();
    uint32_t tgt_db_id_2    = params.value(2).toUInt(); // always the same?
    QString tgt_name        = params.value(3);

    if(tgt_db_id != tgt_db_id_2)
        qWarning() << "TeamAccept db_ids do not match!";

    Entity *from_ent = getEntity(&sess,from_name);
    if(from_ent == nullptr)
        return;

    if(inviteTeam(*from_ent,*sess.m_ent))
    {
        msgfrom = "Inviting " + tgt_name + " to team.";
        msgtgt = "Joining " + from_name + "'s team.";

    }
    else
    {
        msgfrom = "Failed to invite " + tgt_name + ". They are already on a team.";
    }

    qCDebug(logSlashCommand).noquote() << msgfrom;
    sendInfoMessage(MessageChannel::TEAM, msgfrom, *from_ent->m_client);
    sendInfoMessage(MessageChannel::TEAM, msgtgt, sess);
}

void cmdHandler_TeamDecline(const QStringList &params, MapClientSession &sess)
{
    // game command: "team_decline \"From\" to_db_id \"To\""
    QString msg;
    QString from_name   = params.value(0);
    uint32_t tgt_db_id  = params.value(1).toUInt();
    QString tgt_name    = params.value(2);

    Entity *from_ent = getEntity(&sess,from_name);
    if(from_ent == nullptr)
        return;

    msg = tgt_name + " declined a team invite from " + from_name + QString::number(tgt_db_id);
    qCDebug(logSlashCommand).noquote() << msg;

    msg = tgt_name + " declined your team invite."; // to sender
    sendInfoMessage(MessageChannel::TEAM, msg, *from_ent->m_client);
    msg = "You declined the team invite from " + from_name; // to target
    sendInfoMessage(MessageChannel::TEAM, msg, sess);
}

// Sidekick Related
void cmdHandler_SidekickAccept(const QStringList &/*params*/, MapClientSession &sess)
{
    uint32_t db_id  = sess.m_ent->m_char->m_char_data.m_sidekick.m_db_id;
    //TODO: Check that entity is in the same map ?
    Entity *tgt     = getEntityByDBID(sess.m_current_map,db_id);
    if(tgt == nullptr || sess.m_ent->m_char->isEmpty() || tgt->m_char->isEmpty())
        return;

    addSidekick(*sess.m_ent,*tgt);
    // Send message to each player
    QString msg = QString("You are now Mentoring %1.").arg(tgt->name()); // Customize for src.
    sendInfoMessage(MessageChannel::TEAM, msg, sess);
    msg = QString("%1 is now Mentoring you.").arg(sess.m_ent->name()); // Customize for src.
    sendInfoMessage(MessageChannel::TEAM, msg, *tgt->m_client);
}

void cmdHandler_SidekickDecline(const QStringList &/*params*/, MapClientSession &sess)
{
    sess.m_ent->m_char->m_char_data.m_sidekick.m_db_id = 0;
}

//! @}
