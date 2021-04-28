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

void NPCStorage::prepare_dictionaries()
{
    // client expects the indices of npcs to be taken from sorted by name array
    std::sort(std::begin(m_all_npcs),std::end(m_all_npcs),[](const Parse_NPC &a,const Parse_NPC &b)->bool {
        return QString(a.m_Name).compare(b.m_Name,Qt::CaseInsensitive)<0;
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
            pc.m_NumParts = pc.m_CostumeParts.size();
        }
    }
    for(Parse_NPC &npc : m_all_npcs)
    {
        auto iter = m_name_to_npc_def.find(npc.m_Name.toLower());
        if(iter!=m_name_to_npc_def.end())
        {
            qCWarning(logNPCs) << "Duplicate NPC name" << npc.m_Name << "vs" << iter.value()->m_Name;
            continue;
        }
        m_name_to_npc_def[npc.m_Name.toLower()] = &npc;
    }
}

int NPCStorage::npc_idx(const Parse_NPC *npc) const
{
    assert(npc>=m_all_npcs.data() && npc< m_all_npcs.data()+m_all_npcs.size());
    return std::distance(m_all_npcs.data(),npc);
}

int NPCStorage::npc_idx(const QString &name)
{
    return npc_idx(m_name_to_npc_def[name.toLower()]);
}

const Parse_NPC *NPCStorage::npc_by_name(const QStringRef &name) const
{
    auto iter = m_name_to_npc_def.find(name.toString().toLower());
    if(iter!=m_name_to_npc_def.end())
        return *iter;
    return nullptr;
}

//! @}
