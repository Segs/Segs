/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "FriendHandler.h"
#include "Servers/GameServer/GameEvents.h"
#include "Common/Servers/HandlerLocator.h"
#include <QtCore/QDebug>

void FriendHandler::dispatch(SEGSEvent *ev)
{
    qDebug() << "FHandler dispatch";
    assert(ev);

    switch(ev->type())
    {
        case Internal_EventTypes::evClientConnected:
            qDebug() << "FHandler sees connect event";
            on_client_connected(static_cast<ClientConnectedMessage *>(ev));
            break;
        case Internal_EventTypes::evClientDisconnected:
            on_client_disconnected(static_cast<ClientDisconnectedMessage *>(ev));
            break;
        default:
            break;
    }
}

void FriendHandler::on_client_connected(ClientConnectedMessage *msg)
{
    qDebug() << "FHandler received connection";
    //qDebug() << "Connected msg: " << &msg;
}

void FriendHandler::on_client_disconnected(ClientDisconnectedMessage *msg)
{
    qDebug() << "Disconnected msg: " << &msg;
}

FriendHandler::FriendHandler() : m_message_bus_endpoint(*this)
{
    qDebug() << "FHandler constructor";
    assert(HandlerLocator::getFriend_Handler() == nullptr);
    HandlerLocator::setFriend_Handler(this);

    m_message_bus_endpoint.subscribe(Internal_EventTypes::evClientConnected);
    m_message_bus_endpoint.subscribe(Internal_EventTypes::evClientDisconnected);
}
