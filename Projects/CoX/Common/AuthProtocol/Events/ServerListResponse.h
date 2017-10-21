#pragma once
#include "AuthProtocol/AuthEvents.h"
#include <deque>

class GameServerInterface;
struct GameServerInfo
{
    uint8_t  id;
    uint32_t addr; // result of calling get_ip_address() on ACE_Inet_ADDR
    uint16_t port;
    uint8_t  unknown_1;
    uint8_t  unknown_2;
    uint16_t current_players;
    uint16_t max_players;
    uint8_t  online;
};
class ServerListResponse : public AuthLinkEvent
{
    std::deque<GameServerInfo> m_serv_list;
public:
    ServerListResponse() : AuthLinkEvent(evServerListResponse)
    {}
    void set_server_list(const std::deque<GameServerInfo> &srv) {m_serv_list=srv;}
    void serializeto(GrowingBuffer &buf) const;
    void serializefrom(GrowingBuffer &buf);
};
