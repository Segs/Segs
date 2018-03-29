/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2017 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
#include "GameCommandList.h"
#include "Friend.h"
#include "Logging.h"

#include "MapEvents.h"
#include "MapLink.h"

class FriendsListUpdate final : public CRUDLink_Event
{
public:
    FriendsList *m_list;
    FriendsListUpdate(FriendsList *friends_list) : MapLinkEvent(MapEventTypes::evFriendListUpdated),
        m_list(friends_list)
    {
    }
    void    serializeto(BitStream &bs) const override {
        bs.StorePackedBits(1,type()-MapEventTypes::evFirstServerToClient); // 37

        qCDebug(logFriends) << "FL Update:" << m_list->m_has_friends << m_list->m_friends_count << m_list->m_friends.size();
        bs.StorePackedBits(1,m_list->m_friends_v2); // v2
        bs.StorePackedBits(1,m_list->m_friends_count);

        for(int i=0; i<m_list->m_friends_count; ++i)
        {
            bs.StoreBits(1,m_list->m_has_friends); // if false, client will skip this iteration
            bs.StorePackedBits(1,m_list->m_friends[i].fr_field_0);
            bs.StoreBits(1,m_list->m_friends[i].fr_online_status);
            bs.StoreString(m_list->m_friends[i].fr_name);
            bs.StorePackedBits(1,m_list->m_friends[i].fr_class_idx);
            bs.StorePackedBits(1,m_list->m_friends[i].fr_origin_idx);

            if(!m_list->m_friends[i].fr_online_status)
                continue;

            // if friend is offline, these will be skipped
            bs.StorePackedBits(1,m_list->m_friends[i].fr_field_8);
            bs.StoreString(m_list->m_friends[i].fr_mapname);
        }
    }
    void    serializefrom(BitStream &src);
};
