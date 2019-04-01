/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "EnhancementService.h"
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

void EnhancementService::on_move_enhancement(Entity* ent, Event* ev)
{
    MoveEnhancement* casted_ev = static_cast<MoveEnhancement *>(ev);
    moveEnhancement(ent->m_char->m_char_data, casted_ev->m_src_idx, casted_ev->m_dest_idx);
}

void EnhancementService::on_set_enhancement(Entity* ent, Event* ev)
{
    SetEnhancement* casted_ev = static_cast<SetEnhancement *>(ev);
    setEnhancement(*ent, casted_ev->m_pset_idx, casted_ev->m_pow_idx, casted_ev->m_src_idx, casted_ev->m_dest_idx);
}

void EnhancementService::on_trash_enhancement(Entity* ent, Event* ev)
{
    TrashEnhancement* casted_ev = static_cast<TrashEnhancement *>(ev);
    trashEnhancement(ent->m_char->m_char_data, casted_ev->m_idx);
}

void EnhancementService::on_trash_enhancement_in_power(Entity* ent, Event* ev)
{
    TrashEnhancementInPower* casted_ev = static_cast<TrashEnhancementInPower *>(ev);
    trashEnhancementInPower(ent->m_char->m_char_data, casted_ev->m_pset_idx, casted_ev->m_pow_idx, casted_ev->m_eh_idx);
}

void EnhancementService::on_buy_enhancement_slot(Entity* ent, Event* ev)
{
    BuyEnhancementSlot* casted_ev = static_cast<BuyEnhancementSlot *>(ev);
    buyEnhancementSlots(*ent, casted_ev->m_available_slots, casted_ev->m_pset_idx, casted_ev->m_pow_idx);
}

// TODO: return ServiceToClientData
void EnhancementService::on_combine_enhancements(Entity* ent, Event* ev)
{
    CombineEnhancementsReq* casted_ev = static_cast<CombineEnhancementsReq *>(ev);
    CombineResult res = combineEnhancements(*ent, casted_ev->first_power, casted_ev->second_power);
    ent->m_char->m_char_data.m_has_updated_powers = res.success || res.destroyed;

    qCDebug(logMapEvents) << "Entity: " << ent->m_idx << "wants to merge enhancements" /*<< ev->first_power << ev->second_power*/;
    // sendEnhanceCombineResponse(session, res.success, res.destroyed);
}
