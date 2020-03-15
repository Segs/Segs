/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup MapServerEvents Projects/CoX/Servers/MapServer/Events
 * @{
 */

#include "TradeWasCancelledMessage.h"

namespace SEGSEvents
{


TradeWasCancelledMessage::TradeWasCancelledMessage()
    : MapLinkEvent(MapEventTypes::evTradeWasCancelledMessage)
{
}

void TradeWasCancelledMessage::serializefrom(BitStream& bs)
{
    m_reason = bs.GetPackedBits(1);
}

void TradeWasCancelledMessage::serializeto(BitStream &) const
{
    assert(false);
}


} // namespace SEGSEvents
//! @}
