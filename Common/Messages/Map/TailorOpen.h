/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "GameCommand.h"
#include "MapEventTypes.h"
#include "GameData/GameDataStore.h"
#include "GameData/Costume.h"

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
        bs.StorePackedBits(1,type()-evFirstServerToClient); // packet 88
    }

    EVENT_IMPL(TailorOpen)
};

// [[ev_def:type]]
class RecvCostumeChange final : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    Costume m_new_costume;

    RecvCostumeChange() : MapLinkEvent(MapEventTypes::evRecvCostumeChange)
    {
    }
    void    serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,type()-MapEventTypes::evFirstServerToClient); // packet 60
    }
    void    serializefrom(BitStream &bs) override
    {
        ::serializefrom(m_new_costume, bs, getGameData().getPacker());
        qCDebug(logTailor) << "Changing Costume";
    }

    EVENT_IMPL(RecvCostumeChange)
};

} // end of SEGSEvents namespace
