/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Components/EventProcessor.h"
#include "Common/Servers/MessageBusEndpoint.h"
#include "Common/Servers/InternalEvents.h"
#include "Common/Servers/ClientManager.h"
#include "Common/GameData/Friend.h"
#include <unordered_map>
#include <set>
#include <vector>

namespace SEGSEvents
{
struct FriendConnectedMessage;
struct FriendAddedMessage;
struct FriendRemovedMessage;
} // end of namespace SEGSEvents

struct MapInfo
{
    uint64_t session_token;
    uint32_t server_id; //this is the owner ID aka game server id
    uint32_t instance_id; //this is the template ID aka map instance id
};

struct PlayerInfo
{
    std::set<uint32_t> m_players_added; //a set of player ids that added this friend
    FriendsList m_friends_list;
    MapInfo m_map_info;
    bool m_is_online;
};
struct FriendHandlerState
{
    //Key is db ID of char, value is everything associated
    std::unordered_map<uint32_t,PlayerInfo> m_player_info_map;
    int m_game_server_id;

    bool is_online(uint32_t m_db_id) const
    {
        auto search = m_player_info_map.find(m_db_id);
        if(search == m_player_info_map.end())
            return false;
        return search->second.m_is_online;
    }
};

class FriendHandler : public EventProcessor
{
public:
    IMPL_ID(FriendHandler)

    FriendHandler(int for_game_server_id);
    ~FriendHandler() override;
    void dispatch(SEGSEvents::Event *ev) override;

    FriendHandlerState m_state;

    // EventProcessor interface
protected:
    void serialize_from(std::istream &is) override;
    void serialize_to(std::ostream &is) override;
protected:
    // transient value.
    MessageBusEndpoint m_message_bus_endpoint;
};
