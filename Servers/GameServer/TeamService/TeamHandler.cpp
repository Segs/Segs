/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2020 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "TeamHandler.h"
#include "Components/SEGSEventFactory.h"
#include "Messages/Game/GameEvents.h"
#include "Common/Servers/HandlerLocator.h"
#include "Messages/TeamService/TeamEvents.h"
#include "Messages/UserRouterService/UserRouterEvents.h"
#include "Messages/Map/MessageChannels.h"
#include <QtCore/QDebug>

using namespace SEGSEvents;

void TeamHandler::notify_team_of_changes(Team *t, uint64_t session_token)
{
    std::vector<uint32_t> ids;
    std::vector<QString> names;

    for (const auto &member : t->m_data.m_team_members)
    {
        qCDebug(logTeams) << "notifying team members:" << \
            member.tm_idx << \
            member.tm_name << \
            member.tm_pending;

        if (member.tm_pending)
            continue;

        ids.push_back(member.tm_idx);
        names.push_back(member.tm_name);

        if (db_id_is_lfg(member.tm_idx))
        {
            remove_lfg(member.tm_idx);

            TeamToggleLFGMessage *msg = new TeamToggleLFGMessage({member.tm_idx, member.tm_name, {}}, session_token);
            msg->m_data.m_char_data.m_lfg = false;

            ;
            // forward Opaque ToggleLFG message to UserRouter
            // This will cause MapInstance to update the entity's value with m_lfg
            m_state.m_map_handler->putq(new UserRouterOpaqueRequest(
                {to_storage(msg), member.tm_idx, member.tm_name, {member.tm_idx}, {member.tm_name}},
                msg->session_token(), this));
        }
    }

    Team::TeamData d = t->m_data;

    d.m_team_members.erase(
                std::remove_if(d.m_team_members.begin(), d.m_team_members.end(),
                            [](const Team::TeamMember & mem) { return mem.tm_pending; }),
                    d.m_team_members.end());

    TeamUpdatedMessage team_upd({d, d.m_team_members.size() < 2}, 0);
    m_state.m_map_handler->putq(new UserRouterOpaqueRequest(
        {to_storage(&team_upd), d.m_team_leader_idx, "", ids, names},
        0, this));
}

bool TeamHandler::delete_team(Team *t)
{
    int index_to_remove = -1;
    for (unsigned int i = 0; i < m_state.m_team_list.size(); i++)
    {
        Team *team = m_state.m_team_list[i];
        if (team->m_data.m_team_idx == t->m_data.m_team_idx)
        {
            index_to_remove = i;
            qCDebug(logTeams) << "Removing team:" << t->m_data.m_team_idx;
            delete team;
            break;
        }
    }

    if (index_to_remove == -1)
    {
        qCDebug(logTeams) << "Team not found:" << t->m_data.m_team_idx;
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

    qCCritical(logTeams) << "Couldn't find id for name:" << name;

    return 0;
}

bool TeamHandler::name_known(const QString &name)
{
    for (const auto &e : m_state.m_id_to_name)
        if (e.second == name)
            return true;

    return false;
}

void TeamHandler::on_client_connected(ClientConnectedMessage *msg)
{
//    uint32_t id = msg->m_data.m_char_db_id;
//
//    bool player_on_team = false;
//
//    for (Team *t : m_state.m_team_list) 
//	{
//        if (t->containsEntityID(id))
//        {
//            player_on_team = true;
//            break;
//        }
//	}
}

void TeamHandler::on_client_disconnected(ClientDisconnectedMessage *msg)
{
//    if (m_id_to_map_instance.count(msg->m_data.m_char_db_id))
//        m_id_to_map_instance.erase(msg->m_data.m_char_db_id);
}

void TeamHandler::on_user_router_opaque_response(UserRouterOpaqueResponse *msg)
{
	const uint32_t sender_id = msg->m_data.m_req.m_sender_id;
	const UserRouterError e = msg->m_data.m_error;

    Event *src_event = from_storage(msg->m_data.m_req.m_payload);

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
				m = "Weird error on toggling LFG.";
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
				m = "Weird error on refreshing LFG.";
			}
			else 
			{
				m = "Refreshing LFG list";
				c = MessageChannel::TEAM;
			}
			break;
		}
        case evTeamUpdatedMessage:
        {
			if (e == UserRouterError::USER_OFFLINE)
			{
				qCritical() << "got user offline error while trying to update teams";
				m = "Weird error on updating team.";
			}

            return;
        }
        case evTeamMemberKickedMessage:
        {
			if (e == UserRouterError::USER_OFFLINE)
			{
				qCritical() << "got user offline error while trying to kick team member";
				m = "Weird error on kicking member from team.";
			}

            return;
        }
        case evTeamLeaveTeamMessage:
        {
			if (e == UserRouterError::USER_OFFLINE)
			{
				qCritical() << "got user offline error while trying to leave team";
				m = "Weird error on leaving team.";
			}

            return;
        }
		default:
			qCritical() << "MUST HANDLE OPAQUE RESPONSE FOR EVENT:" << src_event->type();
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
    {
		if (elem.first == msg->m_data.m_response_name)
        {
			dispatch(elem.second);
            elem.second->release();
        }
    }

	m_state.m_pending_events.erase(msg->m_data.m_response_name);
}

