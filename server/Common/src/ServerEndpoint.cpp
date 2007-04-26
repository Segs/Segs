/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: ServerEndpoint.cpp 319 2007-01-26 17:03:18Z nemerle $
 */

//#include <ace/SOCK_Stream.h>
#include "ServerManager.h"
#include <ace/Message_Block.h>
#include "ServerEndpoint.h"
#include "NetManager.h"
#include "ClientManager.h"
#include "AdminServerInterface.h"
//#include "GamePacket.h"
//static GamePacketFactory m_fact;
int ServerEndpoint::handle_close (ACE_HANDLE , ACE_Reactor_Mask)
{
	endpoint_.close();
	return 0;
}
int ServerEndpoint::open (void *p)
{
	if (super::open (p) == -1)
		return -1;
	m_notifier.reactor(reactor()); // notify current reactor with write event, 
	msg_queue()->notification_strategy (&m_notifier); // whenever there is a new msg on msg_queue()
	return 0;
}
CrudP_Protocol *ServerEndpoint::getClientLink(const ACE_INET_Addr &from_addr)
{
	CrudP_Protocol *res= client_links[from_addr]; // get packet handling object for this connection
	if(res!=NULL)
		return res;
	// create a new client handler
	IGameProtocol *prot			= GameProtocolManager::CreateGameProtocol();
	Client *client				= ClientManager::CreateClient();
	LinkCommandHandler *handler = createNewHandler();

	res			= prot->getCrudP();
	client->setPeer(from_addr);
	prot->setPacketFactory(getFactory());
	prot->setHandler(handler);
	//	handler->setClient(client);
	res->setClient(client);
	res->setMessageQueue(msg_queue());
	client_links[from_addr]	= res;
	handler->setTargetAddr(from_addr);
	// schedule timeout timer here!!
	return res;
}
int ServerEndpoint::handle_input(ACE_HANDLE)
{
	u8 buf[0x2000];
	ACE_INET_Addr from_addr; 
	//ACE_DEBUG ((LM_DEBUG,"(%P|%t) activity occurred on handle %d!\n",this->endpoint_.get_handle ()));
	ssize_t n = this->endpoint_.recv(buf, sizeof buf,from_addr);
	//ACE_DEBUG ((LM_DEBUG,"(%P|%t) %d bytes from %s:%d!\n",n,from_addr.get_host_addr(),from_addr.get_port_number()));
	if (n == -1)
		ACE_ERROR ((LM_ERROR,ACE_TEXT("%p\n"),"handle_input"));
	else
	{
		u16 port = from_addr.get_port_number();
		CrudP_Protocol *crudp_proto = getClientLink(from_addr); // get packet handling object for this connection
		last_client_activity[port] = ACE_OS::gettimeofday();
		ACE_ASSERT(crudp_proto!=NULL);
		BitStream wrap((u8 *)buf,n);
		crudp_proto->ReceivedBlock((BitStream &)wrap);
	}
	return 0;
}

int ServerEndpoint::handle_timeout (const ACE_Time_Value &,const void *)
{
	return 0;
}
int ServerEndpoint::handle_output(ACE_HANDLE)
{
	ACE_Message_Block *mb;
	ACE_Time_Value nowait (ACE_OS::gettimeofday ());
	while (-1 != getq(mb, &nowait))
	{
		if(mb->length()>=sizeof(void *))
		{
			Client *client = (Client *)mb->rd_ptr();
			ACE_ASSERT(client);
			CrudP_Protocol *crudp_proto = client_links[client->getPeer()]; // get packet handling for this connection
			ACE_ASSERT(crudp_proto);
			list<CrudP_Packet *> packets;
			CrudP_Packet *pkt;
			size_t count_p = crudp_proto->GetUnsentPackets(packets);
			while(count_p)
			{
				ACE_DEBUG((LM_DEBUG,ACE_TEXT ("(%P|%t) Sending \n")));
				pkt=*packets.begin();
				packets.pop_front();
				ssize_t send_cnt = endpoint_.send(pkt->GetStream()->read_ptr(),pkt->GetStream()->GetReadableDataSize(),client->getPeer());
				if (send_cnt == -1)
				{
					ACE_ERROR ((LM_ERROR,ACE_TEXT ("(%P|%t) %p\n"),	ACE_TEXT ("send")));
					break;
				}
				--count_p;
			}
		}
		mb->release();
	}
	if (msg_queue()->is_empty ()) // we don't want to be woken up 
		reactor()->cancel_wakeup(this, ACE_Event_Handler::WRITE_MASK);
	else // unless there is something to send still
		reactor()->schedule_wakeup(this, ACE_Event_Handler::WRITE_MASK);
	return 0;
}

ServerEndpoint::ServerEndpoint(const ACE_INET_Addr &local_addr) : 
	m_notifier(0, this, ACE_Event_Handler::WRITE_MASK),
	endpoint_ (local_addr)
{
	m_notifier.reactor(reactor()); // notify reactor with write event, 
	msg_queue()->notification_strategy (&m_notifier); // whenever there is a new event on msg_queue()
}
ACE_HANDLE ServerEndpoint::get_handle(void) const
{
	return this->endpoint_.get_handle();
}
