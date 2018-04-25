/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#pragma once
#include "GameData/npc_definitions.h"
#include <QHash>

struct NPCStorage
{
    AllNpcs_Data m_all_npcs;
    QHash<QString,Parse_NPC *> m_name_to_npc_def;
    void prepare_dictionaries();
    int npc_idx(const Parse_NPC *npc) const;
    int npc_idx(const QString &name);
    const Parse_NPC *npc_by_name(const QStringRef &name) const;
};
