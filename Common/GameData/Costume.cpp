/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup GameData Projects/CoX/Common/GameData
 * @{
 */

#include "Costume.h"
#include "GameDataStore.h"

#include "Components/BitStream.h"
#include "Components/serialization_common.h"
#include "Components/serialization_types.h"
#include <QtCore/QDebug>

int g_max_num_costume_slots = 4; // client UI cannot handle more than 4 slots

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

Costume Costume::NullCostume;
void Costume::storeCharselParts( BitStream &bs ) const
{
    bs.StorePackedBits(1,m_parts.size());
    for(const CostumePart & part : m_parts)
        serializeto_charsel(part,bs);
}

void Costume::storeCharsel(BitStream &bs) const
{
    bs.StorePackedBits(1, m_body_type); // 0:male normal
    bs.StoreFloat(m_height);
    bs.StoreFloat(m_physique);
    bs.StoreBits(32, m_skin_color); // rgb ?
}

template<class Archive>

void serialize(Archive &arc, CostumePart &cp, uint32_t const version)
{
    if (version != cp.class_version)
    {
        qCritical() << "Failed to serialize CostumePart, incompatible serialization format version " << version;
        return;
    }

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
CEREAL_CLASS_VERSION(CostumePart, CostumePart::class_version)   // register CostumePart class version
SPECIALIZE_VERSIONED_SERIALIZATIONS(CostumePart)

template<class Archive>
void Costume::serialize(Archive &archive, uint32_t const version)
{
    if (version != Costume::class_version)
    {
        qCritical() << "Failed to serialize Costume, incompatible serialization format version " << version;
        return;
    }

    archive(cereal::make_nvp("CharacterID", m_character_id));
    archive(cereal::make_nvp("CostumeIdx", m_index));
    archive(cereal::make_nvp("Height", m_height));
    archive(cereal::make_nvp("Physique", m_physique));
    archive(cereal::make_nvp("BodyType", m_body_type));
    archive(cereal::make_nvp("SkinColor", m_skin_color));
    archive(cereal::make_nvp("SendFullCostume", m_send_full_costume));
    archive(cereal::make_nvp("NumParts", m_num_parts));
    archive(cereal::make_nvp("Parts", m_parts));
}
CEREAL_CLASS_VERSION(Costume, Costume::class_version)   // register Costume class version
SPECIALIZE_CLASS_VERSIONED_SERIALIZATIONS(Costume)

void Costume::serializeToDb(QString &tgt) const
{
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
        part.m_type = part_type++;
}

void Costume::dump() const
{
    qDebug().noquote() << "Costume: " << m_character_id << ":" << m_index;
    qDebug().noquote() << "  Body Type: " << m_body_type;
    qDebug().noquote() << "  Skin Color: " << m_skin_color;
    qDebug().noquote() << "  Height: " << m_height;
    qDebug().noquote() << "  Physique: " << m_physique;
    qDebug().noquote() << "****** " << m_num_parts << " Parts ******";

    for(int i=0; i < m_num_parts; i++)
    {
        const CostumePart &cp(m_parts[i]);
        if(cp.m_full_part)
            qDebug().noquote() << cp.m_geometry << cp.m_texture_1 << cp.m_texture_2 <<
                        cp.m_colors[0] << cp.m_colors[1] <<
                        cp.name_3 << cp.name_4 << cp.name_5;
        else
            qDebug().noquote() << cp.m_geometry << cp.m_texture_1 << cp.m_texture_2 <<
                        cp.m_colors[0] << cp.m_colors[1];
    }

}

void dumpCostumes(vCostumes &costumes)
{
    for(auto costume : costumes)
        costume.dump();
}

void serializeto(const Costume &costume,BitStream &bs, const ColorAndPartPacker *packer)
{
    bs.StorePackedBits(3, costume.m_body_type); // 0:male normal
    bs.StoreBits(32, costume.m_skin_color); // rgb ?

    bs.StoreFloat(costume.m_height);
    bs.StoreFloat(costume.m_physique);

    bs.StoreBits(1, costume.m_send_full_costume);
    assert(!costume.m_parts.empty());
    bs.StorePackedBits(4, costume.m_parts.size());
    try
    {
        for(uint32_t costume_part = 0; costume_part < costume.m_parts.size(); costume_part++)
        {
            CostumePart part = costume.m_parts[costume_part];
            // TODO: this is bad code, it's purpose is to NOT send all part strings if m_send_full_costume is false
            part.m_full_part = costume.m_send_full_costume;
            ::serializeto(part, bs, packer);
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

void serializefrom(Costume &tgt, BitStream &src, const ColorAndPartPacker *packer)
{
    tgt.m_body_type = src.GetPackedBits(3); // 0:male normal
    tgt.m_skin_color = src.GetBits(32); // rgb

    tgt.m_height = src.GetFloat();
    tgt.m_physique = src.GetFloat();

    tgt.m_send_full_costume = src.GetBits(1);
    tgt.m_num_parts = src.GetPackedBits(4);

    try
    {
        for(int costume_part=0; costume_part<tgt.m_num_parts;costume_part++)
        {
            CostumePart part;
            part.m_full_part = tgt.m_send_full_costume;
            part.m_type = costume_part;
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
