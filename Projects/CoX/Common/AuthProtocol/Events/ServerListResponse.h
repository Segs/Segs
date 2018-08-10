/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "AuthProtocol/AuthEvents.h"
#include <deque>

struct GameServerInfo
{
    uint8_t  id;
    uint32_t addr; // result of calling get_ip_address() on ACE_Inet_ADDR
    uint16_t port;
    uint16_t current_players;
    uint16_t max_players;
    uint8_t  online;
};

namespace SEGSEvents
{
// [[ev_def:type]]
class ServerListResponse : public AuthLinkEvent
{
public:
    // [[ev_def:field]]
    std::deque<GameServerInfo> m_serv_list;
    // [[ev_def:field]]
    uint8_t  m_preferred_server_idx;
    ServerListResponse() : AuthLinkEvent(evServerListResponse)
    {}
    void set_server_list(const std::deque<GameServerInfo> &srv) {m_serv_list=srv;}
    void serializeto(GrowingBuffer &buf) const;
    void serializefrom(GrowingBuffer &buf);
};
} // end of namespace SEGSEvents
