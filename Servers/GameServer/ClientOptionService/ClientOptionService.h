/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Common/Servers/InternalEvents.h"

using namespace SEGSEvents;

// The Client Option Services handles the player settings
class ClientOptionService
{
public:
    UPtrServiceToEntityData on_save_client_options(Event* ev);
    UPtrServiceToEntityData on_set_keybind(Event* ev);
    UPtrServiceToEntityData on_select_keybind_profile(Event* ev);
    UPtrServiceToEntityData on_remove_keybind(Event* ev);
    UPtrServiceToEntityData on_reset_keybinds(Event* ev);
    UPtrServiceToEntityData on_switch_viewpoint(Event* ev);

protected:
};
