/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "EventProcessor.h"
#include "Servers/MessageBusEndpoint.h"
#include "Servers/InternalEvents.h"
#include "Common/Servers/ClientManager.h"
#include "GameData/Team.h"
#include <unordered_map>
#include <set>
#include <vector>

namespace SEGSEvents
{
    struct TeamMemberInvitedMessage;
    struct TeamMemberKickedMessage;
    struct TeamMemberInviteAcceptedMessage;
    struct TeamMemberInviteDeclinedMessage;
} // end of namespace SEGSEvents

struct TeamHandlerState
{
    //Key is db ID of char, value is everything associated
    int m_game_server_id;

    std::vector<Team> m_teams;
};

class TeamHandler : public EventProcessor
{
public:
    IMPL_ID(TeamHandler)

    TeamHandler(int for_game_server_id);
    ~TeamHandler() override;
    void dispatch(SEGSEvents::Event *ev) override;

    TeamHandlerState m_state;

    // EventProcessor interface
protected:
    void serialize_from(std::istream &is) override;
    void serialize_to(std::ostream &is) override;
protected:
    // transient value.
    MessageBusEndpoint m_message_bus_endpoint;
};
