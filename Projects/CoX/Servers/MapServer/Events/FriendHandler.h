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
#include "FriendHandlerEvents.h"
#include <unordered_map>
#include <vector>


struct MapInfo
{
    uint64_t session_token;
    uint32_t server_id; //this is the owner ID aka game server id
    uint32_t instance_id; //this is the template ID aka map instance id
};

struct PlayerInfo
{
    bool m_is_online;
    std::set<uint32_t> m_players_added; //a set of player ids that added this friend
    FriendsList m_friends_list;
    MapInfo m_map_info;
};

class FriendHandler : public EventProcessor
{
private:
    //Key is db ID of char, value is everything associated
    static std::unordered_map<uint32_t,PlayerInfo> s_player_info_map;
    static int s_game_server_id;

    void send_update_friends_list(uint32_t char_db_id);
    void update_player_friends(uint32_t char_db_id, FriendsList friends_list);
    void refresh_player_friends(uint32_t char_db_id);
    void on_client_connected(FriendConnectedMessage* msg);
    void on_client_disconnected(ClientDisconnectedMessage* msg);
    void on_friend_added(FriendAddedMessage* msg);
    void on_friend_removed(FriendRemovedMessage* msg);
protected:
    MessageBusEndpoint m_message_bus_endpoint;
public:
    FriendHandler();
    void set_game_server_id(int m_id);
    bool is_online(uint32_t m_db_id);
    void dispatch(SEGSEvent *ev) override;
};
