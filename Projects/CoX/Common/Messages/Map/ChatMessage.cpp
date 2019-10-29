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

#include "ChatMessage.h"
#include "MapEvents.h"
#include <cmath>

using namespace SEGSEvents;

void ChatMessage::serializeto(BitStream &bs) const
{
    bs.StorePackedBits(1,type()-MapEventTypes::evFirstServerToClient); // packet 20
    bs.StorePackedBits(10,m_source_player_id);
    bs.StorePackedBits(3,uint32_t(m_channel_type));
    bs.StoreString(m_msg);
}

void ChatMessage::serializefrom(BitStream &src)
{
    m_source_player_id = src.GetPackedBits(10);
    m_channel_type = static_cast<MessageChannel>(src.GetPackedBits(3));
    src.GetString(m_msg);
}

//! @}
