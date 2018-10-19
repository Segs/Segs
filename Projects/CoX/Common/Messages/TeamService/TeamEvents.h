/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Servers/InternalEvents.h"
#include "GameData/Team.h"
#include "GameData/chardata_serializers.h"
namespace SEGSEvents
{
    
enum TeamEventTypes : uint32_t
{
    evTeamMemberInvitedMessage = Internal_EventTypes::ID_LAST_Internal_EventTypes,
    evTeamMemberKickedMessage,
    evTeamMemberInviteAcceptedMessage,
    evTeamMemberInviteDeclinedMessage,
    // evTeamLeaveTeamMessage,
    // evTeamMakeLeaderMessage,
    // evTeamToggleBuffsViewMessage,
};

struct TeamMemberInvitedData
{
    uint64_t m_leader_id;
    uint64_t m_invitee_id;
    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_leader_id, m_invitee_id);
    }
};

//[[ev_def:macro]]
ONE_WAY_MESSAGE(TeamEventTypes,TeamMemberInvited)

struct TeamMemberKickedData
{
    uint64_t m_leader_id;
    uint64_t m_kickee_id;
    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_leader_id, m_kickee_id);
    }
};

//[[ev_def:macro]]
ONE_WAY_MESSAGE(TeamEventTypes,TeamMemberKicked)

struct TeamMemberInviteAcceptedData
{
    uint64_t m_invitee_id;
    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_invitee_id);
    }
};

//[[ev_def:macro]]
ONE_WAY_MESSAGE(TeamEventTypes,TeamMemberInviteAccepted)

struct TeamMemberInviteDeclinedData
{
    uint64_t m_invitee_id;
    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_invitee_id);
    }
};

//[[ev_def:macro]]
ONE_WAY_MESSAGE(TeamEventTypes,TeamMemberInviteDeclined)

} // end of namespace SEGSEvents
