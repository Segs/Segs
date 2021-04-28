/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "GameCommand.h"
#include "MapEventTypes.h"
#include "Components/BitStream.h"

namespace SEGSEvents
{
// [[ev_def:type]]
class ForceLogout final : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    QString reason;
    ForceLogout() :MapLinkEvent(evForceLogout) {}
    ForceLogout(const QString &_reason) :MapLinkEvent(evForceLogout),reason(_reason)
    {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1, 12); //packet 12
        bs.StoreString(reason);
    }
    void serializefrom(BitStream &/*bs*/) override
    {
        qWarning() << "Server does not receive ForceLogout packet from Client.";
    }
    EVENT_IMPL(ForceLogout)
};

}
