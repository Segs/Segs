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

class FriendHandler : public EventProcessor
{
private:
    //Key is ID of character who comes online, value is set of IDs who have added this friend
    static std::unordered_map<uint32_t,std::set<uint32_t>> s_friend_map;
    static std::unordered_map<uint32_t,bool> s_online_map; //key is char ID, bool is if they are online
    //key is char ID, value should have both map server ID and map instance ID
    static std::unordered_map<uint32_t,MapInfo> s_map_info_map;
    static int s_game_server_id;

    void on_player_friends(GetPlayerFriendsResponse* ev);
    void on_client_connected(ClientConnectedMessage* msg);
    void on_client_disconnected(ClientDisconnectedMessage* msg);
    void on_friend_added(FriendAddedMessage* msg);
    void on_friend_removed(FriendRemovedMessage* msg);
protected:
    MessageBusEndpoint m_message_bus_endpoint;
public:
    FriendHandler();
    void set_game_server_id(int m_id);
    bool is_online(int m_id);
    void dispatch(SEGSEvent *ev) override;
};
