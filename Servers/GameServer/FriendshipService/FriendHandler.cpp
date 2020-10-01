/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "FriendHandler.h"
#include "Messages/Game/GameEvents.h"
#include "Common/Servers/HandlerLocator.h"
#include "Messages/FriendshipService/FriendHandlerEvents.h"
#include <QtCore/QDebug>

using namespace SEGSEvents;
namespace
{
/*
 * This function takes a character's DB id, and if they are online,
 * sends the MapInstance their FriendsList to send to them.
 */
void send_update_friends_list(FriendHandlerState &state,uint32_t char_db_id)
{
    if(state.is_online(char_db_id))
    {
        const PlayerInfo &entry(state.m_player_info_map[char_db_id]);
        EventProcessor *tgt = HandlerLocator::getMapInstance_Handler(
                    entry.m_map_info.server_id,
                    entry.m_map_info.instance_id);

        tgt->putq(new SendFriendListMessage({entry.m_map_info.session_token, entry.m_friends_list},0));
    }
}
/*
 * We take a FriendsList for a player here and update online status.
 */
void update_player_friends(FriendHandlerState &state,uint32_t char_db_id, FriendsList friends_list)
{
    state.m_player_info_map[char_db_id].m_friends_list = friends_list;
    FriendsList *ptr_list = &state.m_player_info_map[char_db_id].m_friends_list; //just done for readability sake

    /*
     * Add our character to the set for each character we've added.
     */
    for(const Friend & fr : ptr_list->m_friends)
    {
        uint32_t id = fr.m_db_id;
        state.m_player_info_map[id].m_players_added.insert(char_db_id);
    }

    //Iterate through the friends list and update online status accordingly
    for(Friend& f : ptr_list->m_friends)
    {
        f.m_online_status = state.is_online(f.m_db_id);
        //f.m_mapname = getFriendDisplayMapName(f);
    }

    send_update_friends_list(state,char_db_id);
}

/*
 * Update online status and map zone of friends for player.
 */
void refresh_player_friends(FriendHandlerState &state,uint32_t char_db_id)
{
    FriendsList *ptr_list = &state.m_player_info_map[char_db_id].m_friends_list; //just done for readability sake

    //Iterate through the friends list and update online status accordingly
    for(Friend& f : ptr_list->m_friends)
    {
        f.m_online_status = state.is_online(f.m_db_id);
        //f.m_mapname = getFriendDisplayMapName(f);
    }

    send_update_friends_list(state,char_db_id);
}

/*
 * This function runs only when a new client connects.  We get their FriendsList,
 * db id, and map instance information and store it.  We also notify other players
 * who may have added this client that they are now online.
 */
void on_client_connected(FriendHandlerState &state,FriendConnectedMessage *msg)
{
    //A player has connected, we need to notify all the people that have added this character as a friend
    uint32_t &char_db_id = msg->m_data.m_char_db_id;

    //Store the map instance ID so that we know where to send the constructed FriendsList
    uint64_t session_token = msg->m_data.m_session;
    uint32_t server_id = msg->m_data.m_server_id;
    uint32_t instance_id = msg->m_data.m_sub_server_id;
    state.m_player_info_map[char_db_id].m_map_info = MapInfo{session_token, server_id, instance_id};

    //Update this player/character's online status
    state.m_player_info_map[char_db_id].m_is_online = true;

    //This might need to go into the for loop below?  Should work either way I think
    EventProcessor *inst_tgt = HandlerLocator::getMapInstance_Handler(
                state.m_player_info_map[char_db_id].m_map_info.server_id,
                state.m_player_info_map[char_db_id].m_map_info.instance_id);

    //Iterate over set of all people who have added this character
    for(auto const& val : state.m_player_info_map[char_db_id].m_players_added)
    {
        //We need to notify all the people who added this player (if they're online)
        if(state.is_online(val))
        {
            uint32_t friend_id = val;
            refresh_player_friends(state,val);
            inst_tgt->putq(new SendNotifyFriendMessage({state.m_player_info_map[char_db_id].m_map_info.session_token,
                                                        state.m_player_info_map[friend_id].m_map_info.session_token},0));
        }
    }

    update_player_friends(state,char_db_id, msg->m_data.m_friends_list);
}

/*
 * When a client disconnects, update their online status and
 * update lists for the friends that added this player.
 */
void on_client_disconnected(FriendHandlerState &state,ClientDisconnectedMessage *msg)
{
    uint32_t char_db_id = msg->m_data.m_char_db_id;
    state.m_player_info_map[char_db_id].m_is_online = false;

    for(auto const& val : state.m_player_info_map[char_db_id].m_players_added)
    {
        refresh_player_friends(state,val);
    }
}

/*
 * This function adds a Friend to a FriendsList, then sends the updated list.
 */
void on_friend_added(FriendHandlerState &state,FriendAddedMessage *msg)
{
    state.m_player_info_map[msg->m_data.m_added_id].m_players_added.insert(msg->m_data.m_char_db_id);

    FriendsList *list = &state.m_player_info_map[msg->m_data.m_char_db_id].m_friends_list;
    list->m_has_friends = true;
    list->m_friends_count++;
    list->m_friends.emplace_back(msg->m_data.m_friend);
    send_update_friends_list(state,msg->m_data.m_char_db_id);
}

/*
 * This function removes a Friend from a FriendsList, then sends the updated list.
 */
void on_friend_removed(FriendHandlerState &state,FriendRemovedMessage *msg)
{
    uint32_t removed_id = msg->m_data.m_removed_id;
    state.m_player_info_map[removed_id].m_players_added.erase(msg->m_data.m_char_db_id);

    FriendsList *list = &state.m_player_info_map[msg->m_data.m_char_db_id].m_friends_list;

    //Look for all friends in your list that have match the given db id, and remove them.
    std::vector<Friend> *tmp = &state.m_player_info_map[msg->m_data.m_char_db_id].m_friends_list.m_friends;
    tmp->erase(std::remove_if(tmp->begin(), tmp->end(), [removed_id](Friend const& f)
        {return f.m_db_id == removed_id;}), tmp->end());

    if(list->m_friends.empty())
        list->m_has_friends = false;

    list->m_friends_count = list->m_friends.size();
    send_update_friends_list(state,msg->m_data.m_char_db_id);
}
} // end of anonymous namespace.

