/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Servers/InternalEvents.h"
#include "NetStructures/Friend.h"

enum FriendHandlerEventTypes : uint32_t
{
    evFriendConnected = Internal_EventTypes::evLAST_EVENT,
    evSendFriendList,
    evSendNotifyFriend,
    evFriendAdded,
    evFriendRemoved,
};

#define ONE_WAY_MESSAGE(name)\
struct name ## Message final : public InternalEvent\
{\
    name ## Data m_data;\
    name ## Message(name ## Data &&d) :  InternalEvent(FriendHandlerEventTypes::ev ## name),m_data(d) {}\
};

/// A message with Request having additional data
#define TWO_WAY_MESSAGE(name)\
struct name ## Request final : public InternalEvent\
{\
    name ## RequestData m_data;\
    name ## Request(name ## RequestData &&d,uint64_t token,EventProcessor *src = nullptr) :\
        InternalEvent(FriendHandlerEventTypes::ev ## name ## Request,src),m_data(d) {session_token(token);}\
};\
struct name ## Response final : public InternalEvent\
{\
    name ## ResponseData m_data;\
    name ## Response(name ## ResponseData &&d,uint64_t token) :  InternalEvent(FriendHandlerEventTypes::ev ## name ## Response),m_data(d) {session_token(token);}\
};

struct FriendConnectedData
{
    uint64_t m_session;
    uint32_t m_server_id;     // id of the server the client connected to.
    uint32_t m_sub_server_id; // only used when server_id is the map server
    uint32_t m_char_db_id;       // id of the character connecting
    FriendsList m_friends_list;
};

ONE_WAY_MESSAGE(FriendConnected)

struct SendFriendListData
{
    uint64_t m_session_token;
    FriendsList m_friendlist;
};

ONE_WAY_MESSAGE(SendFriendList)

struct SendNotifyFriendData
{
    uint64_t m_connected_token; //session of player who connected
    uint64_t m_notify_token;    //session of player to notify
};

ONE_WAY_MESSAGE(SendNotifyFriend)

struct FriendAddedData
{
    uint32_t m_char_db_id;         //character who added a new friend
    uint32_t m_added_id;        //id of player added
    Friend m_friend;
};

ONE_WAY_MESSAGE(FriendAdded)

struct FriendRemovedData
{
    uint32_t m_char_db_id;         //character who removed a friend
    uint32_t m_removed_id;      //id of player removed
};

ONE_WAY_MESSAGE(FriendRemoved)
