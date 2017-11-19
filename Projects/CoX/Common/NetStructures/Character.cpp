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

#include <QtCore/QString>
#include <QtCore/QDebug>

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
    CharacterPower primary,secondary;
    primary.power_id.serializefrom(src);
    secondary.power_id.serializefrom(src);

    m_powers.push_back(primary); // primary_powerset power
    m_powers.push_back(secondary); // secondary_powerset power
    m_trays.serializefrom(src);
}
void Character::SendCharBuildInfo(BitStream &bs) const
{
    PowerPool_Info null_power = {0,0,0};
    bs.StoreString(m_class_name); // class name
    bs.StoreString(m_origin_name); // origin name
    bs.StorePackedBits(5,0); // related to power level  ?
    PUTDEBUG("SendCharBuildInfo after plevel");

    {
        // TODO: this is character powers related, refactor it out of here.
        int count=0;
        bs.StorePackedBits(4,count); // count
        for(int i=0; i<count; i++)
        {
            uint32_t num_powers=m_powers.size();
            bs.StorePackedBits(5,0);
            bs.StorePackedBits(4,uint32_t(num_powers));
            for(const CharacterPower &power : m_powers)
            {
                //sendPower(bs,0,0,0);
                power.power_id.serializeto(bs);
                bs.StorePackedBits(5,power.bought_at_level);
                bs.StoreFloat(power.range);
                bs.StorePackedBits(4,power.boosts.size());

                for(const CharacterPowerBoost &boost : power.boosts)
                {
                    //sendPower(bs,0,0,0);
                    boost.boost_id.serializeto(bs);
                    bs.StorePackedBits(5,boost.level);
                    bs.StorePackedBits(2,boost.num_combines);
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
    for (int i = 0; i < 3; i++)
    {
        qDebug().nospace().noquote() << "    "
                                     << QString("Pool_id[%1]: %2 %3 %4")
                                            .arg(i)
                                            .arg(pool_info.category_idx)
                                            .arg(pool_info.powerset_entry_idx)
                                            .arg(pool_info.power_idx);
    }
}
void Character::DumpBuildInfo()
{
    qDebug() << "    class: "<<m_class_name;
    qDebug() << "    origin: "<<m_origin_name;
    DumpPowerPoolInfo(m_powers[0].power_id);
    DumpPowerPoolInfo(m_powers[1].power_id);
}

void Character::dump()
{
    qDebug() <<"    //---------------Tray------------------";
    m_trays.dump();
    qDebug() <<"    //---------------Costume------------------";
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

    enum KeyName : int32_t {
        COH_INPUT_INVALID           = 0,
        COH_INPUT_ESCAPE            = 1,
        COH_INPUT_1                 = 0x2,
        COH_INPUT_2                 = 0x3,
        COH_INPUT_3                 = 0x4,
        COH_INPUT_4                 = 0x5,
        COH_INPUT_5                 = 0x6,
        COH_INPUT_6                 = 0x7,
        COH_INPUT_7                 = 0x8,
        COH_INPUT_8                 = 0x9,
        COH_INPUT_9                 = 0x0A,
        COH_INPUT_0                 = 0x0B,
        COH_INPUT_MINUS             = 0x0C,
        COH_INPUT_EQUALS            = 0x0D,
        COH_INPUT_TAB               = 0x0F,
        COH_INPUT_Q                 = 0x10,
        COH_INPUT_W                 = 0x11,
        COH_INPUT_E                 = 0x12,
        COH_INPUT_R                 = 0x13,
        COH_INPUT_T                 = 0x14,
        COH_INPUT_Y                 = 0x15,
        COH_INPUT_U                 = 0x16,
        COH_INPUT_I                 = 0x17,
        COH_INPUT_O                 = 0x18,
        COH_INPUT_P                 = 0x19,
        COH_INPUT_LBRACKET          = 0x1A,
        COH_INPUT_RBRACKET          = 0x1B,
        COH_INPUT_RETURN            = 0x1C,
        COH_INPUT_LCONTROL          = 0x1D,
        COH_INPUT_A                 = 0x1E,
        COH_INPUT_S                 = 0x1F,
        COH_INPUT_D                 = 0x20,
        COH_INPUT_F                 = 0x21,
        COH_INPUT_G                 = 0x22,
        COH_INPUT_H                 = 0x23,
        COH_INPUT_J                 = 0x24,
        COH_INPUT_K                 = 0x25,
        COH_INPUT_L                 = 0x26,
        COH_INPUT_SEMICOLON         = 0x27,
        COH_INPUT_APOSTROPHE        = 0x28,
        COH_INPUT_TILDE             = 0x29,
        COH_INPUT_LSHIFT            = 0x2A,
        COH_INPUT_BACKSLASH         = 0x2B,
        COH_INPUT_Z                 = 0x2C,
        COH_INPUT_X                 = 0x2D,
        COH_INPUT_C                 = 0x2E,
        COH_INPUT_V                 = 0x2F,
        COH_INPUT_B                 = 0x30,
        COH_INPUT_N                 = 0x31,
        COH_INPUT_M                 = 0x32,
        COH_INPUT_COMMA             = 0x33,
        COH_INPUT_PERIOD            = 0x34,
        COH_INPUT_SLASH             = 0x35,
        COH_INPUT_RSHIFT            = 0x36,
        COH_INPUT_MULTIPLY          = 0x37,
        COH_INPUT_SPACE             = 0x39,
        COH_INPUT_F1                = 0x3B,
        COH_INPUT_F2                = 0x3C,
        COH_INPUT_F3                = 0x3D,
        COH_INPUT_F4                = 0x3E,
        COH_INPUT_F5                = 0x3F,
        COH_INPUT_F6                = 0x40,
        COH_INPUT_F7                = 0x41,
        COH_INPUT_F8                = 0x42,
        COH_INPUT_F9                = 0x43,
        COH_INPUT_F10               = 0x44,
        COH_INPUT_NUMLOCK           = 0x45,
        COH_INPUT_SCROLL            = 0x46,
        COH_INPUT_NUMPAD7           = 0x47,
        COH_INPUT_NUMPAD8           = 0x48,
        COH_INPUT_NUMPAD9           = 0x49,
        COH_INPUT_NUMPAD4           = 0x4B,
        COH_INPUT_NUMPAD5           = 0x4C,
        COH_INPUT_NUMPAD6           = 0x4D,
        COH_INPUT_NUMPAD1           = 0x4F,
        COH_INPUT_NUMPAD2           = 0x50,
        COH_INPUT_NUMPAD3           = 0x51,
        COH_INPUT_NUMPAD0           = 0x52,
        COH_INPUT_F11               = 0x57,
        COH_INPUT_F12               = 0x58,
        COH_INPUT_F13               = 0x64,
        COH_INPUT_F14               = 0x65,
        COH_INPUT_F15               = 0x66,
        COH_INPUT_NUMPADEQUALS      = 0x8D,
        COH_INPUT_ABNT_C1           = 0x73,
        COH_INPUT_ABNT_C2           = 0x7E,
        COH_INPUT_COLON             = 0x92,
        COH_INPUT_UNDERLINE         = 0x93,
        COH_INPUT_NEXTTRACK         = 0x99,
        COH_INPUT_NUMPADENTER       = 0x9C,
        COH_INPUT_RCONTROL          = 0x9D,
        COH_INPUT_MUTE              = 0x0A0,
        COH_INPUT_CALCULATOR        = 0x0A1,
        COH_INPUT_PLAYPAUSE         = 0x0A2,
        COH_INPUT_MEDIASTOP         = 0x0A4,
        COH_INPUT_VOLUMEDOWN        = 0x0AE,
        COH_INPUT_VOLUMEUP          = 0x0B0,
        COH_INPUT_WEBHOME           = 0x0B2,
        COH_INPUT_NUMPADCOMMA       = 0x0B3,
        COH_INPUT_SYSRQ             = 0x0B7,
        COH_INPUT_PAUSE             = 0x0C5,
        COH_INPUT_HOME              = 0x0C7,
        COH_INPUT_END               = 0x0CF,
        COH_INPUT_INSERT            = 0x0D2,
        COH_INPUT_DELETE            = 0x0D3,
        COH_INPUT_WEBSEARCH         = 0x0E5,
        COH_INPUT_WEBFAVORITES      = 0x0E6,
        COH_INPUT_WEBREFRESH        = 0x0E7,
        COH_INPUT_WEBSTOP           = 0x0E8,
        COH_INPUT_WEBFORWARD        = 0x0E9,
        COH_INPUT_WEBBACK           = 0x0EA,
        COH_INPUT_MYCOMPUTER        = 0x0EB,
        COH_INPUT_MAIL              = 0x0EC,
        COH_INPUT_MEDIASELECT       = 0x0ED,
        COH_INPUT_BACKSPACE         = 0x0E,
        COH_INPUT_NUMPADSTAR        = 0x37,
        COH_INPUT_LALT              = 0x38,
        COH_INPUT_CAPSLOCK          = 0x3A,
        COH_INPUT_NUMPADMINUS       = 0x4A,
        COH_INPUT_NUMPADPLUS        = 0x4E,
        COH_INPUT_NUMPADPERIOD      = 0x53,
        COH_INPUT_NUMPADSLASH       = 0x0B5,
        COH_INPUT_RALT              = 0x0B8,
        COH_INPUT_UPARROW           = 0x0C8,
        COH_INPUT_PGUP              = 0x0C9,
        COH_INPUT_LEFTARROW         = 0x0CB,
        COH_INPUT_RIGHTARROW        = 0x0CD,
        COH_INPUT_DOWNARROW         = 0x0D0,
        COH_INPUT_PGDN              = 0x0D1,
        COH_INPUT_LBUTTON           = 0xEE,
        COH_INPUT_MBUTTON           = 0x0EF,
        COH_INPUT_RBUTTON           = 0x0F0,
        COH_INPUT_BUTTON4           = 0x0F1,
        COH_INPUT_BUTTON5           = 0x0F2,
        COH_INPUT_BUTTON6           = 0x0F3,
        COH_INPUT_BUTTON7           = 0x0F4,
        COH_INPUT_BUTTON8           = 0x0F5,
        COH_MOUSE_SCROLL            = 0xF6,
        COH_INPUT_JOY1              = 0x5A,
        COH_INPUT_JOY2              = 0x5B,
        COH_INPUT_JOY3              = 0x5C,
        COH_INPUT_JOY4              = 0x5D,
        COH_INPUT_JOY5              = 0x5E,
        COH_INPUT_JOY6              = 0x5F,
        COH_INPUT_JOY7              = 0x60,
        COH_INPUT_JOY8              = 0x61,
        COH_INPUT_JOY9              = 0x62,
        COH_INPUT_JOY10             = 0x63,
        COH_INPUT_JOY11             = 0x7F,
        COH_INPUT_JOY12             = 0x80,
        COH_INPUT_JOY13             = 0x81,
        COH_INPUT_JOY14             = 0x82,
        COH_INPUT_JOY15             = 0x83,
        COH_INPUT_JOY16             = 0x84,
        COH_INPUT_JOY17             = 0x85,
        COH_INPUT_JOY18             = 0x86,
        COH_INPUT_JOY19             = 0x87,
        COH_INPUT_JOY20             = 0x88,
        COH_INPUT_JOY21             = 0x89,
        COH_INPUT_JOY22             = 0x8A,
        COH_INPUT_JOY23             = 0x8B,
        COH_INPUT_JOY24             = 0x8C,
        COH_INPUT_JOY25             = 0x59,
        COH_INPUT_JOYPAD_UP         = 0x0B9,
        COH_INPUT_JOYPAD_DOWN       = 0x0BA,
        COH_INPUT_JOYPAD_LEFT       = 0x0BB,
        COH_INPUT_JOYPAD_RIGHT      = 0x0BC,
        COH_INPUT_POV1_UP           = 0x67,
        COH_INPUT_POV1_DOWN         = 0x68,
        COH_INPUT_POV1_LEFT         = 0x69,
        COH_INPUT_POV1_RIGHT        = 0x71,
        COH_INPUT_POV2_UP           = 0x72,
        COH_INPUT_POV2_DOWN         = 0x74,
        COH_INPUT_POV2_LEFT         = 0x75,
        COH_INPUT_POV2_RIGHT        = 0x76,
        COH_INPUT_POV3_UP           = 0x77,
        COH_INPUT_POV3_DOWN         = 0x7A,
        COH_INPUT_POV3_LEFT         = 0x7C,
        COH_INPUT_POV3_RIGHT        = 0x8E,
        COH_INPUT_JOYSTICK1_UP      = 0x0BD,
        COH_INPUT_JOYSTICK1_DOWN    = 0x0BE,
        COH_INPUT_JOYSTICK1_LEFT    = 0x0BF,
        COH_INPUT_JOYSTICK1_RIGHT   = 0x0C0,
        COH_INPUT_JOYSTICK2_UP      = 0x0A5,
        COH_INPUT_JOYSTICK2_DOWN    = 0x0A6,
        COH_INPUT_JOYSTICK2_LEFT    = 0x0A7,
        COH_INPUT_JOYSTICK2_RIGHT   = 0x0A8,
        COH_INPUT_JOYSTICK3_UP      = 0x8F,
        COH_INPUT_JOYSTICK3_DOWN    = 0x98,
        COH_INPUT_JOYSTICK3_LEFT    = 0x9A,
        COH_INPUT_JOYSTICK3_RIGHT   = 0x9B,
        COH_INPUT_LAST_NON_GENERIC  = 0x100,
        COH_KEY_GENERIC_CONTROL     = 0x801,   // either of control keys
        COH_KEY_GENERIC_SHIFT       = 0x802,   // either of shift keys
        COH_KEY_GENERIC_ALT         = 0x803,   // either of alt keys
        COH_MOUSE_MOVE              = 0x1000,
    };

    struct KeybindData {
        KeybindData() : key(), mods(0), command() {}
        KeybindData(int k, int m, QString c)
                : key(k), mods(m), command(c) {}
        KeybindData(int k, QString c)
                : key(k), mods(0), command(c) {}

        int key;
        int mods;
        QString command;
    };

    QMap<int,KeybindData> binds;
    binds[1] = KeybindData(COH_INPUT_APOSTROPHE,"quickchat");                       // (        Qt::Key_ParenLeft
    binds[2] = KeybindData(COH_INPUT_MINUS,"prev_tray");                            // -        Qt::Key_Minus
    binds[3] = KeybindData(COH_INPUT_MINUS,COH_KEY_GENERIC_ALT,"prev_tray_alt");    // ALT+-    Qt::AltModifier + Qt::Key_Minus
    binds[4] = KeybindData(COH_INPUT_SLASH,"show chat$$slashchat");                 // /        Qt::Key_Slash
    binds[5] = KeybindData(COH_INPUT_0,"powexec_slot 10");   // 0    Qt::Key_0
    binds[6] = KeybindData(COH_INPUT_1,"powexec_slot 1");    // 1    Qt::Key_1
    binds[7] = KeybindData(COH_INPUT_2,"powexec_slot 2");    // 2    Qt::Key_2
    binds[8] = KeybindData(COH_INPUT_3,"powexec_slot 3");    // 3    Qt::Key_3
    binds[9] = KeybindData(COH_INPUT_4,"powexec_slot 4");    // 4    Qt::Key_4
    binds[10] = KeybindData(COH_INPUT_5,"powexec_slot 5");   // 5    Qt::Key_5
    binds[11] = KeybindData(COH_INPUT_6,"powexec_slot 6");   // 6    Qt::Key_6
    binds[12] = KeybindData(COH_INPUT_7,"powexec_slot 7");   // 7    Qt::Key_7
    binds[13] = KeybindData(COH_INPUT_8,"powexec_slot 8");   // 8    Qt::Key_8
    binds[13] = KeybindData(COH_INPUT_9,"powexec_slot 9");   // 9    Qt::Key_9
    //binds[14] = KeybindData(COH_INPUT_0,COH_KEY_GENERIC_SHIFT,"unassigned");    // SHIFT+0   Qt::ShiftModifier + Qt::Key_0
    binds[15] = KeybindData(COH_INPUT_1,COH_KEY_GENERIC_SHIFT,"team_select 1");   // SHIFT+1   Qt::ShiftModifier + Qt::Key_1
    binds[16] = KeybindData(COH_INPUT_2,COH_KEY_GENERIC_SHIFT,"team_select 2");   // SHIFT+2   Qt::ShiftModifier + Qt::Key_2
    binds[17] = KeybindData(COH_INPUT_3,COH_KEY_GENERIC_SHIFT,"team_select 3");   // SHIFT+3   Qt::ShiftModifier + Qt::Key_3
    binds[18] = KeybindData(COH_INPUT_4,COH_KEY_GENERIC_SHIFT,"team_select 4");   // SHIFT+4   Qt::ShiftModifier + Qt::Key_4
    binds[19] = KeybindData(COH_INPUT_5,COH_KEY_GENERIC_SHIFT,"team_select 5");   // SHIFT+5   Qt::ShiftModifier + Qt::Key_5
    binds[20] = KeybindData(COH_INPUT_6,COH_KEY_GENERIC_SHIFT,"team_select 6");   // SHIFT+6   Qt::ShiftModifier + Qt::Key_6
    binds[21] = KeybindData(COH_INPUT_7,COH_KEY_GENERIC_SHIFT,"team_select 7");   // SHIFT+7   Qt::ShiftModifier + Qt::Key_7
    binds[22] = KeybindData(COH_INPUT_8,COH_KEY_GENERIC_SHIFT,"team_select 8");   // SHIFT+8   Qt::ShiftModifier + Qt::Key_8
    //binds[23] = KeybindData(COH_INPUT_9,COH_KEY_GENERIC_SHIFT,"unassigned");    // SHIFT+9   Qt::ShiftModifier + Qt::Key_9
    binds[24] = KeybindData(COH_INPUT_0,COH_KEY_GENERIC_CONTROL,"powexec_alt2slot 10"); // CTRL+0   Qt::ControlModifier + Qt::Key_0
    binds[25] = KeybindData(COH_INPUT_1,COH_KEY_GENERIC_CONTROL,"powexec_alt2slot 1");  // CTRL+1   Qt::ControlModifier + Qt::Key_1
    binds[26] = KeybindData(COH_INPUT_2,COH_KEY_GENERIC_CONTROL,"powexec_alt2slot 2");  // CTRL+2   Qt::ControlModifier + Qt::Key_2
    binds[27] = KeybindData(COH_INPUT_3,COH_KEY_GENERIC_CONTROL,"powexec_alt2slot 3");  // CTRL+3   Qt::ControlModifier + Qt::Key_3
    binds[28] = KeybindData(COH_INPUT_4,COH_KEY_GENERIC_CONTROL,"powexec_alt2slot 4");  // CTRL+4   Qt::ControlModifier + Qt::Key_4
    binds[29] = KeybindData(COH_INPUT_5,COH_KEY_GENERIC_CONTROL,"powexec_alt2slot 5");  // CTRL+5   Qt::ControlModifier + Qt::Key_5
    binds[30] = KeybindData(COH_INPUT_6,COH_KEY_GENERIC_CONTROL,"powexec_alt2slot 6");  // CTRL+6   Qt::ControlModifier + Qt::Key_6
    binds[31] = KeybindData(COH_INPUT_7,COH_KEY_GENERIC_CONTROL,"powexec_alt2slot 7");  // CTRL+7   Qt::ControlModifier + Qt::Key_7
    binds[32] = KeybindData(COH_INPUT_8,COH_KEY_GENERIC_CONTROL,"powexec_alt2slot 8");  // CTRL+8   Qt::ControlModifier + Qt::Key_8
    binds[33] = KeybindData(COH_INPUT_9,COH_KEY_GENERIC_CONTROL,"powexec_alt2slot 9");  // CTRL+9   Qt::ControlModifier + Qt::Key_9
    binds[34] = KeybindData(COH_INPUT_0,COH_KEY_GENERIC_ALT,"powexec_altslot 10");  // ALT+0    Qt::Key_ParenRight
    binds[35] = KeybindData(COH_INPUT_1,COH_KEY_GENERIC_ALT,"powexec_altslot 1");   // ALT+1    Qt::Key_Exclam
    binds[36] = KeybindData(COH_INPUT_2,COH_KEY_GENERIC_ALT,"powexec_altslot 2");   // ALT+2    Qt::Key_At
    binds[37] = KeybindData(COH_INPUT_3,COH_KEY_GENERIC_ALT,"powexec_altslot 3");   // ALT+3    Qt::Key_NumberSign
    binds[38] = KeybindData(COH_INPUT_4,COH_KEY_GENERIC_ALT,"powexec_altslot 4");   // ALT+4    Qt::Key_Dollar
    binds[39] = KeybindData(COH_INPUT_5,COH_KEY_GENERIC_ALT,"powexec_altslot 5");   // ALT+5    Qt::Key_Percent
    binds[40] = KeybindData(COH_INPUT_6,COH_KEY_GENERIC_ALT,"powexec_altslot 6");   // ALT+6    Qt::Key_AsciiCircum
    binds[41] = KeybindData(COH_INPUT_7,COH_KEY_GENERIC_ALT,"powexec_altslot 7");   // ALT+7    Qt::Key_Ampersand
    binds[42] = KeybindData(COH_INPUT_8,COH_KEY_GENERIC_ALT,"powexec_altslot 8");   // ALT+8    Qt::Key_Asterisk
    binds[43] = KeybindData(COH_INPUT_9,COH_KEY_GENERIC_ALT,"powexec_altslot 9");   // ALT+9    Qt::Key_ParenLeft
    binds[44] = KeybindData(COH_INPUT_SEMICOLON,"show chat$$beginchat ;");          // ;    Qt::Key_Semicolon
    binds[45] = KeybindData(COH_INPUT_BACKSLASH,"menu");     // \    Qt::Key_Backslash
    binds[46] = KeybindData(COH_INPUT_A,"+left");            // A    Qt::Key_A
    binds[47] = KeybindData(COH_INPUT_B,"++first");          // B    Qt::Key_B
    binds[48] = KeybindData(COH_INPUT_C,"chat");             // C    Qt::Key_C
    binds[49] = KeybindData(COH_INPUT_D,"+right");           // D    Qt::Key_D
    binds[50] = KeybindData(COH_INPUT_E,"+turnright");       // E    Qt::Key_E
    binds[51] = KeybindData(COH_INPUT_F,"follow");           // F    Qt::Key_F
    //binds[52] = KeybindData(COH_INPUT_G,"unassigned");     // G    Qt::Key_G
    binds[53] = KeybindData(COH_INPUT_H,"helpwindow");       // H    Qt::Key_H
    //binds[54] = KeybindData(COH_INPUT_I,"unassigned");     // I    Qt::Key_I
    //binds[55] = KeybindData(COH_INPUT_J,"unassigned");     // J    Qt::Key_J
    //binds[56] = KeybindData(COH_INPUT_K,"unassigned");     // K    Qt::Key_K
    //binds[57] = KeybindData(COH_INPUT_L,"unassigned");     // L    Qt::Key_L
    binds[58] = KeybindData(COH_INPUT_M,"map");              // M    Qt::Key_M
    binds[59] = KeybindData(COH_INPUT_N,"nav");              // N    Qt::Key_N
    //binds[60] = KeybindData(COH_INPUT_O,"unassigned");     // O    Qt::Key_O
    binds[61] = KeybindData(COH_INPUT_P,"powers");           // P    Qt::Key_P
    binds[62] = KeybindData(COH_INPUT_Q,"+turnleft");        // Q    Qt::Key_Q
    binds[63] = KeybindData(COH_INPUT_R,"++autorun");        // R    Qt::Key_R
    binds[64] = KeybindData(COH_INPUT_S,"+backward");        // S    Qt::Key_S
    binds[65] = KeybindData(COH_INPUT_T,"target");           // T    Qt::Key_T
    //binds[66] = KeybindData(COH_INPUT_U,"unassigned");     // U    Qt::Key_U
    binds[67] = KeybindData(COH_INPUT_V,"+ctm_invert");      // V    Qt::Key_V
    binds[68] = KeybindData(COH_INPUT_W,"+forward");         // W    Qt::Key_W
    binds[69] = KeybindData(COH_INPUT_X,"+down");            // X    Qt::Key_X
    //binds[70] = KeybindData(COH_INPUT_Y,"unassigned");     // Y    Qt::Key_Y
    binds[71] = KeybindData(COH_INPUT_Z,"powerexec_abort");  // Z    Qt::Key_Z
    binds[72] = KeybindData(COH_INPUT_ESCAPE,"unselect");         // escape       Qt::Key_Escape
    binds[73] = KeybindData(COH_INPUT_TAB,"target_enemy_next");   // tab          Qt::Key_Tab
    binds[74] = KeybindData(COH_INPUT_TAB,COH_KEY_GENERIC_SHIFT,"unassigned");    // SHIFT+TAB      Qt::Key_Backtab
    binds[75] = KeybindData(COH_INPUT_BACKSPACE,"autoreply");           // backspace    Qt::Key_Backspace
    binds[76] = KeybindData(COH_INPUT_RETURN,"show chat$$startchat");   // return       Qt::Key_Return
    //binds[77] = KeybindData(COH_INPUT_RETURN,"unassigned");           // enter        Qt::Key_Enter
    binds[78] = KeybindData(COH_INPUT_INSERT,"+lookup");                // insert       Qt::Key_Insert
    binds[79] = KeybindData(COH_INPUT_DELETE,"+lookdown");              // delete       Qt::Key_Delete
    binds[80] = KeybindData(COH_INPUT_PAUSE,"++screenshotui");          // pause        Qt::Key_Pause
    //binds[81] = KeybindData(COH_INPUT_PRINT,"unassigned");            // print        Qt::Key_Print
    binds[82] = KeybindData(COH_INPUT_SYSRQ,"screenshot");              // sysreq       Qt::Key_SysReq
    //binds[83] = KeybindData(COH_INPUT_CLEAR,"unassigned");            // clear        Qt::Key_Clear
    binds[84] = KeybindData(COH_INPUT_HOME,"+zoomin");                  // home         Qt::Key_Home
    binds[85] = KeybindData(COH_INPUT_END,"+zoomout");                  // end          Qt::Key_End
    binds[86] = KeybindData(COH_INPUT_LEFTARROW,"+turnleft");           // left         Qt::Key_Left
    binds[87] = KeybindData(COH_INPUT_UPARROW,"+forward");              // up           Qt::Key_Up
    binds[88] = KeybindData(COH_INPUT_RIGHTARROW,"+turnright");         // right        Qt::Key_Right
    binds[89] = KeybindData(COH_INPUT_DOWNARROW,"+backward");           // down         Qt::Key_Down
    binds[90] = KeybindData(COH_INPUT_PGUP,"+camrotate");               // pageup       Qt::Key_PageUp
    binds[91] = KeybindData(COH_INPUT_PGDN,"camreset");                 // pagedown     Qt::Key_PageDown
    //binds[92] = KeybindData(COH_INPUT_LSHIFT,"unassigned");            // shift        Qt::Key_Shift
    binds[93] = KeybindData(COH_INPUT_LCONTROL,"+alt2tray");       // leftcontrol  Qt::Key_Control
    //binds[94] = KeybindData(0x01000022,"unassigned");           // windowskey   Qt::Key_Meta
    binds[95] = KeybindData(COH_INPUT_F1,"inspexec_slot 1");        // F1   Qt::Key_F1
    binds[96] = KeybindData(COH_INPUT_F2,"inspexec_slot 2");        // F2   Qt::Key_F2
    binds[97] = KeybindData(COH_INPUT_F3,"inspexec_slot 3");        // F3   Qt::Key_F3
    binds[98] = KeybindData(COH_INPUT_F4,"inspexec_slot 4");        // F4   Qt::Key_F4
    binds[99] = KeybindData(COH_INPUT_F5,"inspexec_slot 5");        // F5   Qt::Key_F5
    binds[100] = KeybindData(COH_INPUT_F6,"local <color white><bgcolor red>RUN!");                           // F6   Qt::Key_F6
    binds[101] = KeybindData(COH_INPUT_F7,"say <color black><bgcolor #22aa22>Ready! $$ emote thumbsup");     // F7   Qt::Key_F7
    binds[102] = KeybindData(COH_INPUT_F8,"local <color black><bgcolor #aaaa22>HELP! $$ emote whistle");     // F8   Qt::Key_F8
    binds[103] = KeybindData(COH_INPUT_F9,"local <color white><bgcolor #2222aa><scale .75>level $level $archetype$$local <color white><bgcolor #2222aa>Looking for team");   // F9   Qt::Key_F9
    binds[104] = KeybindData(COH_INPUT_F10,"say $battlecry $$ emote attack");                                 // F10  Qt::Key_F10
    //binds[105] = KeybindData(COH_INPUT_F11,"unassigned");          // F11   Qt::Key_F11
    //binds[106] = KeybindData(COH_INPUT_F12,"unassigned");          // F12   Qt::Key_F12
    binds[107] = KeybindData(COH_INPUT_LEFTARROW,"+turnleft");       // directionleft    Qt::Key_Direction_L
    binds[108] = KeybindData(COH_INPUT_RIGHTARROW,"+turnright");     // directionright   Qt::Key_Direction_R
    binds[109] = KeybindData(COH_INPUT_SPACE,"+up");                 // space            Qt::Key_Space
    binds[110] = KeybindData(COH_INPUT_COMMA,"show chat$$beginchat /tell $target, ");    // ,        Qt::Key_Comma
    binds[111] = KeybindData(COH_INPUT_EQUALS,"next_tray");                              // =        Qt::Key_Equal
    binds[112] = KeybindData(COH_INPUT_EQUALS,COH_KEY_GENERIC_ALT,"next_tray_alt");      // ALT+=    Qt::Key_Equal
    binds[113] = KeybindData(COH_INPUT_MBUTTON,"+camrotate");       // MBUTTON
    binds[114] = KeybindData(COH_INPUT_LBUTTON,COH_INPUT_RBUTTON,"+forward_mouse");      // MouseChord "left and right"
    binds[115] = KeybindData(COH_MOUSE_SCROLL,"+camdistadjust");    // MOUSEWHEEL
    binds[116] = KeybindData(COH_INPUT_RALT,"+alttreysticky");      // Right ALT
    binds[117] = KeybindData(COH_INPUT_RBUTTON,"+mouse_look");      // right button
    binds[118] = KeybindData(COH_INPUT_SPACE,"+up");                // space    Qt::Key_Space
    binds[119] = KeybindData(COH_INPUT_TAB,COH_KEY_GENERIC_CONTROL,"target_enemy_near");    // CTRL+TAB    Qt::ControlModifier + Qt::Key_Tab
    binds[120] = KeybindData(COH_INPUT_TAB,COH_KEY_GENERIC_SHIFT,"target_enemy_prev");      // SHIFT+TAB    Qt::ShiftModifier + Qt::Key_Tab

    for(int i=0; i<COH_INPUT_LAST_NON_GENERIC; ++i)
    {
      if(binds.contains(i))
      {
             const KeybindData &kb(binds[i]);
             bs.StoreString(kb.command);
             bs.StoreBits(32,kb.key);
             bs.StoreBits(32,kb.mods);
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
