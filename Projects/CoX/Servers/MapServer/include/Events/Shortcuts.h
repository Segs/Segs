/*
 * Super Entity Game Server Project 
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

#pragma once
// if this file is included stand-alone this will pull in common definitions
#include "MapEvents.h"
#include "NetCommandManager.h"
class Shortcuts : public MapLinkEvent
{
public:
    Shortcuts():MapLinkEvent(MapEventTypes::evShortcuts),m_num_shortcuts2(0)
    {
    }
    void serializeto(BitStream &bs) const
    {
        bs.StorePackedBits(1,4); // opcode
        NetCommandManagerSingleton::instance()->SendCommandShortcuts(m_client,bs,m_commands2);
    }
    void serializefrom(BitStream &)
    {
        assert(!"TODO");
    }
    u32 m_num_shortcuts2;
    //vector<NetCommand *> m_commands;
    std::vector<NetCommand *> m_commands2;  // m_commands2 will get filled after we know more about them
    std::vector<std::string>  m_shortcuts2;
    MapClient *m_client;
};
