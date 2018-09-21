/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "GameCommand.h"
#include "MapEventTypes.h"
#include "DataHelpers.h"
#include "Costume.h"

namespace SEGSEvents
{

// [[ev_def:type]]
class TailorOpen final : public GameCommandEvent
{
public:
    TailorOpen() : GameCommandEvent(evTailorOpen)
    {
    }
    void    serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,type()-evFirstServerToClient); // pkt 88
    }

    EVENT_IMPL(TailorOpen)
};

// [[ev_def:type]]
class RecvCostumeChange final : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    CharacterCostume m_new_costume;

    RecvCostumeChange() : MapLinkEvent(MapEventTypes::evRecvCostumeChange)
    {
    }
    void    serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,type()-MapEventTypes::evFirstServerToClient); // 60
    }
    void    serializefrom(BitStream &bs)
    {
        receiveCostume(m_new_costume, bs);
        qCDebug(logTailor) << "Changing Costume";
    }

    EVENT_IMPL(RecvCostumeChange)
};

} // end of SEGSEvents namespace
