/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Common/Servers/InternalEvents.h"
#include "GameData/Entity.h"

#include <QVector>

class PowerService
{
private:
public:
    void on_activate_power(Entity* ent, SEGSEvents::Event* ev);
    void on_activate_power_at_location(Entity* ent, SEGSEvents::Event* ev);
    void on_recv_new_power(Entity* ent, SEGSEvents::Event* ev);
    void on_powers_dockmode(Entity* ent, SEGSEvents::Event* ev);
    void on_abort_queued_power(Entity* ent, SEGSEvents::Event* ev);
    void on_set_default_power(Entity* ent, SEGSEvents::Event* ev);
    void on_unset_default_power(Entity* ent, SEGSEvents::Event* ev);
    void on_change_stance(Entity* ent, SEGSEvents::Event* ev);
    void on_unqueue_all(Entity* ent, SEGSEvents::Event* ev);
protected:
};


