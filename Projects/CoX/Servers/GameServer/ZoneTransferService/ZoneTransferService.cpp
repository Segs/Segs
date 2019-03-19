/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "ZoneTransferService.h"
#include "GameData/Entity.h"
#include "GameData/Character.h"
#include "Messages/Map/MapEvents.h"
#include "Messages/Map/MapXferRequest.h"
#include "Messages/Map/MapXferWait.h"
#include "Messages/Map/MapXferList.h"
#include "Messages/GameDatabase/GameDBSyncEvents.h"
#include "MapServer/MapLink.h"
#include "MapServer/MapServer.h"
#include "MapServer/MapClientSession.h"
#include "MapServer/MapInstance.h"
#include "Servers/MapServer/DataHelpers.h"
#include "Common/Servers/HandlerLocator.h"
#include <QtCore/QDebug>

using namespace SEGSEvents;

ServiceToClientData* ZoneTransferService::on_initiate_map_transfer(MapServer* map_server, MapLink* link, Entity* ent, Event *ev)
{
    InitiateMapXfer* casted_ev = static_cast<InitiateMapXfer *>(ev);
    // MapServer *map_server = (MapServer *)HandlerLocator::getMap_Handler(m_game_server_id);
    if(!map_server->session_has_xfer_in_progress(link->session_token()))
    {
         qCDebug(logMapXfers) << QString("Client Session %1 attempting to initiate transfer with no map data message received").arg(link->session_token());
         return nullptr;
    }

    // This is used here to get the map idx to send to the client for the transfer, but we
    // remove it from the std::map after the client has sent us the ClientRenderingResumed event so we
    // can prevent motd showing every time.
    MapXferData &map_xfer = map_server->session_map_xfer_idx(link->session_token());
    GameAccountResponseCharacterData c_data;
    QString serialized_data;

    fromActualCharacter(*ent->m_char, *ent->m_player, *ent->m_entity, c_data);
    serializeToQString(c_data, serialized_data);
    ExpectMapClientRequest *map_req = new ExpectMapClientRequest({ent->m_client->auth_id(), ent->m_client->m_access_level, link->peer_addr(),
                                    serialized_data, ent->m_client->m_requested_slot_idx, ent->m_client->m_name, getMapPath(map_xfer.m_target_map_name),
                                    ent->m_client->m_max_slots},
                                    link->session_token());

    return new ServiceToClientData(ent, {map_req}, QString());
}

void ZoneTransferService::on_map_xfer_complete(Entity* ent, glm::vec3 closest_safe_location, Event* /*ev*/)
{
    // unused event
    // MapXferComplete* casted_ev = static_cast<MapXferComplete *>(ev);

    forcePosition(*ent, closest_safe_location);
    ent->m_map_swap_collided = false;
}

ServiceToClientData* ZoneTransferService::on_map_swap_collision(MapLink* link, Entity* ent, Event *ev)
{
    MapSwapCollisionMessage* casted_ev = static_cast<MapSwapCollisionMessage* >(ev);

    if (!m_owner_instance->get_map_transfers().contains(casted_ev->m_data.m_node_name))
    {
        qCDebug(logMapXfers) << QString("Map swap collision triggered on node_name %1, but that node_name doesn't exist in the list of map_transfers.");
        return nullptr;
    }

    MapXferData map_transfer_data = m_owner_instance->get_map_transfers()[casted_ev->m_data.m_node_name];
    //doesn't seem like we are using it atm
    //Entity *e = getEntityByDBID(ent->m_client->m_current_map, casted_ev->m_data.m_ent_db_id);

    link->putq(new MapXferWait(getMapPath(map_transfer_data.m_target_map_name)));

    ClientMapXferMessage* clientMapXferMessage = new ClientMapXferMessage({link->session_token(), map_transfer_data}, 0);
    return new ServiceToClientData(ent, {clientMapXferMessage}, QString());
}

