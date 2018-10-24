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

#include "InfoMessageCmd.h"

//#include "MapClientSession.h"
#include "Logging.h"

using namespace SEGSEvents;
void InfoMessageCmd::serializeto(BitStream &bs) const
{
    bs.StorePackedBits(1,type()-MapEventTypes::evFirstServerToClient);
    bs.StorePackedBits(2,uint8_t(m_channel_type));
    bs.StoreString(m_msg);
}
//! @}
