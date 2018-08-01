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
#include <QtCore/QDebug>

std::unordered_map<int,std::vector<int>> FriendHandler::s_friend_map;

void FriendHandler::dispatch(SEGSEvent *ev)
{
    assert(ev);

    switch(ev->type())
    {
        case GameDBEventTypes::evGetEntityResponse:
            on_entity_response(static_cast<GetEntityResponse *>(ev));
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

void FriendHandler::on_entity_response(GetEntityResponse *ev){
    qDebug() << "Fhandler ent response";
    //MapClientSession &map_session(m_session_store.session_from_event(ev));
    //m_session_store.locked_unmark_session_for_reaping(&map_session);
    //Entity * e = map_session.m_ent;
}

void FriendHandler::on_client_connected(ClientConnectedMessage *msg)
{
    //A player has connected, notify all the people that have added this character as a friend
    //Also update this player/character's online status
    qDebug() << "FHandler received connection, char id: " << msg->m_data.m_char_id;
    //Iterate over s_friend_map[msg->m_data.m_char_id] and send message saying character logged in

    //Also read this player's friend list to see who they've added

    //MapClientSession &session(m_session_store.session_from_event(ev));
    //msg->m_data.m_char_id
}

void FriendHandler::on_client_disconnected(ClientDisconnectedMessage *msg)
{
    //Update this player/character's online status (to offline)
    qDebug() << "Disconnected msg: " << &msg;
}

FriendHandler::FriendHandler() : m_message_bus_endpoint(*this)
{
    s_friend_map[1] = {1,2,3};
    qDebug() << "friend_map: " << s_friend_map[1];
    assert(HandlerLocator::getFriend_Handler() == nullptr);
    HandlerLocator::setFriend_Handler(this);

    m_message_bus_endpoint.subscribe(Internal_EventTypes::evClientConnected);
    m_message_bus_endpoint.subscribe(Internal_EventTypes::evClientDisconnected);
    m_message_bus_endpoint.subscribe(GameDBEventTypes::evGetEntityResponse);
}
