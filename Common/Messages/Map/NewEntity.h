/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
// if this file is included stand-alone this will pull in common definitions
#include "GameCommand.h"
#include "MapEventTypes.h"
#include "Components/BitStream.h"

namespace SEGSEvents
{
//! a new entity entering a map
// [[ev_def:type]]
class NewEntity : public MapLinkEvent
{
public:
                NewEntity():MapLinkEvent(MapEventTypes::evNewEntity),m_character_data(256)
                {

                }
    void        serializeto(BitStream &) const override;
    void        serializefrom(BitStream &) override;
    // [[ev_def:field]]
    uint32_t    m_cookie;
    // [[ev_def:field]]
    bool        m_new_character;
    // [[ev_def:field]]
    BitStream   m_character_data;
    EVENT_IMPL(NewEntity)
};

} // end of SEGSEvents namespace

