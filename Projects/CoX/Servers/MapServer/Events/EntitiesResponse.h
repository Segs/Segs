/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#pragma once
#include <vector>
#include <string>
#include "BitStream.h"
#include "MapRef.h"
#include "CRUD_Events.h"
#include "MapLink.h"

struct MapClientSession;
typedef CRUDLink_Event MapLinkEvent; //<MapLink>

class EntitiesResponse : public MapLinkEvent
{
public:
                    EntitiesResponse(MapClientSession *cl);
        void        is_incremental(bool v) {m_incremental=v;}
        void        serializefrom(BitStream &) override
                    {
                    }

        void        serializeto(BitStream &tgt) const override;
        const char *info() override { return m_incremental ? "Entities_Incremental" : "Entities_Full"; }
        MapClientSession * m_client;
        bool        m_incremental=false; //  if true then this is incremental update
        bool        ent_major_update;
        float       m_map_time_of_day;
        bool        debug_info=true;
        uint32_t    abs_time=0;
        uint32_t    db_time=0;
        uint16_t    m_debug_idx;
        uint8_t     g_interpolation_level;
        uint8_t     g_interpolation_bits;
        uint32_t    m_command_idx[15];
        std::string m_commands[15];
        uint32_t    m_num_commands2;
        std::string m_commands2[15];
};
