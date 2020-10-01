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

#include "TradeWasUpdatedMessage.h"

namespace SEGSEvents
{


TradeWasUpdatedMessage::TradeWasUpdatedMessage()
    : MapLinkEvent(MapEventTypes::evTradeWasUpdatedMessage)
{
}

void TradeWasUpdatedMessage::serializefrom(BitStream& bs)
{
    m_info.m_db_id     = static_cast<uint32_t>(bs.GetPackedBits(1));
    m_info.m_accepted  = bs.GetPackedBits(1);
    m_info.m_influence = static_cast<uint32_t>(bs.GetPackedBits(1));
    const int num_enhs = bs.GetPackedBits(1);
    const int num_insp = bs.GetPackedBits(1);

    for (int i = 0; i < num_enhs; ++i)
    {
        const uint32_t idx = static_cast<uint32_t>(bs.GetPackedBits(1));
        m_info.m_enhancements.push_back(idx);
    }

    for (int i = 0; i < num_insp; ++i)
    {
        const uint32_t col = static_cast<uint32_t>(bs.GetPackedBits(1));
        const uint32_t row = static_cast<uint32_t>(bs.GetPackedBits(1));
        m_info.m_inspirations.push_back(TradeInspiration(col, row));
    }
}

void TradeWasUpdatedMessage::serializeto(BitStream &) const
{
    assert(false);
}


} // namespace SEGSEvents
//! @}
