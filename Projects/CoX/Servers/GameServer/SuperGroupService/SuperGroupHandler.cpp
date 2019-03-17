/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "SuperGroupHandler.h"
#include "SuperGroupHandlerEvents.h"
//#include "Messages/SuperGroupService/SuperGroupEvents.h"
#include "Common/GameData/SuperGroup.h"
#include "Common/Servers/HandlerLocator.h"

using namespace SEGSEvents;
namespace
{
/*
 * This function takes new SuperGroup data and adds it to the SG array
 */
void on_create_supergroup(SuperGroupHandlerState &state, CreateSuperGroupMessage *msg)
{
    //addSuperGroup(msg->m_data);
    /*
    MapClientSession &sess(m_session_store.session_from_event(ev));
    Entity *ent = sess.m_ent;

    SuperGroup *sg = &g_all_supergroups.at(ev->m_data.m_sg_db_id);
    sg->m_sg_db_id = ev->m_data.m_sg_db_id;
    ent->m_char->m_char_data.m_supergroup.m_sg_db_id = ev->m_data.m_sg_db_id;
    */
}

/*
 * This function takes removes a SuperGroup based upon ID
 */
void on_remove_supergroup(SuperGroupHandlerState &state, RemoveSuperGroupMessage *msg)
{
    removeSuperGroup(msg->m_data.m_sg_db_id);
}

/*
 * This function takes a character's DB id, and if they are online,
 * sends the MapInstance their SuperGroup roster to send to them.
 */
void send_update_sg_roster(SuperGroupHandlerState &state, uint32_t sg_db_id)
{
    if(state.is_online(sg_db_id))
    {
        const SuperGroupInfo &entry(state.m_sg_info_map[sg_db_id]);
        EventProcessor *tgt = HandlerLocator::getMapInstance_Handler(
                    entry.m_sg_session_data.server_id,
                    entry.m_sg_session_data.instance_id);

        tgt->putq(new SendSGRosterMessage({entry.m_sg_session_data.session_token, entry.m_sg_members},0));
    }
}
/*
 * We take a SuperGroup roster for a player here and update online status.
 */
void update_sg_roster(SuperGroupHandlerState &state, uint32_t sg_db_id, std::vector<SuperGroupStats> sg_members)
{
    state.m_sg_info_map[sg_db_id].m_sg_members = sg_members;
    /*
     * Add our supergroup to the set
     */
    for(const SuperGroupStats &sgs : state.m_sg_info_map[sg_db_id].m_sg_members)
        state.m_sg_info_map[sgs.m_member_db_id].m_sg_idxs.insert(sg_db_id);

    //Iterate through the roster and update online status accordingly
    for(SuperGroupStats &sgs : state.m_sg_info_map[sg_db_id].m_sg_members)
        sgs.m_is_online = state.is_online(sgs.m_member_db_id);

    send_update_sg_roster(state, sg_db_id);
}

/*
 * Update online status and map zone of friends for player.
 */
void refresh_sg_roster(SuperGroupHandlerState &state, uint32_t sg_db_id)
{
    //Iterate through the supergroup list and update online status accordingly
    for(SuperGroupStats &sgs : state.m_sg_info_map[sg_db_id].m_sg_members)
        sgs.m_is_online = state.is_online(sgs.m_member_db_id);

    send_update_sg_roster(state, sg_db_id);
}

/*
 * This function runs only when a new client connects.  We get their FriendsList,
 * db id, and map instance information and store it.  We also notify other players
 * who may have added this client that they are now online.
 */
void on_client_connected(SuperGroupHandlerState &state, SGMemberConnectedMessage *msg)
{
    //A player has connected, we need to notify all the people that have added this character as a friend
    uint32_t &char_db_id = msg->m_data.m_char_db_id;

    //Store the map instance ID so that we know where to send the constructed FriendsList
    uint64_t session_token = msg->m_data.m_session;
    uint32_t server_id = msg->m_data.m_server_id;
    uint32_t instance_id = msg->m_data.m_sub_server_id;
    state.m_sg_info_map[char_db_id].m_sg_session_data = SuperGroupSessionData{session_token, server_id, instance_id};

    //Update this player/character's online status
    state.m_sg_info_map[char_db_id].m_is_online = true;

    //This might need to go into the for loop below?  Should work either way I think
    EventProcessor *inst_tgt = HandlerLocator::getMapInstance_Handler(
                state.m_sg_info_map[char_db_id].m_sg_session_data.server_id,
                state.m_sg_info_map[char_db_id].m_sg_session_data.instance_id);

    //Iterate over set of all people who have added this character
    for(auto const& val : state.m_sg_info_map[char_db_id].m_sg_idxs)
    {
        //We need to notify all the people who added this player (if they're online)
        if(state.is_online(val)){
            uint32_t friend_id = val;
            refresh_sg_roster(state,val);
            inst_tgt->putq(new SendNotifySGMembersMessage({state.m_sg_info_map[char_db_id].m_sg_session_data.session_token,
                                                        state.m_sg_info_map[friend_id].m_sg_session_data.session_token},0));
        }
    }

    update_sg_roster(state,char_db_id, msg->m_data.m_sg_members);
}

/*
 * When a client disconnects, update their online status and
 * update lists for the friends that added this player.
 */
void on_client_disconnected(SuperGroupHandlerState &state, ClientDisconnectedMessage *msg)
{
    uint32_t char_db_id = msg->m_data.m_char_db_id;
    state.m_sg_info_map[char_db_id].m_is_online = false;

    for(auto const& val : state.m_sg_info_map[char_db_id].m_sg_idxs)
    {
        refresh_sg_roster(state,val);
    }
}

/*
 * This function adds a SGMember to a SuperGroup Roster, then sends the updated list.
 */
void on_sgmember_added(SuperGroupHandlerState &state, SGMemberAddedMessage *msg)
{
    state.m_sg_info_map[msg->m_data.m_added_id].m_sg_idxs.insert(msg->m_data.m_sg_db_id);

    state.m_sg_info_map[msg->m_data.m_sg_db_id].m_sg_members.emplace_back(msg->m_data.m_sgstats);
    send_update_sg_roster(state, msg->m_data.m_sg_db_id);
}

/*
 * This function removes a SGMember from a SuperGroup Roster, then sends the updated list.
 */
void on_sgmember_removed(SuperGroupHandlerState &state, SGMemberRemovedMessage *msg)
{
    uint32_t removed_id = msg->m_data.m_removed_id;
    state.m_sg_info_map[removed_id].m_sg_idxs.erase(msg->m_data.m_sg_db_id);

    //Look for all friends in your list that have match the given db id, and remove them.
    std::vector<SuperGroupStats> *tmp = &state.m_sg_info_map[msg->m_data.m_sg_db_id].m_sg_members;
    tmp->erase(std::remove_if(tmp->begin(), tmp->end(), [removed_id](SuperGroupStats const &sgs)
        {return sgs.m_member_db_id == removed_id;}), tmp->end());

    send_update_sg_roster(state, msg->m_data.m_sg_db_id);
}
} // end of anonymous namespace.

