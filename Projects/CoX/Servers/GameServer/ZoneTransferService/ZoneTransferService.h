/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Common/Servers/InternalEvents.h"
#include "MapServer/MapLink.h"
#include "MapServer/MapServer.h"

class ZoneTransferService
{
private:
    // follows implementation in WorldSimulation
    MapInstance*            m_owner_instance;
public:
    ZoneTransferService(MapInstance* mi) : m_owner_instance(mi){}
    SEGSEvents::ServiceToClientData* on_initiate_map_transfer(MapServer* map_server, MapLink* link, Entity* ent, SEGSEvents::Event *ev);
    SEGSEvents::ServiceToClientData* on_map_swap_collision(MapLink* link, Entity* ent, SEGSEvents::Event *ev);
    SEGSEvents::ServiceToClientData* on_enter_door(MapServer* map_server, MapLink* link, Entity* ent, uint32_t map_index, SEGSEvents::Event* ev);
    SEGSEvents::ServiceToClientData* on_has_entered_door(Entity* ent, SEGSEvents::Event* ev);
    SEGSEvents::ServiceToClientData* on_awaiting_dead_no_gurney(Entity* ent, SEGSEvents::Event* ev);
    SEGSEvents::ServiceToClientData* on_dead_no_gurney_ok(Entity* ent, SEGSEvents::Event* ev);
    void on_map_xfer_complete(Entity* ent, glm::vec3 closest_safe_location, SEGSEvents::Event *ev);

protected:
};

