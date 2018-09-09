/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "CommonNetStructures.h"

class Entity;
// Max number of friends on friendslist -- client caps at 25 entries
static const int g_max_friends = 25;

struct Friend
{
static const constexpr  uint32_t    class_version   = 1;
                        bool        m_online_status;
                        int         m_db_id;           // m_db_id
                        QString     m_name;
                        uint8_t     m_class_idx;
                        uint8_t     m_origin_idx;
                        int         m_map_idx;         // if online, then these Packed Bits
                        QString     m_mapname;         // field_18, probably mapname
};

struct FriendsList
{
static const constexpr  uint32_t    class_version   = 1;
                        bool        m_has_friends;
                        int         m_friends_count = 0;    // v12
                        std::vector<Friend> m_friends;
};

void toggleFriendList(Entity &src);
void dumpFriends(const Entity &src);
void dumpFriendsList(const Friend &f);
