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

#include "ChatDividerMoved.h"

#include "Components/BitStream.h"

using namespace SEGSEvents;

void ChatDividerMoved::serializefrom(BitStream &src)
{
    m_position = src.GetFloat();
}

void ChatDividerMoved::serializeto(BitStream &tgt) const
{
    tgt.StorePackedBits(1,36);
    tgt.StoreFloat(m_position);
}

//! @}
