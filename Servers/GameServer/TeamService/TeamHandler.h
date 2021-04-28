/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Components/EventProcessor.h"
#include "Servers/MessageBusEndpoint.h"
#include "Servers/InternalEvents.h"
#include "Common/Servers/ClientManager.h"
#include "GameData/Team.h"
#include "GameData/LFG.h"
#include "GameData/CharacterData.h"
#include <unordered_map>
#include <set>
#include <vector>

namespace SEGSEvents
{
    struct TeamMemberInvitedMessage;
    struct TeamMemberKickedMessage;
    struct TeamMemberInviteAcceptedMessage;
    struct TeamMemberInviteDeclinedMessage;

	struct TeamToggleLFGMessage;
	struct TeamRefreshLFGMessage;
    struct TeamMakeLeaderMessage;
    struct TeamLeaveTeamMessage;

    struct UserRouterQueryRequest;
    struct UserRouterQueryResponse;
    struct UserRouterOpaqueRequest;
    struct UserRouterOpaqueResponse;

    struct ClientConnectedMessage;
    struct ClientDisconnectedMessage;
} // end of namespace SEGSEvents

struct TeamHandlerState
{
    //Key is db ID of char, value is everything associated
    int m_game_server_id;
    EventProcessor *m_map_handler;

    std::vector<Team *> m_team_list;
	std::vector<LFGMember> m_lfg_list;

    std::map<uint32_t, QString> m_id_to_name;
    std::multimap<QString, SEGSEvents::Event *> m_pending_events;
};

class TeamHandler : public EventProcessor
{
public:
    IMPL_ID(TeamHandler)

    TeamHandler(int for_game_server_id);
    ~TeamHandler() override;
    void dispatch(SEGSEvents::Event *ev) override;

private:
    TeamHandlerState m_state;

	uint32_t id_for_name(const QString &name);
    bool name_known(const QString &name);

    void on_user_router_query_response(SEGSEvents::UserRouterQueryResponse *msg);
    void on_user_router_opaque_response(SEGSEvents::UserRouterOpaqueResponse *msg);

    void on_team_member_invited(SEGSEvents::TeamMemberInvitedMessage *msg);
    void on_team_member_kicked(SEGSEvents::TeamMemberKickedMessage *msg);
    void on_team_member_make_leader(SEGSEvents::TeamMakeLeaderMessage *msg);

    void on_team_member_invite_handled(uint32_t invitee_id, QString &invitee_name, QString &leader_name, bool accepted, uint64_t session_token);

    void on_team_leave_team(SEGSEvents::TeamLeaveTeamMessage *msg);
    void on_team_toggle_lfg(SEGSEvents::TeamToggleLFGMessage *msg);
    void on_team_refresh_lfg(SEGSEvents::TeamRefreshLFGMessage *msg);

    void notify_team_of_changes(Team *t, uint64_t session_token=0);
	bool delete_team(Team *t);

	bool name_is_lfg(const QString &name);
	bool db_id_is_lfg(const uint32_t db_id);
	void add_lfg(LFGMember m);
	void remove_lfg(const uint32_t db_id);
	Team* team_for_db_id(const uint32_t db_id);
	Team* team_for_name(const QString &name);

    void on_client_connected(SEGSEvents::ClientConnectedMessage *msg);
    void on_client_disconnected(SEGSEvents::ClientDisconnectedMessage *msg);


    // EventProcessor interface
protected:
    void serialize_from(std::istream &is) override;
    void serialize_to(std::ostream &is) override;
protected:
    // transient value.
    MessageBusEndpoint m_message_bus_endpoint;
};
