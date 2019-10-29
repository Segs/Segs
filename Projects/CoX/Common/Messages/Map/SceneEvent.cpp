/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup MapServerEvents Projects/CoX/Servers/MapServer/Events
 * @{
 */

#include "Messages/Map/SceneEvent.h"
#include "MapEvents.h"

using namespace SEGSEvents;

Scene::Scene():MapLinkEvent(MapEventTypes::evScene)
{
}

void Scene::getGrpElem(BitStream &src,int idx)
{
    if(src.GetBits(1))
    {
        src.GetString(m_trays[idx]);
        m_crc[idx] = src.GetBits(32);
        if(!src.GetBits(1))
        {
            int tmp2=0;
            int tmp;
            while((tmp=src.GetPackedBits(1)-1)>=0)
            {
                groupnetrecv_5(src,tmp,tmp2);
            }
        }
        else
            reqWorldUpdateIfPak(src);
    }
}

void Scene::reqWorldUpdateIfPak(BitStream &)
{
    //src.GetBits(1);
    assert(0);
}

void Scene::groupnetrecv_5(BitStream &src,int /*a*/,int /*b*/)
{
    if(!src.GetBits(1))
        return;
    assert(0);
    QString def_filename;
    src.GetString(def_filename);
}

void Scene::serializefrom(BitStream &src)
{
    unkn1=false;
    //bool IAmAnArtist=false;
    undos_PP = src.GetPackedBits(1);
    is_new_world = src.GetBits(1);
    if(is_new_world)
    {
        m_outdoor_mission_map = src.GetBits(1);
        m_map_number = src.GetPackedBits(1);
        unkn1 = src.GetPackedBits(1);
        if(unkn1)
        {
            //IAmAnArtist=isDevelopmentMode())
        }
        src.GetString(m_map_desc);
    }
    current_map_flags = src.GetBits(1); // is beaconized
    // FixMe: GetPackedBits() can return signed values which can cause the num_base_elems assignment to be extremely high.
    num_base_elems = src.GetPackedBits(1);
    m_crc.resize(num_base_elems);
    m_trays.resize(num_base_elems);
    for(size_t i=1; i<num_base_elems; i++)
    {
        getGrpElem(src,i);
    }
    if(num_base_elems!=0)
    {
        getGrpElem(src,0);
    }
    MapRef r;
    do
    {
        r.serializefrom(src);
        m_refs.push_back(r);
    } while(r.m_idx>=0);
    src.GetPackedBits(1); //unused
    ref_count = src.GetPackedBits(1);
    assert(ref_count==m_refs.size());
    src.GetBits(32); //unused - crc ?
    ref_crc=src.GetBits(32); // 0x3f6057cf
}

struct NodeDef
{
    int idx;
    bool exist;
    const char *base_dir;
    const char *name;
    int tag_id=0;
    int mod_time=0;
};
static void storeStringWithOptionalPrefix(BitStream &tgt,const char *prefix,const char *b)
{
    tgt.StoreBits(1,prefix!=nullptr);
    if(prefix)
        tgt.StoreString(prefix);
    tgt.StoreString(b);
}
void Scene::serializeto(BitStream &tgt) const
{
    tgt.StorePackedBits(1,6); // opcode
    tgt.StorePackedBits(1,undos_PP);
    tgt.StoreBits(1,is_new_world);
    if(is_new_world)
    {
        tgt.StoreString(m_map_desc);
        tgt.StoreBits(1,m_outdoor_mission_map);
        tgt.StorePackedBits(1,m_map_number);
    }
    //tgt.StoreBits(1,current_map_flags);
    //tgt.StorePackedBits(1,m_trays.size());
    uint32_t hashes[] = {0x00000000,0xAFD34459,0xE63A2B76,0xFBBAD9D4,
        0x9AE0A9D4,0x06BDEF70,0xA47A21F8,0x5FBF835D,
        0xFF25F3F6,0x70E6C422,0xF1CCC459,0xCBD35A55,
        0x64CCCC31,0x535B08CC};

    std::vector<NodeDef> def_overrides;
    int prev_def_idx=-1;
    for(NodeDef &d : def_overrides)
    {
        int delta = (prev_def_idx == -1) ? d.idx : (d.idx - prev_def_idx - 1);
        prev_def_idx  = d.idx;
        tgt.StorePackedBits(1,delta); // finisher
        tgt.StoreBits(1,d.exist);
        if(!d.exist)
            continue;
        storeStringWithOptionalPrefix(tgt,d.base_dir,d.name);
        // TODO: finish this :)
    }
    // overriding defs
    tgt.StorePackedBits(1,-1); // finisher
    // overriding groups
    tgt.StorePackedBits(1,-1); // fake it all the way
    return;
    if(0) // FixMe: The interior blocks are unreachable due to the literal 0 here.
    {
        for(size_t i=1; i<m_refs.size(); i++)
        {
            //          ACE_TRACE(!"Hold yer horses!");
            tgt.StorePackedBits(1,0); // next element idx
            tgt.StoreBits(1,0);
            if(0)
            {
                // old coh use it that way, first string of the 2 is optional (path), second is mandatory
                //NetStructure::storeStringConditional(tgt,m_trays[i]);
                //tgt.StoreString("");
                storeStringConditional(tgt,"");
                tgt.StoreString(m_trays[i]);
                if(i<12)
                    tgt.StoreBits(32,hashes[i]); // crc ?
                else
                    tgt.StoreBits(32,0); // crc ?
                //tgt.StoreBits(1,0);
                tgt.StorePackedBits(1,0);
                tgt.StorePackedBits(1,0);
            }
        }
        tgt.StorePackedBits(1,-1); // finisher
    }
    if(0)
    {
        for(size_t i=0; i<m_refs.size(); i++)
        {
            tgt.StorePackedBits(1,0); // next element idx
            m_refs[i].serializeto(tgt);
        }
    }
    tgt.StorePackedBits(1,~0); // finishing marker,-1
    tgt.StorePackedBits(1,0xD8); //unused
    tgt.StorePackedBits(1,ref_count);
    tgt.StoreBits(32,0); //unused - crc ?
    tgt.StoreBits(32,ref_crc); // 0x3f6057cf
}

//! @}
