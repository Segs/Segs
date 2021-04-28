/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Components/Colors.h"

#include <QString>
#include <vector>

struct Parse_CostumePart
{
    QByteArray m_Name;
    RGBA m_Color1;
    RGBA m_Color2;
    QByteArray m_Texture1;
    QByteArray m_Texture2;
    QByteArray m_CP_Geometry;
};

enum class BodyType : int
{
    Male        = 0,
    Female      = 1,
    BasicMale   = 2,
    BasicFemale = 3,
    Huge        = 4,
    Enemy       = 5,
    Villain     = 6,
};

struct Parse_Costume
{
    QByteArray  m_EntTypeFile;
    QByteArray  m_CostumeFilePrefix;
    BodyType m_BodyType  = BodyType::Male;
    float    m_Scale     = 0;
    float    m_BoneScale = 0;
    RGBA     m_SkinColor;
    uint32_t m_NumParts = 0;
    std::vector<Parse_CostumePart> m_CostumeParts;
};

struct NPCPower_Desc
{
    QByteArray PowerCategory;
    QByteArray PowerSet;
    QByteArray Power;
    int Level;
    int Remove;
};

struct Parse_NPC
{
    QByteArray m_Name;
    QByteArray m_DisplayName;
    int m_Rank;
    QByteArray m_Class;
    int m_Level;
    int m_XP;
    std::vector<NPCPower_Desc> m_Powers;
    std::vector<Parse_Costume> m_Costumes;
    bool has_variant(uint32_t idx) const { return idx<m_Costumes.size(); }
};
using AllNpcs_Data = std::vector<Parse_NPC>;

BodyType bodyTypeForEntType(const QString &enttypename);
QString entTypeFileName(const Parse_Costume *costume);
QByteArray bodytype_prefix_fixup(const Parse_Costume *a1, const QByteArray &a2);
