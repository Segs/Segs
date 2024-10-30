/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup MapServer Projects/CoX/Servers/MapServer
 * @{
 */

#include "NpcStore.h"

#include "npc_definitions.h"
#include "Components/Logging.h"
#include "EASTL/sort.h"

void NPCStorage::prepare_dictionaries()
{
    // client expects the indices of npcs to be taken from sorted by name array
    eastl::sort(eastl::begin(m_all_npcs),eastl::end(m_all_npcs),[](const Parse_NPC &a,const Parse_NPC &b)->bool {
        return String(a.m_Name).comparei(b.m_Name)<0;
    });
    // fixup the npc body type,colors, and costume part counts
    for (Parse_NPC & pnpc : m_all_npcs )
    {
        for (Parse_Costume & pc : pnpc.m_Costumes)
        {
            pc.m_BodyType = bodyTypeForEntType(pc.m_EntTypeFile);
            for (Parse_CostumePart &pcp : pc.m_CostumeParts)
            {
                pcp.m_Color1.rgba.a = 255;
                pcp.m_Color2.rgba.a = 255;
            }
            pc.m_NumParts = (uint32_t)pc.m_CostumeParts.size();
        }
    }
    for(Parse_NPC &npc : m_all_npcs)
    {
        auto iter = m_name_to_npc_def.find(npc.m_Name.to_lower());
        if(iter!=m_name_to_npc_def.end())
        {
            sCWarning(logNPCs) << "Duplicate NPC name" << npc.m_Name << "vs" << iter->second->m_Name;
            continue;
        }
        m_name_to_npc_def[npc.m_Name.to_lower()] = &npc;
    }
}

int NPCStorage::npc_idx(const Parse_NPC *npc) const
{
    assert(npc>=m_all_npcs.data() && npc< m_all_npcs.data()+m_all_npcs.size());
    return (int)eastl::distance(m_all_npcs.data(),npc);
}

int NPCStorage::npc_idx(const String &name)
{
    return npc_idx(m_name_to_npc_def[name.to_lower()]);
}

const Parse_NPC *NPCStorage::npc_by_name(StringView name) const
{
    auto iter =
        m_name_to_npc_def.find_as(String(name), m_name_to_npc_def.hash_function(), [](const String &k, const String &ot) -> bool {
        return StringUtils::compare(k,ot,StringUtils::CaseInsensitive);
    });
    if(iter!=m_name_to_npc_def.end())
        return iter->second;
    return nullptr;
}

//! @}
