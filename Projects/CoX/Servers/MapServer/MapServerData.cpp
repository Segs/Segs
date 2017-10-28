#include "MapServerData.h"

#include "Common/GameData/DataStorage.h"
#include "Common/GameData/costume_serializers.h"
#include "NetStructures/CommonNetStructures.h"

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
    void packColor(uint32_t col, BitStream &bs)
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
    void unpackColor(BitStream &bs, uint32_t &tgt)
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
    void packPartname(const QString &str, BitStream &bs)
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
    void unpackPartname(BitStream &bs, QString &tgt)
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
}

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
    qWarning().noquote() << "Reading map data from "<<directory_path<<" folder";
    if (!read_costumes("./data/bin/"))
        return false;
    if (!read_colors("./data/bin/"))
        return false;
    qWarning().noquote() << " All map data read";
    {
        QDebug warnLine = qWarning().noquote();
        warnLine << " Postprocessing runtime data .. ";
        static_cast<HashBasedPacker *>(packer_instance)->fill_hashes(*this);
        warnLine << "Hashes filled";
    }
    return true;
}

bool MapServerData::read_costumes(const QString &directory_path)
{
    QDebug deb=qDebug().noquote();
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
    QDebug deb=qDebug().noquote();
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
