/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

/*!
 * @addtogroup MapServerEvents Projects/CoX/Servers/MapServer/Events
 * @{
 */

#include "PlaqueVisited.h"

void PlaqueVisited::serializefrom(BitStream & src)
{
    src.GetString(m_name);
    m_pos.x = src.GetFloat();
    m_pos.y = src.GetFloat();
    m_pos.z = src.GetFloat();
}

void PlaqueVisited::serializeto(BitStream &) const
{
    assert(!"unimplemented");
}

//! @}
