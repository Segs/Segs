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
        VisitMapCells(bool is_opaque, int32_t num_cells, std::vector<bool> visible_map_cells) : 
            GameCommandEvent(evVisitMapCells),
            m_is_opaque(is_opaque),
            m_num_cells(num_cells),
            m_visible_map_cells(visible_map_cells)
        {}

        void serializefrom(BitStream &) override
        {}

        void serializeto(BitStream &bs) const override
        {
            bs.StorePackedBits(1, type() - evFirstServerToClient); // Packet 22
            bs.StorePackedBits(1, 1);
            bs.StoreBits(1, m_is_opaque);
            bs.StorePackedBits(1, m_num_cells);
            std::vector<uint8_t> cells_arr;
            cells_arr.resize((m_num_cells + 7) / 8);
            std::fill(std::begin(cells_arr), std::end(cells_arr), 0);

            for (uint16_t i = 0; i < cells_arr.size(); i++)
            {
                int32_t byte_sum = 0;
                for (uint16_t j = 0; j < 8; j++)
                {
                    if (m_visible_map_cells[i * 8 + j])
                    {
                        byte_sum += std::pow(2, j);
                    }
                }
                cells_arr[i] =  byte_sum;
            }

            bs.StoreBitArray(cells_arr.data(), m_num_cells);
        }

        EVENT_IMPL(VisitMapCells)
    protected:
        // [[ev_def:field]]
        bool                m_is_opaque = false;
        // [[ev_def:field]]
        int32_t             m_num_cells = 1024;
        // [[ev_def:field]]
        std::vector<bool>   m_visible_map_cells;
    };
};
