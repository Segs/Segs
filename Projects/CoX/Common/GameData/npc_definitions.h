/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#pragma once

#include "Colors.h"

#include <QString>
#include <vector>

struct Parse_CostumePart
{
    QString m_Name;
    RGBA m_Color1;
    RGBA m_Color2;
    QString m_Texture1;
    QString m_Texture2;
    QString m_CP_Geometry;
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
    QString  m_EntTypeFile;
    QString  m_CostumeFilePrefix;
    BodyType m_BodyType  = BodyType::Male;
    float    m_Scale     = 0;
    float    m_BoneScale = 0;
    RGBA     m_SkinColor;
    uint32_t m_NumParts = 0;
    std::vector<Parse_CostumePart> m_CostumeParts;
};

struct NPCPower_Desc
{
    QString PowerCategory;
    QString PowerSet;
    QString Power;
    int Level;
    int Remove;
};

struct Parse_NPC
{
    QString m_Name;
    QString m_DisplayName;
    int m_Rank;
    QString m_Class;
    int m_Level;
    int m_XP;
    std::vector<NPCPower_Desc> m_Powers;
    std::vector<Parse_Costume> m_Costumes;
    bool has_variant(size_t idx) const { return idx<m_Costumes.size(); }
};
using AllNpcs_Data = std::vector<Parse_NPC>;
