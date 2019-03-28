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
#include "GameDatabase/GameDBSyncHandler.h"
//#include "Common/Servers/ClientManager.h"
#include "Common/GameData/SuperGroup.h"
//#include <unordered_map>
//#include <set>
//#include <vector>

namespace SEGSEvents
{
struct CreateSuperGroupMessage;
struct RemoveSuperGroupMessage;
struct SGMemberConnectedMessage;
struct SGMemberAddedMessage;
struct SGMemberRemovedMessage;
struct CreateNewSuperGroupResponse;
struct GetSuperGroupResponse;
struct SuperGroupNameDuplicateResponse;
struct RemoveSuperGroupResponse;
struct ClientConnectedMessage;
struct ClientDisconnectedMessage;
} // end of namespace SEGSEvents

struct SuperGroupSessionData
{
    uint64_t m_session_token;
    uint32_t m_server_id; //this is the owner ID aka game server id
    uint32_t m_instance_id; //this is the template ID aka map instance id
};

struct SuperGroupInfo
{
    std::set<uint32_t> m_sg_idxs; //a set of sg_db_ids
    vSuperGroupRoster m_sg_members;
    SuperGroupSessionData m_sg_session_data;
    bool m_is_online;
};

struct SuperGroupHandlerState
{
    //Key is db ID of char, value is everything associated
    std::unordered_map<uint32_t, SuperGroupInfo> m_sg_info_map;
    int m_game_server_id;

    bool is_online(uint32_t m_db_id) const
    {
        auto search = m_sg_info_map.find(m_db_id);
        if(search == m_sg_info_map.end())
            return false;
        return search->second.m_is_online;
    }
};

class SuperGroupHandler : public EventProcessor
{
private:
    GameDBSyncHandler* m_db_handler;

    // EventProcessor interface
    void dispatch(SEGSEvents::Event *ev) override;

    void on_create_supergroup(SEGSEvents::CreateSuperGroupMessage* msg);
    void on_remove_supergroup(SEGSEvents::RemoveSuperGroupMessage* msg);
    void send_update_sg_roster(uint32_t sg_db_id);
    void update_sg_roster(uint32_t sg_db_id);
    void refresh_sg_roster(uint32_t sg_db_id);
    void on_sgmember_added(SEGSEvents::SGMemberAddedMessage* msg);
    void on_sgmember_removed(SEGSEvents::SGMemberRemovedMessage* msg);
    void on_supergroup_created(SEGSEvents::CreateNewSuperGroupResponse* msg);
    void on_get_supergroup(SEGSEvents::GetSuperGroupResponse* msg);
    void on_supergroup_name_clash(SEGSEvents::SuperGroupNameDuplicateResponse* msg);
    void on_remove_supergroup(SEGSEvents::RemoveSuperGroupResponse* msg);
    void on_client_connected(SEGSEvents::ClientConnectedMessage* msg);
    void on_client_disconnected(SEGSEvents::ClientDisconnectedMessage *msg);
protected:
    MessageBusEndpoint m_message_bus_endpoint;
    void serialize_from(std::istream &is) override;
    void serialize_to(std::ostream &is) override;
    SuperGroupHandlerState m_state;
public:
    SuperGroupHandler(int for_game_server_id);
    void set_db_handler(uint8_t id);
    IMPL_ID(SuperGroupHandler)
};
