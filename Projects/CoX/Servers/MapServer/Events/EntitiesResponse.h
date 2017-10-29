#pragma once
#include <vector>
#include <string>
#include "BitStream.h"
#include "MapRef.h"
#include "CRUD_Events.h"
#include "MapLink.h"

class MapClient;
typedef CRUDLink_Event MapLinkEvent; //<MapLink>

class EntitiesResponse : public MapLinkEvent
{
public:
    EntitiesResponse(MapClient *cl);
        void is_incremental(bool v) {m_incremental=v;}
virtual void serializefrom(BitStream &) override
        {
        }

virtual void serializeto(BitStream &tgt) const override
        {
            assert(finalized);
            int size=m_finalized_into.GetReadableBits();
            while(size>32) {
                tgt.StoreBits(32,m_finalized_into.GetBits(32));
                size-=32;
            }
            tgt.StoreBits(size,m_finalized_into.GetBits(size));
        }
        void serializeto_internal(BitStream &tgt) const;
        void finalize()
        {
            assert(!finalized);
            serializeto_internal(m_finalized_into);
            finalized = true;
        }
        const char *info() override { return m_incremental ? "Entities_Incremental" : "Entities_Full"; }
        MapClient *       m_client;
        bool              m_incremental; //  if true then this is incremental update
        bool              finalized = false;
        mutable BitStream m_finalized_into;
        bool              ent_major_update;
        float             m_map_time_of_day;
        bool              unkn2;
        bool              debug_info;
        bool              m_interpolating;
        uint32_t          abs_time;
        uint32_t          db_time;
        uint16_t          m_debug_idx;
        uint8_t           m_interpolation_level;
        uint8_t           m_interpolation_bits;
        uint32_t          m_command_idx[15];
        std::string       m_commands[15];
        uint32_t          m_num_commands2;
        std::string       m_commands2[15];
};
