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
#include "NetStructures/Friend.h"
#include <unordered_map>
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

class FriendHandler : public EventProcessor
{
public:
    IMPL_ID(FriendHandler)

    FriendHandler(int for_game_server_id);
    ~FriendHandler() override;
    bool is_online(uint32_t m_db_id);
    void dispatch(SEGSEvents::Event *ev) override;

private:

    void send_update_friends_list(uint32_t char_db_id);
    void update_player_friends(uint32_t char_db_id, FriendsList friends_list);
    void refresh_player_friends(uint32_t char_db_id);
    void on_client_connected(SEGSEvents::FriendConnectedMessage* msg);
    void on_client_disconnected(SEGSEvents::ClientDisconnectedMessage* msg);
    void on_friend_added(SEGSEvents::FriendAddedMessage* msg);
    void on_friend_removed(SEGSEvents::FriendRemovedMessage* msg);
    // EventProcessor interface
protected:
    void serialize_from(std::istream &is) override;
    void serialize_to(std::ostream &is) override;
protected:
    //Key is db ID of char, value is everything associated
    std::unordered_map<uint32_t,PlayerInfo> m_player_info_map;
    int m_game_server_id;
    // transient value.
    MessageBusEndpoint m_message_bus_endpoint;
};
