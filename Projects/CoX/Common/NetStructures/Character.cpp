/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */
#include <ace/OS_NS_strings.h>
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
    m_full_options = false;
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
    m_first_person_view_toggle=false;
    m_full_options = false;

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
    /*uint32_t unkn3 =*/ src.GetPackedBits(1);
    //uint32_t unkn4 = src.GetBits(32);
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
void Character::sendWindow(BitStream &bs) const
{
    bs.StorePackedBits(1,0);
    bs.StorePackedBits(1,0);
    bs.StorePackedBits(1,0); // visible ?
    bs.StorePackedBits(1,0);
    bs.StorePackedBits(1,0); // color
    bs.StorePackedBits(1,0); // alpha
    bool a=false;
    bs.StoreBits(1,a);
    if(a)
    {
        bs.StorePackedBits(1,0);
        bs.StorePackedBits(1,0);
    }
    //storeFloatConditional(bs,1.0f);
}

void Character::setName( const std::string &val )
{
    if(val.length()>0)
        m_name = val;
    else
        m_name = "EMPTY";
}
void Character::sendTray(BitStream &bs)
{
    m_trays.serializeto(bs);
}
void Character::sendTrayMode(BitStream &bs) const
{
    bs.StoreBits(1,0);
}

bool Character::serializeFromDB( uint64_t /*user_id*/,uint32_t /*slot_index*/ )
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
void Character::SendCharBuildInfo(BitStream &bs) const
{
    PowerPool_Info null_power;
    null_power.id[0]=null_power.id[1]=null_power.id[2]=0;
    bs.StoreString(m_class_name); // class name
    bs.StoreString(m_origin_name); // origin name
    bs.StorePackedBits(5,0); // ?
    {
        //m_powers[0].serializeto(bs);
        //m_powers[1].serializeto(bs);
        // TODO: this is character powers related, refactor it out of here.
        int count=0;
        bs.StorePackedBits(4,count); // count
        for(int i=0; i<count; i++)
        {
            uint32_t num_powers=0;
            bs.StorePackedBits(5,0);
            bs.StorePackedBits(4,uint32_t(num_powers));
            for(uint32_t idx=0; idx<num_powers; ++idx)
            {
                uint32_t num_somethings=0;

                //sendPower(bs,0,0,0);
                m_powers[idx].serializeto(bs);
                bs.StorePackedBits(5,0);
                bs.StoreFloat(1.0);
                bs.StorePackedBits(4,num_somethings);

                for(size_t idx2=0; idx2<num_somethings; ++idx2)
                {
                    //sendPower(bs,0,0,0);
                    m_powers[idx2].serializeto(bs);
                    bs.StorePackedBits(5,0);
                    bs.StorePackedBits(2,0);
                }
            }
        }
    }
    // main tray powers/stats ?
    {
        uint32_t max_num_cols=3;
        uint32_t max_num_rows=1;
        bs.StorePackedBits(3,max_num_cols); // count
        bs.StorePackedBits(3,max_num_rows); // count
        for(uint32_t i=0; i<max_num_cols; i++)
        {
            for(size_t idx=0; idx<max_num_rows; ++idx)
            {
                bool is_power=false;
                bs.StoreBits(1,is_power);
                if(is_power)
                    null_power.serializeto(bs);
            }
        }
    }
    // boosts
    uint32_t num_boosts=0;
    bs.StorePackedBits(5,num_boosts); // count
    for(size_t idx=0; idx<num_boosts; ++idx)
    {
        bool set_boost=false;
        bs.StorePackedBits(3,0); // bost idx
        bs.uStoreBits(1,set_boost); // 1 set, 0 clear
        if(set_boost)
        {
            null_power.serializeto(bs);
            bs.StorePackedBits(5,0); // bost idx
            bs.StorePackedBits(2,0); // bost idx
        }
    }
}

void Character::serializetoCharsel( BitStream &bs )
{
    bs.StorePackedBits(1,m_level);
    bs.StoreString(m_name);
    bs.StoreString(m_class_name);
    bs.StoreString(m_origin_name);
    if(m_costumes.size()==0)
    {
        assert(m_name.compare("EMPTY")==0); // only empty characters can have no costumes
        CharacterCostume::NullCostume.storeCharsel(bs);
    }
    else
        m_costumes[m_current_costume_set]->storeCharsel(bs);
    bs.StoreString(m_mapName);
    bs.StorePackedBits(1,1);
}

