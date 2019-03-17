/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Common/Servers/InternalEvents.h"
#include "Servers/MapServer/ScriptingEngine.h"

class TransactionService
{
private:
public:
    SEGSEvents::ServiceToClientData* on_store_sell_item(Entity* playerEnt, SEGSEvents::Event *ev);
    SEGSEvents::ServiceToClientData* on_store_buy_item(Entity* playerEnt, SEGSEvents::Event *ev);
    SEGSEvents::ServiceToClientData* on_trade_cancelled(Entity* playerEnt, SEGSEvents::Event *ev);
    SEGSEvents::ServiceToClientData* on_trade_updated(Entity* playerEnt, SEGSEvents::Event *ev);

protected:
};


