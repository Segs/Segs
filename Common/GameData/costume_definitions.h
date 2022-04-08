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

using AllTailorCosts_Data = std::vector<TailorCost_Data>;

struct ColorEntry_Data
{
    glm::vec3 color;
};

struct Pallette_Data
{
    std::vector<ColorEntry_Data> m_Colors;
    QByteArray m_Name; // loaded, but always empty, even in I24
};

struct GeoSet_Mask_Data
{
    QByteArray              m_Name;
    QByteArray              m_DisplayName;
    std::vector<QByteArray> m_Keys;     //i24
    std::vector<QByteArray> m_Key;      //i24
    QByteArray              m_Product;  //i24
    std::vector<QByteArray> m_Tag;      //i24
    int                     m_Legacy;   //i24
    int                     m_DevOnly;  //i24
    int                     m_COV;      //i24
    int                     m_COH;      //i24
    int                     m_COHV;     //i24
};

struct GeoSet_Info_Data
{
    QByteArray              m_DisplayName;
    QByteArray              m_GeoName;
    QByteArray              m_Geo;
    QByteArray              m_Tex1;
    QByteArray              m_Tex2;
    QByteArray              m_Fx; // i24
    QByteArray              m_Product;// i24
    std::vector<QByteArray> m_Keys; // i24
    std::vector<QByteArray> m_Tag; // i24
    std::vector<QByteArray> m_Flags;// i24
    int                     m_DevOnly;
    int                     m_COV; //i24
    int                     m_COH; //i24
    int                     m_COHV; //i24
    int                     m_IsMask; //i24
    int                     m_Level; //i24
    int                     m_Legacy; //i24
};

struct CostumeFaceScaleSet
{
    QByteArray  m_DisplayName;
    glm::vec3   m_Head;
    glm::vec3   m_Brow;
    glm::vec3   m_Cheek;
    glm::vec3   m_Chin;
    glm::vec3   m_Cranium;
    glm::vec3   m_Jaw;
    glm::vec3   m_Nose;
    int         m_COV;
    int         m_COH;
};

struct GeoSet_Data
{
    QByteArray m_DisplayName;
    QByteArray m_BodyPart;
    QByteArray m_ColorLink; //i24
    std::vector<QByteArray> m_Keys; //i24
    std::vector<QByteArray> m_Flags; //i24
    QByteArray m_Product; //i24
    int m_Type;
    std::vector<QByteArray> m_AnimBits; //i24
    std::vector<QByteArray> m_ZoomBits; //i24
    glm::vec3 m_DefaultView;
    glm::vec3 m_ZoomView;
    int m_NumColor;
    int m_NoDisplay;
    std::vector<GeoSet_Info_Data> m_Infos;
    std::vector<GeoSet_Mask_Data> m_Masks;
    std::vector<QByteArray> m_MaskStrings;
    std::vector<QByteArray> m_MaskNames;
    int m_Legacy;
    std::vector<CostumeFaceScaleSet> m_Faces;
    int m_COH;
    int m_COV;

    int isOpen;
    int sel_info_idx; // m_Infos index
    int sel_mask_idx; // Mask or MaskString index
    float timing1;
    float timing2;
};

struct BoneSet_Data
{
    QByteArray m_Name;
    QByteArray m_FileName; //i24
    QByteArray m_Displayname;
    std::vector<QByteArray> m_Keys; //i24
    QByteArray m_Product; //i24
    std::vector<QByteArray> m_Flags; //i24
    std::vector<GeoSet_Data> m_GeoSets;
    int m_Legacy;
    int m_COH;
    int m_COV;
    int rs=0; // selected geoset index?
};

struct Region_Data
{
    QByteArray m_Name;
    QByteArray m_FileName; //i24
    QByteArray m_Displayname;
    std::vector<QByteArray> m_Keys; //i24
    std::vector<BoneSet_Data> m_BoneSets;
    QByteArray m_StoreCategory; //i24
    int rs=0; // selected boneset
};

struct CostumeSetSet
{
    QByteArray m_Name;
    QByteArray m_FileName;
    QByteArray m_DisplayName;
    QByteArray m_NpcName;
    std::vector<QByteArray> m_Keys;
    std::vector<QByteArray> m_StoreProductList;
};

struct CostumeGeoData
{
    QByteArray m_Name;
    QByteArray m_FileName;
    glm::vec3 m_ShieldPos;
    glm::vec3 m_ShieldPYR;
};

struct CostumeOrigin_Data
{
    QByteArray m_Name; // Name of the group Male/Female/Huge/BasicFemale ...
    QByteArray m_FileName; // source file name
    std::vector<Pallette_Data> m_BodyPalette;
    std::vector<Pallette_Data> m_SkinPalette;
    std::vector<Pallette_Data> m_PowerPalette; //i24
    std::vector<Region_Data>   m_Region;
    std::vector<CostumeSetSet> m_CostumeSets;
    std::vector<CostumeGeoData> m_GeoData;
};

struct Costume2_Data
{
    QByteArray m_Name;
    std::vector<CostumeOrigin_Data> m_Origins;
};

using CostumeSet_Data = std::vector<Costume2_Data>;
