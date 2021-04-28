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

#include "LFG.h"

#include "Servers/MapServer/DataHelpers.h"
#include "Entity.h"
#include "Components/Logging.h"
#include "Character.h"

/*
 * LFG Methods
 */
std::vector<LFGMember> g_lfg_list;

void addLFG(Entity &src)
{
    src.m_char->m_char_data.m_lfg = true;

    LFGMember list;
    list.m_name         = src.name();
    list.m_classname    = src.m_char->m_char_data.m_class_name;
    list.m_origin       = src.m_char->m_char_data.m_origin_name;
    list.m_level        = src.m_char->m_char_data.m_level;

    g_lfg_list.emplace_back(list);
}

void removeLFG(Entity &tgt)
{
    tgt.m_char->m_char_data.m_lfg = false;
    // remove by name
    QString name_to_find = tgt.name();
    auto iter = std::find_if( g_lfg_list.begin(), g_lfg_list.end(),
                              [name_to_find](const LFGMember& list)->bool {return name_to_find==list.m_name;});
    if(iter!=g_lfg_list.end())
    {
        iter = g_lfg_list.erase(iter);

        qCDebug(logLFG) << "Removing" << iter->m_name << "from LFG List";
        if(logLFG().isDebugEnabled())
            dumpLFGList();
    }
}

void dumpLFGList()
{
    QString output = "LFG List:" + QString::number(g_lfg_list.size());

    for (auto &m : g_lfg_list)
    {
        output += "\n\t" + m.m_name
                + " " + m.m_classname
                + " " + m.m_origin
                + " " + QString::number(m.m_level);
    }

    qDebug().noquote() << output;
}

//! @}
