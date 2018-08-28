/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
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
