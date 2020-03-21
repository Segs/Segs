/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Messages/Map/GameCommand.h"
#include "Messages/Map/MapEventTypes.h"
#include "Messages/Map/MapRef.h"

#include <QtCore/QString>
#include <vector>
#include <string>
#include <stdint.h>

class BitStream;
namespace SEGSEvents
{

using MapLinkEvent = CRUDLink_Event; //<MapLink>

// [[ev_def:type]]
class Scene : public MapLinkEvent
{
public:
            Scene();

    void    serializefrom(BitStream &src) override;
    void    serializeto(BitStream &tgt) const override;

    // [[ev_def:field]]
    QString m_map_desc;
    // [[ev_def:field]]
    uint32_t ref_count;
    // [[ev_def:field]]
    int ref_crc;
    // [[ev_def:field]]
    int var_4;
    // [[ev_def:field]]
    bool m_outdoor_mission_map;
    // [[ev_def:field]]
    bool current_map_flags;
    // [[ev_def:field]]
    size_t num_base_elems;
    // [[ev_def:field]]
    int undos_PP;
    // [[ev_def:field]]
    bool is_new_world;
    // [[ev_def:field]]
    std::vector<QString> m_trays;
    // [[ev_def:field]]
    std::vector<uint32_t> m_crc;
    // [[ev_def:field]]
    std::vector<MapRef> m_refs;
    // [[ev_def:field]]
    int unkn1;
    // [[ev_def:field]]
    int m_map_number;
    // [[ev_def:field]]
    bool unkn2;
    EVENT_IMPL(Scene)
protected:
    void getGrpElem(BitStream &src,int idx);
    void reqWorldUpdateIfPak(BitStream &src);
    void groupnetrecv_5(BitStream &src,int a,int b);
};
} // end of SEGSEvents namespace

