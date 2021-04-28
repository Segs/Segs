/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "GameCommand.h"
#include "MapEventTypes.h"
#include "GameData/Friend.h"
#include "Components/BitStream.h"
#include "Components/Logging.h"

namespace SEGSEvents
{

// [[ev_def:type]]
class FriendsListUpdate final : public GameCommandEvent
{
public:
    // [[ev_def:field]]
    FriendsList m_list;

    FriendsListUpdate(const FriendsList &friends_list) : GameCommandEvent(evFriendsListUpdate),
        m_list(friends_list)
    {
    }
    explicit FriendsListUpdate() : GameCommandEvent(evFriendsListUpdate) {}
    void    serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,type()-evFirstServerToClient); // 37

        qCDebug(logFriends) << "FL Update:" << m_list.m_has_friends << m_list.m_friends_count << m_list.m_friends.size();
        bs.StorePackedBits(1,1); // v2 = force_update
        bs.StorePackedBits(1,m_list.m_friends_count);

        for(int i=0; i<m_list.m_friends_count; ++i)
        {
            bs.StoreBits(1,m_list.m_has_friends); // if false, client will skip this iteration
            bs.StorePackedBits(1,m_list.m_friends[i].m_db_id);

            // TODO: Lookup online status, implement: isFriendOnline()
            //bool is_online = isFriendOnline(m_list.m_friends[i].fr_db_id);

            bs.StoreBits(1,m_list.m_friends[i].m_online_status);
            bs.StoreString(m_list.m_friends[i].m_name);
            bs.StorePackedBits(1,m_list.m_friends[i].m_class_idx);
            bs.StorePackedBits(1,m_list.m_friends[i].m_origin_idx);

            if(!m_list.m_friends[i].m_online_status)
                continue; // if friend is offline, the rest is skipped

            bs.StorePackedBits(1,m_list.m_friends[i].m_map_idx);
            bs.StoreString(m_list.m_friends[i].m_mapname);
        }
    }
    EVENT_IMPL(FriendsListUpdate)
};

} // end of SEGSEvents namespace
