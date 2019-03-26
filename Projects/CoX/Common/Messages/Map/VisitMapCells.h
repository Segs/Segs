/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "GameCommandList.h"
#include "MapEvents.h"

namespace SEGSEvents
{
    // [[ev_def:type]]
    class VisitMapCells : public GameCommandEvent
    {
    public:
        explicit VisitMapCells() : GameCommandEvent(evVisitMapCells) {}
        VisitMapCells(bool is_opaque, std::array<bool, 1024> visible_map_cells) : 
            GameCommandEvent(evVisitMapCells),
            m_is_opaque(is_opaque),
            m_visible_map_cells(visible_map_cells)
        {}

        void serializefrom(BitStream &) override
        {}

        void serializeto(BitStream &bs) const override
        {
            bs.StorePackedBits(1, type() - evFirstServerToClient); // Packet 22
            bs.StorePackedBits(1, 1);
            bs.StoreBits(1, m_is_opaque);
            bs.StorePackedBits(1, 1024); // 1024 possible map cells to reveal
            std::array<uint8_t, 128> cells_arr;
            std::fill(std::begin(cells_arr), std::end(cells_arr), 0);

            for (uint16_t i = 0; i < 128; i++)
            {
                int32_t byte_sum = 0;
                for (uint16_t j = 0; j < 8; j++)
                {
                    if (m_visible_map_cells[i * 8 + j])
                    {
                        byte_sum += std::pow(2,j);
                    }
                }
                cells_arr[i] =  byte_sum;
            }

            bs.StoreBitArray(cells_arr.data(), 1024);
        }

        EVENT_IMPL(VisitMapCells)
    protected:
        // [[ev_def:field]]
        bool                    m_is_opaque = false;
        // [[ev_def:field]]
        std::array<bool,1024>   m_visible_map_cells;
    };
};
