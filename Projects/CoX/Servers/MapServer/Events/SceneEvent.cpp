#include "Events/SceneEvent.h"
#include "MapEvents.h"

#include <ace/Log_Msg.h>

SceneEvent::SceneEvent():MapLinkEvent(MapEventTypes::evScene)
{
}

void SceneEvent::dependent_dump(void)
{
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IpktSC_SceneResp\n%I{\n")));
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    undos_PP 0x%08x\n"),undos_PP));
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    var_14 0x%08x\n"),var_14));
    if(var_14)
    {
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_outdoor_map 0x%08x\n"),m_outdoor_map));
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_map_number 0x%08x\n"),m_map_number));
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    unkn1 0x%08x\n"),unkn1));
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_map_desc %s\n"),qPrintable(m_map_desc)));
    }
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    flag_dword_151D5D8 0x%08x\n"),current_map_flags));
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    num_base_elems 0x%08x\n"),num_base_elems));
    for(size_t i=0; i<num_base_elems; i++)
    {
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    unkn2 0x%08x\n"),unkn2));
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_trays[i] %s\n"),qPrintable(m_trays[i])));
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_crc[i] 0x%08x\n"),m_crc[i]));
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    unkn2 0x%08x\n"),unkn2));
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    unkn1 0x%08x\n"),unkn1));
    }
    if(num_base_elems!=0)
    {
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    unkn2 0x%08x\n"),unkn2));
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_trays[0] %s\n"),qPrintable(m_trays[0])));
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_crc[0] 0x%08x\n"),m_crc[0]));
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    unkn2 0x%08x\n"),unkn2));
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    unkn1 0x%08x\n"),unkn1));
    }
    for(size_t i=0; i<m_refs.size(); i++)
    {
        m_refs[i].dump();
    }
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
}
void SceneEvent::getGrpElem(BitStream &src,int idx)
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

void SceneEvent::reqWorldUpdateIfPak(BitStream &)
{
    //src.GetBits(1);
    assert(0);
}
void SceneEvent::groupnetrecv_5(BitStream &src,int /*a*/,int /*b*/)
{
    if(!src.GetBits(1))
        return;
    assert(0);
    QString def_filename;
    src.GetString(def_filename);
}

void SceneEvent::serializefrom(BitStream &src)
{
    unkn1=false;
    //bool IAmAnArtist=false;
    undos_PP = src.GetPackedBits(1);
    var_14 = src.GetBits(1);
    if(var_14)
    {
        m_outdoor_map = src.GetBits(1);
        m_map_number = src.GetPackedBits(1);
        unkn1 = src.GetPackedBits(1);
        if(unkn1)
        {
            //IAmAnArtist=isDevelopmentMode())
        }
        src.GetString(m_map_desc);
    }
    current_map_flags = src.GetBits(1); // is beaconized
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
void SceneEvent::serializeto(BitStream &tgt) const
{
    tgt.StorePackedBits(1,6); // opcode
    tgt.StorePackedBits(1,undos_PP);
    tgt.StoreBits(1,var_14);
    if(var_14)
    {
        tgt.StoreString(m_map_desc);
        tgt.StoreBits(1,m_outdoor_map);
        tgt.StorePackedBits(1,m_map_number);
    }
    //tgt.StoreBits(1,current_map_flags);
    //tgt.StorePackedBits(1,m_trays.size());
    uint32_t hashes[] = {0x00000000,0xAFD34459,0xE63A2B76,0xFBBAD9D4,
        0x9AE0A9D4,0x06BDEF70,0xA47A21F8,0x5FBF835D,
        0xFF25F3F6,0x70E6C422,0xF1CCC459,0xCBD35A55,
        0x64CCCC31,0x535B08CC};

    //      ACE_TRACE(!"Hold yer horses!");
    // overriding defs
    tgt.StorePackedBits(1,-1); // finisher
    // overriding groups
    tgt.StorePackedBits(1,-1); // fake it all the way
    return;
    if(0)
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
                NetStructure::storeStringConditional(tgt,"");
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
