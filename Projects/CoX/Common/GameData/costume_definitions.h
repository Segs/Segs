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
    QString m_Name;
    QString m_DisplayName;
};
struct GeoSet_Info_Data
{
    QString m_DisplayName;
    QString m_GeoName;
    QString m_Geo;
    QString m_Tex1;
    QString m_Tex2;
    int m_DevOnly;
};
struct GeoSet_Data
{
    int res;
    QString m_Displayname;
    QString m_BodyPart;
    int m_Type;
    int flag1;
    int sel_info_idx; // m_Infos index
    int sel_mask_idx; // Mask or MaskString index
    float timing1;
    float timing2;
    std::vector<QString> m_MaskStrings;
    std::vector<QString> m_MaskNames;
    std::vector<GeoSet_Mask_Data *> m_Masks;
    std::vector<GeoSet_Info_Data *> m_Infos;
};
struct BoneSet_Data
{
    QString m_Name;
    QString m_Displayname;
    int rs; // selected geoset index?
    std::vector<GeoSet_Data *> m_GeoSets;
};

struct Region_Data
{
    QString m_Name;
    QString m_Displayname;
    int rs;
    std::vector<BoneSet_Data *>m_BoneSets;
};

struct CostumeOrigin_Data
{
    QString m_Name; // Name of the group Male/Female/Huge/BasicFemale ...
    std::vector<Pallette_Data *> m_BodyPalette;
    std::vector<Pallette_Data *> m_SkinPalette;
    std::vector<Region_Data *>   m_Region;
};

struct Costume2_Data
{
    QString m_Name;
    std::vector<CostumeOrigin_Data *> m_Origins;
};

typedef std::vector<Costume2_Data> CostumeSet_Data;