Costume * Character::getCurrentCostume() const
{
    assert(m_costumes.size()>0);
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
            bs.StoreBits(32,uint32_t(m_costumes.size()));
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
    assert(m_costumes.size()==0);
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
                bs.StorePackedBits(7,46); // character end/1.0
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
        // nested into MaxAttribs:MaxAttribs
        bs.StoreBits(1,1); // has more data
            bs.StorePackedBits(1,0); // HitPoints entry
            // field type 0xA, param 2
            // Type15_Params 2 1.0 1.0 7
            if(1) // absolute values
            {
                bs.StorePackedBits(7,47); // character health/1.0
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
                bs.StorePackedBits(7,48); // character end/1.0
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

void Character::sendWindows( BitStream &bs ) const
{
    for(int i=0; i<35; i++)
    {
        bs.StorePackedBits(1,i); // window index
        sendWindow(bs);
    }
}
void Character::sendTeamBuffMode(BitStream &bs) const
{
    bs.StoreBits(1,0);
}
void Character::sendDockMode(BitStream &bs) const
{
    bs.StoreBits(32,0); // unused on the client
    bs.StoreBits(32,0); //
}
void Character::sendChatSettings(BitStream &bs) const
{
    //int i;
    bs.StoreFloat(0.8f); // chat window transparency
    bs.StorePackedBits(1,(1<<19)-1); // bitmask of channels (top window )
    bs.StorePackedBits(1,0); // bitmask of channels (bottom )
    bs.StorePackedBits(1,10); // selected channel, Local=10, 11 broadcast,
/*
    bs.StorePackedBits(1,4);
    bs.StorePackedBits(1,5);
    bs.StorePackedBits(1,6);
    for(i=0; i<5; i++)
    {
        bs.StorePackedBits(1,1);
        bs.StorePackedBits(1,2);
        bs.StorePackedBits(1,3);
        bs.StoreFloat(1.0f);
    }
    for(i=0; i<10; i++)
    {
        bs.StoreString("TestChat1");
        bs.StorePackedBits(1,1);
        bs.StorePackedBits(1,2);
        bs.StorePackedBits(1,3);
        bs.StorePackedBits(1,4);
        bs.StorePackedBits(1,5);
    }
    for(i=0; i<10; i++)
    {
        bs.StoreString("TestChat2");
        bs.StorePackedBits(1,1);
    }
*/
}
void Character::sendDescription(BitStream &bs) const
{
    bs.StoreString("Desc1");
    bs.StoreString("Desc2");

}
void Character::sendTitles(BitStream &bs) const
{
    bs.StoreBits(1,1);
    bs.StoreString("Tz1");
    bs.StoreString("Tz2");
    bs.StoreString("Tz3");
}
void Character::sendKeybinds(BitStream &bs) const
{
    bs.StoreString("Default"); // keybinding profile name
    for(int i=0; i<256; i++)
    {
        if(i==0)
        {
            bs.StoreString("+forward"); //w = +forward
            bs.StoreBits(32,0x11);
            bs.StoreBits(32,0);
        }
        else
        {
            bs.StoreString("");
            bs.StoreBits(32,0);
            bs.StoreBits(32,0);
        }
    }

}
void Character::sendFriendList(BitStream &bs) const
{
    bs.StorePackedBits(1,0);
    bs.StorePackedBits(1,0);
}
void Character::sendOptionsFull(BitStream &bs) const
{
    bs.StoreFloat(m_options.mouselook_scalefactor);//MouseFlt1
    bs.StoreFloat(m_options.degrees_for_turns);//MouseFlt2
    bs.StoreBits(1,m_options.mouse_invert);//MouseSet1
    bs.StoreBits(1,0);//g_DimChatWindow
    bs.StoreBits(1,0); //g_DimNavWindow
    bs.StoreBits(1,1);//g_ToolTips
    bs.StoreBits(1,1);//g_AllowProfanity
    bs.StoreBits(1,1);//g_ChatBalloons
    bs.StoreBits(3,0);//dword_729E58
    bs.StoreBits(3,0);//dword_729E5C
    bs.StoreBits(3,0);//dword_729E60
    bs.StoreBits(3,0);//dword_729E68
    bs.StoreBits(3,0);//dword_729E6C
    bs.StoreBits(3,0);//dword_729E70
    bs.StoreBits(3,0);//dword_729E74
    bs.StoreBits(3,0);//dword_729E78
    bs.StoreBits(3,0);//dword_729E7C
    bs.StorePackedBits(5,2);//v2 =
    //	if ( v1 >= 5 )
    //	{
    //		word_91A7A4 = v2;
    //		word_91A7A0 = v2;
    //	}

}

void Character::sendOptions( BitStream &bs ) const
{
    bs.StoreBits(1,m_full_options);
    if(m_full_options)
    {
        sendOptionsFull(bs);
    }
    else
    {
        bs.StoreBits(1,m_options.mouse_invert);
        bs.StoreFloat(m_options.mouselook_scalefactor);
        bs.StoreFloat(m_options.degrees_for_turns);
    }
    bs.StoreBits(1,m_first_person_view_toggle);
}

#define ADD_OPT(type,var) \
{\
    ClientOption o(#var);\
    o.m_args.push_back(ClientOption::Arg(type,&var));\
    m_opts.push_back(o);\
}

void ClientOptions::init()
{
    ADD_OPT(1,control_debug);
    ADD_OPT(1,no_strafe);
    ADD_OPT(1,alwaysmobile);
    ADD_OPT(1,repredict);
    ADD_OPT(1,neterrorcorrection);
    ADD_OPT(3,speed_scale);
    ADD_OPT(1,svr_lag);
    ADD_OPT(1,svr_lag_vary);
    ADD_OPT(1,svr_pl);
    ADD_OPT(1,svr_oo_packets);
    ADD_OPT(1,client_pos_id);
    ADD_OPT(1,atest0);
    ADD_OPT(1,atest1);
    ADD_OPT(1,atest2);
    ADD_OPT(1,atest3);
    ADD_OPT(1,atest4);
    ADD_OPT(1,atest5);
    ADD_OPT(1,atest6);
    ADD_OPT(1,atest7);
    ADD_OPT(1,atest8);
    ADD_OPT(1,atest9);
    ADD_OPT(1,predict);
    ADD_OPT(1,notimeout);
    ADD_OPT(1,selected_ent_server_index);
}
#undef ADD_OPT
