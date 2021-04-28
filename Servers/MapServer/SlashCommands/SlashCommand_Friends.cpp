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

#include "SlashCommand_Friends.h"

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
void cmdHandler_FriendsListDebug(const QStringList &/*params*/, MapClientSession &sess)
{
    QString msg = "Sending FriendsList dump to console output.";
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);

    dumpFriends(*sess.m_ent); // Send FriendsList dump
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Access Level 1 Commands
void cmdHandler_Friend(const QStringList &params, MapClientSession &sess)
{
    Entity* tgt = getEntity(&sess, params.join(" "));
    if(tgt == nullptr || sess.m_ent->m_char->isEmpty() || tgt->m_char->isEmpty())
        return;

    FriendListChangeStatus status = addFriend(*sess.m_ent, *tgt,getEntityDisplayMapName(tgt->m_entity_data));
    if(status==FriendListChangeStatus::MAX_FRIENDS_REACHED)
    {
        QString msg = "You cannot have more than " + QString::number(g_max_friends) + " friends.";
        qCDebug(logFriends).noquote() << msg;
        sendInfoMessage(MessageChannel::USER_ERROR, msg, sess);
    }
    else
    {
        QString msg = "Adding " + tgt->name() + " to your friendlist.";
        qCDebug(logFriends).noquote() << msg;
        sendInfoMessage(MessageChannel::FRIENDS, msg, sess);
        // Send FriendsListUpdate
        sendFriendsListUpdate(sess, sess.m_ent->m_char->m_char_data.m_friendlist);
    }
}

void cmdHandler_Unfriend(const QStringList &params, MapClientSession &sess)
{
    // Cannot use getEntityFromCommand as we need to be able to unfriend logged out characters.
    QString name = params.join(" ");
    if(name.isEmpty())
    {
        const Entity* const tgt = getTargetEntity(sess);
        if(tgt == nullptr)
            return;

        name = tgt->name();
    }

    FriendListChangeStatus status =  removeFriend(*sess.m_ent, name);
    if(status==FriendListChangeStatus::FRIEND_REMOVED)
    {
        QString msg = "Removing " + name + " from your friends list.";
        sendInfoMessage(MessageChannel::FRIENDS, msg, sess);

        // Send FriendsListUpdate
        sendFriendsListUpdate(sess, sess.m_ent->m_char->m_char_data.m_friendlist);
    }
    else
    {
        QString msg = name + " is not on your friends list.";
        sendInfoMessage(MessageChannel::USER_ERROR, msg, sess);
    }
}

void cmdHandler_FriendList(const QStringList &/*params*/, MapClientSession &sess)
{
    if(sess.m_ent->m_char->isEmpty())
        return;

    toggleFriendList(*sess.m_ent);
}

//! @}
