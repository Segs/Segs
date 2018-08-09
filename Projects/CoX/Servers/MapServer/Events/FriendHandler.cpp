/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "FriendHandler.h"
#include "Servers/GameServer/GameEvents.h"
#include "Common/Servers/HandlerLocator.h"
#include "FriendHandlerEvents.h"
#include "DataHelpers.h"
#include <QtCore/QDebug>

std::unordered_map<uint32_t,PlayerInfo> FriendHandler::s_player_info_map;

std::unordered_map<uint32_t,std::set<uint32_t>> FriendHandler::s_friend_map;
std::unordered_map<uint32_t,bool> FriendHandler::s_online_map;
std::unordered_map<uint32_t,MapInfo> FriendHandler::s_map_info_map;
int FriendHandler::s_game_server_id;

void FriendHandler::dispatch(SEGSEvent *ev)
{
    assert(ev);

    switch(ev->type())
    {
        case FriendHandlerEventTypes::evFriendConnected:
            on_client_connected(static_cast<FriendConnectedMessage *>(ev));
            break;
        case Internal_EventTypes::evClientDisconnected:
            on_client_disconnected(static_cast<ClientDisconnectedMessage *>(ev));
            break;
        case FriendHandlerEventTypes::evFriendAdded:
            on_friend_added(static_cast<FriendAddedMessage *>(ev));
            break;
        case FriendHandlerEventTypes::evFriendRemoved:
            on_friend_removed(static_cast<FriendRemovedMessage *>(ev));
            break;
        default:
            break;
    }
}

/*
 * This function takes a character's DB id, and if they are online,
 * sends the MapInstance their FriendsList to send to them.
 */
void FriendHandler::send_update_friends_list(uint32_t char_db_id)
{
    if(is_online(char_db_id))
    {
        EventProcessor *tgt = HandlerLocator::getMapInstance_Handler(
                    s_player_info_map[char_db_id].m_map_info.server_id,
                    s_player_info_map[char_db_id].m_map_info.instance_id);

        tgt->putq(new SendFriendListMessage({s_player_info_map[char_db_id].m_map_info.session_token,
                                            s_player_info_map[char_db_id].m_friends_list}));
    }
}

/*
 * We take a FriendsList for a player here and update online status.
 */
void FriendHandler::update_player_friends(uint32_t char_db_id, FriendsList friends_list)
{
    s_player_info_map[char_db_id].m_friends_list = friends_list;
    FriendsList *ptr_list = &s_player_info_map[char_db_id].m_friends_list; //just done for readability sake

    /*
     * Add our character to the set for each character we've added.
     */
    for(uint i=0; i<ptr_list->m_friends.size(); i++){
        uint32_t id = ptr_list->m_friends[i].m_db_id;
        s_player_info_map[id].m_players_added.insert(char_db_id);
    }

    //Iterate through the friends list and update online status accordingly
    for(Friend& f : ptr_list->m_friends)
    {
        f.m_online_status = is_online(f.m_db_id);
        f.m_mapname = getFriendDisplayMapName(f);
    }

    send_update_friends_list(char_db_id);
}

/*
 * Update online status and map zone of friends for player.
 */
void FriendHandler::refresh_player_friends(uint32_t char_db_id)
{
    FriendsList *ptr_list = &s_player_info_map[char_db_id].m_friends_list; //just done for readability sake

    //Iterate through the friends list and update online status accordingly
    for(Friend& f : ptr_list->m_friends)
    {
        f.m_online_status = is_online(f.m_db_id);
        f.m_mapname = getFriendDisplayMapName(f);
    }

    send_update_friends_list(char_db_id);
}

/*
 * This function runs only when a new client connects.  We get their FriendsList,
 * db id, and map instance information and store it.  We also notify other players
 * who may have added this client that they are now online.
 */
