/*
 * Super Entity Game Server Project 
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 
 */

#pragma once
// if this file is included stand-alone this will pull in common definitions
#include "MapEvents.h"
class NewEntity : public MapLinkEvent
{
public:
                NewEntity():MapLinkEvent(MapEventTypes::evEntityEnteringMap),m_character_data(256)
                {

                }
    void        serializeto(BitStream &) const;
    void        serializefrom(BitStream &);
    uint32_t    m_cookie;
    bool        m_new_character;
    BitStream   m_character_data;
};
