/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "GameCommand.h"
#include "MapEvents.h"

namespace SEGSEvents
{

// [[ev_def:type]]
class VisitMapCells : public GameCommandEvent
{
public:
    explicit VisitMapCells() : GameCommandEvent(evVisitMapCells) {}
    VisitMapCells(bool is_opaque, std::vector<bool> visible_map_cells) :
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
        uint32_t num_cells = m_visible_map_cells.size();
        bs.StorePackedBits(1, num_cells);

        std::vector<uint8_t> cells_arr;
        cells_arr.resize((num_cells + 7) / 8);
        std::fill(std::begin(cells_arr), std::end(cells_arr), 0);
        for (uint16_t i = 0; i < cells_arr.size(); i++)
        {
            int32_t byte_sum = 0;
            for (uint16_t j = 0; j < 8; j++)
            {
                if (m_visible_map_cells[i * 8 + j])
                    byte_sum |= 1<<j;
            }
            cells_arr[i] =  byte_sum;
        }

        bs.StoreBitArray(cells_arr.data(), num_cells);
    }

    EVENT_IMPL(VisitMapCells)
protected:
    // [[ev_def:field]]
    bool                m_is_opaque = false;
    // [[ev_def:field]]
    std::vector<bool>   m_visible_map_cells;
};

} // end of namespace SEGSEvents
