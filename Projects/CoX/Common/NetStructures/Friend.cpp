/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup NetStructures Projects/CoX/Common/NetStructures
 * @{
 */

#include "Friend.h"

#include "Servers/MapServer/DataHelpers.h"
#include "GameData/playerdata_definitions.h"
#include "Entity.h"
#include "Logging.h"
#include "Character.h"

// Max number of friends on friendslist -- client caps at 25 entries
static const int g_max_friends = 25;

void addFriend(Entity &src, Entity &tgt)
{
    QString msg;
    FriendsList *src_data(&src.m_char->m_char_data.m_friendlist);

    if(src_data->m_friends_count >= g_max_friends)
    {
        msg = "You cannot have more than " + QString::number(g_max_friends) + " friends.";
        qCDebug(logFriends).noquote() << msg;
        messageOutput(MessageChannel::USER_ERROR, msg, src);
        return; // break early
    }

    src_data->m_has_friends = true;
    src_data->m_friends_count++;

    Friend f;
    f.m_online_status   = (tgt.m_client != nullptr); // need some other method for this.
    f.m_db_id           = tgt.m_db_id;
    f.m_name            = tgt.name();
    f.m_class_idx       = tgt.m_entity_data.m_class_idx;
    f.m_origin_idx      = tgt.m_entity_data.m_origin_idx;
    f.m_map_idx         = tgt.m_entity_data.m_map_idx;
    f.m_mapname         = getEntityMapName(tgt.m_entity_data);

    // add to friendlist
    src_data->m_friends.emplace_back(f);
    qCDebug(logFriends) << "friendslist size:" << src_data->m_friends_count << src_data->m_friends.size();

    msg = "Adding " + tgt.name() + " to your friendlist.";
    qCDebug(logFriends).noquote() << msg;
    messageOutput(MessageChannel::FRIENDS, msg, src);

    if(logFriends().isDebugEnabled())
        dumpFriends(src);

    sendFriendsListUpdate(&src, src_data); // Send FriendsListUpdate
}

void removeFriend(Entity &src, Entity &tgt)
{
    QString msg;
    FriendsList *src_data(&src.m_char->m_char_data.m_friendlist);

    qCDebug(logFriends) << "Searching for friend" << tgt.name() << "to remove them.";
    int id_to_find = tgt.m_db_id;
    auto iter = std::find_if( src_data->m_friends.begin(), src_data->m_friends.end(),
                              [id_to_find](const Friend& f)->bool {return id_to_find==f.m_db_id;});
    if(iter!=src_data->m_friends.end())
    {
        iter = src_data->m_friends.erase(iter);

        msg = "Removing " + iter->m_name + " from your friends list.";
        qCDebug(logFriends) << msg;
        if(logFriends().isDebugEnabled())
            dumpFriends(src);
    }
    else
        msg = tgt.name() + "is not on your friends list.";

    if(src_data->m_friends.empty())
        src_data->m_has_friends = false;

    src_data->m_friends_count = src_data->m_friends.size();

    qCDebug(logFriends).noquote() << msg;
    messageOutput(MessageChannel::FRIENDS, msg, src);

    // Send FriendsListUpdate
    sendFriendsListUpdate(&src, src_data);
}

bool isFriendOnline(Entity &src, uint32_t db_id)
{
    // TODO: src is needed for mapclient
    return getEntityByDBID(src.m_client, db_id) != nullptr;
}

void toggleFriendList(Entity &src)
{
    GUIWindow *friendlist = &src.m_player->m_gui.m_wnds.at(WindowIDX::wdw_Friends);
    QString msg = "Toggling FriendList visibility.";
    msg += " " + QString::number(friendlist->m_mode);

    // TODO: How to actually change window visibility?
    if(friendlist->m_mode != WindowVisibility::wv_Visible)
        friendlist->setWindowVisibility(WindowVisibility::wv_Visible);
    else
        friendlist->setWindowVisibility(WindowVisibility::wv_DockedOrHidden);

    msg += " to " + QString::number(friendlist->m_mode);
    qCDebug(logFriends).noquote() << msg;
}

void dumpFriends(const Entity &src)
{
    const FriendsList *fl(&src.m_char->m_char_data.m_friendlist);
    QString msg = QString("FriendsList\n  has_friends: %1 \n friends_count: %2 ")
            .arg(fl->m_has_friends)
            .arg(fl->m_friends_count);

    qDebug().noquote() << msg;

    for(const auto &f : fl->m_friends)
        dumpFriendsList(f);
}

void dumpFriendsList(const Friend &f)
{
    qDebug().noquote() << "Friend:" << f.m_name
             << "\n\t" << "online:" << f.m_online_status
             << "\n\t" << "db_id:" << f.m_db_id
             << "\n\t" << "class_id:" << f.m_class_idx
             << "\n\t" << "origin_id:" << f.m_origin_idx
             << "\n\t" << "map_idx:" << f.m_map_idx
             << "\n\t" << "mapname:" << f.m_mapname;
}

//! @}
