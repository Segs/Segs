/*
 * Super Entity Game Server Project
 * https://github.com/Segs/Segs
 * Copyright (c) 2006 - 2018 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
#include "CommonNetStructures.h"

class Entity;

struct LFGMember
{
    QString m_name;         // player name
    QString m_classname;    // player class
    QString m_origin;       // player origin
    uint32_t m_level;       // player level
};

/*
 * LFG Methods
 */
extern std::vector<LFGMember> g_lfg_list;

void addLFG(Entity &src);
void removeLFG(Entity &src);
void findTeamMember(Entity &tgt);
void dumpLFGList();
