/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Common/Servers/ClientManager.h"
#include "Common/GameData/Entity.h"

class ClientOptionService
{
private:
public:
    void on_select_keybind_profile(Entity* ent, SEGSEvents::Event *ev);
    void on_reset_keybinds(Entity* ent, SEGSEvents::Event *ev);
    void on_set_keybind(Entity* ent, SEGSEvents::Event *ev);
    void on_remove_keybind(Entity* ent, SEGSEvents::Event *ev);
    void on_client_options(Entity* ent, SEGSEvents::Event *ev);
    void on_switch_viewpoint(Entity* ent, SEGSEvents::Event *ev);
    void on_window_state(Entity* ent, SEGSEvents::Event *ev);
protected:
};
