/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "MapEventTypes.h"
#include "Components/BitStream.h"
#include <vector>
#include <string>

struct MapClientSession;

namespace SEGSEvents
{

using MapLinkEvent = CRUDLink_Event; //<MapLink>

// [[ev_def:type]]
class EntitiesResponse : public MapLinkEvent
{
public:
explicit            EntitiesResponse();
        void        is_incremental(bool v) {m_incremental=v;}
        void        serializefrom(BitStream &) override;
        void        serializeto(BitStream &tgt) const override;
        const char *info() override { return m_incremental ? "Entities_Incremental" : "Entities_Full"; }
        /// Note: all bitstream members are marked as mutable, since they are appended, and append needs to update the bitstream.
        /// The `mutable` keyword has to be put before the event_gen comment :(
        mutable 
        // [[ev_def:field]]
        BitStream blob_of_death {220};
        // [[ev_def:field]]
        bool        m_incremental=false; //  if true then this is incremental update
        // [[ev_def:field]]
        bool        ent_major_update;
        // [[ev_def:field]]
        float       m_map_time_of_day;
        // [[ev_def:field]]
        bool        debug_info=true;
        // [[ev_def:field]]
        uint32_t    abs_time=0;
        // [[ev_def:field]]
        uint32_t    db_time=0;
        // [[ev_def:field]]
        uint16_t    m_debug_idx;
        // [[ev_def:field]]
        uint8_t     g_interpolation_level;
        // [[ev_def:field]]
        uint8_t     g_interpolation_bits;
        EVENT_IMPL(EntitiesResponse)
};

} // end of SEGSEvents namespace

