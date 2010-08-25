/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */
#include <ace/OS.h>
#include "BitStream.h"
#include "Character.h"
#include "Costume.h"

Character::Character()
{
    m_unkn3=0;
    m_multiple_costumes=false;
    m_current_costume_idx=0;
    m_current_costume_set=false;
    m_supergroup_costume=false;
    m_sg_costume=0;
    m_using_sg_costume=false;
}
void Character::reset()
{
    m_level=0;
    m_name="EMPTY";
    m_class_name="EMPTY";
    m_origin_name="EMPTY";
    m_villain=0;
    m_mapName="";
    m_unkn3=0;
    m_multiple_costumes=false;
    m_current_costume_idx=0;
    m_current_costume_set=false;
    m_supergroup_costume=false;
    m_sg_costume=0;
    m_using_sg_costume=false;

}

bool Character::isEmpty()
{
    return (0==ACE_OS::strcasecmp(m_name.c_str(),"EMPTY")&&(0==ACE_OS::strcasecmp(m_class_name.c_str(),"EMPTY")));
}

void Character::serializefrom( BitStream &src)
{
    m_level = src.GetPackedBits(1);
    src.GetString(m_name);
    src.GetString(m_class_name);
    src.GetString(m_origin_name);
    m_unkn1 =src.GetFloat();
    m_unkn2 =src.GetFloat();
    src.GetString(m_mapName);
    u32 unkn3 = src.GetPackedBits(1);
    //u32 unkn4 = src.GetBits(32);
}
void Character::serializeto( BitStream &tgt) const
{
    tgt.StorePackedBits(1,m_level);
    tgt.StoreString(m_name);
    tgt.StoreString(m_class_name);
    tgt.StoreString(m_origin_name);

    //tgt.StorePackedBits(1,m_villain);
    tgt.StoreString(m_mapName);
    tgt.StorePackedBits(1,m_unkn3);
    //tgt.StorePackedBits(32,m_unkn4); // if != 0 UpdateCharacter is called
}

void Character::setName( const std::string &val )
{
    if(val.length()>0)
        m_name = val;
    else
        m_name = "EMPTY";
}

bool Character::serializeFromDB( u64 user_id,u32 slot_index )
{
    return false;
}

void Character::GetCharBuildInfo(BitStream &src)
{
    m_level=0;
    src.GetString(m_class_name);
    src.GetString(m_origin_name);
    PowerPool_Info primary,secondary;
    primary.serializefrom(src);
    secondary.serializefrom(src);

    m_powers.push_back(primary); // primary_powerset power
    m_powers.push_back(secondary); // secondary_powerset power
    m_trays.serializefrom(src);
}

void Character::serializetoCharsel( BitStream &bs )
{
    bs.StorePackedBits(1,m_level);
    bs.StoreString(m_name);
    bs.StoreString(m_class_name);
    bs.StoreString(m_origin_name);
    if(m_costumes.size()==0)
    {
        ACE_ASSERT(m_name.compare("EMPTY")==0); // only empty characters can have no costumes
        CharacterCostume::NullCostume.storeCharsel(bs);
    }
    else
        m_costumes[m_current_costume_set]->storeCharsel(bs);
    bs.StoreString(m_mapName);
    bs.StorePackedBits(1,1);
}

Costume * Character::getCurrentCostume() const
{
    ACE_ASSERT(m_costumes.size()>0);
    if(m_current_costume_set)
        return m_costumes[m_current_costume_idx];
    else
        return m_costumes[0];
}

void Character::serialize_costumes( BitStream &bs,bool all_costumes ) const
{
    // full costume
    if(all_costumes) // this is only sent to the current player
    {
        bs.StoreBits(1,m_current_costume_set);
        if(m_current_costume_set)
        {
            bs.StoreBits(32,m_current_costume_idx);
            bs.StoreBits(32,m_costumes.size());
        }
        bs.StoreBits(1,m_multiple_costumes);
        if(m_multiple_costumes)
        {
            for(size_t idx=0; idx<=m_costumes.size(); idx++)
            {
                m_costumes[idx]->serializeto(bs);
            }
        }
        else
        {
            m_costumes[m_current_costume_idx]->serializeto(bs);
        }
        bs.StoreBits(1,m_supergroup_costume);
        if(m_supergroup_costume)
        {
            m_sg_costume->serializeto(bs);
            bs.StoreBits(1,m_using_sg_costume);
        }
    }
    else // other player's costumes we're sending only their current.
    {
        getCurrentCostume()->serializeto(bs);
    }
}
void Character::DumpPowerPoolInfo( const PowerPool_Info &pool_info )
{
    for(int i=0; i<3; i++)
    {
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    Pool_id[%d]: 0x%08x\n"),i,pool_info.id[i]));
    }
}
void Character::DumpBuildInfo()
{
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    class: %s\n"),m_class_name.c_str()));
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    origin: %s\n"),m_origin_name.c_str()));
    DumpPowerPoolInfo(m_powers[0]);
    DumpPowerPoolInfo(m_powers[1]);
}

void Character::dump()
{
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    //---------------Tray------------------\n")));
    m_trays.dump();
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    //---------------Costume---------------\n")));
//    if(getCurrentCostume())
//        getCurrentCostume()->dump();

}

