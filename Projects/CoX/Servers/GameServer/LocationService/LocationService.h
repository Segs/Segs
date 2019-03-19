/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Common/Servers/InternalEvents.h"
#include "Servers/MapServer/ScriptingEngine.h"

// The LocationServices deals with things you visit (locations, plaques, landmarks?) and destinations set
class LocationService
{
private:
    using GameCommandVector = std::vector<std::unique_ptr<SEGSEvents::GameCommandEvent>>;
    using ScriptVector = std::vector<SEGSEvents::ScriptingServiceToClientData*>;
public:
    std::unique_ptr<SEGSEvents::ServiceToClientData> on_location_visited(Entity* ent, SEGSEvents::Event *ev);
    std::unique_ptr<SEGSEvents::ServiceToClientData> on_plaque_visited(Entity* ent, SEGSEvents::Event *ev);
    std::unique_ptr<SEGSEvents::ServiceToClientData> on_set_destination(Entity* ent, SEGSEvents::Event *ev);

protected:
};


