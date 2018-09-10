/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup NetStructures Projects/CoX/Common/NetStructures
 * @{
 */

#include "Costume.h"

#include "BitStream.h"
#include "serialization_common.h"
#include "serialization_types.h"
#include <QtCore/QDebug>

namespace
{
    void serializeto_charsel(const CostumePart &part, BitStream &bs )
    {
        // character selection needs to get part names as strings
        static const char *names[] = {
            "Pants","Chest","Head","Gloves","Boots","Belt","Hair","Face","EyeDetail","ChestDetail",
            "Shoulders","Back","WepR","Neck","UarmR",
        };

        bs.StoreString(part.m_geometry);
        bs.StoreString(part.m_texture_1);
        bs.StoreString(part.m_texture_2);
        bs.StoreString(names[part.m_type]); //name_6 bonename ?
        bs.StoreBits(32,part.m_colors[0]);
        bs.StoreBits(32,part.m_colors[1]);
    }
}

void serializeto(const CostumePart &part, BitStream &bs,const ColorAndPartPacker *packingContext )
{
    try
    {
        packingContext->packPartname(part.m_geometry,bs);
        packingContext->packPartname(part.m_texture_1,bs);
        packingContext->packPartname(part.m_texture_2,bs);
        packingContext->packColor(part.m_colors[0],bs);
        packingContext->packColor(part.m_colors[1],bs);
        if(part.m_full_part)
        {
            packingContext->packPartname(part.name_3,bs);
            packingContext->packPartname(part.name_4,bs);
            packingContext->packPartname(part.name_5,bs);
        }
    }
    catch(cereal::RapidJSONException &e)
    {
        qWarning() << e.what();
    }
    catch(std::exception &e)
    {
        qCritical() << e.what();
    }
}

void serializefrom(CostumePart &part,BitStream &bs,const ColorAndPartPacker *packingContext )
{
    try
    {
        packingContext->unpackPartname(bs,part.m_geometry);
        packingContext->unpackPartname(bs,part.m_texture_1);
        packingContext->unpackPartname(bs,part.m_texture_2);
        packingContext->unpackColor(bs,part.m_colors[0]);
        packingContext->unpackColor(bs,part.m_colors[1]);
        if(part.m_full_part)
        {
            packingContext->unpackPartname(bs,part.name_3);
            packingContext->unpackPartname(bs,part.name_4);
            packingContext->unpackPartname(bs,part.name_5);
        }
    }
    catch(cereal::RapidJSONException &e)
    {
        qWarning() << e.what();
    }
    catch(std::exception &e)
    {
        qCritical() << e.what();
    }
}

CharacterCostume CharacterCostume::NullCostume;
void Costume::storeCharselParts( BitStream &bs ) const
{
    bs.StorePackedBits(1,m_parts.size());
    for(const CostumePart & part : m_parts)
    {
        serializeto_charsel(part,bs);
    }
}

void Costume::serializeToDb(QString &tgt) const
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
    // Set the part types
    uint8_t part_type=0;
    for(CostumePart & part : m_parts)
    {
        part.m_type = part_type++;
    }
}

void Costume::dump() const
{

    qDebug().noquote() << "Costume \n";
    qDebug().noquote() << "body type: " << m_body_type;
    qDebug().noquote() << "skin color: "<< skin_color;
    qDebug().noquote() << "Height " << m_height;
    qDebug().noquote() << "Physique " << m_physique;
    qDebug().noquote() << "****** "<< m_num_parts << " Parts *******";
    for(int i=0; i<m_num_parts; i++)
    {
        const CostumePart &cp(m_parts[i]);
        if(cp.m_full_part)
            qDebug() << cp.m_geometry << cp.m_texture_1 << cp.m_texture_2 <<
                        cp.m_colors[0] << cp.m_colors[1] <<
                        cp.name_3 << cp.name_4 << cp.name_5;
        else
            qDebug() << cp.m_geometry << cp.m_texture_1 << cp.m_texture_2 <<
                        cp.m_colors[0] << cp.m_colors[1];
    }
    qDebug().noquote() << "*************";
}

void serializeto(const Costume &costume,BitStream &bs,const ColorAndPartPacker *packer)
{
    bs.StorePackedBits(3,costume.m_body_type); // 0:male normal
    bs.StoreBits(32,costume.skin_color); // rgb ?

    bs.StoreFloat(costume.m_height);
    bs.StoreFloat(costume.m_physique);

    bs.StoreBits(1,costume.m_non_default_costme_p);
    //m_num_parts = m_parts.size();
    assert(!costume.m_parts.empty());
    bs.StorePackedBits(4,costume.m_parts.size());

    try
    {
        for(uint32_t costume_part=0; costume_part<costume.m_parts.size();costume_part++)
        {
            CostumePart part=costume.m_parts[costume_part];
            // TODO: this is bad code, it's purpose is to NOT send all part strings if m_non_default_costme_p is false
            part.m_full_part = costume.m_non_default_costme_p;
            ::serializeto(part,bs,packer);
        }
    }
    catch(cereal::RapidJSONException &e)
    {
        qWarning() << e.what();
    }
    catch(std::exception &e)
    {
        qCritical() << e.what();
    }
}

void serializefrom(Costume &tgt, BitStream &src,const ColorAndPartPacker *packer)
{
    tgt.m_body_type = src.GetPackedBits(3); // 0:male normal
    tgt.skin_color = src.GetBits(32); // rgb

    tgt.m_height = src.GetFloat();
    tgt.m_physique = src.GetFloat();

    tgt.m_non_default_costme_p = src.GetBits(1);
    tgt.m_num_parts = src.GetPackedBits(4);

    try
    {
        for(int costume_part=0; costume_part<tgt.m_num_parts;costume_part++)
        {
            CostumePart part;
            part.m_full_part = tgt.m_non_default_costme_p;
            ::serializefrom(part,src,packer);
            tgt.m_parts.push_back(part);
        }
    }
    catch(cereal::RapidJSONException &e)
    {
        qWarning() << e.what();
    }
    catch(std::exception &e)
    {
        qCritical() << e.what();
    }
}

//! @}