void Character::recv_initial_costume( BitStream &src )
{
    ACE_ASSERT(m_costumes.size()==0);
    MapCostume *res=new MapCostume;
    m_current_costume_idx=0;
    res->serializefrom(src);
    m_costumes.push_back(res);

}
void Character::sendFullStats(BitStream &bs) const
{
    // if sendAbsolutoOverride

    // this uses the character schema from the xml -> FullStats and children

    // CurrentAttributes
    bs.StoreBits(1,1);
    bs.StorePackedBits(1,0); // CurrentAttribs entry idx
    {
            // nested into CurrentAttribs:LiveAttribs
            bs.StoreBits(1,1); // has more data
            bs.StorePackedBits(1,0); // HitPoints entry
            // field type 0xA, param 2
            // Type15_Params 2 1.0 1.0 7
            if(1) // absolute values
            {
                bs.StorePackedBits(7,45); // character health/1.0
            }
            else
            {
                // StoreOptionalSigned(
                    // Bits(1) ?( Bits(1) ? -packedBits(1) : PackedBits(1) ) : 0
            }
            bs.StoreBits(1,1); // has more data
            bs.StorePackedBits(1,1); // EndurancePoints entry
            // field type 0xA, param 2
            if(1) // absolute values
            {
                bs.StorePackedBits(7,45); // character end/1.0
            }
            else
            {
                // StoreOptionalSigned(
                // Bits(1) ?( Bits(1) ? -packedBits(1) : PackedBits(1) ) : 0
            }
            bs.StoreBits(1,0); // nest out
    }
    bs.StoreBits(1,1); // has more data
    bs.StorePackedBits(1,1); // MaxAttribs entry idx
    {
        // nested into MaxAttribs:LiveAttribs
        bs.StoreBits(1,1); // has more data
            bs.StorePackedBits(1,0); // HitPoints entry
            // field type 0xA, param 2
            // Type15_Params 2 1.0 1.0 7
            if(1) // absolute values
            {
                bs.StorePackedBits(7,45); // character health/1.0
            }
            else
            {
                // StoreOptionalSigned(
                // Bits(1) ?( Bits(1) ? -packedBits(1) : PackedBits(1) ) : 0
            }
        bs.StoreBits(1,1); // has more data
            bs.StorePackedBits(1,1); // EndurancePoints entry
            // field type 0xA, param 2
            if(1) // absolute values
            {
                bs.StorePackedBits(7,45); // character end/1.0
            }
            else
            {
                // StoreOptionalSigned(
                // Bits(1) ?( Bits(1) ? -packedBits(1) : PackedBits(1) ) : 0
            }
        bs.StoreBits(1,0); // nest out
    }
    bs.StoreBits(1,1); // has more data
    bs.StorePackedBits(1,2); // SendLevels entry idx
    {
        // nested into SendLevels:LiveLevels
        bs.StoreBits(1,1); // has more data
            bs.StorePackedBits(1,0); // Level entry
            // field type 0x5, param 4
            if(1) // absolute values
            {
                bs.StorePackedBits(4,1); //
            }
            else
            {
                // StoreOptionalSigned(
                // Bits(1) ?( Bits(1) ? -packedBits(1) : PackedBits(1) ) : 0
                // send prev_lev-new_lev
            }
        bs.StoreBits(1,1); // has more data
            bs.StorePackedBits(1,1); // CombatLevel entry
            if(1) // absolute values
            {
                bs.StorePackedBits(4,1);
            }
            else
            {
                // StoreOptionalSigned(
                // Bits(1) ?( Bits(1) ? -packedBits(1) : PackedBits(1) ) : 0
            }
        bs.StoreBits(1,0); // nest out
    }
    bs.StoreBits(1,1); // has more data
    bs.StorePackedBits(1,3); // Experience
    {
        // field type 0x5, param 16
        if(1) // absolute values
        {
            bs.StorePackedBits(16,1); //
        }
        else
        {
            // StoreOptionalSigned(
            // Bits(1) ?( Bits(1) ? -packedBits(1) : PackedBits(1) ) : 0
            // send prev_lev-new_lev
        }
    }
    bs.StoreBits(1,1); // has more data
    bs.StorePackedBits(1,4); // ExperienceDebt
    {
        // field type 0x5, param 16
        if(1) // absolute values
        {
            bs.StorePackedBits(16,1); //
        }
        else
        {
            // StoreOptionalSigned(
            // Bits(1) ?( Bits(1) ? -packedBits(1) : PackedBits(1) ) : 0
            // send prev_lev-new_lev
        }
    }
    bs.StoreBits(1,1); // has more data
    bs.StorePackedBits(1,5); // Influence
    {
        // field type 0x5, param 16
        if(1) // absolute values
        {
            bs.StorePackedBits(16,0); //
        }
        else
        {
            // StoreOptionalSigned(
            // Bits(1) ?( Bits(1) ? -packedBits(1) : PackedBits(1) ) : 0
            // send prev_lev-new_lev
        }
    }
    bs.StoreBits(1,0); // has more data, nest out from the root
}
