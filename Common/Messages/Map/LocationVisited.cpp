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

#include "LocationVisited.h"

#include "Components/BitStream.h"

using namespace SEGSEvents;

void LocationVisited::serializefrom(BitStream & src)
{
    src.GetString(m_name);
    m_pos.x = src.GetFloat();
    m_pos.y = src.GetFloat();
    m_pos.z = src.GetFloat();
}

void LocationVisited::serializeto(BitStream &) const
{
    assert(!"unimplemented");
}

//! @}
