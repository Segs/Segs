#pragma once
#include "AuthEvents.h"
#include <deque>

class GameServerInterface;
class ServerListResponse : public AuthLinkEvent
{
    const deque<GameServerInterface *> *m_serv_list;
public:
	ServerListResponse() : AuthLinkEvent(evServerListResponse),m_serv_list(0)
	{}
    void set_server_list(const deque<GameServerInterface *> *srv) {m_serv_list=srv;}
	void serializeto(GrowingBuffer &buf) const;
	void serializefrom(GrowingBuffer &buf);
};
