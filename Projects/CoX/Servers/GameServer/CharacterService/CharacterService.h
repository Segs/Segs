/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Common/Servers/InternalEvents.h"
#include "GameData/Entity.h"

class CharacterService
{
private:
public:
    void on_switch_tray(Entity* ent, SEGSEvents::Event* ev);
    void on_levelup_response(Entity* ent, SEGSEvents::Event* ev);
protected:
};


