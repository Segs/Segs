/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Common/Servers/InternalEvents.h"
#include "Servers/MapServer/ScriptingEngine.h"

// The TradingService deals with trades, transactions, and such
class TradingService
{
private:
public:
    std::unique_ptr<SEGSEvents::ServiceToClientData> on_store_sell_item(Entity* playerEnt, SEGSEvents::Event *ev);
    std::unique_ptr<SEGSEvents::ServiceToClientData> on_store_buy_item(Entity* playerEnt, SEGSEvents::Event *ev);
    std::unique_ptr<SEGSEvents::ServiceToClientData> on_trade_cancelled(Entity* playerEnt, SEGSEvents::Event *ev);
    std::unique_ptr<SEGSEvents::ServiceToClientData> on_trade_updated(Entity* playerEnt, SEGSEvents::Event *ev);

protected:
};