void TeamHandler::on_team_member_invited(TeamMemberInvitedMessage *msg)
{

    const uint32_t leader_id = msg->m_data.m_leader_id;
    QString leader_name = msg->m_data.m_leader_name;
    QString invitee_name = msg->m_data.m_invitee_name;

    // TODO: look this up for verification
    m_state.m_id_to_name[leader_id] = leader_name;

    if (!name_known(invitee_name))
    {
        qCDebug(logTeams) << "looking up name:" << invitee_name;
        m_state.m_pending_events.insert({invitee_name, msg->shallow_copy()});

        m_state.m_map_handler->putq(new UserRouterQueryRequest(
            {0, invitee_name}, 
            msg->session_token(), this));

        return;
    }

    const uint32_t invitee_id = id_for_name(invitee_name);

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

        if (t->containsEntityID(invitee_id))
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

        TeamingError e = leader_team->addTeamMember(leader_id, leader_name, false);

        if (e != TeamingError::OK)
        {
            qCritical() << "Error adding entity ID to new team" << int(e);
            delete leader_team;
            return;
        }

        m_state.m_team_list.emplace_back(leader_team);
    }

    TeamingError e = leader_team->addTeamMember(invitee_id, invitee_name, true);

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
    m_state.m_map_handler->putq(new UserRouterOpaqueRequest({to_storage(msg), leader_id, msg->m_data.m_leader_name, {}, {invitee_name}}, msg->session_token(), this));
}

void TeamHandler::on_team_member_kicked(TeamMemberKickedMessage *msg) {

    uint32_t leader_id = msg->m_data.m_leader_id;
    QString kickee_name = msg->m_data.m_kickee_name;

    qCDebug(logTeams) << "kicked_by: | " << leader_id << "sent_to: " << kickee_name;

    // first make sure kicker is the leader of a team
    Team *team = nullptr;

    for (Team *t : m_state.m_team_list) 
    {
        if (t->isTeamLeader(leader_id))
        {
            team = t;
            break;
        }
    }

    if (team == nullptr)
    {
		QString m = "You are not the leader of a team.";
		qCritical() << m << leader_id;

        m_state.m_map_handler->putq(new UserRouterInfoMessage(
			{m, MessageChannel::USER_ERROR, leader_id, leader_id}, 0));

        return;
    }

    TeamingError e = team->removeTeamMember(id_for_name(kickee_name));

    if (e == TeamingError::OK || \
            e == TeamingError::TEAM_DISBANDED)
    {
        // forward kick message to the kickee
        m_state.m_map_handler->putq(new UserRouterOpaqueRequest(
            {to_storage(msg), leader_id, "", {}, {kickee_name}},
                msg->session_token(), this));

        notify_team_of_changes(team);

        if (e == TeamingError::TEAM_DISBANDED)
        {
            delete_team(team);
        }
    }
    else 
    {
		QString m = "There was an error kicking that player.";
		qCritical() << m << leader_id << kickee_name;

        m_state.m_map_handler->putq(new UserRouterInfoMessage(
			{m, MessageChannel::USER_ERROR, leader_id, leader_id}, 0));
    }

    
}

void TeamHandler::on_team_member_make_leader(SEGSEvents::TeamMakeLeaderMessage *msg)
{
    uint32_t leader_id = msg->m_data.m_leader_id;
    QString new_leader_name = msg->m_data.m_new_leader_name;

    qCDebug(logTeams) << "passing leadership from:" << leader_id << "to:" << new_leader_name;

    Team *team = nullptr;

    for (Team *t : m_state.m_team_list) 
    {
        if (t->isTeamLeader(leader_id))
        {
            team = t;
            break;
        }
    }

    if (team == nullptr)
    {
		QString m = "You are not the leader of a team.";
		qCritical() << m << leader_id;

        m_state.m_map_handler->putq(new UserRouterInfoMessage(
			{m, MessageChannel::USER_ERROR, leader_id, leader_id}, 0));

        return;
    }

    if (!team->containsEntityName(new_leader_name) || team->isNamePending(new_leader_name))
    {
		QString m = "You must pass leadership to a player on your team.";
		qCritical() << m << leader_id;

        m_state.m_map_handler->putq(new UserRouterInfoMessage(
			{m, MessageChannel::USER_ERROR, leader_id, leader_id}, 0));

        return;
    }

    if (!name_known(new_leader_name))
    {
        // queue event and query for new name
        
        m_state.m_pending_events.insert({new_leader_name, msg->shallow_copy()});

        m_state.m_map_handler->putq(new UserRouterQueryRequest(
            {0, new_leader_name}, 
            msg->session_token(), this));
        
        return;
    }

    team->m_data.m_team_leader_idx = id_for_name(new_leader_name);
    notify_team_of_changes(team);
}

