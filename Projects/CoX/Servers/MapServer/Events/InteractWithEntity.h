/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "GameCommandList.h"
#include "Logging.h"

#include "MapEvents.h"
#include "MapLink.h"

namespace SEGSEvents
{
// [[ev_def:type]]
class InteractWithEntity final : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    uint32_t m_srv_idx = 0;
    InteractWithEntity() : MapLinkEvent(MapEventTypes::evInteractWithEntity) {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1, type() - MapEventTypes::evConsoleCommand); // 7
        bs.StorePackedBits(12, m_srv_idx);
    }
    void serializefrom(BitStream &src)
    {
        m_srv_idx = src.GetPackedBits(12);
    }
};
} // end of SEGSEvents namespace

