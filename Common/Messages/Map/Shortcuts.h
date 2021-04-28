/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
// if this file is included stand-alone this will pull in common definitions
#include "MapEventTypes.h"
#include "MapSessionReference.h"
#include "Components/BitStream.h"

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
        for(uint32_t i=0; i<m_commands.size(); i++)
        {
            bs.StorePackedBits(1,i+1);
            bs.StoreString(m_commands[i]);
        }
        bs.StorePackedBits(1,~0u); // end of command list
    }
    void serializefrom(BitStream &bs) override
    {
        int idx=0;
        // while more commands available and the size is sane
        while(-1!=(idx=bs.GetPackedBits(1)) && m_commands.size()<200)
        {
            int cmd_idx = bs.GetPackedBits(1);
            assert(cmd_idx>0 && cmd_idx<200);
            m_commands.resize(std::max<int>(m_commands.size(),cmd_idx));
            bs.GetString(m_commands[cmd_idx-1]);
        }
    }
    // [[ev_def:field]]
    SessionReference m_session;
    // [[ev_def:field]]
    std::vector<QString> m_commands;
    EVENT_IMPL(Shortcuts)
};
} // end of SEGSEvents namespace

