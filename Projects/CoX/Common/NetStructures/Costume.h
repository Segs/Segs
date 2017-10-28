/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once

#include "CommonNetStructures.h"
#include "BitStream.h"

#include <QtCore/QString>
#include <vector>

struct CostumePart
{
    enum class Type {
        Pants=0,
        Chest,
        Head,
        Gloves,
        Boots,
        Belt,
        Hair,
        Face,
        EyeDetail,
        ChestDetail,
        Shoulders,
        Back,
        WepR,
        Neck,
        UarmR,
    };
    QString m_geometry;
    QString m_texture_1;
    QString m_texture_2;
    QString name_3;
    QString name_4;
    QString name_5;
    QString name_6;
    uint32_t m_colors[2] = {0};
    uint8_t m_type=0; // arms/legs etc..
    bool m_full_part;
};
void serializeto(const CostumePart &part, BitStream &bs, ColorAndPartPacker *packingContext);
void serializefrom(CostumePart &part, BitStream &bs, ColorAndPartPacker *packingContext);
struct Costume
{
    float m_height=0;
    float m_physique=0;
    uint32_t skin_color=0;
    bool m_non_default_costme_p;
    int m_num_parts=0;
    float m_floats[8];
    std::vector<CostumePart> m_parts;
    uint32_t m_body_type;
    void storeCharselParts(BitStream &bs);
    void storeCharsel(BitStream &bs)
    {
        bs.StorePackedBits(1,m_body_type); // 0:male normal
        bs.StoreFloat(m_height);
        bs.StoreFloat(m_physique);
        bs.StoreBits(32,skin_color); // rgb ?
    }
    void serializeToDb(QString &tgt);
    void serializeFromDb(const QString &src);
    void    dump();
protected:
};
void serializefrom(Costume &tgt, BitStream &bs, ColorAndPartPacker *packer);
void serializeto(const Costume &tgt,BitStream &bs, ColorAndPartPacker *packer);

class CharacterCostume : public Costume
{
    uint8_t  m_slot_index;
    uint64_t m_character_id; //! Character to whom this costume belongs
public:
    static CharacterCostume NullCostume;

    uint8_t     getSlotIndex() const { return m_slot_index; }
    void        setSlotIndex(uint8_t val) { m_slot_index = val; }
    uint64_t    getCharacterId() const { return m_character_id; }
    void        setCharacterId(uint64_t val) { m_character_id = val; }
};
