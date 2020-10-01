/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "CommonNetStructures.h"

class Entity;

struct LFGMember
{
    uint32_t m_db_id;       // player DBID
    QString m_name;         // player name
    QString m_classname;    // player class
    QString m_origin;       // player origin
    uint32_t m_level;       // player level
    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_db_id, m_name, m_classname, m_origin, m_level);
    }
};

/*
 * LFG Methods
 */
extern std::vector<LFGMember> g_lfg_list;

void addLFG(Entity &src);
void removeLFG(Entity &src);
void dumpLFGList();
