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

#include "Messages/Map/NewEntity.h"

using namespace SEGSEvents;

void NewEntity::serializeto( BitStream & ) const
{
    assert(!"TODO");
}

void NewEntity::serializefrom( BitStream &bs )
{
    // the very first time an entity is created (character creation)
    m_cookie = bs.GetPackedBits(1);
    //m_city_of_developers = src.GetBits(1);
    m_new_character=bs.GetBits(1);
    if(m_new_character)
    {
        m_character_data.appendBitStream(bs);
    }
}

//! @}
