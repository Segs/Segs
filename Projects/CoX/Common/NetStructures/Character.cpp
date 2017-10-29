/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */
#include "Character.h"

#include "BitStream.h"
#include "Costume.h"

#include <ace/Log_Msg.h>
#include <QtCore/QString>

Character::Character()
{
    m_unkn3=0;
    m_multiple_costumes=false;
    m_current_costume_idx=0;
    m_current_costume_set=false;
    m_supergroup_costume=false;
    m_sg_costume=nullptr;
    m_using_sg_costume=false;
    m_current_attribs.m_HitPoints = 25;
    m_max_attribs.m_HitPoints = 50;
    m_current_attribs.m_Endurance = 33;
    m_max_attribs.m_Endurance = 43;
}
void Character::reset()
{
    m_level=0;
    m_name="EMPTY";
    m_class_name="EMPTY";
    m_origin_name="EMPTY";
    m_villain=false;
    m_mapName="";
    m_unkn3=0;
    m_multiple_costumes=false;
    m_current_costume_idx=0;
    m_current_costume_set=false;
    m_supergroup_costume=false;
    m_sg_costume=nullptr;
    m_using_sg_costume=false;
    m_first_person_view_toggle=false;
    m_full_options = false;
}


bool Character::isEmpty()
{
    return ( 0==m_name.compare("EMPTY",Qt::CaseInsensitive)&&
            (0==m_class_name.compare("EMPTY",Qt::CaseInsensitive)));
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
    assert(!m_name.isEmpty());
    tgt.StoreString(m_name);
    if(m_name!="EMPTY")
        assert(!m_class_name.isEmpty() && !m_origin_name.isEmpty());

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
    bool draggable=false;
    bs.StoreBits(1,draggable);
    if(draggable)
    {
        int width=0;
        int height=0;
        bs.StorePackedBits(1,width);
        bs.StorePackedBits(1,height);
    }
    //storeFloatConditional(bs,1.0f);
}

