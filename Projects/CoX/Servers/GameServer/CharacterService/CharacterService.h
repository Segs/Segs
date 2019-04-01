/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Common/Servers/InternalEvents.h"
#include "GameData/Entity.h"

// The CharacterService deals with character-related things that is NOT about Enhancement, Inspiration or Power
// Notice that those three services are located inside CharacterService folder
class CharacterService
{
private:
public:
    void on_switch_tray(Entity* ent, SEGSEvents::Event* ev);
    void on_levelup_response(Entity* ent, SEGSEvents::Event* ev);
    void on_recv_costume_change(Entity* ent, SEGSEvents::Event* ev);
    void on_recv_selected_titles(Entity* ent, SEGSEvents::Event* ev);
    void on_description_and_battlecry(Entity* ent, SEGSEvents::Event* ev);
protected:
};


