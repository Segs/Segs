/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Common/GameData/CommonNetStructures.h"
#include "Components/serialization_common.h"
#include "Components/BitStream.h"

#include <QtCore/QString>
#include <vector>

extern int g_max_num_costume_slots;

struct CostumePart
{
    enum : uint32_t {class_version = 1};
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
    uint32_t m_colors[2] = {0};
    uint8_t m_type = 0; // arms/legs etc..
    bool m_full_part;
};

void serializeto(const CostumePart &part, BitStream &bs, const ColorAndPartPacker *packingContext);
void serializefrom(CostumePart &part, BitStream &bs, const ColorAndPartPacker *packingContext);

class Costume
{
public:
    enum : uint32_t {class_version = 1};
    size_t      m_index         = 0;
    uint32_t    m_character_id  = 0; // who does this costume belong to?
    float       m_height        = 0;
    float       m_physique      = 0;
    uint32_t    m_body_type     = 0;
    uint32_t    m_skin_color    = 0;
    bool        m_send_full_costume;
    int         m_num_parts     = 0;
    std::vector<CostumePart> m_parts;

    static Costume NullCostume;

    void storeCharselParts(BitStream &bs) const;
    void storeCharsel(BitStream &bs) const;
    void serializeToDb(QString &tgt) const;
    void serializeFromDb(const QString &src);
    void dump() const;

    template<class Archive>
    void serialize(Archive &archive, uint32_t const version);
};

using vCostumes = std::vector<Costume>;

void serializefrom(Costume &tgt, BitStream &bs, const ColorAndPartPacker *packer);
void serializeto(const Costume &tgt, BitStream &bs, const ColorAndPartPacker *packer);
void dumpCostumes(vCostumes &costumes);