void Character::setName(const QString &val )
{
    if(val.length()>0)
        m_name = val;
    else
        m_name = "EMPTY";
}
void Character::sendTray(BitStream &bs) const
{
    m_trays.serializeto(bs);
}
void Character::sendTrayMode(BitStream &bs) const
{
    bs.StoreBits(1,0);
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
    bs.StorePackedBits(5,0); // related to power level  ?
    PUTDEBUG("SendCharBuildInfo after plevel");

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
    PUTDEBUG("SendCharBuildInfo after powers");
    // main tray inspirations
    {
        uint32_t max_num_cols=3;
        uint32_t max_num_rows=1;
        bs.StorePackedBits(3,max_num_cols); // count
        bs.StorePackedBits(3,max_num_rows); // count
        for(uint32_t col=0; col<max_num_cols; col++)
        {
            for(size_t row=0; row<max_num_rows; ++row)
            {
                bool is_power=false;
                bs.StoreBits(1,is_power);
                if(is_power)
                    null_power.serializeto(bs);
            }
        }
    }
    PUTDEBUG("SendCharBuildInfo after inspirations");
    // boosts
    uint32_t num_boosts=0;
    bs.StorePackedBits(5,num_boosts); // count
    for(size_t idx=0; idx<num_boosts; ++idx)
    {
        bool set_boost=false;
        bs.StorePackedBits(3,0); // bost idx
        bs.StoreBits(1,set_boost); // 1 set, 0 clear
        if(set_boost)
        {
            int level=0;
            int num_combines=0;
            null_power.serializeto(bs);
            bs.StorePackedBits(5,level); // bost idx
            bs.StorePackedBits(2,num_combines); // bost idx
        }
    }
    PUTDEBUG("SendCharBuildInfo after boosts");
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

void Character::serialize_costumes(BitStream &bs, ColorAndPartPacker *packer , bool all_costumes) const
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
            for(Costume * c : m_costumes)
            {
                ::serializeto(*c,bs,packer);
            }
        }
        else
        {
            ::serializeto(*m_costumes[m_current_costume_idx],bs,packer);
        }
        bs.StoreBits(1,m_supergroup_costume);
        if(m_supergroup_costume)
        {
            ::serializeto(*m_sg_costume,bs,packer);

            bs.StoreBits(1,m_using_sg_costume);
        }
    }
    else // other player's costumes we're sending only their current.
    {
        ::serializeto(*getCurrentCostume(),bs,packer);
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
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    class: %s\n"),qPrintable(m_class_name)));
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    origin: %s\n"),qPrintable(m_origin_name)));
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

void Character::recv_initial_costume( BitStream &src, ColorAndPartPacker *packer )
{
    assert(m_costumes.size()==0);
    Costume *res=new Costume;
    m_current_costume_idx=0;
    ::serializefrom(*res,src,packer);
    m_costumes.push_back(res);

}
void serializeStats(const Parse_CharAttrib &src,BitStream &bs, bool sendAbsolute)
{
    int field_idx=0;
    bs.StoreBits(1,1); // we have more data
    bs.StorePackedBits(1,0);
    bs.StorePackedBits(5,src.m_HitPoints/5.0f);
    bs.StoreBits(1,1); // we have more data
    bs.StorePackedBits(1,1);
    bs.StorePackedBits(5,src.m_Endurance/5.0f);
    bs.StoreBits(1,0); // no more data
}
void serializeFullStats(const Parse_CharAttrib &src,BitStream &bs, bool sendAbsolute)
{
    int field_idx=0;
    bs.StoreBits(1,1); // we have more data
    bs.StorePackedBits(1,0);
    bs.StorePackedBits(7,src.m_HitPoints);
    bs.StoreBits(1,1); // we have more data
    bs.StorePackedBits(1,1);
    bs.StorePackedBits(7,src.m_Endurance);
    bs.StoreBits(1,0); // no more data
}
void serializeLevelsStats(const Character &src,BitStream &bs, bool sendAbsolute)
{
    int field_idx=0;
    bs.StoreBits(1,1); // we have more data
    bs.StorePackedBits(1,0);
    bs.StorePackedBits(4,src.m_level);
    bs.StoreBits(1,1); // we have more data
    bs.StorePackedBits(1,1);
    bs.StorePackedBits(4,src.m_combat_level);
    bs.StoreBits(1,0); // no more data
}

void serializeStats(const Character &src,BitStream &bs, bool sendAbsolute)
{
    // Send CurrentAttribs
    // Send MaxAttribs
    // Send levels
    int field_idx=0;
    bs.StoreBits(1,1); // we have more data
    bs.StorePackedBits(1,field_idx++); // CurrentAttribs
    serializeStats(src.m_current_attribs,bs,sendAbsolute);
    bs.StoreBits(1,1); // we have more data
    bs.StorePackedBits(1,field_idx++); // MaxAttribs
    serializeStats(src.m_max_attribs,bs,sendAbsolute);
    bs.StoreBits(1,1); // we have more data
    bs.StorePackedBits(1,field_idx++); // levels
    serializeLevelsStats(src,bs,sendAbsolute);
    bs.StoreBits(1,0); // we have no more data
}
void serializeFullStats(const Character &src,BitStream &bs, bool sendAbsolute)
{
    int field_idx=0;
    bs.StoreBits(1,1); // we have more data
    bs.StorePackedBits(1,field_idx++); // first field is CurrentAttribs
    serializeFullStats(src.m_current_attribs,bs,sendAbsolute);
    bs.StoreBits(1,1); // we have more data
    bs.StorePackedBits(1,field_idx++); // first field is MaxAttribs
    serializeFullStats(src.m_max_attribs,bs,sendAbsolute);
    bs.StoreBits(1,1); // we have more data
    bs.StorePackedBits(1,field_idx++); // levels
    serializeLevelsStats(src,bs,sendAbsolute);
    bs.StoreBits(1,1); // we have more data
    bs.StorePackedBits(1,field_idx++); // ExperiencePoints
    bs.StorePackedBits(16,src.m_experience_points);
    bs.StoreBits(1,1); // we have more data
    bs.StorePackedBits(1,field_idx++); // ExperienceDebt
    bs.StorePackedBits(16,src.m_experience_debt);
    bs.StoreBits(1,1); // we have more data
    bs.StorePackedBits(1,field_idx++); // ExperienceDebt
    bs.StorePackedBits(16,src.m_influence);

    bs.StoreBits(1,0); // we have no more data
}
void Character::sendFullStats(BitStream &bs) const
{
    // if sendAbsolutoOverride
    serializeFullStats(*this,bs,false);
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
    bs.StoreFloat(m_options.mouselook_scalefactor);
    bs.StoreFloat(m_options.degrees_for_turns);
    bs.StoreBits(1,m_options.mouse_invert);
    bs.StoreBits(1,m_options.m_ChatWindow_fading);
    bs.StoreBits(1,m_options.m_NavWindow_fading);
    bs.StoreBits(1,m_options.showTooltips);
    bs.StoreBits(1,m_options.allowProfanity);
    bs.StoreBits(1,m_options.chatBallons);//g_ChatBalloons

    bs.StoreBits(3,m_options.showArchetype);
    bs.StoreBits(3,m_options.showSupergroup);
    bs.StoreBits(3,m_options.showPlayerName);
    bs.StoreBits(3,m_options.showPlayerBars);
    bs.StoreBits(3,m_options.showVillainName);
    bs.StoreBits(3,m_options.showVillainBars);
    bs.StoreBits(3,m_options.showPlayerReticles);
    bs.StoreBits(3,m_options.showVillainReticles);
    bs.StoreBits(3,m_options.showAssistReticles);

    bs.StorePackedBits(5,m_options.chatFontSize); // value only used on client if >=5
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
#define CLIENT_OPT(type,var)\
    ClientOption {#var,{{type,&var}} }

void ClientOptions::init()
{
    m_opts = {
        CLIENT_OPT(ClientOption::t_int,control_debug),
        CLIENT_OPT(ClientOption::t_int,no_strafe),
        CLIENT_OPT(ClientOption::t_int,alwaysmobile),
        CLIENT_OPT(ClientOption::t_int,repredict),
        CLIENT_OPT(ClientOption::t_int,neterrorcorrection),
        CLIENT_OPT(ClientOption::t_float,speed_scale),
        CLIENT_OPT(ClientOption::t_int,svr_lag),
        CLIENT_OPT(ClientOption::t_int,svr_lag_vary),
        CLIENT_OPT(ClientOption::t_int,svr_pl),
        CLIENT_OPT(ClientOption::t_int,svr_oo_packets),
        CLIENT_OPT(ClientOption::t_int,client_pos_id),
        CLIENT_OPT(ClientOption::t_int,atest0),
        CLIENT_OPT(ClientOption::t_int,atest1),
        CLIENT_OPT(ClientOption::t_int,atest2),
        CLIENT_OPT(ClientOption::t_int,atest3),
        CLIENT_OPT(ClientOption::t_int,atest4),
        CLIENT_OPT(ClientOption::t_int,atest5),
        CLIENT_OPT(ClientOption::t_int,atest6),
        CLIENT_OPT(ClientOption::t_int,atest7),
        CLIENT_OPT(ClientOption::t_int,atest8),
        CLIENT_OPT(ClientOption::t_int,atest9),
        CLIENT_OPT(ClientOption::t_int,predict),
        CLIENT_OPT(ClientOption::t_int,notimeout),
        CLIENT_OPT(ClientOption::t_int,selected_ent_server_index),
    };
}
#undef ADD_OPT
