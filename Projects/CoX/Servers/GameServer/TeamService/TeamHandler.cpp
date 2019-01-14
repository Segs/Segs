/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "TeamHandler.h"
#include "SEGSEventFactory.h"
#include "Messages/Game/GameEvents.h"
#include "Common/Servers/HandlerLocator.h"
#include "Messages/TeamService/TeamEvents.h"
#include "Messages/UserRouterService/UserRouterEvents.h"
#include "Messages/Map/MessageChannels.h"
#include <QtCore/QDebug>

#include "SEGSEventFactory.h"

using namespace SEGSEvents;

bool TeamHandler::delete_team(Team *t)
{

    int index_to_remove = -1;
    for (unsigned int i = 0; i < m_state.m_team_list.size(); i++)
    {
        Team *team = m_state.m_team_list[i];
        if (team->m_team_idx == t->m_team_idx)
        {
            index_to_remove = i;
            qCDebug(logTeams) << "Removing team:" << t->m_team_idx;
            delete team;
            break;
        }
    }

    if (index_to_remove == -1)
    {
        qCDebug(logTeams) << "Team not found:" << t->m_team_idx;
        return false;
    }


    m_state.m_team_list.erase(m_state.m_team_list.begin() + index_to_remove);

    return true;
}

bool TeamHandler::name_is_lfg(const QString &name)
{
    for (LFGMember &m : m_state.m_lfg_list)
	{
        if (m.m_name == name) 
            return true;
	}

	return false;
}

bool TeamHandler::db_id_is_lfg(const uint32_t db_id)
{
    for (LFGMember &m : m_state.m_lfg_list) 
	{
        if (m.m_db_id == db_id) 
            return true;
	}

	return false;
}

void TeamHandler::add_lfg(LFGMember m)
{
	m_state.m_lfg_list.emplace_back(m);
}

void TeamHandler::remove_lfg(const uint32_t db_id)
{
    auto iter = std::find_if( m_state.m_lfg_list.begin(), m_state.m_lfg_list.end(),
                              [db_id](const LFGMember& mem)-> bool {return db_id == mem.m_db_id;});

    if(iter != m_state.m_lfg_list.end())
    {
        iter = m_state.m_lfg_list.erase(iter);
        qCDebug(logLFG) << "Removing" << iter->m_name << "from LFG List";
	}
}

Team *TeamHandler::team_for_db_id(const uint32_t db_id)
{
    for (Team *t : m_state.m_team_list) 
	{
        if (t->containsEntityID(db_id)) 
            return t;
	}

	return nullptr;
}

