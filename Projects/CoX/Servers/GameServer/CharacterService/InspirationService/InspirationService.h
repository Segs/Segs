/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Common/Servers/InternalEvents.h"
#include "GameData/Entity.h"

class InspirationService
{
private:
    const float ACTIVATE_INSPIRATION_DELAY = 4.0;
public:
    void on_move_inspiration(Entity* ent, SEGSEvents::Event* ev);
    void on_inspiration_dockmode(Entity* ent, SEGSEvents::Event* ev);
    SEGSEvents::MapServiceToClientData* on_activate_inspiration(Entity* ent, SEGSEvents::Event* ev);
protected:
};