void FriendHandler::on_client_connected(FriendConnectedMessage *msg)
{
    //A player has connected, we need to notify all the people that have added this character as a friend
    uint32_t &char_db_id = msg->m_data.m_char_db_id;

    //Store the map instance ID so that we know where to send the constructed FriendsList
    uint64_t session_token = msg->m_data.m_session;
    uint32_t server_id = msg->m_data.m_server_id;
    uint32_t instance_id = msg->m_data.m_sub_server_id;
    s_player_info_map[char_db_id].m_map_info = MapInfo{session_token, server_id, instance_id};

    //Update this player/character's online status
    s_player_info_map[char_db_id].m_is_online = true;

    //This might need to go into the for loop below?  Should work either way I think
    EventProcessor *inst_tgt = HandlerLocator::getMapInstance_Handler(
                s_player_info_map[char_db_id].m_map_info.server_id,
                s_player_info_map[char_db_id].m_map_info.instance_id);

    //Iterate over set of all people who have added this character
    for(auto const& val : s_player_info_map[char_db_id].m_players_added)
    {
        //We need to notify all the people who added this player (if they're online)
        if(is_online(val)){
            uint32_t friend_id = val;
            refresh_player_friends(val);
            inst_tgt->putq(new SendNotifyFriendMessage({s_player_info_map[char_db_id].m_map_info.session_token,
                                                        s_player_info_map[friend_id].m_map_info.session_token}));
        }
    }

    update_player_friends(char_db_id, msg->m_data.m_friends_list);
}

/*
 * When a client disconnects, update their online status and
 * update lists for the friends that added this player.
 */
void FriendHandler::on_client_disconnected(ClientDisconnectedMessage *msg)
{
    uint32_t &char_db_id = msg->m_data.m_char_db_id;
    s_player_info_map[char_db_id].m_is_online = false;

    for(auto const& val : s_player_info_map[char_db_id].m_players_added)
    {
        refresh_player_friends(val);
    }
}

/*
 * This function adds a Friend to a FriendsList, then sends the updated list.
 */
void FriendHandler::on_friend_added(FriendAddedMessage *msg)
{
    s_player_info_map[msg->m_data.m_added_id].m_players_added.insert(msg->m_data.m_char_db_id);

    FriendsList *list = &s_player_info_map[msg->m_data.m_char_db_id].m_friends_list;
    list->m_has_friends = true;
    list->m_friends_count++;
    list->m_friends.emplace_back(msg->m_data.m_friend);
    send_update_friends_list(msg->m_data.m_char_db_id);
}

/*
 * This function removes a Friend from a FriendsList, then sends the updated list.
 */
void FriendHandler::on_friend_removed(FriendRemovedMessage *msg)
{
    uint32_t removed_id = msg->m_data.m_removed_id;
    s_player_info_map[removed_id].m_players_added.erase(msg->m_data.m_char_db_id);

    FriendsList *list = &s_player_info_map[msg->m_data.m_char_db_id].m_friends_list;

    //Look for all friends in your list that have match the given db id, and remove them.
    std::vector<Friend> *tmp = &s_player_info_map[msg->m_data.m_char_db_id].m_friends_list.m_friends;
    tmp->erase(std::remove_if(tmp->begin(), tmp->end(), [removed_id](Friend const& f)
        {return f.m_db_id == removed_id;}), tmp->end());

    if(list->m_friends.empty())
        list->m_has_friends = false;

    list->m_friends_count = list->m_friends.size();
    send_update_friends_list(msg->m_data.m_char_db_id);
}

bool FriendHandler::is_online(uint32_t m_db_id)
{
    auto search = s_player_info_map.find(m_db_id);
    if(search != s_player_info_map.end())
        return s_player_info_map[m_db_id].m_is_online;
    else
        return false;
}

void FriendHandler::set_game_server_id(int m_id)
{
    s_game_server_id = m_id;
}

FriendHandler::FriendHandler() : m_message_bus_endpoint(*this)
{
    assert(HandlerLocator::getFriend_Handler() == nullptr);
    HandlerLocator::setFriend_Handler(this);

    m_message_bus_endpoint.subscribe(Internal_EventTypes::evClientDisconnected);
}
