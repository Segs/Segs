/*
 * Super Entity Game Server Project 
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

#pragma once
// if this file is included stand-alone this will pull in common definitions
#include "MapEvents.h" 
class InputState : public MapLinkEvent
{
public:
    InputState():MapLinkEvent(MapEventTypes::evInputState)
    {

    }
    void serializeto(BitStream &) const;
    void serializefrom(BitStream &);
    Entity *	m_ent;
    u32			m_cookie;
    bool		m_new_character;
};
