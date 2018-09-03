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

#include "CharacterHelpers.h"
#include "GameData/playerdata_definitions.h"
#include "Entity.h"
#include "Logging.h"
#include "Character.h"



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
