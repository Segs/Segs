/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Servers/InternalEvents.h"
#include "NetStructures/SuperGroup.h"
#include "GameData/chardata_serializers.h"
namespace SEGSEvents
{

enum SuperGroupHandlerEventTypes : uint32_t
{
    evSGMemberConnectedMessage = Internal_EventTypes::ID_LAST_Internal_EventTypes,
    evSendSGRosterMessage,
    evSendNotifySGMembersMessage,
    evSGMemberAddedMessage,
    evSGMemberRemovedMessage,
};

struct SGMemberConnectedData
{
    uint64_t m_session;
    uint32_t m_server_id;       // id of the server the client connected to.
    uint32_t m_sub_server_id;   // only used when server_id is the map server
    uint32_t m_char_db_id;      // id of the character connecting
    vSuperGroupRoster m_sg_members;
    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_session, m_server_id, m_sub_server_id, m_char_db_id, m_sg_members);
    }
};

//[[ev_def:macro]]
ONE_WAY_MESSAGE(SuperGroupHandlerEventTypes, SGMemberConnected)

struct SendSGRosterData
{
    uint64_t m_session_token;
    vSuperGroupRoster m_sg_members;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_session_token, m_sg_members);
    }
};

//[[ev_def:macro]]
ONE_WAY_MESSAGE(SuperGroupHandlerEventTypes, SendSGRoster)

struct SendNotifySGMembersData
{
    uint64_t m_connected_token; // session of player who connected
    uint64_t m_notify_token;    // session of player to notify
    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_connected_token, m_notify_token);
    }
};

//[[ev_def:macro]]
ONE_WAY_MESSAGE(SuperGroupHandlerEventTypes, SendNotifySGMembers)

struct SGMemberAddedData
{
    uint32_t m_sg_db_id;        // supergroup adding a new member
    uint32_t m_added_id;        // id of player added
    SuperGroupStats m_sgstats;  // sg stats for the member
    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_sg_db_id, m_added_id, m_sgstats);
    }
};

//[[ev_def:macro]]
ONE_WAY_MESSAGE(SuperGroupHandlerEventTypes, SGMemberAdded)

struct SGMemberRemovedData
{
    uint32_t m_sg_db_id;        // supergroup removing a member
    uint32_t m_removed_id;      // id of player removed
    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_sg_db_id, m_removed_id);
    }
};

//[[ev_def:macro]]
ONE_WAY_MESSAGE(SuperGroupHandlerEventTypes, SGMemberRemoved)
} // end of namespace SEGSEvents
