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
#include "GameData/keybind_definitions.h"
#include "GameData/hitpoints_definitions.h"
#include <QtCore/QString>
#include <QtCore/QDebug>

// Anonymous namespace
namespace {
    
}
// End Anonymous namespace

Character::Character()
{
    m_unkn3=0;
    m_multiple_costumes=false;
    m_current_costume_idx=0;
    m_current_costume_set=false;
    m_supergroup_costume=false;
    m_sg_costume=nullptr;
    m_using_sg_costume=false;
    m_current_attribs.m_HitPoints = 45; // setHPBase(self)
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
    qDebug() << m_unkn1;
    qDebug() << m_unkn2;
    
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
    qDebug() << m_unkn3;
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

    struct KeybindData {
        KeybindData() : key(), mods(NO_MOD), command() {}
        KeybindData(int k, int m, QString c)
                : key(k), mods(m), command(c) {}
        KeybindData(int k, QString c)
                : key(k), mods(NO_MOD), command(c) {}

        int key;
        int mods;
        QString command;
    };

    QMap<int,KeybindData> binds;
    binds[1] = KeybindData(COH_INPUT_APOSTROPHE,"quickchat");           // (        Qt::Key_ParenLeft
    binds[2] = KeybindData(COH_INPUT_MINUS,"prev_tray");                // -        Qt::Key_Minus
    binds[3] = KeybindData(COH_INPUT_MINUS,ALT_MOD,"prev_tray_alt");    // ALT+-    Qt::AltModifier + Qt::Key_Minus
    binds[4] = KeybindData(COH_INPUT_SLASH,"show chat$$slashchat");     // /        Qt::Key_Slash
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
    //binds[14] = KeybindData(COH_INPUT_0,SHIFT_MOD,"unassigned");    // SHIFT+0   Qt::ShiftModifier + Qt::Key_0
    binds[15] = KeybindData(COH_INPUT_1,SHIFT_MOD,"team_select 1");   // SHIFT+1   Qt::ShiftModifier + Qt::Key_1
    binds[16] = KeybindData(COH_INPUT_2,SHIFT_MOD,"team_select 2");   // SHIFT+2   Qt::ShiftModifier + Qt::Key_2
    binds[17] = KeybindData(COH_INPUT_3,SHIFT_MOD,"team_select 3");   // SHIFT+3   Qt::ShiftModifier + Qt::Key_3
    binds[18] = KeybindData(COH_INPUT_4,SHIFT_MOD,"team_select 4");   // SHIFT+4   Qt::ShiftModifier + Qt::Key_4
    binds[19] = KeybindData(COH_INPUT_5,SHIFT_MOD,"team_select 5");   // SHIFT+5   Qt::ShiftModifier + Qt::Key_5
    binds[20] = KeybindData(COH_INPUT_6,SHIFT_MOD,"team_select 6");   // SHIFT+6   Qt::ShiftModifier + Qt::Key_6
    binds[21] = KeybindData(COH_INPUT_7,SHIFT_MOD,"team_select 7");   // SHIFT+7   Qt::ShiftModifier + Qt::Key_7
    binds[22] = KeybindData(COH_INPUT_8,SHIFT_MOD,"team_select 8");   // SHIFT+8   Qt::ShiftModifier + Qt::Key_8
    //binds[23] = KeybindData(COH_INPUT_9,SHIFT_MOD,"unassigned");    // SHIFT+9   Qt::ShiftModifier + Qt::Key_9
    binds[24] = KeybindData(COH_INPUT_0,CTRL_MOD,"powexec_alt2slot 10"); // CTRL+0   Qt::ControlModifier + Qt::Key_0
    binds[25] = KeybindData(COH_INPUT_1,CTRL_MOD,"powexec_alt2slot 1");  // CTRL+1   Qt::ControlModifier + Qt::Key_1
    binds[26] = KeybindData(COH_INPUT_2,CTRL_MOD,"powexec_alt2slot 2");  // CTRL+2   Qt::ControlModifier + Qt::Key_2
    binds[27] = KeybindData(COH_INPUT_3,CTRL_MOD,"powexec_alt2slot 3");  // CTRL+3   Qt::ControlModifier + Qt::Key_3
    binds[28] = KeybindData(COH_INPUT_4,CTRL_MOD,"powexec_alt2slot 4");  // CTRL+4   Qt::ControlModifier + Qt::Key_4
    binds[29] = KeybindData(COH_INPUT_5,CTRL_MOD,"powexec_alt2slot 5");  // CTRL+5   Qt::ControlModifier + Qt::Key_5
    binds[30] = KeybindData(COH_INPUT_6,CTRL_MOD,"powexec_alt2slot 6");  // CTRL+6   Qt::ControlModifier + Qt::Key_6
    binds[31] = KeybindData(COH_INPUT_7,CTRL_MOD,"powexec_alt2slot 7");  // CTRL+7   Qt::ControlModifier + Qt::Key_7
    binds[32] = KeybindData(COH_INPUT_8,CTRL_MOD,"powexec_alt2slot 8");  // CTRL+8   Qt::ControlModifier + Qt::Key_8
    binds[33] = KeybindData(COH_INPUT_9,CTRL_MOD,"powexec_alt2slot 9");  // CTRL+9   Qt::ControlModifier + Qt::Key_9
    binds[34] = KeybindData(COH_INPUT_0,ALT_MOD,"powexec_altslot 10");  // ALT+0    Qt::Key_ParenRight
    binds[35] = KeybindData(COH_INPUT_1,ALT_MOD,"powexec_altslot 1");   // ALT+1    Qt::Key_Exclam
    binds[36] = KeybindData(COH_INPUT_2,ALT_MOD,"powexec_altslot 2");   // ALT+2    Qt::Key_At
    binds[37] = KeybindData(COH_INPUT_3,ALT_MOD,"powexec_altslot 3");   // ALT+3    Qt::Key_NumberSign
    binds[38] = KeybindData(COH_INPUT_4,ALT_MOD,"powexec_altslot 4");   // ALT+4    Qt::Key_Dollar
    binds[39] = KeybindData(COH_INPUT_5,ALT_MOD,"powexec_altslot 5");   // ALT+5    Qt::Key_Percent
    binds[40] = KeybindData(COH_INPUT_6,ALT_MOD,"powexec_altslot 6");   // ALT+6    Qt::Key_AsciiCircum
    binds[41] = KeybindData(COH_INPUT_7,ALT_MOD,"powexec_altslot 7");   // ALT+7    Qt::Key_Ampersand
    binds[42] = KeybindData(COH_INPUT_8,ALT_MOD,"powexec_altslot 8");   // ALT+8    Qt::Key_Asterisk
    binds[43] = KeybindData(COH_INPUT_9,ALT_MOD,"powexec_altslot 9");   // ALT+9    Qt::Key_ParenLeft
    binds[44] = KeybindData(COH_INPUT_SEMICOLON,"show chat$$beginchat ;");    // ;    Qt::Key_Semicolon
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
    binds[72] = KeybindData(COH_INPUT_ESCAPE,"unselect");               // escape       Qt::Key_Escape
    binds[73] = KeybindData(COH_INPUT_TAB,"target_enemy_next");         // tab          Qt::Key_Tab
    binds[74] = KeybindData(COH_INPUT_TAB,SHIFT_MOD,"unassigned");      // SHIFT+TAB      Qt::Key_Backtab
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
    //binds[92] = KeybindData(COH_INPUT_LSHIFT,"unassigned");           // shift        Qt::Key_Shift
    binds[93] = KeybindData(COH_INPUT_LCONTROL,"+alt2tray");            // leftcontrol  Qt::Key_Control
    //binds[94] = KeybindData(0x01000022,"unassigned");                 // windowskey   Qt::Key_Meta
    binds[95] = KeybindData(COH_INPUT_F1,"inspexec_slot 1");            // F1   Qt::Key_F1
    binds[96] = KeybindData(COH_INPUT_F2,"inspexec_slot 2");            // F2   Qt::Key_F2
    binds[97] = KeybindData(COH_INPUT_F3,"inspexec_slot 3");            // F3   Qt::Key_F3
    binds[98] = KeybindData(COH_INPUT_F4,"inspexec_slot 4");            // F4   Qt::Key_F4
    binds[99] = KeybindData(COH_INPUT_F5,"inspexec_slot 5");            // F5   Qt::Key_F5
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
    binds[112] = KeybindData(COH_INPUT_EQUALS,ALT_MOD,"next_tray_alt");                  // ALT+=    Qt::Key_Equal
    binds[113] = KeybindData(COH_INPUT_MBUTTON,"+camrotate");                            // MBUTTON
    binds[114] = KeybindData(COH_INPUT_LBUTTON,COH_INPUT_RBUTTON,"+forward_mouse");      // MouseChord "left and right"
    binds[115] = KeybindData(COH_INPUT_MOUSE_SCROLL,"+camdistadjust");    // MOUSEWHEEL
    binds[116] = KeybindData(COH_INPUT_RALT,"+alttreysticky");      // Right ALT
    binds[117] = KeybindData(COH_INPUT_RBUTTON,"+mouse_look");      // right button
    binds[118] = KeybindData(COH_INPUT_SPACE,"+up");                          // space    Qt::Key_Space
    binds[119] = KeybindData(COH_INPUT_TAB,CTRL_MOD,"target_enemy_near");     // CTRL+TAB    Qt::ControlModifier + Qt::Key_Tab
    binds[120] = KeybindData(COH_INPUT_TAB,SHIFT_MOD,"target_enemy_prev");    // SHIFT+TAB    Qt::ShiftModifier + Qt::Key_Tab

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
