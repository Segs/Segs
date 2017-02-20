/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */
#include "WorldData.h"
#include "DataStorage.h"
#include "Common/GameData/costume_serializers.h"

#include <ace/ACE.h>
#include <QtCore/QString>

namespace
{
static uint32_t color_to_4ub(const glm::vec3 &rgb)
{
    return ((uint32_t)rgb[0]) | (((uint32_t)rgb[1])<<8) | (((uint32_t)rgb[2])<<16) | (0xFF<<24);
}
}
bool WorldData_impl::read_costumes( const QString &directory_path )
{
    ACE_DEBUG((LM_WARNING,ACE_TEXT("(%P|%t) Reading Costumes ... ") ));
    BinStore costumes_store;
    if(!costumes_store.open(directory_path+"costume.bin",costumesets_i0_requiredCrc)) {
        ACE_DEBUG((LM_WARNING,ACE_TEXT("(%P|%t) Couldn't load Costume!!\n") ));
    }

    bool res=loadFrom(&costumes_store,&m_costume_store);
    if(res)
        ACE_DEBUG((LM_WARNING,ACE_TEXT("Done.\n") ));
    else
        ACE_DEBUG((LM_WARNING,ACE_TEXT("Failed!\n") ));
    return res;
}

bool WorldData_impl::read_colors( const QString &directory_path )
{
    ACE_DEBUG((LM_WARNING,ACE_TEXT("(%P|%t) Reading Supergroup colors ... ") ));
    BinStore sg_color_store;
    if(!sg_color_store.open(directory_path+"supergroupColors.bin",palette_i0_requiredCrc)) {
        ACE_DEBUG((LM_WARNING,ACE_TEXT("(%P|%t) Couldn't load Supergroup colors!!\n") ));
    }

    bool res=loadFrom(&sg_color_store,&m_supergroup_colors);
    if(res)
        ACE_DEBUG((LM_WARNING,ACE_TEXT("Done.\n") ));
    else
        ACE_DEBUG((LM_WARNING,ACE_TEXT("Failed!\n") ));
    return res;
}
void WorldData_impl::add_colors(const std::vector<ColorEntry_Data> &clr)
{
    for(size_t idx=0; idx<clr.size(); ++idx)
    {
        uint32_t color=color_to_4ub(clr[idx].color);
        m_colors.insert_entry(color,color);
    }

}
void WorldData_impl::fill_hashes()
{
    m_colors.init(557);
    m_strings.init(4096,0x3D);
    add_colors(m_supergroup_colors.m_Colors);
    //TODO: Use reverse iterators here ?
    for(int idx=m_costume_store.size()-1; idx>=0; --idx)
    {
        Costume2_Data &ce(m_costume_store[idx]);
        m_strings.insert_entry(ce.m_Name,"");
        for(int orign_idx=ce.m_Origins.size()-1; orign_idx>=0; --orign_idx)
        {
            CostumeOrigin_Data &orig(ce.m_Origins[orign_idx]);
            add_colors(orig.m_BodyPalette[0].m_Colors);
            add_colors(orig.m_SkinPalette[0].m_Colors);
            for(int region_idx=orig.m_Region.size()-1; region_idx>=0; --region_idx)
            {
                Region_Data &region(orig.m_Region[region_idx]);
                m_strings.insert_entry(region.m_Name,"");
                for(int bone_idx=region.m_BoneSets.size()-1; bone_idx>=0; --bone_idx)
                {
                    BoneSet_Data &bset(region.m_BoneSets[bone_idx]);
                    m_strings.insert_entry(bset.m_Name,"");
                    m_strings.insert_entry(bset.m_Displayname,"");

                    for(int geo_idx=bset.m_GeoSets.size()-1; geo_idx>=0; --geo_idx)
                    {
                        GeoSet_Data &geo_set(bset.m_GeoSets[geo_idx]);
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
                            GeoSet_Mask_Data &mask(geo_set.m_Masks[mask_idx]);
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
