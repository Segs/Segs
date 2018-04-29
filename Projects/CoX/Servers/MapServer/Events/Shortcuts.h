/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#pragma once
// if this file is included stand-alone this will pull in common definitions
#include "MapEvents.h"
#include "NetCommandManager.h"

class Shortcuts : public MapLinkEvent
{
public:
    Shortcuts(MapClientSession *sess):MapLinkEvent(MapEventTypes::evShortcuts),m_num_shortcuts2(0),m_client(sess)
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
    uint32_t m_num_shortcuts2;
    //vector<NetCommand *> m_commands;
    std::vector<NetCommand *> m_commands2;  // m_commands2 will get filled after we know more about them
    std::vector<std::string>  m_shortcuts2;
    MapClientSession *m_client;
};
