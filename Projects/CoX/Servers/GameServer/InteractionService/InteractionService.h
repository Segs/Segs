/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Common/Servers/InternalEvents.h"
#include "Servers/MapServer/ScriptingEngine.h"

class InteractionService
{
private:
    using GameCommandVector = std::vector<std::unique_ptr<SEGSEvents::GameCommandEvent>>;
public:
    SEGSEvents::ServiceToClientData* on_entity_info_request(Entity* ent, SEGSEvents::Event* ev);
    SEGSEvents::ServiceToClientData* on_interact_with(Entity* ent, SEGSEvents::Event* ev);
    SEGSEvents::ServiceToClientData* on_receive_contact_status(Entity* ent, SEGSEvents::Event* ev);
    SEGSEvents::ServiceToClientData* on_receive_task_detail_request(Entity* ent, SEGSEvents::Event* ev);
    SEGSEvents::ServiceToClientData* on_dialog_button(Entity* ent, SEGSEvents::Event* ev);
    SEGSEvents::ServiceToClientData* on_souvenir_detail_request(Entity* ent, SEGSEvents::Event* ev);
protected:
};


