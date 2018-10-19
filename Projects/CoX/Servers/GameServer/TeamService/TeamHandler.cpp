/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "TeamHandler.h"
#include "Messages/Game/GameEvents.h"
#include "Common/Servers/HandlerLocator.h"
#include "Messages/TeamService/TeamEvents.h"
#include <QtCore/QDebug>

using namespace SEGSEvents;

void on_team_member_invited(TeamHandlerState &state,TeamMemberInvitedMessage *msg) {

    qCDebug(logTeams) << "invite sent_by: | " << msg->m_data.m_leader_id << "sent_to: " << msg->m_data.m_invitee_id;
}

void on_team_member_kicked(TeamHandlerState &state,TeamMemberKickedMessage *msg) {

    qCDebug(logTeams) << "kicked_by: | " << msg->m_data.m_leader_id << "sent_to: " << msg->m_data.m_kickee_id;
}

void on_team_member_invite_accepted(TeamHandlerState &state,TeamMemberInviteAcceptedMessage *msg) {

    qCDebug(logTeams) << "invite accepted_by: " << msg->m_data.m_invitee_id;
}

void on_team_member_invite_declined(TeamHandlerState &state,TeamMemberInviteDeclinedMessage *msg) {

    qCDebug(logTeams) << "invite declined_by: " << msg->m_data.m_invitee_id;
}

void TeamHandler::dispatch(SEGSEvents::Event *ev)
{
    assert(ev);

    switch(ev->type())
    {
        case evTeamMemberInvitedMessage:
            on_team_member_invited(m_state,static_cast<TeamMemberInvitedMessage *>(ev));
            break;
        case evTeamMemberKickedMessage:
            on_team_member_kicked(m_state,static_cast<TeamMemberKickedMessage *>(ev));
            break;
        case evTeamMemberInviteAcceptedMessage:
            on_team_member_invite_accepted(m_state,static_cast<TeamMemberInviteAcceptedMessage *>(ev));
            break;
        case evTeamMemberInviteDeclinedMessage:
            on_team_member_invite_declined(m_state,static_cast<TeamMemberInviteDeclinedMessage *>(ev));
            break;
        default:
            assert(false);
            break;
    }
}

void TeamHandler::serialize_from(std::istream &/*is*/)
{
    assert(false);
}

void TeamHandler::serialize_to(std::ostream &/*is*/)
{
    assert(false);
}

TeamHandler::TeamHandler(int for_game_server_id) : m_message_bus_endpoint(*this)
{
    m_state.m_game_server_id = for_game_server_id;

    assert(HandlerLocator::getTeam_Handler() == nullptr);
    HandlerLocator::setTeam_Handler(this);

    m_message_bus_endpoint.subscribe(evClientDisconnectedMessage);
}

TeamHandler::~TeamHandler()
{
    HandlerLocator::setTeam_Handler(nullptr);
}
