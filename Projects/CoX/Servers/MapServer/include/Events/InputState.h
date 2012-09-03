/*
 * Super Entity Game Server Project 
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

#pragma once
// if this file is included stand-alone this will pull in common definitions
#include "MapEvents.h"
class InputState : public MapLinkEvent
{
    uint8_t m_csc_deltabits;
    bool m_send_deltas;
    u64 m_perf_cntr_diff;
    u64 m_perf_freq_diff;
    u16 controlBits;
public:
    InputState() : MapLinkEvent(MapEventTypes::evInputState)
    {}
    void serializeto(BitStream &) const;
    void partial_2(BitStream &bs);
    void partial(BitStream &bs);
    void serializefrom(BitStream &bs);
    void recv_client_opts(BitStream &bs);
};
