/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "SuperGroupHandler.h"
#include "SuperGroupHandlerEvents.h"
#include "Common/Servers/HandlerLocator.h"
#include "Messages/GameDatabase/GameDBSyncEvents.h"
#include "Common/GameData/SuperGroup.h"
#include "Logging.h"

using namespace SEGSEvents;
namespace
{

} // end of anonymous namespace.

void SuperGroupHandler::dispatch(Event *ev)
{
    assert(ev);
    switch(ev->type())
    {
        // SGEvents
        case SuperGroupHandlerEventTypes::evCreateSuperGroupMessage:
            on_create_supergroup(static_cast<CreateSuperGroupMessage *>(ev));
            break;
        case SuperGroupHandlerEventTypes::evRemoveSuperGroupMessage:
            on_remove_supergroup(static_cast<RemoveSuperGroupMessage *>(ev));
            break;
        case SuperGroupHandlerEventTypes::evSGMemberAddedMessage:
            on_sgmember_added(static_cast<SGMemberAddedMessage *>(ev));
            break;
        case SuperGroupHandlerEventTypes::evSGMemberRemovedMessage:
            on_sgmember_removed(static_cast<SGMemberRemovedMessage *>(ev));
            break;
        // GameDbEvents
        case GameDBEventTypes::evCreateNewSuperGroupResponse:
            on_supergroup_created(static_cast<CreateNewSuperGroupResponse *>(ev));
            break;
        case GameDBEventTypes::evGetSuperGroupResponse:
            on_get_supergroup(static_cast<GetSuperGroupResponse *>(ev));
            break;
        case GameDBEventTypes::evSuperGroupNameDuplicateResponse:
            on_supergroup_name_clash(static_cast<SuperGroupNameDuplicateResponse *>(ev));
            break;
        case GameDBEventTypes::evRemoveSuperGroupResponse:
            on_remove_supergroup(static_cast<RemoveSuperGroupResponse *>(ev));
            break;
        // will be obtained from MessageBusEndpoint
        case Internal_EventTypes::evClientConnectedMessage:
            on_client_connected(static_cast<ClientConnectedMessage *>(ev));
            break;
        case Internal_EventTypes::evClientDisconnectedMessage:
            on_client_disconnected(static_cast<ClientDisconnectedMessage *>(ev));
            break;
        default:
            qCritical() << "SGHandler dispatch hits default! Event info: " + QString(ev->info());
            break;
    }
}

void SuperGroupHandler::set_db_handler(uint8_t id)
{
    m_db_handler = static_cast<GameDBSyncHandler*>(
                HandlerLocator::getGame_DB_Handler(id));
}

/*
 * This function takes new SuperGroup data and adds it to the SG array
 */
