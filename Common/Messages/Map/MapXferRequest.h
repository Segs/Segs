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
#include <QtCore/QString>

namespace SEGSEvents
{

// [[ev_def:type]]
class MapXferRequest final : public MapLinkEvent
{
public:
        MapXferRequest() : MapLinkEvent(MapEventTypes::evMapXferRequest)
        {
            unused1 = unused2 = unused3 = unused4 = 0;
        }

        void serializeto(BitStream &bs) const override
        {
            bs.StorePackedBits(1, 11); // opcode
            uint32_t ipaddr = htonl(m_address.get_ip_address());
            uint16_t port   = m_address.get_port_number();
            bs.StorePackedBits(1,unused1);
            bs.StorePackedBits(1,unused2);
            bs.StorePackedBits(1,ipaddr);
            bs.StorePackedBits(1,unused3);
            bs.StorePackedBits(1,port);
            bs.StorePackedBits(1,unused4);
            bs.StorePackedBits(1,m_map_cookie);
        }
        void serializefrom(BitStream &/*src*/) override
        {
            assert(false);
        }

        // [[ev_dev:field]]
        uint8_t unused1;
        // [[ev_def:field]]
        uint8_t unused2;
        // [[ev_def:field]]
        uint8_t unused3;
        // [[ev_def:field]]
        uint8_t unused4;
        // [[ev_def:field]]
        ACE_INET_Addr m_address;
        // 0 - Name already taken.
        // 1 - Problem detected in the game database system
        // [[ev_def:field]]
        uint32_t m_map_cookie;

        EVENT_IMPL(MapXferRequest)
};

}
