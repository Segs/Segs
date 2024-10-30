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

#include "GameDataStore.h"
#include "Common/GameData/trick_definitions.h"

#include "Common/GameData/DataStorage.h"
#include "Common/GameData/costume_serializers.h"
#include "Common/GameData/def_serializers.h"
#include "Common/GameData/charclass_serializers.h"
#include "Common/GameData/keybind_serializers.h"
#include "Common/GameData/npc_serializers.h"
#include "Common/GameData/fx_definitions.h"
#include "Common/GameData/fx_serializers.h"
#include "Common/GameData/power_serializers.h"
#include "Common/GameData/trick_serializers.h"
#include "Common/GameData/seq_serializers.h"
#include "Common/GameData/shop_serializers.h"
#include "Common/GameData/shop_definitions.h"
#include "Common/GameData/bodypart_serializers.h"
#include "Common/GameData/CommonNetStructures.h"
#include "Common/Utils/ElapsedTimer.h"
#include "Components/Logging.h"
#include "Components/Settings.h"

#include "Containers/DateTime.h"
#include "Containers/String.h"
#include <EASTL/sort.h>


namespace
{
constexpr uint32_t    stringcachecount_bitlength=12;
constexpr uint32_t    colorcachecount_bitlength =10;
constexpr int    minimumTicksPerSecond = 1;
constexpr int    maximumTicksPerSecond = 1000;

uint32_t color_to_4ub(const glm::vec3 &rgb)
{
    return ((uint32_t)rgb[0]) | (((uint32_t)rgb[1])<<8) | (((uint32_t)rgb[2])<<16) | (0xFFu<<24);
}
class IndexedPacker final : public IndexedStringPacker
{
    Vector<String> m_known_strings;
    HashMap<String,int> m_string_to_index;

public:
    void sortEntries() {
        eastl::sort(m_known_strings.begin(),m_known_strings.end(),[](const String &a,const String &b)->bool {
            // all added strings have been lower-cased, so no need to case-insensitive compare here.
            return a.compare(b)<0;
        });
        // record the new order in map.
        int i=1;
        for(const String &str : m_known_strings)
            m_string_to_index[str] = i++;
    }

