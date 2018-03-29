/*
 * Super Entity Game Server Project
 * https://github.com/Segs/Segs
 * Copyright (c) 2006 - 2018 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#include "Servers/MapServer/DataHelpers.h"
#include "Friend.h"
#include "Entity.h"
#include "Logging.h"

// Max number of friends on friendslist -- client caps at 25 entries
static const int g_max_friends = 25;

void addFriend(Entity &src, Entity &tgt)
{
    QString msg;
    FriendsList *src_data(&src.m_char.m_char_data.m_friendlist);
    FriendsList *tgt_data(&tgt.m_char.m_char_data.m_friendlist);

    if(src_data->m_friends_count >= g_max_friends)
    {
        msg = "You cannot have more than " + QString::number(g_max_friends) + " friends.";
        qCDebug(logFriends).noquote() << msg;
        messageOutput(MessageChannel::USER_ERROR, msg, src);
        return; // break early
    }

    src_data->m_has_friends = true;
    src_data->m_friends_count++;
    src_data->m_friends_v2 = 1;                          // what do we do with this?

    Friend f;
    f.fr_idx = tgt.m_db_id;
    f.fr_online_status = (tgt.m_client != nullptr); // need some other method for this.
    f.fr_field_0 = 1;                                   // what do we do with this?
    f.fr_name = tgt.name();
    f.fr_class_idx = tgt.m_entity_data.m_class_idx;
    f.fr_origin_idx = tgt.m_entity_data.m_origin_idx;
    f.fr_field_8 = 1;                                   // what do we do with this?
    f.fr_mapname = tgt.m_char.m_char_data.m_mapName;

    // add to friendlist
    src_data->m_friends.emplace_back(f);
    msg = "Adding " + tgt.name() + " to your friendlist.";

    qCDebug(logFriends) << "friendslist size:" << src_data->m_friends_count << src_data->m_friends.size();

    qCDebug(logFriends).noquote() << msg;
    messageOutput(MessageChannel::FRIENDS, msg, src);

    if(logFriends().isDebugEnabled())
        dumpFriends(src);
}

void removeFriend(Entity &src)
{
    QString     msg = "Unable to remove friend.";

    qCDebug(logFriends).noquote() << msg;
    messageOutput(MessageChannel::FRIENDS, msg, src);
}

void toggleFriendList(Entity &src)
{
    GUIWindow friendlist = src.m_char.m_gui.m_wnds.at(WindowIDX::wdw_Friends);
    QString msg = "Toggling FriendList visibility.";
    msg += " " + QString::number(friendlist.m_mode);

    if(friendlist.m_mode != WindowVisibility::wv_Visible)
        friendlist.setWindowVisibility(WindowVisibility::wv_Visible);
    else
        friendlist.setWindowVisibility(WindowVisibility::wv_DockedOrHidden);

    msg += " to " + QString::number(friendlist.m_mode);

    qCDebug(logFriends).noquote() << msg;
}

void dumpFriends(Entity &src)
{
    const FriendsList *fl(&src.m_char.m_char_data.m_friendlist);
    QString msg = QString("FriendsList\n  has_friends: %1 \n  v2: %2 \n  friends_count: %3 ")
            .arg(fl->m_has_friends)
            .arg(fl->m_friends_v2)
            .arg(fl->m_friends_count);

    qDebug().noquote() << msg;

    for(auto f : fl->m_friends)
        dumpFriendsList(f);
}

void dumpFriendsList(Friend &f)
{
    qDebug().noquote() << "Friend:" << f.fr_idx
             << "\n\t" << "online:" << f.fr_online_status
             << "\n\t" << "field_0:" << f.fr_field_0
             << "\n\t" << "name:" << f.fr_name
             << "\n\t" << "class_id:" << f.fr_class_idx
             << "\n\t" << "origin_id:" << f.fr_origin_idx
             << "\n\t" << "field_8:" << f.fr_field_8
             << "\n\t" << "mapname:" << f.fr_mapname;
}
