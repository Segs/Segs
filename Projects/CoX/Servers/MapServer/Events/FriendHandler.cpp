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
#include "DataHelpers.h"
#include "MapServer.h"
#include <QtCore/QDebug>

std::unordered_map<int,std::vector<int>> FriendHandler::s_friend_map;
std::unordered_map<int,bool> FriendHandler::s_online_map;
std::unordered_map<int,int> FriendHandler::s_map_instance_map;
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
    uint32_t &m_char_id = ev->m_data.m_char_id;
    FriendsList &m_friendslist = ev->m_data.m_friendslist;

    /*
     * Iterate over the friends list, and put each friend id as a key,
     * and push the current character id into the vector value
     */
    for(uint i=0; i<m_friendslist.m_friends.size(); i++){
        s_friend_map[m_friendslist.m_friends[i].m_db_id].push_back(m_char_id);
    }

    /*
     * Iterate through the friends list and update online status accordingly
     */
    for(Friend f : m_friendslist.m_friends)
    {
        //if(s_online_map[f.m_db_id])
        auto search = s_online_map.find(f.m_db_id);
        if(search != s_online_map.end())
            f.m_online_status = true;
        else
            f.m_online_status = false;
    }

    //Send the FriendsList to MapInstance, which will call FriendsListUpdate
    EventProcessor *tgt = HandlerLocator::getMap_Handler(s_game_server_id);
    MapServer* m_mapserv = static_cast<MapServer*>(HandlerLocator::getMap_Handler(s_game_server_id));
}

void FriendHandler::on_client_connected(ClientConnectedMessage *msg)
{
    //A player has connected, notify all the people that have added this character as a friend
    //MapClientSession &session(m_session_store.session_from_event(msg));

    uint32_t &m_char_id = msg->m_data.m_char_id;
    //Iterate over map and send message saying character logged in
    for(auto const& val : s_friend_map[m_char_id])
    {
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

    //Update this player/character's online status
    s_online_map[m_char_id] = true;

    //Store the map instance ID so that we know where to send the constructed FriendsList
    s_map_instance_map[m_char_id] = msg->m_data.m_sub_server_id;
    qDebug() << "Serv id: " << msg->m_data.m_server_id; //this is the owner ID aka game server id
    qDebug() << "Sub id: " << msg->m_data.m_sub_server_id; //this is the template ID aka map instance id

    //Also read this player's friend list to see who they've added
    //To do this, we send a GetFriendsListRequest to GameDBSyncHandler
    EventProcessor *tgt = HandlerLocator::getGame_DB_Handler(s_game_server_id);
    tgt->putq(new GetPlayerFriendsRequest({msg->m_data.m_char_id},msg->session_token(),this));
}

void FriendHandler::on_client_disconnected(ClientDisconnectedMessage *msg)
{
    //Update this player/character's online status (to offline)
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
