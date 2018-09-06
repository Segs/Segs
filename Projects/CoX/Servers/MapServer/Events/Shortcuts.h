/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
// if this file is included stand-alone this will pull in common definitions
#include "MapEventTypes.h"
#include "NetCommandManager.h"
#include "MapSessionReference.h"

namespace SEGSEvents
{
// [[ev_def:type]]
class Shortcuts : public MapLinkEvent
{
public:
explicit Shortcuts():MapLinkEvent(MapEventTypes::evShortcuts) {}
    Shortcuts(MapClientSession *sess):MapLinkEvent(MapEventTypes::evShortcuts),m_session({0,sess})
    {
    }
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,4); // opcode
        NetCommandManagerSingleton::instance()->SendCommandShortcuts(m_session.m_client,bs);
    }
    void serializefrom(BitStream &) override
    {
        assert(!"TODO");
    }
    // [[ev_def:field]]
    SessionReference m_session;
    EVENT_IMPL(Shortcuts)
};
} // end of SEGSEvents namespace

