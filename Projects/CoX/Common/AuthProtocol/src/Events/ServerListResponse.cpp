#include "ServerListResponse.h"
#include "GameServerInterface.h"

void ServerListResponse::serializeto( GrowingBuffer &buf ) const
{
    assert(m_serv_list);
    buf.uPut((uint8_t)4);
    buf.uPut((uint8_t)m_serv_list->size());
    buf.uPut((uint8_t)1); //preferred server number
    for(size_t i=0; i<m_serv_list->size(); i++)
    {
        IGameServer *srv=(*m_serv_list)[i];
        buf.Put(srv->getId());
        uint32_t addr= srv->getAddress().get_ip_address();
        buf.Put((uint32_t)ACE_SWAP_LONG(addr)); //must be network byte order
        buf.Put((uint32_t)srv->getAddress().get_port_number());
        buf.Put(srv->getUnkn1());
        buf.Put(srv->getUnkn2());
        buf.Put(srv->getCurrentPlayers());
        buf.Put(srv->getMaxPlayers());
        buf.Put((uint8_t)srv->Online());
    }
}

void ServerListResponse::serializefrom( GrowingBuffer &buf )
{
    uint8_t op;
    buf.uGet(op);
    assert(!"TODO");
}
