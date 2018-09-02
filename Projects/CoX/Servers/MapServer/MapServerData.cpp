/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup MapServer Projects/CoX/Servers/MapServer
 * @{
 */

#include "MapServerData.h"

#include "Common/GameData/DataStorage.h"
#include "Common/GameData/costume_serializers.h"
#include "Common/GameData/def_serializers.h"
#include "Common/GameData/charclass_serializers.h"
#include "Common/GameData/keybind_serializers.h"
#include "Common/GameData/npc_serializers.h"
#include "Common/GameData/power_serializers.h"
#include "NetStructures/CommonNetStructures.h"
#include "Logging.h"

#include <QtCore/QDebug>

namespace
{
constexpr uint32_t    stringcachecount_bitlength=12;
constexpr uint32_t    colorcachecount_bitlength =10;

uint32_t color_to_4ub(const glm::vec3 &rgb)
{
    return ((uint32_t)rgb[0]) | (((uint32_t)rgb[1])<<8) | (((uint32_t)rgb[2])<<16) | (0xFF<<24);
}

class HashBasedPacker final : public ColorAndPartPacker
{
    StringHash  m_strings;
    ColorHash   m_colors;
    void add_colors(const std::vector<ColorEntry_Data> &clr)
    {
        for(size_t idx=0; idx<clr.size(); ++idx)
        {
            uint32_t color=color_to_4ub(clr[idx].color);
            m_colors.insert_entry(color,color);
        }

    }
public:
    ~HashBasedPacker() {

    }
    void fill_hashes(const MapServerData &data)
    {
        m_colors.init(557);
        m_strings.init(4096,0x3D);
        add_colors(data.m_supergroup_colors.m_Colors);
        //TODO: Use reverse iterators here ?
        for(int idx=data.m_costume_store.size()-1; idx>=0; --idx)
        {
            const Costume2_Data &ce(data.m_costume_store[idx]);
            m_strings.insert_entry(ce.m_Name,"");
            for(int orign_idx=ce.m_Origins.size()-1; orign_idx>=0; --orign_idx)
            {
                const CostumeOrigin_Data &orig(ce.m_Origins[orign_idx]);
                add_colors(orig.m_BodyPalette[0].m_Colors);
                add_colors(orig.m_SkinPalette[0].m_Colors);
                for(int region_idx=orig.m_Region.size()-1; region_idx>=0; --region_idx)
                {
                    const Region_Data &region(orig.m_Region[region_idx]);
                    m_strings.insert_entry(region.m_Name,"");
                    for(int bone_idx=region.m_BoneSets.size()-1; bone_idx>=0; --bone_idx)
                    {
                        const BoneSet_Data &bset(region.m_BoneSets[bone_idx]);
                        m_strings.insert_entry(bset.m_Name,"");
                        m_strings.insert_entry(bset.m_Displayname,"");

                        for(int geo_idx=bset.m_GeoSets.size()-1; geo_idx>=0; --geo_idx)
                        {
                            const GeoSet_Data &geo_set(bset.m_GeoSets[geo_idx]);
                            m_strings.insert_entry(geo_set.m_Displayname,"");
                            m_strings.insert_entry(geo_set.m_BodyPart,"");
                            for(int info_idx=geo_set.m_Infos.size()-1; info_idx>=0; --info_idx)
                            {
                                const GeoSet_Info_Data &info(geo_set.m_Infos[info_idx]);
                                m_strings.insert_entry(info.m_DisplayName,"");
                                m_strings.insert_entry(info.m_GeoName,"");
                                m_strings.insert_entry(info.m_Geo,"");
                                m_strings.insert_entry(info.m_Tex1,"");
                                m_strings.insert_entry(info.m_Tex2,"");
                            }
                            for(int name_idx=geo_set.m_MaskNames.size()-1; name_idx>=0; --name_idx)
                            {
                                m_strings.insert_entry(geo_set.m_MaskNames[name_idx],"");
                            }
                            for(int name_idx=geo_set.m_MaskStrings.size()-1; name_idx>=0; --name_idx)
                            {
                                m_strings.insert_entry(geo_set.m_MaskStrings[name_idx],"");
                            }
                            for(int mask_idx=geo_set.m_Masks.size()-1; mask_idx>=0; --mask_idx)
                            {
                                const GeoSet_Mask_Data &mask(geo_set.m_Masks[mask_idx]);
                                m_strings.insert_entry(mask.m_Name,"");
                                m_strings.insert_entry(mask.m_DisplayName,"");
                            }
                        }
                    }
                }
                m_strings.insert_entry(orig.m_Name,"");
            }
        }
    }
    // ColorAndPartPacker interface
public:
    void packColor(uint32_t col, BitStream &bs) const override
    {
        uint32_t cache_idx=0;
        uint32_t prev_val=0;
        if(col && m_colors.find_index(col,cache_idx,prev_val,false))
        {
            cache_idx+=1;
        }
        bs.StoreBits(1,(cache_idx||col==0));
        if(cache_idx||col==0)
        {
            bs.StorePackedBits(colorcachecount_bitlength,cache_idx);
        }
        else
        {
            bs.StoreBits(32,col);
        }

    }

    void unpackColor(BitStream &bs, uint32_t &tgt)const override
    {
        bool in_hash= bs.GetBits(1);
        if(in_hash)
        {
            uint16_t hash_idx = bs.GetBits(colorcachecount_bitlength);
            const uint32_t *kv = m_colors.key_for_idx(hash_idx);
            tgt = kv!=nullptr ? *kv : 0;
            return;
        }
        tgt = bs.GetBits(32);
    }

    void packPartname(const QString &str, BitStream &bs) const override
    {
        uint32_t cache_idx=0;
        uint32_t prev_val=0;
        if(str.size() && m_strings.find_index(str,cache_idx,prev_val,false))
        {
            cache_idx+=1;
        }
        bs.StoreBits(1,(cache_idx||str.size()==0));
        if(cache_idx||str.size()==0)
            bs.StorePackedBits(stringcachecount_bitlength,cache_idx);
        else
            bs.StoreString(str);
    }

    void unpackPartname(BitStream &bs, QString &tgt)const override
    {
        tgt.clear();
        bool in_cache= bs.GetBits(1);
        if(in_cache)
        {
            int in_cache_idx = bs.GetPackedBits(stringcachecount_bitlength);
            const QString *kv = m_strings.key_for_idx(in_cache_idx);
            if(kv)
                tgt=*kv;
            return;
        }
        bs.GetString(tgt);
    }
};

template<class TARGET,unsigned int CRC>
bool read_data_to(const QString &directory_path,const QString &storage,TARGET &target)
{
    QDebug deb=qDebug().noquote().nospace();
    deb << "Reading "<<directory_path<<storage<<" ... ";
    BinStore bin_store;
    if(!bin_store.open(directory_path+storage,CRC))
    {
        deb << "failure";
        qWarning().noquote() << "Couldn't load "<<storage<<" from" << directory_path;
        qWarning().noquote() << "Using piggtool, ensure that bin.pigg has been extracted to ./data/";
        return false;
    }

    bool res=loadFrom(&bin_store,target);
    if(res)
        deb << "OK";
    else
    {
        deb << "failure";
        qWarning().noquote() << "Couldn't load" << directory_path<<storage<<": wrong file format?";
    }
    return res;
}
///////////////////////////////////////////////////////////
} // End of anonymous namespace

MapServerData::MapServerData()
{
    packer_instance = new HashBasedPacker;
}

MapServerData::~MapServerData()
{
    delete (HashBasedPacker *)packer_instance;
    packer_instance = nullptr;
}

bool MapServerData::read_runtime_data(const QString &directory_path)
{
    qInfo().noquote() << "Reading game data from" << directory_path << "folder";

    if (!read_costumes(directory_path))
        return false;
    if (!read_colors(directory_path))
        return false;
    if (!read_origins(directory_path))
        return false;
    if (!read_classes(directory_path))
        return false;
    if(!read_exp_and_debt(directory_path))
        return false;
    if(!read_keybinds(directory_path))
        return false;
    if(!read_commands(directory_path))
        return false;
    if(!read_npcs(directory_path))
        return false;
    if(!read_powers(directory_path))
        return false;
    if(!read_combine_chances(directory_path))
        return false;
    if(!read_effectiveness(directory_path))
        return false;
    if(!read_pi_schedule(directory_path))
        return false;
    qInfo().noquote() << "Finished reading game data.";
    {
        TIMED_LOG({
                      static_cast<HashBasedPacker *>(packer_instance)->fill_hashes(*this);
                      m_npc_store.prepare_dictionaries();
                  },"Postprocessing runtime data .. ");

    }
    return true;
}

int MapServerData::expForLevel(int lev) const
{
    assert(lev>0 && lev<(int)m_experience_and_debt_per_level.m_ExperienceRequired.size());
    return m_experience_and_debt_per_level.m_ExperienceRequired.at(lev - 1);
}

int MapServerData::expDebtForLevel(int lev) const
{
    assert(lev>0 && lev<(int)m_experience_and_debt_per_level.m_DefeatPenalty.size());
    return m_experience_and_debt_per_level.m_DefeatPenalty.at(lev - 1);
}

int MapServerData::expMaxLevel()
{
    return m_experience_and_debt_per_level.m_ExperienceRequired.size();
}

int MapServerData::countForLevel(int lvl, std::vector<uint32_t> &schedule) const
{
    int i = 0;
    if(lvl < 0)
        lvl = 0;
    for(i = 0; i < schedule.size(); ++i)
    {
        if (lvl < schedule[i])
            break;
    }
    return i;
}

bool MapServerData::read_costumes(const QString &directory_path)
{
    QDebug deb=qDebug().noquote().nospace();
    deb << "Reading "<<directory_path<<"costume.bin ... ";
    BinStore costumes_store;
    if(!costumes_store.open(directory_path+"costume.bin",costumesets_i0_requiredCrc))
    {
        deb << "failure";
        qWarning().noquote() << "Couldn't load costume.bin from" << directory_path;
        qWarning().noquote() << "Using piggtool, ensure that bin.pigg has been extracted to ./data/";
        return false;
    }

    bool res=loadFrom(&costumes_store,&m_costume_store);
    if(res)
        deb << "OK";
    else
    {
        deb << "failure";
        qWarning().noquote() << "Couldn't load" << directory_path<<"costume.bin: wrong file format?";
    }
    return res;
}

bool MapServerData::read_colors( const QString &directory_path )
{
    QDebug deb=qDebug().noquote().nospace();
    deb << "Reading "<<directory_path<<"supergroupColors.bin ... ";
    BinStore sg_color_store;
    if(!sg_color_store.open(directory_path+"supergroupColors.bin",palette_i0_requiredCrc))
    {
        deb << "failure";
        qWarning().noquote() << "Couldn't load supergroupColors.bin from" << directory_path;
        qWarning().noquote() << "Using piggtool, ensure that bin.pigg has been extracted to ./data/";
        return false;
    }

    bool res=loadFrom(&sg_color_store,&m_supergroup_colors);
    if(res)
        deb << "OK";
    else
    {
        deb << "failure";
        qWarning().noquote() << "Couldn't load" << directory_path<<"supergroupColors.bin: wrong file format?";
    }
    return res;
}

bool MapServerData::read_origins(const QString &directory_path)
{
    qDebug() << "Loading origins:";
    if(!read_data_to<Parse_AllOrigins,origins_i0_requiredCrc>(directory_path,"origins.bin",m_player_origins))
        return false;
    if(!read_data_to<Parse_AllOrigins,origins_i0_requiredCrc>(directory_path,"villain_origins.bin",m_other_origins))
        return false;
    return true;
}

bool MapServerData::read_classes(const QString &directory_path)
{
    qDebug() << "Loading classes:";
    if (!read_data_to<Parse_AllCharClasses, charclass_i0_requiredCrc>(directory_path, "classes.bin", m_player_classes))
        return false;
    if (!read_data_to<Parse_AllCharClasses, charclass_i0_requiredCrc>(directory_path, "villain_classes.bin",
                                                                      m_other_classes))
        return false;
    return true;
}

bool MapServerData::read_exp_and_debt(const QString &directory_path)
{
    qDebug() << "Loading exp and debt tables:";
    if (!read_data_to<LevelExpAndDebt, levelsdebts_i0_requiredCrc>(directory_path, "experience.bin",
                                                                   m_experience_and_debt_per_level))
        return false;
    return true;
}

bool MapServerData::read_keybinds(const QString &directory_path)
{
    qDebug() << "Loading keybinds:";
    if(!read_data_to<Parse_AllKeyProfiles,keyprofile_i0_requiredCrc>(directory_path,"kb.bin",m_keybind_profiles))
        return false;
    return true;
}

bool MapServerData::read_commands(const QString &directory_path)
{
    qDebug() << "Loading commands:";
    if (!read_data_to<Parse_AllCommandCategories, keycommands_i0_requiredCrc>(directory_path, "command.bin",
                                                                              m_command_categories))
        return false;
    return true;
}

bool MapServerData::read_npcs(const QString &directory_path)
{
    qDebug() << "Loading npcs:";
    if (!read_data_to<AllNpcs_Data, npccostumesets_i0_requiredCrc>(directory_path, "VillainCostume.bin",
                                                                   m_npc_store.m_all_npcs))
        return false;
    return true;
}

bool MapServerData::read_powers(const QString &directory_path)
{
    qDebug() << "Loading powers:";
    if (!read_data_to<AllPowerCategories, powers_i0_requiredCrc>(directory_path, "powers.bin",
                                                                   m_all_powers))
        return false;
    return true;
}

bool MapServerData::read_combine_chances(const QString &directory_path)
{
    qDebug() << "Loading Combining schedule:";
    if (!read_data_to<Parse_Combining, combining_i0_requiredCrc>(directory_path, "combine_chances.bin",
                                                                   m_combine_chances))
        return false;
    if (!read_data_to<Parse_Combining, combining_i0_requiredCrc>(directory_path, "combine_same_set_chances.bin",
                                                                   m_combine_same))
        return false;
    return true;
}

bool MapServerData::read_effectiveness(const QString &directory_path)
{
    qDebug() << "Loading Enhancement Effectiveness:";
    if (!read_data_to<Parse_Effectiveness, boosteffectiveness_i0_requiredCrc>(directory_path, "boost_effect_above.bin",
                                                                   m_effectiveness_above))
        return false;
    if (!read_data_to<Parse_Effectiveness, boosteffectiveness_i0_requiredCrc>(directory_path, "boost_effect_below.bin",
                                                                   m_effectiveness_below))
        return false;
    return true;
}

bool MapServerData::read_pi_schedule(const QString &directory_path)
{
    qDebug() << "Loading PI Schedule:";
    if (!read_data_to<Parse_PI_Schedule, pischedule_i0_requiredCrc>(directory_path, "schedules.bin",
                                                                   m_pi_schedule))
        return false;
    return true;
}

const Parse_PowerSet& MapServerData::get_powerset(uint32_t pcat_idx, uint32_t pset_idx)
{
    return m_all_powers.m_categories[pcat_idx].m_PowerSets[pset_idx];
}

const Power_Data& MapServerData::get_power_template(uint32_t pcat_idx, uint32_t pset_idx, uint32_t pow_idx)
{
    return m_all_powers.m_categories[pcat_idx].m_PowerSets[pset_idx].m_Powers[pow_idx];
}

const StoredPowerCategory& MapServerData::get_power_category(uint32_t pcat_idx)
{
    return m_all_powers.m_categories[pcat_idx];
}

//! @}
