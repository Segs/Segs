/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
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
        Store m_store;

        explicit StoreOpen() : GameCommandEvent(MapEventTypes::evStoreOpen){}
        StoreOpen(Store store) : GameCommandEvent(MapEventTypes::evStoreOpen),
             m_store(store)
        {
        }

        void serializeto(BitStream &bs) const override
        {
            bs.StorePackedBits(1, type()-evFirstServerToClient); // packet 78
            bs.StorePackedBits(12, m_store.m_npc_idx);
            bs.StorePackedBits(2, m_store.m_store_Items.size());

            for(const StoreItem &store_item: m_store.m_store_Items)
            {
                bs.StoreString(store_item.m_store_name);
                bs.StorePackedBits(8, store_item.m_item_count);
            }
        }
        EVENT_IMPL(StoreOpen)
    };

    // [[ev_def:type]]
    class StoreBuyItem : public MapLinkEvent
    {
    public:
        // [[ev_def:field]
        uint32_t m_npc_idx;
        QString m_item_name;

        explicit StoreBuyItem() : MapLinkEvent(MapEventTypes::evStoreBuyItem){}

        void serializeto(BitStream &/*bs*/) const override
        {
            assert(!"StoreBuyItem serializeto");
        }

        void serializefrom(BitStream &bs) final   // Packet 69
        {
            m_npc_idx = bs.GetPackedBits(12);
            bs.GetString(m_item_name);
            qCDebug(logMapEvents) << "StoreBuyItem Event";
        }
        EVENT_IMPL(StoreBuyItem)
    };

    // [[ev_def:type]]
    class StoreSellItem : public MapLinkEvent
    {
    public:
        // [[ev_def:field]
        uint32_t m_npc_idx;
        bool m_is_enhancement = false;
        uint32_t m_enhancement_idx;
        uint32_t m_tray_number = 0;

        explicit StoreSellItem() : MapLinkEvent(MapEventTypes::evStoreSellItem){}

        void serializeto(BitStream &/*bs*/) const final
        {
            assert(!"StoreSellItem serializeto");
        }

        void serializefrom(BitStream &bs) final // Packet 70
        {
            m_npc_idx = bs.GetPackedBits(12);
            m_is_enhancement = bs.GetBits(1);
            if(!m_is_enhancement)
            {
                m_enhancement_idx = bs.GetPackedBits(3);
                m_tray_number = bs.GetPackedBits(3);
            }
            else
            {
                qCWarning(logMapEvents) << "Trying to sell inspiration!";
                m_enhancement_idx = bs.GetPackedBits(4);
            }
            qCDebug(logMapEvents) << "StoreSellItem Event";
        }
        EVENT_IMPL(StoreSellItem)
    };

} // end of SEGSEvents namespace
