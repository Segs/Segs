#include "Powers.h"

#include <ace/Log_Msg.h>

void Power::serializeto(BitStream &tgt) const
{
    tgt.StoreBits(4,uint32_t(entry_type));
    switch(entry_type)
    {
    case TrayItemType::Power:
        tgt.StoreBits(32,powerset_idx);
        tgt.StoreBits(32,power_idx);
        break;
    case TrayItemType::Inspiration:
        tgt.StorePackedBits(3,powerset_idx);
        tgt.StorePackedBits(3,power_idx);
        break;
    case TrayItemType::Macro:
        tgt.StoreString(command);
        tgt.StoreString(short_name);
        tgt.StoreString(icon_name);
        break;
    case TrayItemType::None:
        break;
    default:
        ACE_DEBUG((LM_WARNING,ACE_TEXT("(%P|%t) Unknown tray entry type %d\n"),entry_type));
    }
}

void Power::serializefrom(BitStream &src)
{
    entry_type = (TrayItemType)src.GetBits(4);
    switch(entry_type)
    {
    case TrayItemType::Power:
        powerset_idx = src.GetBits(32);
        power_idx = src.GetBits(32);
        break;
    case TrayItemType::Inspiration:
        powerset_idx = src.GetPackedBits(3);
        power_idx = src.GetPackedBits(3);
        break;
    case TrayItemType::Macro:
        src.GetString(command);
        src.GetString(short_name);
        src.GetString(icon_name);
        break;
    case TrayItemType::None:
        break;
    default:
        ACE_DEBUG((LM_WARNING,ACE_TEXT("(%P|%t) Unknown tray entry type %d\n"),entry_type));
    }
}

void Power::Dump()
{
    switch(entry_type)
    {
    case TrayItemType::Power:
    case TrayItemType::Inspiration:
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("[(0x%x,0x%x)]"),powerset_idx,power_idx));
        break;
    case TrayItemType::Macro:
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("[(%s,%s,%s)]"),qPrintable(command),qPrintable(short_name),qPrintable(icon_name)));
        break;
    case TrayItemType::None:
        break;
    default:
        ACE_DEBUG((LM_WARNING,ACE_TEXT("(%P|%t) Unknown tray entry type %d\n"),entry_type));
    }

}

void PowerTrayGroup::serializeto(BitStream &tgt) const
{
    tgt.StoreBits(32,primary_tray_idx);
    tgt.StoreBits(32,secondary_tray_idx);
    for(int bar_num=0; bar_num<9; bar_num++)
        m_trays[bar_num].serializeto(tgt);
    bool m_c = false;
    tgt.StoreBits(1,m_c);
    if(m_c) // selected ???
    {
        tgt.StoreBits(32,m_default_powerset_idx);
        tgt.StoreBits(32,m_default_power_idx);
    }
}

void PowerTrayGroup::serializefrom(BitStream &src)
{
    primary_tray_idx = src.GetBits(32);
    secondary_tray_idx = src.GetBits(32);
    for(PowerTray &tray : m_trays)
    {
        tray.serializefrom(src);
    }
    m_c = src.GetBits(1);
    if(m_c)
    {
        m_default_powerset_idx= src.GetBits(32);
        m_default_power_idx= src.GetBits(32);
    }
}

void PowerTrayGroup::dump()
{
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    unkn1: 0x%08x\n"),primary_tray_idx));
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    unkn2: 0x%08x\n"),secondary_tray_idx));
    for(int bar_num=0; bar_num<9; bar_num++)
    {
        if(m_trays[bar_num].setPowers()==0)
            continue;
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    Tray %d ***"),bar_num));
        m_trays[bar_num].Dump();
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("***\n")));
    }
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_c %d\n"),m_c));
    if(m_c)
    {
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_power_rel1 0x%08x\n"),m_default_powerset_idx));
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_power_rel2 0x%08x\n"),m_default_power_idx));
    }
}

Power *PowerTray::getPower(size_t idx)
{
    if(idx<10)
        return &m_powers[idx];
    return nullptr;
}

int PowerTray::setPowers()
{
    int res=0;
    for(const Power &pow : m_powers)
    {
        res += (pow.entry_type!=TrayItemType::None);
    }
    return res;
}

void PowerTray::serializefrom(BitStream &src)
{
    for(Power &pow : m_powers)
        pow.serializefrom(src);
}

void PowerTray::serializeto(BitStream &tgt) const
{
    for(const Power &pow : m_powers)
        pow.serializeto(tgt);
}

void PowerTray::Dump()
{
    for(Power &pow : m_powers)
    {
        pow.Dump();
    }

}

void PowerPool_Info::serializefrom(BitStream &src)
{
    id[0] = src.GetPackedBits(3);
    id[1] = src.GetPackedBits(3);
    id[2] = src.GetPackedBits(3);
}

void PowerPool_Info::serializeto(BitStream &src) const
{
    src.StorePackedBits(3,id[0]);
    src.StorePackedBits(3,id[1]);
    src.StorePackedBits(3,id[2]);
}
