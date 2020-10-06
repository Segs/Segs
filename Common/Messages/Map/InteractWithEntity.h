/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "GameCommand.h"
#include "MapEventTypes.h"
#include "Components/Logging.h"
#include "Components/BitStream.h"

namespace SEGSEvents
{
// [[ev_def:type]]
class InteractWithEntity final : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    int32_t m_srv_idx = 0;
    InteractWithEntity() : MapLinkEvent(MapEventTypes::evInteractWithEntity) {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1, type() - MapEventTypes::evConsoleCommand); // 7
        bs.StorePackedBits(12, m_srv_idx);
    }
    void serializefrom(BitStream &src) override
    {
        m_srv_idx = src.GetPackedBits(12);
    }
    EVENT_IMPL(InteractWithEntity)
};
} // end of SEGSEvents namespace

