/*
 * Super Entity Game Server Project
 * https://github.com/Segs/Segs
 * Copyright (c) 2006 - 2018 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
#include "CommonNetStructures.h"

class Entity;

struct FriendsResults
{
    int         status              = 0;
    int         src_ent_id;
    int         tgt_ent_id;
    QString     *optional_result_msg = nullptr; // as pointer since in case of success we won't have anything to tell the target_ent_id ?
};

struct Friend
{
    bool        fr_online_status;
    int         fr_db_id;         // m_db_id
    QString     fr_name;
    uint8_t     fr_class_idx;
    uint8_t     fr_origin_idx;
    int         fr_map_idx;         // if online, then these Packed Bits
    QString     fr_mapname;         // field_18, probably mapname
};

struct FriendsList
{
    bool        m_has_friends;
    int         m_friends_count     = 0;    // v12
    std::vector<Friend> m_friends;
};

void addFriend(Entity &src, Entity &tgt);
void removeFriend(Entity &src, Entity &tgt);
int findFriendByIdx(uint32_t &idx);
int findFriendByName(QString &name);
void toggleFriendList(Entity &src);
void dumpFriends(Entity &src);
void dumpFriendsList(Friend &f);
