/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "MapEvents.h"
#include "GameData/Trade.h"

namespace SEGSEvents
{


// [[ev_def:type]]
class TradeWasUpdatedMessage : public MapLinkEvent
{
public:
    EVENT_IMPL(TradeWasUpdatedMessage)
    TradeWasUpdatedMessage();

    // SerializableEvent interface
    void serializefrom(BitStream &bs) override;
    void serializeto(BitStream &) const override;

    // [[ev_def:field]]
    TradeInfo m_info;
};


} // namespace SEGSEvents