void SuperGroupHandler::on_create_supergroup(CreateSuperGroupMessage *msg)
{
    qCDebug(logSuperGroups) << "firing on_create_supergroup in SGHandler";
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
void SuperGroupHandler::on_remove_supergroup(RemoveSuperGroupMessage *msg)
{
    removeSuperGroup(msg->m_data.m_sg_db_id);
}

/*
 * This function takes a character's DB id, and if they are online,
 * sends the MapInstance their SuperGroup roster to send to them.
 */
void SuperGroupHandler::send_update_sg_roster(uint32_t sg_db_id)
{
    if(m_state.is_online(sg_db_id))
    {
        const SuperGroupInfo &entry(m_state.m_sg_info_map[sg_db_id]);
        EventProcessor *tgt = HandlerLocator::getMapInstance_Handler(
                    entry.m_sg_session_data.m_server_id,
                    entry.m_sg_session_data.m_instance_id);

        tgt->putq(new SendSGRosterMessage({entry.m_sg_session_data.m_session_token, entry.m_sg_members},0));
    }
}
/*
 * We take a SuperGroup roster for a player here and update online status.
 */
void SuperGroupHandler::update_sg_roster(uint32_t sg_db_id)
{
    /*
     * Add our member to the set
     */
    for(const SuperGroupStats &sgs : m_state.m_sg_info_map[sg_db_id].m_sg_members)
        m_state.m_sg_info_map[sgs.m_member_db_id].m_sg_idxs.insert(sg_db_id);

    //Iterate through the roster and update online status accordingly
    for(SuperGroupStats &sgs : m_state.m_sg_info_map[sg_db_id].m_sg_members)
        sgs.m_is_online = m_state.is_online(sgs.m_member_db_id);

    send_update_sg_roster(sg_db_id);
}

/*
 * Update online status and map zone of friends for player.
 */
void SuperGroupHandler::refresh_sg_roster(uint32_t sg_db_id)
{
    //Iterate through the supergroup list and update online status accordingly
    for(SuperGroupStats &sgs : m_state.m_sg_info_map[sg_db_id].m_sg_members)
        sgs.m_is_online = m_state.is_online(sgs.m_member_db_id);

    send_update_sg_roster(sg_db_id);
}

/*
 * This function runs only when a new client connects.  We get their FriendsList,
 * db id, and map instance information and store it.  We also notify other players
 * who may have added this client that they are now online.
 */
void SuperGroupHandler::on_client_connected(ClientConnectedMessage *msg)
{
    //A player has connected, we need to notify all the people that have added this character as a friend
    uint32_t &char_db_id = msg->m_data.m_char_db_id;

    //Store the map instance ID so that we know where to send the constructed FriendsList
    uint64_t session_token = msg->m_data.m_session;
    uint32_t server_id = msg->m_data.m_server_id;
    uint32_t instance_id = msg->m_data.m_sub_server_id;
    m_state.m_sg_info_map[char_db_id].m_sg_session_data = SuperGroupSessionData{session_token, server_id, instance_id};

    //Update this player/character's online status
    m_state.m_sg_info_map[char_db_id].m_is_online = true;

    //This might need to go into the for loop below?  Should work either way I think
    EventProcessor *inst_tgt = HandlerLocator::getMapInstance_Handler(
                m_state.m_sg_info_map[char_db_id].m_sg_session_data.m_server_id,
                m_state.m_sg_info_map[char_db_id].m_sg_session_data.m_instance_id);

    //Iterate over set of all people in this supergroup
    for(auto const& val : m_state.m_sg_info_map[char_db_id].m_sg_idxs)
    {
        //We need to notify all the people who added this player (if they're online)
        if(m_state.is_online(val)){
            uint32_t sgmember_id = val;
            refresh_sg_roster(val);
            inst_tgt->putq(new SendNotifySGMembersMessage({m_state.m_sg_info_map[char_db_id].m_sg_session_data.m_session_token,
                                                        m_state.m_sg_info_map[sgmember_id].m_sg_session_data.m_session_token},0));
        }
    }

    update_sg_roster(char_db_id);
}

/*
 * When a client disconnects, update their online status and
 * update lists for the friends that added this player.
 */
void SuperGroupHandler::on_client_disconnected(ClientDisconnectedMessage *msg)
{
    uint32_t char_db_id = msg->m_data.m_char_db_id;
    m_state.m_sg_info_map[char_db_id].m_is_online = false;

    for(auto const& val : m_state.m_sg_info_map[char_db_id].m_sg_idxs)
    {
        refresh_sg_roster(val);
    }
}

/*
 * This function adds a SGMember to a SuperGroup Roster, then sends the updated list.
 */
void SuperGroupHandler::on_sgmember_added(SGMemberAddedMessage *msg)
{
    m_state.m_sg_info_map[msg->m_data.m_added_id].m_sg_idxs.insert(msg->m_data.m_sg_db_id);

    m_state.m_sg_info_map[msg->m_data.m_sg_db_id].m_sg_members.emplace_back(msg->m_data.m_sgstats);
    send_update_sg_roster(msg->m_data.m_sg_db_id);
}

/*
 * This function removes a SGMember from a SuperGroup Roster, then sends the updated list.
 */
void SuperGroupHandler::on_sgmember_removed(SGMemberRemovedMessage *msg)
{
    uint32_t removed_id = msg->m_data.m_removed_id;
    m_state.m_sg_info_map[removed_id].m_sg_idxs.erase(msg->m_data.m_sg_db_id);

    //Look for all friends in your list that have match the given db id, and remove them.
    std::vector<SuperGroupStats> *tmp = &m_state.m_sg_info_map[msg->m_data.m_sg_db_id].m_sg_members;
    tmp->erase(std::remove_if(tmp->begin(), tmp->end(), [removed_id](SuperGroupStats const &sgs)
        {return sgs.m_member_db_id == removed_id;}), tmp->end());

    send_update_sg_roster(msg->m_data.m_sg_db_id);
}

SuperGroupHandler::SuperGroupHandler(int for_game_server_id) : m_message_bus_endpoint(*this)
{
    m_state.m_game_server_id = for_game_server_id;

    assert(HandlerLocator::getSuperGroup_Handler() == nullptr);
    HandlerLocator::setSuperGroup_Handler(this);

    m_message_bus_endpoint.subscribe(evClientDisconnectedMessage);
    m_message_bus_endpoint.subscribe(evClientDisconnectedMessage);

    set_db_handler(for_game_server_id);
    m_db_handler->putq(new GetSuperGroupRequest({}, uint64_t(1)));
}

void SuperGroupHandler::serialize_from(std::istream &/*is*/)
{
    assert(false);
}

void SuperGroupHandler::serialize_to(std::ostream &/*is*/)
{
    assert(false);
}