void TeamHandler::on_team_leave_team(SEGSEvents::TeamLeaveTeamMessage *msg)
{
    uint32_t id = msg->m_data.m_id;

    Team *team = nullptr;

    for (Team *t : m_state.m_team_list) 
    {
        if (t->containsEntityID(id))
        {
            team = t;
            break;
        }
    }

    if (team == nullptr)
    {
		QString m = "You are not on a team.";
		qCritical() << m << id;

        m_state.m_map_handler->putq(new UserRouterInfoMessage(
			{m, MessageChannel::USER_ERROR, id, id}, 0));

        return;
    }

    TeamingError e = team->removeTeamMember(id);

    if (e == TeamingError::OK || \
            e == TeamingError::TEAM_DISBANDED)
    {
        // forward leave team message to the mapinstance
        m_state.m_map_handler->putq(new UserRouterOpaqueRequest(
            {to_storage(msg), id, "", {id}, {}},
                msg->session_token(), this));

        notify_team_of_changes(team);

        if (e == TeamingError::TEAM_DISBANDED)
        {
            delete_team(team);
        }
    }
    else
    {
		QString m = "There was an error leaving the team.";
		qCritical() << m << id;

        m_state.m_map_handler->putq(new UserRouterInfoMessage(
			{m, MessageChannel::USER_ERROR, id, id}, 0));
    }
}

void TeamHandler::on_team_member_invite_handled(uint32_t invitee_id, QString &invitee_name, QString &leader_name, bool accepted, uint64_t session_token) {

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
    else if (!invitee_team->isTeamLeader(id_for_name(leader_name)))
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
        TeamingError e = invitee_team->acceptTeamInvite(invitee_name, invitee_id);

        if (e == TeamingError::OK) 
        {
            invitee_team->m_transient = false;
            // update clients
            
            notify_team_of_changes(invitee_team, session_token);
        }
        else
        {
            qCCritical(logTeams) << "Team invitation responded to, but invalid:" << invitee_team->m_data.m_team_idx << invitee_name;
        }
    }
    else
    {
        TeamingError e = invitee_team->removeTeamMember(id_for_name(invitee_name));

        if (e == TeamingError::TEAM_DISBANDED)
        {
            qCDebug(logTeams) << "Team disbanded:" << invitee_team->m_data.m_team_idx;
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
        {to_storage(msg), db_id, name, {db_id}, {name}},
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
            {to_storage(msg), db_id, name, {db_id}, {name}},
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
        {to_storage(msg), db_id, name, {db_id}, {name}},
		msg->session_token(), this));

	// since the LFG flag is true
	// send message to pop up window also
    TeamRefreshLFGMessage refresh_lfg_msg({db_id, name, m_state.m_lfg_list}, 0);
	m_state.m_map_handler->putq(new UserRouterOpaqueRequest(
        {to_storage(&refresh_lfg_msg), db_id, name, {db_id}, {name}},
		msg->session_token(), this));
}

void TeamHandler::dispatch(SEGSEvents::Event *ev)
{
    assert(ev);

    if (m_state.m_map_handler == nullptr)
    {
        m_state.m_map_handler = HandlerLocator::getMap_Handler(m_state.m_game_server_id);
    }

    qCDebug(logTeams) << ev->type();

    switch(ev->type())
    {
        case evTeamMemberInvitedMessage:
            on_team_member_invited(static_cast<TeamMemberInvitedMessage *>(ev));
            break;
        case evTeamMemberKickedMessage:
            on_team_member_kicked(static_cast<TeamMemberKickedMessage *>(ev));
            break;
        case evTeamMakeLeaderMessage:
            on_team_member_make_leader(static_cast<TeamMakeLeaderMessage *>(ev));
            break;
        case evTeamLeaveTeamMessage:
            on_team_leave_team(static_cast<TeamLeaveTeamMessage *>(ev));
            break;
        case evTeamMemberInviteAcceptedMessage: {
            TeamMemberInviteAcceptedMessage *msg = static_cast<TeamMemberInviteAcceptedMessage *>(ev);
            on_team_member_invite_handled(msg->m_data.m_invitee_id, msg->m_data.m_invitee_name, msg->m_data.m_leader_name, true, msg->session_token());
            break;
        }
        case evTeamMemberInviteDeclinedMessage: {
            TeamMemberInviteDeclinedMessage *msg = static_cast<TeamMemberInviteDeclinedMessage *>(ev);
            on_team_member_invite_handled(msg->m_data.m_invitee_id, msg->m_data.m_invitee_name, msg->m_data.m_leader_name, false, msg->session_token());
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
        case Internal_EventTypes::evClientConnectedMessage:
			on_client_connected(static_cast<ClientConnectedMessage *>(ev));
			break;
        case Internal_EventTypes::evClientDisconnectedMessage:
			on_client_disconnected(static_cast<ClientDisconnectedMessage *>(ev));
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

    m_message_bus_endpoint.subscribe(evClientConnectedMessage);
    m_message_bus_endpoint.subscribe(evClientDisconnectedMessage);
}

TeamHandler::~TeamHandler()
{
    HandlerLocator::setTeam_Handler(nullptr);
}
