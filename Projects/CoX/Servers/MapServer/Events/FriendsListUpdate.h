/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "GameCommandList.h"
#include "NetStructures/Friend.h"
#include "Logging.h"

#include "MapEvents.h"
#include "MapLink.h"

class FriendsListUpdate final : public GameCommand
{
public:
    FriendsList *m_list;
    FriendsList m_objlist;
    FriendsListUpdate(FriendsList *friends_list) : GameCommand(MapEventTypes::evFriendListUpdated),
        m_list(friends_list)
    {
    }
    FriendsListUpdate(FriendsList friends_list) : GameCommand(MapEventTypes::evFriendListUpdated)
    {
        m_objlist = friends_list;
        m_list = &m_objlist;
    }
    void    serializeto(BitStream &bs) const override {
        bs.StorePackedBits(1,type()-MapEventTypes::evFirstServerToClient); // 37

        qDebug() << "Serialize flist";
        qDebug() << "FL Update:" << m_list->m_has_friends << m_list->m_friends_count << m_list->m_friends.size();
        qCDebug(logFriends) << "FL Update:" << m_list->m_has_friends << m_list->m_friends_count << m_list->m_friends.size();
        bs.StorePackedBits(1,1); // v2 = force_update
        bs.StorePackedBits(1,m_list->m_friends_count);

        qDebug() << "Going over friends " << m_list->m_friends_count;
        for(int i=0; i<m_list->m_friends_count; ++i)
        {
            qDebug() << "Friend" << m_list->m_friends[i].m_name << " is " << m_list->m_friends[i].m_online_status;
            bs.StoreBits(1,m_list->m_has_friends); // if false, client will skip this iteration
            bs.StorePackedBits(1,m_list->m_friends[i].m_db_id);

            // TODO: Lookup online status, implement: isFriendOnline()
            //bool is_online = isFriendOnline(m_list->m_friends[i].fr_db_id);

            bs.StoreBits(1,m_list->m_friends[i].m_online_status);
            bs.StoreString(m_list->m_friends[i].m_name);
            bs.StorePackedBits(1,m_list->m_friends[i].m_class_idx);
            bs.StorePackedBits(1,m_list->m_friends[i].m_origin_idx);

            if(!m_list->m_friends[i].m_online_status)
                continue; // if friend is offline, the rest is skipped

            bs.StorePackedBits(1,m_list->m_friends[i].m_map_idx);
            bs.StoreString(m_list->m_friends[i].m_mapname);


        }
        qDebug() << "Done";
    }
    void    serializefrom(BitStream &src);
};
