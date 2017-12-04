/*
 * Super Entity Game Server Project 
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 
 */
#include "Events/NewEntity.h"
#include "Entity.h"
#include "MapServer.h"
#include "MapServerData.h"
#include "DataHelpers.h"
#include "CommonNetStructures.h"

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
