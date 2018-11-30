/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "GameCommand.h"
#include "MapEventTypes.h"
#include "Common/GameData/Store.h"

namespace SEGSEvents
{
    // [[ev_def:type]]
    class StoreOpen : public GameCommandEvent
    {
    public:
        // [[ev_def:field]
        Store m_store_items;

        explicit StoreOpen() : GameCommandEvent(MapEventTypes::evStoreOpen){}
        StoreOpen(Store store_items) : GameCommandEvent(MapEventTypes::evStoreOpen),
             m_store_items(store_items)
        {
        }

        void serializeto(BitStream &bs) const override
        {
            bs.StorePackedBits(1, type()-evFirstServerToClient); // packet 78
            bs.StorePackedBits(12, m_store_items.m_npc_idx);
            bs.StorePackedBits(2, m_store_items.m_store_items.size());

            uint32_t loop_count_1 = 0;
            for(const QString &store_item: m_store_items.m_store_items)
            {
                bs.StoreString(store_item);
                bs.StorePackedBits(8, m_store_items.m_item_count[loop_count_1]);
                ++loop_count_1;
            }
        }
        EVENT_IMPL(StoreOpen)
    };

}
