/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */
#include <ace/ACE.h>
#include "Costume.h"
#include "BitStream.h"

void CostumePart::serializeto( BitStream &bs ) const
{
    storeCached_String(bs,name_0);
    storeCached_String(bs,name_1);
    storeCached_String(bs,name_2);
    storeCached_Color(bs,m_colors[0]);
    storeCached_Color(bs,m_colors[1]);
    if(m_full_part)
    {
        storeCached_String(bs,name_3);
        storeCached_String(bs,name_4);
        storeCached_String(bs,name_5);
    }
}

void CostumePart::serializefrom( BitStream &bs )
{
    name_0=getCached_String(bs);
    name_1=getCached_String(bs);
    name_2=getCached_String(bs);
    m_colors[0]=getCached_Color(bs);
    m_colors[1]=getCached_Color(bs);
    if(m_full_part)
    {
        name_3=getCached_String(bs);
        name_4=getCached_String(bs);
        name_5=getCached_String(bs);
    }
}

void CostumePart::serializeto_charsel( BitStream &bs ) const
{
    // character selection needs to get part names as strings
    static const char *names[] = {
        "Pants","Chest","Head","Gloves","Boots","Belt","Hair","Face","EyeDetail","ChestDetail",
        "Shoulders","Back","WepR","Neck","UarmR",
    };

    bs.StoreString(name_0);
    bs.StoreString(name_1);
    bs.StoreString(name_2);
    bs.StoreString(names[m_type]); //name_6 bonename ?
    bs.StoreBits(32,m_colors[0]);
    bs.StoreBits(32,m_colors[1]);
}
CharacterCostume CharacterCostume::NullCostume;
void Costume::storeCharselParts( BitStream &bs )
{
    bs.StorePackedBits(1,m_parts.size());
    for(size_t costume_part=0; costume_part<m_parts.size();costume_part++)
    {
        m_parts[costume_part].m_type=costume_part;
        m_parts[costume_part].serializeto_charsel(bs);
    }
}
namespace  {
    std::string toHex(uint32_t v) {
        char buf[32];
        snprintf(buf,32,"0x%08x",v);
        return buf;
    }
    uint32_t fromHex(const std::string &v) {
        uint32_t i;
        sscanf(v.c_str(),"0x%08x",&i);
        return i;
    }
}
void Costume::serializeToDb(std::string &tgt)
{
// for now only parts are serialized
    // format is a simple [[]]
    std::ostringstream ostr;
    ostr << '[';
    for(size_t idx=0; idx<m_parts.size(); ++idx)
    {
        CostumePart &prt(m_parts[idx]);
        ostr << '[' << prt.name_0 << ',' << prt.name_1 << ',' << prt.name_2 << ','
             << toHex(prt.m_colors[0]) << ',' << toHex(prt.m_colors[1]) << ']';
        if(idx!=m_parts.size()-1)
            ostr<<',';
    }
    ostr << ']';
    tgt = ostr.str();
}

void Costume::serializeFromDb(const std::string &src)
{
    std::istringstream istr(src);
    if(src.empty())
        return;
    char brckt;
    istr>>brckt;
    bool error_encountered = brckt=='[';
    std::string color1,color2;
    while(!error_encountered) {
        CostumePart prt(false);
        istr >> brckt >> prt.name_0 >> brckt >> prt.name_1 >> brckt >> prt.name_2 >> brckt
             >> color1 >> brckt >> color2 >> brckt;
        prt.m_colors[0] = fromHex(color1);
        prt.m_colors[1] = fromHex(color2);
        istr >> brckt;
        if(brckt!=',' && brckt!=']')
            error_encountered = true;
        m_parts.push_back(prt);
        if(brckt==']')
            break;
    }
}