void FriendHandler::dispatch(SEGSEvents::Event *ev)
{
    assert(ev);

    switch(ev->type())
    {
        case evFriendConnectedMessage:
            on_client_connected(m_state,static_cast<FriendConnectedMessage *>(ev));
            break;
        case evClientDisconnectedMessage:
            on_client_disconnected(m_state,static_cast<ClientDisconnectedMessage *>(ev));
            break;
        case evFriendAddedMessage:
            on_friend_added(m_state,static_cast<FriendAddedMessage *>(ev));
            break;
        case evFriendRemovedMessage:
            on_friend_removed(m_state,static_cast<FriendRemovedMessage *>(ev));
            break;
        default:
            break;
    }
}

void FriendHandler::serialize_from(std::istream &/*is*/)
{
    assert(false);
}

void FriendHandler::serialize_to(std::ostream &/*is*/)
{
    assert(false);
}

FriendHandler::FriendHandler(int for_game_server_id) : m_message_bus_endpoint(*this)
{
    m_state.m_game_server_id = for_game_server_id;

    assert(HandlerLocator::getFriend_Handler() == nullptr);
    HandlerLocator::setFriend_Handler(this);

    m_message_bus_endpoint.subscribe(evClientDisconnectedMessage);
}

FriendHandler::~FriendHandler()
{
    HandlerLocator::setFriend_Handler(nullptr);
}
