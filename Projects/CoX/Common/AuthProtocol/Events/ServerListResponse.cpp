#include "ServerListResponse.h"

#ifdef _MSC_VER
#include <ciso646>
#endif
void ServerListResponse::serializeto( GrowingBuffer &buf ) const
{
    assert(not m_serv_list.empty());
    buf.uPut((uint8_t)4);
    buf.uPut((uint8_t)m_serv_list.size());
    buf.uPut((uint8_t)1); //preferred server number
    for(size_t i=0; i<m_serv_list.size(); i++)
    {
        const GameServerInfo &srv(m_serv_list[i]);
        buf.Put(srv.id);
        uint32_t addr= srv.addr;
        buf.Put((uint32_t)ACE_SWAP_LONG(addr)); //must be network byte order
        buf.Put((uint32_t)srv.port);
        buf.Put(srv.unknown_1);
        buf.Put(srv.unknown_2);
        buf.Put(srv.current_players);
        buf.Put(srv.max_players);
        buf.Put((uint8_t)srv.online);
    }
}

void ServerListResponse::serializefrom( GrowingBuffer &buf )
{
    uint8_t op;
    buf.uGet(op);
    assert(!"TODO");
}
