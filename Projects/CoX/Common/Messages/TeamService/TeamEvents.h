/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Messages/UserRouterService/UserRouterEvents.h"
#include "Servers/InternalEvents.h"
#include "GameData/CharacterData.h"
#include "GameData/Team.h"
#include "GameData/LFG.h"

namespace SEGSEvents
{
    
enum TeamEventTypes : uint32_t
{
    evTeamMemberInvitedMessage = UserRouterEventTypes::evUserRouterQueryResponse + 1,
    evTeamMemberKickedMessage,
    evTeamMemberInviteAcceptedMessage,
    evTeamMemberInviteDeclinedMessage,
	evTeamToggleLFGMessage,
	evTeamRefreshLFGMessage,
    evTeamUpdatedMessage,
    // evTeamLeaveTeamMessage,
    // evTeamMakeLeaderMessage,
    // evTeamToggleBuffsViewMessage,
};

struct TeamMemberInvitedData
{
    QString m_leader_name;
    uint32_t m_leader_id;
    QString m_invitee_name;
    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_leader_name, m_leader_id, m_invitee_name);
    }
};

//[[ev_def:macro]]
ONE_WAY_MESSAGE(TeamEventTypes,TeamMemberInvited)

struct TeamMemberKickedData
{
    uint32_t m_leader_id;
    uint32_t m_kickee_id;
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
    uint32_t m_invitee_id;
	QString m_invitee_name;
	QString m_leader_name;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_invitee_id, m_invitee_name, m_leader_name);
    }
};

//[[ev_def:macro]]
ONE_WAY_MESSAGE(TeamEventTypes,TeamMemberInviteAccepted)

struct TeamMemberInviteDeclinedData
{
    uint32_t m_invitee_id;
	QString m_invitee_name;
	QString m_leader_name;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_invitee_id, m_invitee_name, m_leader_name);
    }
};

//[[ev_def:macro]]
ONE_WAY_MESSAGE(TeamEventTypes,TeamMemberInviteDeclined)

struct TeamToggleLFGData
{
    uint32_t m_db_id;
	QString m_name;
	CharacterData m_char_data;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_db_id, m_name, m_char_data);
    }
};

//[[ev_def:macro]]
ONE_WAY_MESSAGE(TeamEventTypes,TeamToggleLFG)

struct TeamRefreshLFGData
{
    uint32_t m_db_id;
	QString m_name;
	std::vector<LFGMember> m_lfg_list;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_db_id, m_name, m_lfg_list);
    }
};

//[[ev_def:macro]]
ONE_WAY_MESSAGE(TeamEventTypes,TeamRefreshLFG)

struct TeamUpdatedData
{
    Team::TeamData m_team_data;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_team_data);
    }
};

//[[ev_def:macro]]
ONE_WAY_MESSAGE(TeamEventTypes,TeamUpdated)

} // end of namespace SEGSEvents
