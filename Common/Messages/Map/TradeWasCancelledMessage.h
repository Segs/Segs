/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "MapEvents.h"
//#include "MapLink.h"

namespace SEGSEvents
{


// [[ev_def:type]]
class TradeWasCancelledMessage : public MapLinkEvent
{
public:
    EVENT_IMPL(TradeWasCancelledMessage)
    TradeWasCancelledMessage();

    // SerializableEvent interface
    void serializefrom(BitStream &bs) override;
    void serializeto(BitStream &) const override;

    // [[ev_def:field]]
    int m_reason = 0; // 0 - target entity no longer exists, 1 - user selects cancel button.
};


} // namespace SEGSEvents