ServiceToClientData* ZoneTransferService::on_enter_door(MapServer* map_server, MapLink* link, Entity* ent, uint32_t map_index, Event *ev)
{
    EnterDoor* casted_ev = static_cast<EnterDoor* >(ev);

    QString output_msg = "Door entry request to: " + casted_ev->name;

    ClientMapXferMessage* clientMapXferMessage = nullptr;
    if(casted_ev->no_location)
    {
        qCDebug(logMapXfers).noquote() << output_msg << " No location provided";

        // Doors with no location may be a /mapmenu call.
        if(ent->m_is_using_mapmenu)
        {
            // ev->name is the map_idx when using /mapmenu
            if(!map_server->session_has_xfer_in_progress(link->session_token()))
            {
                uint32_t map_idx = casted_ev->name.toInt();
                if (map_idx == map_index)
                {
                    QString door_msg = "You're already here!";
                    sendDoorMessage(*ent->m_client, 2, door_msg);
                }
                else
                {
                    MapXferData map_data = MapXferData();
                    map_data.m_target_map_name = getMapName(map_idx);
                    link->putq(new MapXferWait(getMapPath(map_idx)));

                    clientMapXferMessage = new ClientMapXferMessage({link->session_token(), map_data}, 0);
                }
            }

            ent->m_is_using_mapmenu = false;
        }
        else
        {
            QString door_msg = "Door coordinates unavailable.";
            sendDoorMessage(*ent->m_client, 2, door_msg);
        }
    }
    else
    {
        qCDebug(logMapXfers).noquote() << output_msg << " loc:" << casted_ev->location.x << casted_ev->location.y << casted_ev->location.z;

        // Check if any doors in range have the GotoSpawn property.
        // TODO: if the node also has a GotoMap property, start a map transfer
        //       and put them in the given SpawnLocation in the target map.
        QString gotoSpawn = m_owner_instance->getNearestDoor(casted_ev->location);

        if (gotoSpawn.isEmpty())
        {
            QString door_msg = "You cannot enter.";
            sendDoorMessage(*ent->m_client, 2, door_msg);
        }
        else
        {
            // Attempt to send the player to that SpawnLocation in the current map.
            QString anim_name = "RUNIN";
            glm::vec3 offset = casted_ev->location + glm::vec3 {0,0,2};
            sendDoorAnimStart(*ent->m_client, casted_ev->location, offset, true, anim_name);
            // ent->m_client->m_current_map->setSpawnLocation(*session.m_ent, gotoSpawn);
        }
    }

    // should return a GameCommandVector because sendDoorAnimStart and sendDoorAnimStart do them at this point in time
    if (clientMapXferMessage != nullptr)
        return new ServiceToClientData(ent, {clientMapXferMessage}, QString());
    else
        return nullptr;
}

ServiceToClientData* ZoneTransferService::on_has_entered_door(Entity* ent, Event */*ev*/)
{
    // unused event
    // HasEnteredDoor* casted_ev = static_cast<HasEnteredDoor*>(ev);

    //MapServer *map_server = (MapServer *)HandlerLocator::getMap_Handler(m_game_server_id);

    sendDoorAnimExit(*ent->m_client, false);

    QString output_msg = "Enter door animation has finished.";
    qCDebug(logAnimations).noquote() << output_msg;

    // should return a data with DoorAnimExit GameCommand
    return nullptr;
}

ServiceToClientData* ZoneTransferService::on_awaiting_dead_no_gurney(Entity* ent, Event */*ev*/)
{
    // AwaitingDeadNoGurney* casted_ev = static_cast<AwaitingDeadNoGurney *>(ev);
    qCDebug(logMapEvents) << "Entity: " << ent->m_idx << "has received AwaitingDeadNoGurney";

    // TODO: Check if disablegurney
    /*
    setStateMode(*session.m_ent, ClientStates::AWAITING_GURNEY_XFER);
    sendClientState(session, ClientStates::AWAITING_GURNEY_XFER);
    sendDeadNoGurney(session);
    */
    // otherwise

    ScriptingServiceToClientData* scriptData = new ScriptingServiceToClientData();
    scriptData->flags |= uint32_t(ScriptingServiceFlags::CallFuncWithClientContext);
    scriptData->funcName = "revive_ok";
    scriptData->intArg = ent->m_idx;
    scriptData->on_val_empty = [](Entity& ent)
    {
        // Set statemode to Resurrect
        setStateMode(ent, ClientStates::RESURRECT);
        // TODO: spawn in hospital, resurrect animations, "summoning sickness"
        revivePlayer(ent, ReviveLevel::FULL);
    };

    return nullptr;
}

ServiceToClientData* ZoneTransferService::on_dead_no_gurney_ok(Entity* ent, Event */*ev*/)
{
    //DeadNoGurneyOK* casted_ev = static_cast<DeadNoGurneyOK *>(ev);
    qCDebug(logMapEvents) << "Entity: " << ent->m_idx << "has received DeadNoGurneyOK";

    // Set statemode to Ressurrect
    setStateMode(*ent, ClientStates::RESURRECT);
    revivePlayer(*ent, ReviveLevel::FULL);

    // TODO: Spawn where you go with no gurneys (no hospitals)
    return nullptr;
}
