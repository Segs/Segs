/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include <glm/vec3.hpp>
#include <QtCore/QString>
#include <stdint.h>
#include <vector>

struct TailorCost_Data
{
    uint32_t m_MinLevel;
    uint32_t m_MaxLevel;
    uint32_t m_EntryFee;
    uint32_t m_Global;
    uint32_t m_HeadCost;
    uint32_t m_HeadSubCost;
    uint32_t m_UpperCost;
    uint32_t m_UpperSubCost;
    uint32_t m_LowerCost;
    uint32_t m_LoserSubCost;
    uint32_t m_NumCostumes;
};
typedef std::vector<TailorCost_Data> AllTailorCosts_Data;

struct ColorEntry_Data
{
    glm::vec3 color;
};

struct Pallette_Data
{
    std::vector<ColorEntry_Data> m_Colors;
};

struct GeoSet_Mask_Data
{
    QByteArray m_Name;
    QByteArray m_DisplayName;
};

struct GeoSet_Info_Data
{
    QByteArray m_DisplayName;
    QByteArray m_GeoName;
    QByteArray m_Geo;
    QByteArray m_Tex1;
    QByteArray m_Tex2;
    int m_DevOnly;
};

struct GeoSet_Data
{
    QByteArray m_Displayname;
    QByteArray m_BodyPart;
    int m_Type;
    int isOpen;
    int sel_info_idx; // m_Infos index
    int sel_mask_idx; // Mask or MaskString index
    float timing1;
    float timing2;
    std::vector<QByteArray> m_MaskStrings;
    std::vector<QByteArray> m_MaskNames;
    std::vector<GeoSet_Mask_Data> m_Masks;
    std::vector<GeoSet_Info_Data> m_Infos;
};

struct BoneSet_Data
{
    QByteArray m_Name;
    QByteArray m_Displayname;
    std::vector<GeoSet_Data> m_GeoSets;
    int rs=0; // selected geoset index?
};

struct Region_Data
{
    QByteArray m_Name;
    QByteArray m_Displayname;
    std::vector<BoneSet_Data> m_BoneSets;
    int rs=0; // selected boneset
};

struct CostumeOrigin_Data
{
    QByteArray m_Name; // Name of the group Male/Female/Huge/BasicFemale ...
    std::vector<Pallette_Data> m_BodyPalette;
    std::vector<Pallette_Data> m_SkinPalette;
    std::vector<Region_Data>   m_Region;
};

struct Costume2_Data
{
    QByteArray m_Name;
    std::vector<CostumeOrigin_Data> m_Origins;
};

typedef std::vector<Costume2_Data> CostumeSet_Data;
