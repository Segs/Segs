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
#include "Messages/UserRouterService/UserRouterEvents.h"
#include "Messages/Map/MessageChannels.h"
#include <QtCore/QDebug>

#include "SEGSEventFactory.h"

using namespace SEGSEvents;

uint32_t TeamHandler::id_for_name(const QString &name)
{
    for (const auto &e : m_state.m_id_to_name)
        if (e.second == name)
            return e.first;

    return 0;
}

bool TeamHandler::name_known(const QString &name)
{
    for (const auto &e : m_state.m_id_to_name)
        if (e.second == name)
            return true;

    return false;
}

void TeamHandler::on_user_router_opaque_response(UserRouterOpaqueResponse *msg)
{
	if (msg->m_data.m_error == UserRouterError::USER_OFFLINE)
	{
		QString m = "The user you invited is offline.";
		m_state.m_map_handler->putq(new UserRouterInfoMessage({m, MessageChannel::USER_ERROR, msg->m_data.m_sender_id, msg->m_data.m_sender_id}, 0));
	}
	else 
	{
		QString m = "Team invite sent!";
		m_state.m_map_handler->putq(new UserRouterInfoMessage({m, MessageChannel::TEAM, msg->m_data.m_sender_id, msg->m_data.m_sender_id}, 0));
	}
}

void TeamHandler::on_user_router_query_response(UserRouterQueryResponse *msg)
{
    qCritical() << " got response a" << msg->m_data.m_request_id << msg->m_data.m_request_name;
    qCritical() << " got response b" << msg->m_data.m_response_id << msg->m_data.m_response_name;

	assert(m_state.m_pending_events.count(msg->m_data.m_response_name));

	m_state.m_id_to_name[msg->m_data.m_response_id] = msg->m_data.m_response_name;

	for (const auto &elem : m_state.m_pending_events)
		if (elem.first == msg->m_data.m_response_name)
			dispatch(elem.second);

	m_state.m_pending_events.erase(msg->m_data.m_response_name);
}

void TeamHandler::on_team_member_invited(TeamMemberInvitedMessage *msg) {

    const uint32_t leader_id = msg->m_data.m_leader_id;
    QString invitee_name = msg->m_data.m_invitee_name;
    QString leader_name = msg->m_data.m_leader_name;

	if (invitee_name == leader_name)
	{
		QString m = "You cannot invite yourself to a team.";
		m_state.m_map_handler->putq(new UserRouterInfoMessage({m, MessageChannel::USER_ERROR, leader_id, leader_id}, 0));
		return;
	}

    Team *leader_team = nullptr;
    Team *invitee_team = nullptr;

    for (Team *t : m_state.m_teams) 
    {
        if (t->containsEntityID(leader_id)) 
            leader_team = t;
        if (t->containsEntityName(invitee_name))
            invitee_team = t;
    }

    // if the leader has a team, make sure it's not full
    if (leader_team != nullptr) 
    {
        if (leader_team->m_team_leader_idx != leader_id) {
            QString m = "You must be the team leader to invite a player.";
			m_state.m_map_handler->putq(new UserRouterInfoMessage({m, MessageChannel::USER_ERROR, leader_id, leader_id}, 0));
            return;
        }

        if (leader_team->isFull()) 
        {
            QString m = "Your team is full or has too many pending invites. You cannot invite another player.";
			m_state.m_map_handler->putq(new UserRouterInfoMessage({m, MessageChannel::USER_ERROR, leader_id, leader_id}, 0));
            return;
        }
    } 

    if (invitee_team != nullptr) 
    {
        QString m;

        if (invitee_team->m_transient)
            m = QString("%1 is deciding about another team invitation.").arg(invitee_name);
        else
            m = QString("%1 is already on a team.").arg(invitee_name);

        m_state.m_map_handler->putq(new UserRouterInfoMessage({m, MessageChannel::TEAM, leader_id, leader_id}, 0));

        return;
    }

    if (leader_team == nullptr)
    {
        // create Team with transient=true
        leader_team = new Team(true);

        TeamingError e = leader_team->addTeamMember(leader_id);

        if (e != TeamingError::OK)
        {
            qCritical() << "Error adding entity ID to new team" << int(e);
            delete leader_team;
            return;
        }

        m_state.m_teams.emplace_back(leader_team);
    }

    TeamingError e = leader_team->addTeamMember(invitee_name);

    if (e != TeamingError::OK)
    {
        qCritical() << "Error adding entity name to team" << int(e);
        return;
    }

    invitee_team = leader_team;

    assert(leader_team && invitee_team);

    qCDebug(logTeams) << "team invite sent_by: " << leader_name << "(" << leader_id << ") | sent_to: " << invitee_name;

	std::stringstream stream;
    to_storage(stream, msg);
	QByteArray payload(QByteArray::fromStdString(stream.str()));

    // forward Opaque Invite message to UserRouter
    // This will cause MapInstance to send a TeamOffer to the Invitee
    m_state.m_map_handler->putq(new UserRouterOpaqueRequest({payload, leader_id, 0, msg->m_data.m_leader_name, invitee_name}, msg->session_token(), this));
}

void TeamHandler::on_team_member_kicked(TeamMemberKickedMessage *msg) {

    qCDebug(logTeams) << "kicked_by: | " << msg->m_data.m_leader_id << "sent_to: " << msg->m_data.m_kickee_id;
}

void TeamHandler::on_team_member_invite_accepted(TeamMemberInviteAcceptedMessage *msg) {

    qCDebug(logTeams) << "invite accepted_by: " << msg->m_data.m_invitee_id << msg->m_data.m_invitee_name << msg->m_data.m_leader_name;
}

void TeamHandler::on_team_member_invite_declined(TeamMemberInviteDeclinedMessage *msg) {

    qCDebug(logTeams) << "invite declined_by: " << msg->m_data.m_invitee_id << msg->m_data.m_invitee_name << msg->m_data.m_leader_name;
}

void TeamHandler::dispatch(SEGSEvents::Event *ev)
{
    assert(ev);

    if (m_state.m_map_handler == nullptr)
    {
        m_state.m_map_handler = HandlerLocator::getMap_Handler(m_state.m_game_server_id);
    }

    switch(ev->type())
    {
        case evTeamMemberInvitedMessage:
            on_team_member_invited(static_cast<TeamMemberInvitedMessage *>(ev));
            break;
        case evTeamMemberKickedMessage:
            on_team_member_kicked(static_cast<TeamMemberKickedMessage *>(ev));
            break;
        case evTeamMemberInviteAcceptedMessage:
            on_team_member_invite_accepted(static_cast<TeamMemberInviteAcceptedMessage *>(ev));
            break;
        case evTeamMemberInviteDeclinedMessage:
            on_team_member_invite_declined(static_cast<TeamMemberInviteDeclinedMessage *>(ev));
            break;
        case evUserRouterQueryResponse:
            on_user_router_query_response(static_cast<UserRouterQueryResponse *>(ev));
            break;
        case evUserRouterOpaqueResponse:
            on_user_router_opaque_response(static_cast<UserRouterOpaqueResponse *>(ev));
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
    m_state.m_map_handler = HandlerLocator::getMap_Handler(for_game_server_id);

    assert(HandlerLocator::getTeam_Handler() == nullptr);
    HandlerLocator::setTeam_Handler(this);
}

TeamHandler::~TeamHandler()
{
    HandlerLocator::setTeam_Handler(nullptr);
}
