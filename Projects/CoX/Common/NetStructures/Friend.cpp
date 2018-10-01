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
#include "CharacterHelpers.h"
#include "GameData/playerdata_definitions.h"
#include "GameData/map_definitions.h"
#include "Common/Servers/HandlerLocator.h"
#include "Servers/GameServer/FriendshipService/FriendHandlerEvents.h"
#include "Entity.h"
#include "Logging.h"
#include "Character.h"


/*
 * Friend Methods -- Sidekick system requires teaming.
 */
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

FriendListChangeStatus addFriend(Entity &src, const Entity &tgt)
{
    QString msg;
    FriendsList &src_data(src.m_char->m_char_data.m_friendlist);

    if(src_data.m_friends_count >= g_max_friends)
    {
        return FriendListChangeStatus::MAX_FRIENDS_REACHED;
    }

    Friend f;
    f.m_online_status   = (tgt.m_client != nullptr); // need some other method for this.
    f.m_db_id           = tgt.m_db_id;
    f.m_name            = tgt.name();
    f.m_class_idx       = tgt.m_entity_data.m_class_idx;
    f.m_origin_idx      = tgt.m_entity_data.m_origin_idx;
    f.m_map_idx         = tgt.m_entity_data.m_map_idx;
    f.m_mapname         = getEntityDisplayMapName(tgt.m_entity_data);

    if(logFriends().isDebugEnabled())
        dumpFriends(src);

    EventProcessor *friend_tgt = HandlerLocator::getFriend_Handler();
    friend_tgt->putq(new SEGSEvents::FriendAddedMessage({src.m_char->m_db_id,tgt.m_db_id, f},0));

    return FriendListChangeStatus::FRIEND_ADDED;
}

FriendListChangeStatus removeFriend(Entity &src, QString friend_name)
{
    QString msg;
    uint32_t m_tgt_id;
    FriendsList *src_data(&src.m_char->m_char_data.m_friendlist);

    qCDebug(logFriends) << "Searching for friend" << friend_name << "to remove them.";

    QString lower_name = friend_name.toLower();
    auto iter = std::find_if( src_data->m_friends.begin(), src_data->m_friends.end(),
                              [lower_name](const Friend& f)->bool {return lower_name==f.m_name.toLower();});

    if(iter==src_data->m_friends.end())
    {
        msg = friend_name + " is not on your friends list.";
        qCDebug(logFriends) << msg;
        return FriendListChangeStatus::FRIEND_NOT_FOUND;
    }
    else
    {
        m_tgt_id = iter->m_db_id;
        msg = "Removing " + iter->m_name + " from your friends list.";

        EventProcessor *friend_tgt = HandlerLocator::getFriend_Handler();
        friend_tgt->putq(new SEGSEvents::FriendRemovedMessage({src.m_char->m_db_id,m_tgt_id},0));

        qCDebug(logFriends) << msg;
        if(logFriends().isDebugEnabled())
            dumpFriends(src);

        qCDebug(logFriends).noquote() << msg;
        return FriendListChangeStatus::FRIEND_REMOVED;
    }
}

const QString &getFriendDisplayMapName(const Friend &f)
{
    static const QString offline(QStringLiteral("OFFLINE"));
    if (!f.m_online_status)
        return offline;
    return getDisplayMapName(f.m_map_idx);
}

//! @}
