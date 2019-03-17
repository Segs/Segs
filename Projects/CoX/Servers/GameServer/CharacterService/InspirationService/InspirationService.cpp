/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "InspirationService.h"
#include "GameData/Entity.h"
#include "GameData/playerdata_definitions.h"
#include "GameData/Character.h"
#include "GameData/EntityHelpers.h"
#include "Messages/Map/MapEvents.h"
#include "Servers/MapServer/DataHelpers.h"
#include <QtCore/QDebug>

using namespace SEGSEvents;

void InspirationService::on_move_inspiration(Entity* ent, Event* ev)
{
    MoveInspiration* casted_ev = static_cast<MoveInspiration* >(ev);
    moveInspiration(ent->m_char->m_char_data, casted_ev->src_col, casted_ev->src_row,
                    casted_ev->dest_col, casted_ev->dest_row);
}

void InspirationService::on_inspiration_dockmode(Entity* ent, Event* ev)
{
    InspirationDockMode* casted_ev = static_cast<InspirationDockMode *>(ev);

    ent->m_player->m_gui.m_insps_tray_mode = casted_ev->dock_mode;
    qCDebug(logMapEvents) << "Saving inspirations dock mode to GUISettings:" << casted_ev->dock_mode;
}

ServiceToClientData* InspirationService::on_activate_inspiration(Entity* ent, Event* ev)
{
    ActivateInspiration* casted_ev = static_cast<ActivateInspiration *>(ev);

    ent->m_has_input_on_timeframe = true;
    bool success = useInspiration(*ent, casted_ev->slot_idx, casted_ev->row_idx);

    if(!success)
        return nullptr;

    std::unique_ptr<FloatingInfo> cmd = std::make_unique<FloatingInfo>(
                ent->m_idx, "Inspired!", FloatingInfoStyle::FloatingInfo_Attention, ACTIVATE_INSPIRATION_DELAY);

    std::vector<std::unique_ptr<GameCommandEvent>> cmds;
    cmds.push_back(std::move(cmd));

    return new ServiceToClientData(ent, {std::move(cmds)}, QString());
    // qCWarning(logPowers) << "Unhandled use inspiration request." << ev->row_idx << ev->slot_idx;
}