Team* TeamHandler::team_for_name(const QString &name)
{
    for (Team *t : m_state.m_team_list) 
	{
        if (t->containsEntityName(name)) 
            return t;
	}

	return nullptr;
}

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
	const uint32_t sender_id = msg->m_data.m_req.m_sender_id;
	const UserRouterError e = msg->m_data.m_error;
	Event *src_event = __route_unpack(msg->m_data.m_req.m_payload);

	QString m = "Unknown Error";
	MessageChannel c = MessageChannel::USER_ERROR;

	switch (src_event->type())
	{
		case evTeamMemberInvitedMessage:
		{
			if (e == UserRouterError::USER_OFFLINE)
			{
				m = "The user you invited is offline.";
			}
			else 
			{
				m = "Team invite sent!";
				c = MessageChannel::TEAM;
			}
			break;
		}
		case evTeamToggleLFGMessage:
		{
			if (e == UserRouterError::USER_OFFLINE)
			{
		
				qCritical() << "got user offline error for toggling lfg";
				m = "The user you invited is offline.";
			}
			else 
			{
				m = "Toggling LFG";
				c = MessageChannel::TEAM;
			}
			break;
		}
		case evTeamRefreshLFGMessage:
		{
			if (e == UserRouterError::USER_OFFLINE)
			{
		
				qCritical() << "got user offline error for refreshing lfg list";
				m = "The user you invited is offline.";
			}
			else 
			{
				m = "Refreshing LFG list";
				c = MessageChannel::TEAM;
			}
			break;
		}
		default:
			assert(false);
			break;
	}


	m_state.m_map_handler->putq(new UserRouterInfoMessage({m, c, sender_id, sender_id}, 0));
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

    for (Team *t : m_state.m_team_list) 
    {
        if (t->containsEntityID(leader_id)) 
            leader_team = t;
        if (t->containsEntityName(invitee_name))
            invitee_team = t;
    }

    // if the leader has a team, make sure it's not full
    if (leader_team != nullptr) 
    {
        if (!leader_team->isTeamLeader(leader_id)) {
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

        if (invitee_team->isNamePending(invitee_name))
            m = QString("%1 is already deciding about a team invitation.").arg(invitee_name);
        else
            m = QString("%1 is already on a team.").arg(invitee_name);

        m_state.m_map_handler->putq(new UserRouterInfoMessage({m, MessageChannel::TEAM, leader_id, leader_id}, 0));

        return;
    }

    if (leader_team == nullptr)
    {
        // create Team with transient=true
		leader_team = new Team(true);

        TeamingError e = leader_team->addTeamMember(leader_id, leader_name);

        if (e != TeamingError::OK)
        {
            qCritical() << "Error adding entity ID to new team" << int(e);
            return;
        }

        m_state.m_team_list.emplace_back(leader_team);
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

    // forward Opaque Invite message to UserRouter
    // This will cause MapInstance to send a TeamOffer to the Invitee
    m_state.m_map_handler->putq(new UserRouterOpaqueRequest({__route(msg), leader_id, 0, msg->m_data.m_leader_name, invitee_name}, msg->session_token(), this));
}

void TeamHandler::on_team_member_kicked(TeamMemberKickedMessage *msg) {

    qCDebug(logTeams) << "kicked_by: | " << msg->m_data.m_leader_id << "sent_to: " << msg->m_data.m_kickee_id;
}

void TeamHandler::on_team_member_invite_handled(uint32_t invitee_id, QString &invitee_name, QString &leader_name, bool accepted) {

    qCDebug(logTeams) << "invite handled_by: " << accepted << invitee_id << invitee_name << leader_name;

    Team *invitee_team = nullptr;

    for (Team *t : m_state.m_team_list) 
    {
        if (t->containsEntityName(invitee_name))
            invitee_team = t;
    }

	if (invitee_team == nullptr)
	{
		QString m = "The team invitation has expired or is invalid.";
		qCritical() << m << invitee_id << invitee_name << leader_name;

        m_state.m_map_handler->putq(new UserRouterInfoMessage(
			{m, MessageChannel::USER_ERROR, invitee_id, invitee_id}, 0));

		return;
	} 
    else if (!invitee_team->isTeamLeader(leader_name))
	{
		QString m = "The team leader has changed since this invite was sent.";
		qCritical() << m << invitee_id << invitee_name << leader_name;

        m_state.m_map_handler->putq(new UserRouterInfoMessage(
			{m, MessageChannel::USER_ERROR, invitee_id, invitee_id}, 0));

		return;
	}
	else if (!invitee_team->isNamePending(invitee_name))
	{
		QString m = "The team invitation has has already been handled.";
		qCritical() << m << invitee_id << invitee_name << leader_name;

        m_state.m_map_handler->putq(new UserRouterInfoMessage(
			{m, MessageChannel::USER_ERROR, invitee_id, invitee_id}, 0));

		return;
	}

    if (accepted)
    {
        TeamingError e = invitee_team->acceptTeamInvite(invitee_name);

        if (e == TeamingError::OK) 
        {
            invitee_team->m_transient = false;
            // update clients
        }
        else
        {
            qCCritical(logTeams) << "Team invitation responded to, but invalid:" << invitee_team->m_team_idx << invitee_name;
        }
    }
    else
    {
        TeamingError e = invitee_team->removeTeamMember(invitee_name);

        if (e == TeamingError::TEAM_DISBANDED)
        {
            qCDebug(logTeams) << "Team disbanded:" << invitee_team->m_team_idx;
            delete_team(invitee_team);
            qCDebug(logTeams) <<"Num teams:" << m_state.m_team_list.size();
        }

        if (e == TeamingError::TEAM_DISBANDED || \
            e == TeamingError::OK)
        {
            QString m = "The team invitation has been declined.";
            m_state.m_map_handler->putq(new UserRouterInfoMessage(
                {m, MessageChannel::USER_ERROR, invitee_id, invitee_id}, 0));
        }
    }
}

void TeamHandler::on_team_refresh_lfg(TeamRefreshLFGMessage *msg)
{
	const uint32_t db_id = msg->m_data.m_db_id;
	const QString &name = msg->m_data.m_name;

	msg->m_data.m_lfg_list = m_state.m_lfg_list;

	m_state.m_map_handler->putq(new UserRouterOpaqueRequest(
		{__route(msg), db_id, db_id, name, name}, 
		msg->session_token(), this));
}

void TeamHandler::on_team_toggle_lfg(TeamToggleLFGMessage *msg)
{
	const uint32_t db_id = msg->m_data.m_db_id;
	const QString &name = msg->m_data.m_name;

	if (db_id_is_lfg(db_id))
	{
		remove_lfg(db_id);
		msg->m_data.m_char_data.m_lfg = false;

		// forward Opaque ToggleLFG message to UserRouter
		// This will cause MapInstance to update the entity's value with m_lfg
		m_state.m_map_handler->putq(new UserRouterOpaqueRequest(
			{__route(msg), db_id, db_id, name, name}, 
			msg->session_token(), this));

		return;
	}

	// enable LFG

	Team *t = team_for_db_id(db_id);

	if (t != nullptr && !t->isNamePending(name))
	{
		QString m = "You are already on a team! You cannot enable LFG.";
        m_state.m_map_handler->putq(new UserRouterInfoMessage(
			{m, MessageChannel::USER_ERROR, db_id, db_id}, 0));

		return;
	}

	LFGMember m;
	m.m_db_id 		= db_id;
	m.m_name 		= name;
    m.m_classname   = msg->m_data.m_char_data.m_class_name;
    m.m_origin      = msg->m_data.m_char_data.m_origin_name;
    m.m_level       = msg->m_data.m_char_data.m_level;

	add_lfg(m);

	msg->m_data.m_char_data.m_lfg = true;

	// forward Opaque ToggleLFG message to UserRouter
	// This will cause MapInstance to update the entity's value with m_lfg
	m_state.m_map_handler->putq(new UserRouterOpaqueRequest(
		{__route(msg), db_id, db_id, name, name}, 
		msg->session_token(), this));

	// since the LFG flag is true
	// send message to pop up window also
	m_state.m_map_handler->putq(new UserRouterOpaqueRequest(
		{__route(new TeamRefreshLFGMessage({db_id, name, m_state.m_lfg_list}, 0)), db_id, db_id, name, name}, 
		msg->session_token(), this));
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
        case evTeamMemberInviteAcceptedMessage: {
            TeamMemberInviteAcceptedMessage *msg = static_cast<TeamMemberInviteAcceptedMessage *>(ev);
            on_team_member_invite_handled(msg->m_data.m_invitee_id, msg->m_data.m_invitee_name, msg->m_data.m_leader_name, true);
            break;
        }
        case evTeamMemberInviteDeclinedMessage: {
            TeamMemberInviteDeclinedMessage *msg = static_cast<TeamMemberInviteDeclinedMessage *>(ev);
            on_team_member_invite_handled(msg->m_data.m_invitee_id, msg->m_data.m_invitee_name, msg->m_data.m_leader_name, false);
            break;
        }
		case evTeamToggleLFGMessage:
            on_team_toggle_lfg(static_cast<TeamToggleLFGMessage *>(ev));
            break;
		case evTeamRefreshLFGMessage:
            on_team_refresh_lfg(static_cast<TeamRefreshLFGMessage *>(ev));
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