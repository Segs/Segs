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
#include "MapClientSession.h"
#include "Common/Servers/ClientManager.h"
#include <unordered_map>
#include <vector>

class FriendHandler : public EventProcessor
{
    using SessionStore = ClientSessionStore<MapClientSession>;

    //Key is ID of character who comes online, value is vector of IDs who have added this friend
    static std::unordered_map<int,std::vector<int>> s_friend_map;
private:
    void on_entity_response(GetEntityResponse *ev);
    void on_client_connected(ClientConnectedMessage* msg);
    void on_client_disconnected(ClientDisconnectedMessage* msg);
protected:
    MessageBusEndpoint m_message_bus_endpoint;
public:
    SessionStore m_session_store;
    FriendHandler();
    void dispatch(SEGSEvent *ev) override;
};
