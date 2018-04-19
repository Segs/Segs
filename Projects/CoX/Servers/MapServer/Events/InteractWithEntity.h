/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2018 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
#include "GameCommandList.h"
#include "Logging.h"

#include "MapEvents.h"
#include "MapLink.h"

class InteractWithEntity final : public MapLinkEvent
{
public:
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
