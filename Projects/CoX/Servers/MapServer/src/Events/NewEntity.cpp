/*
 * Super Entity Game Server Project 
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */
#include "Events/NewEntity.h"
#include "Entity.h"
void NewEntity::serializeto( BitStream & ) const
{
    assert(!"TODO");
}

void NewEntity::serializefrom( BitStream &bs )
{
    m_cookie = bs.GetPackedBits(1);
    //m_city_of_developers = src.GetBits(1);
    m_new_character=bs.GetBits(1);
    if(m_new_character)
    {
        m_ent = new PlayerEntity();
        ((PlayerEntity*)m_ent)->serializefrom_newchar(bs);
    }
}
