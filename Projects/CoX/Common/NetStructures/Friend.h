/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#pragma once
#include "CommonNetStructures.h"

class Entity;

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

void addFriend(Entity &src, Entity &tgt);
void removeFriend(Entity &src, Entity &tgt);
bool isFriendOnline(Entity &src, uint32_t db_id);
void toggleFriendList(Entity &src);
void dumpFriends(const Entity &src);
void dumpFriendsList(const Friend &f);
