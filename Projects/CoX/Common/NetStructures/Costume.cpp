/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */
#include "Costume.h"

#include "BitStream.h"
#include "Common/GameData/serialization_common.h"

void CostumePart::serializeto( BitStream &bs ) const
{
    storeCached_String(bs,m_geometry);
    storeCached_String(bs,m_texture_1);
    storeCached_String(bs,m_texture_2);
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
    m_geometry=getCached_String(bs);
    m_texture_1=getCached_String(bs);
    m_texture_2=getCached_String(bs);
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

    bs.StoreString(m_geometry);
    bs.StoreString(m_texture_1);
    bs.StoreString(m_texture_2);
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


template<class Archive>
void serialize(Archive &arc, CostumePart &cp) {
    arc(cp.m_type);
    arc(cp.m_geometry);
    arc(cp.m_texture_1);
    arc(cp.m_texture_2);
    arc(cp.name_3);
    arc(cp.name_4);
    arc(cp.name_5);
    arc(cp.m_full_part);
    arc(cp.m_colors[0]);
    arc(cp.m_colors[1]);
}

template<class Archive>
void serialize(Archive &arc, Costume &c) {
    arc(c.m_parts);
}

void Costume::serializeToDb(QString &tgt)
{
// for now only parts are serialized
    // format is a simple [[]]
    std::ostringstream ostr;
    {
        cereal::JSONOutputArchive ar( ostr );
        ar(*this);
    }
    tgt = QString::fromStdString(ostr.str());
}

void Costume::serializeFromDb(const QString &src)
{
    if(src.isEmpty())
        return;
    std::istringstream istr;
    istr.str(src.toStdString());
    {
        cereal::JSONInputArchive ar(istr);
        ar(*this);
    }
}
