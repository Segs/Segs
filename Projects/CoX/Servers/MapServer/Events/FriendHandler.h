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
#include "GameDatabase/GameDBSyncEvents.h"
#include "Common/Servers/ClientManager.h"
#include <unordered_map>
#include <vector>


struct MapInfo
{
    uint64_t session_token;
    uint8_t server_id;
    int instance_id;
};

class FriendHandler : public EventProcessor
{

    //Key is ID of character who comes online, value is set of IDs who have added this friend
    static std::unordered_map<int,std::set<int>> s_friend_map;
    static std::unordered_map<int,bool> s_online_map; //key is char ID, bool is if they are online
    //key is char ID, value should have both map server ID and map instance ID
    static std::unordered_map<int,MapInfo> s_map_info_map;
    static int s_game_server_id;
private:
    void on_player_friends(GetPlayerFriendsResponse* ev);
    void on_client_connected(ClientConnectedMessage* msg);
    void on_client_disconnected(ClientDisconnectedMessage* msg);
protected:
    MessageBusEndpoint m_message_bus_endpoint;
public:
    FriendHandler();
    void set_game_server_id(int m_id);
    bool is_online(int m_id);
    void dispatch(SEGSEvent *ev) override;
};
