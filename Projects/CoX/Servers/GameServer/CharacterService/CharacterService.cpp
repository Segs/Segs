/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "CharacterService.h"
#include "GameData/Entity.h"
#include "GameData/playerdata_definitions.h"
#include "GameData/Character.h"
#include "GameData/EntityHelpers.h"
#include "Messages/Map/MapEvents.h"
#include "Servers/MapServer/DataHelpers.h"
#include <QtCore/QDebug>

using namespace SEGSEvents;

void CharacterService::on_switch_tray(Entity* ent, Event* ev)
{
    SwitchTray* casted_ev = static_cast<SwitchTray *>(ev);

    ent->m_player->m_gui.m_tray1_number = casted_ev->tray_group.m_primary_tray_idx;
    ent->m_player->m_gui.m_tray2_number = casted_ev->tray_group.m_second_tray_idx;
    ent->m_char->m_char_data.m_trays = casted_ev->tray_group;
    markEntityForDbStore(ent, DbStoreFlags::PlayerData);

   //qCDebug(logMapEvents) << "Saving Tray States to GUISettings. Tray1:" << ev->tray_group.m_primary_tray_idx+1 << "Tray2:" << ev->tray_group.m_second_tray_idx+1;
}

void CharacterService::on_levelup_response(Entity* ent, Event *ev)
{
    LevelUpResponse* casted_ev = static_cast<LevelUpResponse *>(ev);

    // if successful, set level
    if(ent->m_char->m_in_training) // if training, raise level
        increaseLevel(*ent);

    qCDebug(logMapEvents) << "Entity: " << ent->m_idx << "has received LevelUpResponse" << casted_ev->button_id << casted_ev->result;
}


