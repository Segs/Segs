/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */
#include <ace/ACE.h>
#include "Filesystem.h"
#include "CoXHash.h"
bool WorldData_impl::read_costumes( const std::string &directory_path )
{
    ACE_DEBUG((LM_WARNING,ACE_TEXT("(%P|%t) Reading Costumes ... ") ));
    m_costume_store.build_schema();
    bool res=m_costume_store.read(directory_path+"costume.bin");
    if(res)
        ACE_DEBUG((LM_WARNING,ACE_TEXT("Done.\n") ));
    else
        ACE_DEBUG((LM_WARNING,ACE_TEXT("Failed!\n") ));
    return res;
}

bool WorldData_impl::read_colors( const std::string &directory_path )
{
    ACE_DEBUG((LM_WARNING,ACE_TEXT("(%P|%t) Reading Supergroup colors ... ") ));
    m_supergroup_colors.build_schema();
    bool res=m_supergroup_colors.read(directory_path+"supergroupcolors.bin");
    if(res)
        ACE_DEBUG((LM_WARNING,ACE_TEXT("Done.\n") ));
    else
        ACE_DEBUG((LM_WARNING,ACE_TEXT("Failed!\n") ));
    return res;
}
void WorldData_impl::add_colors(const std::vector<BinReadable *> &clr)
{
    for(size_t idx=0; idx<clr.size(); ++idx)
    {
        ColorEntry * entry=(ColorEntry *)clr[idx];
        uint32_t color=ColorStorage::color_to_4ub(entry);
        m_colors.insert_entry(color,color);
    }

}
void WorldData_impl::fill_hashes()
{
    m_colors.init(557);
    m_strings.init(4096,0x3D);
    add_colors(m_supergroup_colors.m_colors);
    //TODO: Use reverse iterators here ?
    for(int idx=m_costume_store.m_costumes.size()-1; idx>=0; --idx)
    {
        CostumeEntry *ce=(CostumeEntry *)m_costume_store.m_costumes[idx];
        m_strings.insert_entry(ce->m_name,"");
        for(int orign_idx=ce->m_origins.size()-1; orign_idx>=0; --orign_idx)
        {
            OriginEntry *orig=(OriginEntry *)ce->m_origins[orign_idx];
            add_colors(orig->m_body_palette);
            add_colors(orig->m_skin_palette);
            for(int region_idx=orig->m_regions.size()-1; region_idx>=0; --region_idx)
            {
                RegionEntry *region=(RegionEntry *)orig->m_regions[region_idx];
                m_strings.insert_entry(region->m_name,"");
                for(int bone_idx=region->m_bonsets.size()-1; bone_idx>=0; --bone_idx)
                {
                    BoneSetEntry *bset=(BoneSetEntry *)region->m_bonsets[bone_idx];
                    m_strings.insert_entry(bset->m_name,"");
                    m_strings.insert_entry(bset->m_display_name,"");

                    for(int geo_idx=bset->m_geoset.size()-1; geo_idx>=0; --geo_idx)
                    {
                        GeoSetEntry *geo_set=(GeoSetEntry *)bset->m_geoset[geo_idx];
                        m_strings.insert_entry(geo_set->m_display_name,"");
                        m_strings.insert_entry(geo_set->m_body_part,"");
                        for(int info_idx=geo_set->m_mask_infos.size()-1; info_idx>=0; --info_idx)
                        {
                            GeoSetInfoEntry *info=(GeoSetInfoEntry *)geo_set->m_mask_infos[info_idx];
                            m_strings.insert_entry(info->m_display_name,"");
                            m_strings.insert_entry(info->m_geo_name,"");
                            m_strings.insert_entry(info->m_geo,"");
                            m_strings.insert_entry(info->m_tex1,"");
                            m_strings.insert_entry(info->m_tex2,"");
                        }
                        for(int name_idx=geo_set->m_mask_names.size()-1; name_idx>=0; --name_idx)
                        {
                            m_strings.insert_entry(geo_set->m_mask_names[name_idx],"");
                        }
                        for(int name_idx=geo_set->m_masks.size()-1; name_idx>=0; --name_idx)
                        {
                            m_strings.insert_entry(geo_set->m_masks[name_idx],"");
                        }
                        for(int mask_idx=geo_set->m_mask_vals.size()-1; mask_idx>=0; --mask_idx)
                        {
                            GeoSetMaskEntry *mask=(GeoSetMaskEntry*)geo_set->m_mask_vals[mask_idx];
                            m_strings.insert_entry(mask->m_name,"");
                            m_strings.insert_entry(mask->m_display_name,"");
                        }
                    }
                }
            }
            m_strings.insert_entry(orig->m_name,"");
        }
    }
}