void SuperGroupHandler::dispatch(SEGSEvents::Event *ev)
{
    assert(ev);

    switch(ev->type())
    {
        case evCreateSuperGroupMessage:
            on_create_supergroup(m_state,static_cast<CreateSuperGroupMessage *>(ev));
            break;
        case evRemoveSuperGroupMessage:
            on_remove_supergroup(m_state,static_cast<RemoveSuperGroupMessage *>(ev));
            break;
        case evSGMemberConnectedMessage:
            on_client_connected(m_state,static_cast<SGMemberConnectedMessage *>(ev));
            break;
        case evClientDisconnectedMessage:
            on_client_disconnected(m_state,static_cast<ClientDisconnectedMessage *>(ev));
            break;
        case evSGMemberAddedMessage:
            on_sgmember_added(m_state,static_cast<SGMemberAddedMessage *>(ev));
            break;
        case evSGMemberRemovedMessage:
            on_sgmember_removed(m_state,static_cast<SGMemberRemovedMessage *>(ev));
            break;
        default:
            break;
    }
}

void SuperGroupHandler::serialize_from(std::istream &/*is*/)
{
    assert(false);
}

void SuperGroupHandler::serialize_to(std::ostream &/*is*/)
{
    assert(false);
}

SuperGroupHandler::SuperGroupHandler(int for_game_server_id) : m_message_bus_endpoint(*this)
{
    m_state.m_game_server_id = for_game_server_id;

    assert(HandlerLocator::getSuperGroup_Handler() == nullptr);
    HandlerLocator::setSuperGroup_Handler(this);

    m_message_bus_endpoint.subscribe(evClientDisconnectedMessage);
}

SuperGroupHandler::~SuperGroupHandler()
{
    HandlerLocator::setSuperGroup_Handler(nullptr);
}
