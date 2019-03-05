/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "PowerService.h"
#include "GameData/Powers.h"
#include "GameData/Entity.h"
#include "GameData/playerdata_definitions.h"
#include "GameData/Character.h"
#include "GameData/map_definitions.h"
#include "GameData/EntityHelpers.h"
#include "Messages/Map/MapEvents.h"
#include "Servers/MapServer/DataHelpers.h"
#include <QtCore/QDebug>

using namespace SEGSEvents;

void PowerService::on_abort_queued_power(Entity* ent, Event* ev)
{
    // unused event
    AbortQueuedPower* casted_ev = static_cast<AbortQueuedPower*>(ev);

    if(ent->m_queued_powers.isEmpty())
        return;

    // remove first queued power
    ent->m_queued_powers.dequeue();
    ent->m_char->m_char_data.m_has_updated_powers = true; // this must be true, because we're updating queued powers

    qCWarning(logMapEvents) << "Aborting queued power";
}

void PowerService::on_activate_power(Entity* ent, Event* ev)
{
    ActivatePower* casted_ev = static_cast<ActivatePower* >(ev);
    ent->m_has_input_on_timeframe = true;
    uint32_t tgt_idx = casted_ev->target_idx;

    if(casted_ev->target_idx <= 0 || casted_ev->target_idx == ent->m_idx)
        tgt_idx = -1;

    qCDebug(logPowers) << "Entity: " << ent->m_idx << "has activated power"
                       << casted_ev->pset_idx << casted_ev->pow_idx
                       << casted_ev->target_idx << casted_ev->target_db_id;

    usePower(*ent, casted_ev->pset_idx, casted_ev->pow_idx, tgt_idx, casted_ev->target_db_id);
}

// TODO: Return ServiceToClientData
void PowerService::on_activate_power_at_location(Entity* ent, Event* ev)
{
    ActivatePowerAtLocation* casted_ev = static_cast<ActivatePowerAtLocation* >(ev);
    ent->m_has_input_on_timeframe = true;

    // TODO: Check that target is valid, then Do Power!
    QString contents = QString("To Location: <%1, %2, %3>")
            .arg(casted_ev->location.x)
            .arg(casted_ev->location.y)
            .arg(casted_ev->location.z);

    //sendFloatingInfo(session, contents, FloatingInfoStyle::FloatingInfo_Attention, 4.0);
    //sendFaceLocation(session, ev->location);

    qCDebug(logPowers) << "Entity: " << ent->m_idx << "has activated power"
                       << casted_ev->pset_idx << casted_ev->pow_idx
                       << casted_ev->target_idx << casted_ev->target_db_id;
}

void PowerService::on_powers_dockmode(Entity* ent, Event* ev)
{
    PowersDockMode* casted_ev = static_cast<PowersDockMode *>(ev);
    ent->m_player->m_gui.m_powers_tray_mode = casted_ev->toggle_secondary_tray;
    //qCDebug(logMapEvents) << "Saving powers tray dock mode to GUISettings:" << ev->toggle_secondary_tray;
}

void PowerService::on_set_default_power(Entity* ent, Event* ev)
{
    SetDefaultPower* casted_ev = static_cast<SetDefaultPower *>(ev);
    PowerTrayGroup *ptray = &ent->m_char->m_char_data.m_trays;

    ptray->m_has_default_power = true;
    ptray->m_default_pset_idx = casted_ev->powerset_idx;
    ptray->m_default_pow_idx = casted_ev->power_idx;

    qCDebug(logMapEvents) << "Set Default Power:" << casted_ev->powerset_idx << casted_ev->power_idx;
}

void PowerService::on_unset_default_power(Entity* ent, Event* ev)
{
    UnsetDefaultPower* casted_ev = static_cast<UnsetDefaultPower *>(ev);
    PowerTrayGroup *ptray = &ent->m_char->m_char_data.m_trays;

    ptray->m_has_default_power = false;
    ptray->m_default_pset_idx = 0;
    ptray->m_default_pow_idx = 0;

    qCDebug(logMapEvents) << "Unset Default Power.";
}

void PowerService::on_recv_new_power(Entity* ent, Event* ev)
{
    RecvNewPower* casted_ev = static_cast<RecvNewPower *>(ev);
    addPower(ent->m_char->m_char_data, casted_ev->ppool);
}
