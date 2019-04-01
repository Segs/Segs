/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Common/Servers/InternalEvents.h"
#include "GameData/Entity.h"

// The Enhancement Service deals with enhancement-related things
class EnhancementService
{
private:
public:
    void on_combine_enhancements(Entity* ent, SEGSEvents::Event* ev);
    void on_move_enhancement(Entity* ent, SEGSEvents::Event* ev);
    void on_set_enhancement(Entity* ent, SEGSEvents::Event* ev);
    void on_trash_enhancement(Entity* ent, SEGSEvents::Event* ev);
    void on_trash_enhancement_in_power(Entity* ent, SEGSEvents::Event* ev);
    void on_buy_enhancement_slot(Entity* ent, SEGSEvents::Event* ev);
protected:
};