    // IndexedStringPacker interface
    void addString(const String &str)
    {
        int idx = m_string_to_index.at(str.to_lower(),0);
        if(idx)
        {
            assert(0==m_known_strings[idx-1].comparei(str));
            return;
        }
        m_known_strings.push_back(str.to_lower());
        m_string_to_index[str.to_lower()] = (int)m_known_strings.size();
    }
    int getIndex(const String &str) const
    {
        return m_string_to_index.at(str.to_lower(),0);
    }
};

class HashBasedPacker final : public ColorAndPartPacker
{
    StringHash  m_strings;
    ColorHash   m_colors;
    void add_colors(const Vector<ColorEntry_Data> &clr)
    {
        for(auto idx : clr)
        {
            uint32_t color=color_to_4ub(idx.color);
            m_colors.insert_entry(color,color);
        }
    }
public:
    ~HashBasedPacker() = default;
    void fill_hashes(const GameDataStore &data)
    {
        m_colors.init(557);
        m_strings.init(4096,0x3D);
        add_colors(data.m_supergroup_colors.m_Colors);
        //TODO: Use reverse iterators here ?
        for(int idx=int(data.m_costume_store.size()-1); idx>=0; --idx)
        {
            const Costume2_Data &ce(data.m_costume_store[idx]);
            m_strings.insert_entry(ce.m_Name,"");
            for(int orign_idx=int(ce.m_Origins.size()-1); orign_idx>=0; --orign_idx)
            {
                const CostumeOrigin_Data &orig(ce.m_Origins[orign_idx]);
                add_colors(orig.m_BodyPalette[0].m_Colors);
                add_colors(orig.m_SkinPalette[0].m_Colors);
                for(int region_idx=int(orig.m_Region.size()-1); region_idx>=0; --region_idx)
                {
                    const Region_Data &region(orig.m_Region[region_idx]);
                    m_strings.insert_entry(region.m_Name,"");
                    for(int bone_idx=int(region.m_BoneSets.size()-1); bone_idx>=0; --bone_idx)
                    {
                        const BoneSet_Data &bset(region.m_BoneSets[bone_idx]);
                        m_strings.insert_entry(bset.m_Name,"");
                        m_strings.insert_entry(bset.m_Displayname,"");

                        for(int geo_idx=int(bset.m_GeoSets.size()-1); geo_idx>=0; --geo_idx)
                        {
                            const GeoSet_Data &geo_set(bset.m_GeoSets[geo_idx]);
                            m_strings.insert_entry(geo_set.m_Displayname,"");
                            m_strings.insert_entry(geo_set.m_BodyPart,"");
                            for(int info_idx=int(geo_set.m_Infos.size()-1); info_idx>=0; --info_idx)
                            {
                                const GeoSet_Info_Data &info(geo_set.m_Infos[info_idx]);
                                m_strings.insert_entry(info.m_DisplayName,"");
                                m_strings.insert_entry(info.m_GeoName,"");
                                m_strings.insert_entry(info.m_Geo,"");
                                m_strings.insert_entry(info.m_Tex1,"");
                                m_strings.insert_entry(info.m_Tex2,"");
                            }
                            for(int name_idx=int(geo_set.m_MaskNames.size()-1); name_idx>=0; --name_idx)
                            {
                                m_strings.insert_entry(geo_set.m_MaskNames[name_idx],"");
                            }
                            for(int name_idx=int(geo_set.m_MaskStrings.size())-1; name_idx>=0; --name_idx)
                            {
                                m_strings.insert_entry(geo_set.m_MaskStrings[name_idx],"");
                            }
                            for(int mask_idx=int(geo_set.m_Masks.size()-1); mask_idx>=0; --mask_idx)
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

    void packPartname(const String &str, BitStream &bs) const override
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

    void unpackPartname(BitStream &bs, String &tgt)const override
    {
        tgt.clear();
        bool in_cache= bs.GetBits(1);
        if(in_cache)
        {
            int in_cache_idx = bs.GetPackedBits(stringcachecount_bitlength);
            const String *kv = m_strings.key_for_idx(in_cache_idx);
            if(kv)
                tgt=*kv;
            return;
        }
        bs.GetString(tgt);
    }
};

template<class TARGET,unsigned int CRC>
bool read_data_to(const String &directory_path,const String &storage,TARGET &target)
{
    SEGS::ElapsedTimer timer;
    DebugOutput deb(sDebug());
    deb << "Reading "<<directory_path<<storage<<" ... ";
    timer.start();
    BinStore bin_store;
    if(!bin_store.open(directory_path+storage,CRC))
    {
        deb << "failure";
        sWarning() << "Couldn't load "<<storage<<" from" << directory_path;
        sWarning() << "Using piggtool, ensure that bin.pigg has been extracted to ./data/";
        return false;
    }

    bool res=loadFrom(&bin_store,target);
    if(res)
        deb << " OK in "<<StringUtils::num_scientific(float(timer.elapsed())/1000.0f,4)<<"s";
    else
    {
        deb << "failure";
        sWarning() << "Couldn't load" << directory_path<<storage<<": wrong file format?";
    }
    return res;
}
///////////////////////////////////////////////////////////
} // End of anonymous namespace

GameDataStore::GameDataStore()
{
    static bool was_created = false;
    if(!was_created)
        was_created = true;
    else
    {
        sCritical() << "Multiple instances of GameDataStore created in a single process, expect trouble";
    }
    packer_instance = new HashBasedPacker;
    m_index_based_packer = new IndexedPacker;
}

GameDataStore::~GameDataStore()
{
    delete (HashBasedPacker *)packer_instance;
    delete (IndexedPacker *)m_index_based_packer;
    packer_instance = nullptr;
}

bool GameDataStore::read_game_data(const String &directory_path)
{
    sInfo() << "Reading game data from" << directory_path << "folder";
    SEGS::ElapsedTimer load_timer;
    load_timer.start();

    if(!read_costumes(directory_path))
        return false;
    if(!read_colors(directory_path))
        return false;
    if(!read_origins(directory_path))
        return false;
    if(!read_classes(directory_path))
        return false;
    if(!read_exp_and_debt(directory_path))
        return false;
    if(!read_keybinds(directory_path))
        return false;
    if(!read_commands(directory_path))
        return false;
    if(!read_npcs(directory_path))
        return false;
    if(!read_settings(directory_path))
        return false;
    if(!read_powers(directory_path))
        return false;
    if(!read_combine_chances(directory_path))
        return false;
    if(!read_effectiveness(directory_path))
        return false;
    if(!read_pi_schedule(directory_path))
        return false;
    if(!read_fx(directory_path))
        return false;
    if(!read_sequencer_definitions(directory_path))
        return false;
    if(!read_store_data(directory_path))
        return false;
    if(!read_store_items_data(directory_path))
        return false;
    if(!read_store_depts_data(directory_path)) //Not needed?
        return false;
    if(!read_sequencer_types(directory_path))
        return false;
    if(!read_body_parts(directory_path))
        return false;
    sInfo() << "Finished reading game data:  done in"<<float(load_timer.elapsed())/1000.0f<<"s";
    {
        TIMED_LOG({
                      static_cast<HashBasedPacker *>(packer_instance)->fill_hashes(*this);
                      m_npc_store.prepare_dictionaries();
                      auto packer = static_cast<IndexedPacker *>(m_index_based_packer);
                      int idx=0;
                      for(const FxInfo &fx : m_fx_infos)
                      {
                          packer->addString(fx.fxname);
                          m_name_to_fx_index[fx.fxname.to_lower()] = idx++;
                      }
                      packer->sortEntries();
                  },"Postprocessing runtime data .. ");

    }
    return true;
}

uint32_t GameDataStore::expForLevel(uint32_t lev) const
{
    lev = std::max<uint32_t>(0,std::min<uint32_t>(expMaxLevel(), lev));
    return m_experience_and_debt_per_level.m_ExperienceRequired.at(lev);
}

uint32_t GameDataStore::expDebtForLevel(uint32_t lev) const
{
    lev = std::max<uint32_t>(0,std::min<uint32_t>(expMaxLevel(), lev));
    return m_experience_and_debt_per_level.m_DefeatPenalty.at(lev);
}

uint32_t GameDataStore::expMaxLevel() const
{
    // return -1 because level is stored in indexed array (starting 0)
    return uint32_t(m_experience_and_debt_per_level.m_ExperienceRequired.size()-1);
}

uint32_t GameDataStore::countForLevel(uint32_t lvl, const Vector<uint32_t> &schedule) const
{
    uint32_t i = 0;

    for(i = 0; i < schedule.size(); ++i)
    {
        if(lvl < schedule[i])
            break; // i must pass through for values at the end of schedule array
    }

    return i;
}

FxInfo *GameDataStore::getFxInfoByName(const String &name)
{
    int idx = m_name_to_fx_index.at(name.to_lower(),-1);
    if(idx==-1)
        return nullptr;
    return m_fx_infos.data()+idx;
}

bool GameDataStore::read_costumes(const String &directory_path)
{
    auto deb(sDebug());
    deb << "Reading " << directory_path << "bin/costume.bin ... ";
    BinStore costumes_store;
    if(!costumes_store.open(directory_path + "bin/costume.bin", costumesets_i0_requiredCrc))
    {
        deb << "failure";
        sWarning()<< "Couldn't load bin/costume.bin from" << directory_path;
        sWarning()<< "Using piggtool, ensure that bin.pigg has been extracted to ./data/bin/";
        return false;
    }

    bool res=loadFrom(&costumes_store,&m_costume_store);
    if(res)
        deb << "OK";
    else
    {
        deb << "failure";
        sWarning()<< "Couldn't load" << directory_path << "bin/costume.bin: wrong file format?";
    }
    return res;
}

bool GameDataStore::read_colors( const String &directory_path )
{
    auto deb(sDebug());
    deb << "Reading " << directory_path << "bin/supergroupColors.bin ... ";
    BinStore sg_color_store;

    if(!sg_color_store.open(directory_path + "bin/supergroupColors.bin", palette_i0_requiredCrc))
    {
        deb << "failure";
        sWarning()<< "Couldn't load bin/supergroupColors.bin from" << directory_path;
        sWarning()<< "Using piggtool, ensure that bin.pigg has been extracted to ./data/bin/";
        return false;
    }

    bool res=loadFrom(&sg_color_store,&m_supergroup_colors);
    if(res)
        deb << "OK";
    else
    {
        deb << "failure";
        sWarning()<< "Couldn't load" << directory_path << "bin/supergroupColors.bin: wrong file format?";
    }
    return res;
}

bool GameDataStore::read_origins(const String &directory_path)
{
    sDebug() << "Loading origins:";
    if(!read_data_to<Parse_AllOrigins,origins_i0_requiredCrc>(directory_path, "bin/origins.bin", m_player_origins))
        return false;
    if(!read_data_to<Parse_AllOrigins,origins_i0_requiredCrc>(directory_path, "bin/villain_origins.bin", m_other_origins))
        return false;
    return true;
}

bool GameDataStore::read_classes(const String &directory_path)
{
    sDebug() << "Loading classes:";
    if(!read_data_to<Parse_AllCharClasses, charclass_i0_requiredCrc>(directory_path, "bin/classes.bin", m_player_classes))
        return false;
    if(!read_data_to<Parse_AllCharClasses, charclass_i0_requiredCrc>(directory_path, "bin/villain_classes.bin",
                                                                      m_other_classes))
        return false;
    return true;
}

bool GameDataStore::read_exp_and_debt(const String &directory_path)
{
    sDebug() << "Loading exp and debt tables:";
    if(!read_data_to<LevelExpAndDebt, levelsdebts_i0_requiredCrc>(directory_path, "bin/experience.bin",
                                                                   m_experience_and_debt_per_level))
        return false;
    return true;
}

bool GameDataStore::read_keybinds(const String &directory_path)
{
    sDebug() << "Loading keybinds:";
    if(!read_data_to<Parse_AllKeyProfiles,keyprofile_i0_requiredCrc>(directory_path, "bin/kb.bin", m_keybind_profiles))
        return false;
    return true;
}

bool GameDataStore::read_commands(const String &directory_path)
{
    sDebug() << "Loading commands:";
    if(!read_data_to<Parse_AllCommandCategories, keycommands_i0_requiredCrc>(directory_path, "bin/command.bin",
                                                                              m_command_categories))
        return false;
    return true;
}

bool GameDataStore::read_npcs(const String &directory_path)
{
    sDebug() << "Loading npcs:";
    return read_data_to<AllNpcs_Data, npccostumesets_i0_requiredCrc>(directory_path, "bin/VillainCostume.bin",
                                                                   m_npc_store.m_all_npcs);
}

bool GameDataStore::read_settings(const String &/*directory_path*/)
{
    Settings config(Settings::getSettingsPath());

    sInfo() << "Loading AFK settings...";
    config.beginGroup("AFK Settings");
        m_time_to_afk = config.value<int>("time_to_afk", 300);
        m_time_to_logout_msg = config.value<int>("time_to_logout_msg", 1080);
        m_time_to_auto_logout = config.value<int>("time_to_auto_logout", 120);
        m_uses_auto_logout = config.value("uses_auto_logout", true);
    config.endGroup(); // AFK Settings

    sInfo() << "Loading Modifier settings...";
    config.beginGroup(("Modifiers"));
        m_uses_xp_mod = config.value("uses_xp_mod", false);
        m_xp_mod_multiplier = config.value("xp_mod_multiplier", 1.0);
        m_xp_mod_startdate = StringUtils::parseDate("%m/%d/%Y %I:%M %p",config.value<String>("xp_mod_startdate", ""));
        m_xp_mod_enddate = StringUtils::parseDate("%m/%d/%Y %I:%M %p",config.value<String>("xp_mod_enddate", ""));
    config.endGroup(); // Modifiers

    sInfo() << "Loading Experimental settings...";
    config.beginGroup(("Experimental"));

    // constrain to a reasonable range
    int ticks = config.value("world_update_ticks_per_sec", 30);
    m_world_update_ticks_per_sec = std::min(std::max(ticks, minimumTicksPerSecond), maximumTicksPerSecond);

    config.endGroup(); // Experiemental

    return true;
}

bool GameDataStore::read_powers(const String &directory_path)
{
    auto fs = SEGS::getServiceLocator()->getFS();
    sDebug() << "Loading powers:";

    if(fs->exists(directory_path+"powers.json") && loadFrom(directory_path+"powers.json", m_all_powers))
    {
        sDebug() << "Loaded power data from powers.json!";
            return true;
    }
    else if(read_data_to<AllPowerCategories, powers_i0_requiredCrc>(directory_path,
                                                                    "bin/powers.bin",m_all_powers))
    {
        sDebug() << "Loaded power data from powers.bin!";
    }
    else
        return false;

    return true;
}

bool GameDataStore::read_combine_chances(const String &directory_path)
{
    sDebug() << "Loading Combining schedule:";
    if(!read_data_to<Parse_Combining, combining_i0_requiredCrc>(directory_path, "bin/combine_chances.bin",
                                                                   m_combine_chances))
        return false;
    if(!read_data_to<Parse_Combining, combining_i0_requiredCrc>(directory_path, "bin/combine_same_set_chances.bin",
                                                                   m_combine_same))
        return false;
    return true;
}

bool GameDataStore::read_effectiveness(const String &directory_path)
{
    sDebug() << "Loading Enhancement Effectiveness:";
    if(!read_data_to<Parse_Effectiveness, boosteffectiveness_i0_requiredCrc>(directory_path, "bin/boost_effect_above.bin",
                                                                   m_effectiveness_above))
        return false;
    if(!read_data_to<Parse_Effectiveness, boosteffectiveness_i0_requiredCrc>(directory_path, "bin/boost_effect_below.bin",
                                                                   m_effectiveness_below))
        return false;
    return true;
}

bool GameDataStore::read_pi_schedule(const String &directory_path)
{
    sDebug() << "Loading PI Schedule:";
    return read_data_to<Parse_PI_Schedule, pischedule_i0_requiredCrc>(directory_path, "bin/schedules.bin",
                                                                      m_pi_schedule);
}

bool GameDataStore::read_fx(const String &directory_path)
{
    sDebug() << "Loading FX Information:";
    return read_data_to<Vector<struct FxInfo>, fxinfos_i0_requiredCrc>(directory_path, "bin/fxinfo.bin",
                                                                            m_fx_infos);
}

bool GameDataStore::read_sequencer_definitions(const String &directory_path)
{
    sDebug() << "Loading Sequencer Information:";
    return read_data_to<SequencerList, seqencerlist_i0_requiredCrc>(directory_path, "bin/sequencers.bin",m_seq_definitions);
}

bool GameDataStore::read_store_data(const String &directory_path)
{
    sDebug() << "Loading shop data:";
    return read_data_to<AllShops_Data, shoplist_i0_requiredCrc>(directory_path, "bin/stores.bin", m_shops_data);
}

bool GameDataStore::read_store_items_data(const String &directory_path)
{
    sDebug() << "Loading shop items:";
    return read_data_to<AllShopItems_Data, shopitems_i0_requiredCrc>(directory_path, "bin/items.bin", m_shop_items_data);
}

bool GameDataStore::read_store_depts_data(const String &directory_path)
{
    sDebug() << "Loading shop depts:";
    return read_data_to<AllShopDepts_Data, shopdepts_i0_requiredCrc>(directory_path, "bin/depts.bin", m_shop_depts_data);
}
bool GameDataStore::read_sequencer_types(const String &directory_path)
{
    SEGS::ElapsedTimer timer;

    sDebug() << "Loading Seq types:";

    auto deb(sDebug());

    bool res=loadFrom(directory_path+"converted/ent_types.crl.json",m_seq_types);
    if(res)
        deb << " OK in "<<StringUtils::num_scientific(float(timer.elapsed())/1000.0f,4)<<"s";
    else
    {
        deb << "failure";
        sWarning() << "Couldn't load" << directory_path<<"ent_types.crl_json: wrong file format?";
    }
    return res;
}

bool GameDataStore::read_body_parts(const String &directory_path)
{
    sDebug() << "Loading body parts:";
    bool res =
        read_data_to<BodyPartsStorage, bodyparts_i0_requiredCrc>(directory_path, "bin/BodyParts.bin", m_body_parts);
    if(res)
        m_body_parts.postProcess();
    return res;
}

const Parse_PowerSet& GameDataStore::get_powerset(uint32_t pcat_idx, uint32_t pset_idx)
{
    return m_all_powers.m_categories.at(pcat_idx).m_PowerSets.at(pset_idx);
}

const Power_Data& GameDataStore::get_power_template(uint32_t pcat_idx, uint32_t pset_idx, uint32_t pow_idx)
{
    return m_all_powers.m_categories.at(pcat_idx).m_PowerSets.at(pset_idx).m_Powers.at(pow_idx);
}

Power_Data * GameDataStore::editable_power_tpl(uint32_t pcat_idx, uint32_t pset_idx, uint32_t pow_idx)
{
    return &m_all_powers.m_categories[pcat_idx].m_PowerSets[pset_idx].m_Powers[pow_idx];
}

int GameDataStore::getFxNamePackId(const String &name)
{
    return m_index_based_packer->getIndex(name);
}

const StoredPowerCategory& GameDataStore::get_power_category(uint32_t pcat_idx)
{
    return m_all_powers.m_categories.at(pcat_idx);
}

int getEntityOriginIndex(const GameDataStore &data, bool is_player, StringView origin_name)
{
    const Parse_AllOrigins &origins_to_search(is_player ? data.m_player_origins : data.m_other_origins);

    int idx = 0;
    for(const Parse_Origin &orig : origins_to_search)
    {
        if(StringUtils::compare(origin_name,orig.Name,StringUtils::CaseInsensitive)==0)
            return idx;
        idx++;
    }
    sCDebug(logNpcSpawn) << "Failed to locate origin index for" << origin_name;
    return -1;
}

int getEntityClassIndex(const GameDataStore &data, bool is_player, StringView class_name)
{
    const Parse_AllCharClasses &classes_to_search(is_player ? data.m_player_classes : data.m_other_classes);

    int idx = 0;
    for(const CharClass_Data &classdata : classes_to_search)
    {
        if(StringUtils::compare(class_name,classdata.m_Name,StringUtils::CaseInsensitive)==0)
            return idx;
        idx++;
    }
    sCDebug(logNpcSpawn) << "Failed to locate class index for" << class_name;
    return -1;
}

GameDataStore &getGameData() {
    static GameDataStore instance;
    return instance;
}

//! @}
