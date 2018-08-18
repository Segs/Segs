/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup MapServerEvents Projects/CoX/Servers/MapServer/Events
 * @{
 */

#include "TradeWasUpdatedMessage.h"

TradeWasUpdatedMessage::TradeWasUpdatedMessage()
    : MapLinkEvent(MapEventTypes::evTradeWasUpdatedMessage)
{
}

void TradeWasUpdatedMessage::serializefrom(BitStream& bs)
{
    m_info.m_db_id     = static_cast<uint32_t>(bs.GetPackedBits(1));
    m_info.m_accepted  = bs.GetPackedBits(1);
    m_info.m_influence = static_cast<uint32_t>(bs.GetPackedBits(1));
    /*const int num_enhs = */bs.GetPackedBits(1);
    /*const int num_insp = */bs.GetPackedBits(1);
    // TODO: Inspirations and enhancements.
}

void TradeWasUpdatedMessage::serializeto(BitStream &) const
{
    assert(!"unimplemented");
}

//! @}
