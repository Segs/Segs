/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "FriendHandler.h"
#include "Servers/GameServer/GameEvents.h"
#include "Common/Servers/HandlerLocator.h"
#include "GameDatabase/GameDBSyncEvents.h"
#include "FriendHandlerEvents.h"
#include "DataHelpers.h"
#include <QtCore/QDebug>

std::unordered_map<int,std::set<int>> FriendHandler::s_friend_map;
std::unordered_map<int,bool> FriendHandler::s_online_map;
std::unordered_map<int,MapInfo> FriendHandler::s_map_info_map;
int FriendHandler::s_game_server_id;

void FriendHandler::dispatch(SEGSEvent *ev)
{
    assert(ev);

    switch(ev->type())
    {
        case GameDBEventTypes::evGetPlayerFriendsResponse:
            on_player_friends(static_cast<GetPlayerFriendsResponse *>(ev));
            break;
        case Internal_EventTypes::evClientConnected:
            on_client_connected(static_cast<ClientConnectedMessage *>(ev));
            break;
        case Internal_EventTypes::evClientDisconnected:
            on_client_disconnected(static_cast<ClientDisconnectedMessage *>(ev));
            break;
        default:
            break;
    }
}

void FriendHandler::on_player_friends(GetPlayerFriendsResponse* ev)
{
    qDebug() << "who it be1 " << ev->m_data.m_friendslist.m_friends[0].m_name;
    uint32_t &m_char_id = ev->m_data.m_char_id;
    FriendsList m_friendslist = ev->m_data.m_friendslist;

    qDebug() << "who it be " << m_friendslist.m_friends[0].m_name;
    //This code might execute multiple times whenever a player changes zone,
    //but won't do any harm.  Inefficient though.
    /*
     * Iterate over the friends list, and put each friend id as a key,
     * and push the current character id into the set value
     */
    for(uint i=0; i<m_friendslist.m_friends.size(); i++){
        s_friend_map[m_friendslist.m_friends[i].m_db_id].insert(m_char_id);
    }

    /*
     * Iterate through the friends list and update online status accordingly
     */
    for(Friend f : m_friendslist.m_friends)
    {
        f.m_online_status = is_online(f.m_db_id);
    }

    //Send the FriendsList to MapInstance, which will call FriendsListUpdate
    EventProcessor *tgt = HandlerLocator::getMapInstance_Handler(
                s_map_info_map[m_char_id].server_id, s_map_info_map[m_char_id].instance_id);
    tgt->putq(new SendFriendListMessage({s_map_info_map[m_char_id].session_token,
                                        m_friendslist, s_friend_map[m_char_id]},s_map_info_map[m_char_id].session_token));
}

void FriendHandler::on_client_connected(ClientConnectedMessage *msg)
{
    //A player has connected, notify all the people that have added this character as a friend
    uint32_t &m_char_id = msg->m_data.m_char_id;

    //Update this player/character's online status
    s_online_map[m_char_id] = true;

    EventProcessor *tgt = HandlerLocator::getGame_DB_Handler(s_game_server_id);
    //Iterate over map and update friends list of all people who have added this character
    for(auto const& val : s_friend_map[m_char_id])
    {
        //We need to notify all the people who added this player (if they're online)
        if(is_online(val)){
            uint32_t friend_id = val;
            tgt->putq(new GetPlayerFriendsRequest({friend_id},msg->session_token(),this));
        }

        //We might need to check later if this character is still online?
        //if s_online_map[val]
        qDebug() << "Hey char id " << val << ", cid " << m_char_id << " just logged on";


        //Looks like we'll need a MessageHandler to do this part?
        /*
        char buf[256];
        std::string welcome_msg = std::string("Your friend logged on! ");
        welcome_msg += buf;
        sendInfoMessage(MessageChannel::SERVER,QString::fromStdString(welcome_msg),&session);
        */
    }

    //Store the map instance ID so that we know where to send the constructed FriendsList
    uint64_t session_token = msg->m_data.m_session;
    uint8_t server_id = msg->m_data.m_server_id;
    int instance_id = msg->m_data.m_sub_server_id;
    s_map_info_map[m_char_id] = MapInfo{session_token, server_id, instance_id};
    qDebug() << "Serv id: " << msg->m_data.m_server_id; //this is the owner ID aka game server id
    qDebug() << "Sub id: " << msg->m_data.m_sub_server_id; //this is the template ID aka map instance id

    //Also read this player's friend list to see who they've added
    //To do this, we send a GetFriendsListRequest to GameDBSyncHandler
    tgt->putq(new GetPlayerFriendsRequest({msg->m_data.m_char_id},msg->session_token(),this));
}

void FriendHandler::on_client_disconnected(ClientDisconnectedMessage *msg)
{
    //Update this player/character's online status (to offline)
    s_map_info_map.erase(msg->m_data.m_char_id);
    s_online_map.erase(msg->m_data.m_char_id);
}

bool FriendHandler::is_online(int m_id)
{
    //if(s_online_map[m_id])
    auto search = s_online_map.find(m_id);
    if(search != s_online_map.end())
        return true;
    else
        return false;
    //return s_online_map[m_id];
}

void FriendHandler::set_game_server_id(int m_id)
{
    s_game_server_id = m_id;
}

FriendHandler::FriendHandler() : m_message_bus_endpoint(*this)
{
    assert(HandlerLocator::getFriend_Handler() == nullptr);
    HandlerLocator::setFriend_Handler(this);

    m_message_bus_endpoint.subscribe(Internal_EventTypes::evClientConnected);
    m_message_bus_endpoint.subscribe(Internal_EventTypes::evClientDisconnected);
}
