#include "ServerListResponse.h"
#include "GameServerInterface.h"

void ServerListResponse::serializeto( GrowingBuffer &buf ) const
{
    ACE_ASSERT(m_serv_list);
    buf.uPut((u8)4);
    buf.uPut((u8)m_serv_list->size());
    buf.uPut((u8)1); //preferred server number
    for(size_t i=0; i<m_serv_list->size(); i++)
    {
        IGameServer *srv=(*m_serv_list)[i];
        buf.Put(srv->getId());
        u32 addr= srv->getAddress().get_ip_address();
        buf.Put((u32)ACE_SWAP_LONG(addr)); //must be network byte order
        buf.Put((u32)srv->getAddress().get_port_number());
        buf.Put(srv->getUnkn1());
        buf.Put(srv->getUnkn2());
        buf.Put(srv->getCurrentPlayers());
        buf.Put(srv->getMaxPlayers());
        buf.Put((u8)srv->Online());
    }
}

void ServerListResponse::serializefrom( GrowingBuffer &buf )
{
    u8 op;
    buf.uGet(op);
    ACE_ASSERT(!"TODO");
}