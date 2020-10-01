/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Servers/InternalEvents.h"
#include "GameData/Friend.h"
#include "GameData/chardata_serializers.h"
namespace SEGSEvents
{
enum FriendHandlerEventTypes : uint32_t
{
    evFriendConnectedMessage = Internal_EventTypes::ID_LAST_Internal_EventTypes,
    evSendFriendListMessage,
    evSendNotifyFriendMessage,
    evFriendAddedMessage,
    evFriendRemovedMessage,
};

struct FriendConnectedData
{
    uint64_t m_session;
    uint32_t m_server_id;     // id of the server the client connected to.
    uint32_t m_sub_server_id; // only used when server_id is the map server
    uint32_t m_char_db_id;       // id of the character connecting
    FriendsList m_friends_list;
    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_session,m_server_id,m_sub_server_id,m_char_db_id,m_friends_list);
    }
};

//[[ev_def:macro]]
ONE_WAY_MESSAGE(FriendHandlerEventTypes,FriendConnected)

struct SendFriendListData
{
    uint64_t m_session_token;
    FriendsList m_friendlist;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_session_token,m_friendlist);
    }
};

//[[ev_def:macro]]
ONE_WAY_MESSAGE(FriendHandlerEventTypes,SendFriendList)

struct SendNotifyFriendData
{
    uint64_t m_connected_token; //session of player who connected
    uint64_t m_notify_token;    //session of player to notify
    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_connected_token,m_notify_token);
    }
};

//[[ev_def:macro]]
ONE_WAY_MESSAGE(FriendHandlerEventTypes,SendNotifyFriend)

struct FriendAddedData
{
    uint32_t m_char_db_id;         //character who added a new friend
    uint32_t m_added_id;        //id of player added
    Friend m_friend;
    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_char_db_id,m_added_id,m_friend);
    }
};

//[[ev_def:macro]]
ONE_WAY_MESSAGE(FriendHandlerEventTypes,FriendAdded)

struct FriendRemovedData
{
    uint32_t m_char_db_id;         //character who removed a friend
    uint32_t m_removed_id;      //id of player removed
    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_char_db_id,m_removed_id);
    }
};

//[[ev_def:macro]]
ONE_WAY_MESSAGE(FriendHandlerEventTypes,FriendRemoved)
} // end of namespace SEGSEvents
