/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "CommonNetStructures.h"

class Entity;
// Max number of friends on friendslist -- client caps at 25 entries
static const int g_max_friends = 25;

struct Friend
{
        enum : uint32_t { class_version=1 };

        bool        m_online_status;
        uint32_t    m_db_id;           // m_db_id
        QString     m_name;
        uint8_t     m_class_idx;
        uint8_t     m_origin_idx;
        int         m_map_idx;         // if online, then these Packed Bits
        QString     m_mapname;         // field_18, probably mapname
        template<class Archive>
        void        serialize(Archive &archive, uint32_t const version);
};

struct FriendsList
{
    enum : uint32_t { class_version = 1 };

    bool                m_has_friends   = false;
    int                 m_friends_count = 0;
    std::vector<Friend> m_friends;
};
template<class Archive>
void serialize(Archive &archive, FriendsList &fl, uint32_t const version);

enum class FriendListChangeStatus
{
    FRIEND_ADDED,
    FRIEND_REMOVED,
    MAX_FRIENDS_REACHED,
    FRIEND_NOT_FOUND,
};

/*
 * Friend Methods
 */
void toggleFriendList(Entity &src);
void dumpFriends(const Entity &src);
void dumpFriendsList(const Friend &f);

FriendListChangeStatus addFriend(Entity &src, const Entity &tgt, const QString &mapname);
FriendListChangeStatus removeFriend(Entity &src, const QString& friendName);

const QString &getFriendDisplayMapName(const Friend &f);
