/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: AuthClientServicer.cpp 253 2006-08-31 22:00:14Z malign $
 */

#include "AuthClientServicer.h"
#include <ace/Message_Block.h>
#include <ace/Reactor.h>
#include <ace/OS.h>
AuthClientService::AuthClientService(void) : 
	m_notifier(0, this, ACE_Event_Handler::WRITE_MASK) 
{
}

AuthClientService::~AuthClientService(void)
{
	delete client;
	client = NULL;
}
int AuthClientService::open (void *p)
{
	if (super::open (p) == -1)
		return -1;
	
	m_notifier.reactor(reactor());						// notify reactor with write event, 
	msg_queue()->notification_strategy (&m_notifier);	// whenever there is a new event on msg_queue()
	
	client = new AuthConnection; // this object will handle incoming data, interface with AuthServerProxy and also will post Message blocks on the queue
	ACE_INET_Addr peer_addr;
	if(peer().get_remote_addr(peer_addr)!=0)
		peer_addr.set("255.255.255.255"); // invalid peer addr
	client->setMessageQueue(msg_queue());
	client->Established(peer_addr); // this will start the whole thing rolling
	return 0;
}

int AuthClientService::handle_input (ACE_HANDLE)
{
	const size_t INPUT_SIZE = 4096;
	char buffer[INPUT_SIZE];
	ssize_t recv_cnt;
	if ((recv_cnt = peer().recv(buffer, sizeof(buffer))) <= 0)
	{
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("(%P|%t) Connection closed\n")));
		return -1;
	}
	client->ReceivedBytes((u8*)buffer,recv_cnt);
	// if this client sent too much data lately, choke it !
	// client->Choked();
	return 0;
}

int AuthClientService::handle_output(ACE_HANDLE)
{
	ACE_Message_Block *mb;
	ACE_Time_Value nowait (ACE_OS::gettimeofday ());
	while (-1 != getq(mb, &nowait))
	{
		if(mb->msg_type()==ACE_Message_Block::MB_BREAK)
		{
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("(%P|%t) Error sent, closing connection\n")));
			return -1;
		}
		ssize_t send_cnt = peer().send(mb->rd_ptr(), mb->length());
		if (send_cnt == -1)
			ACE_ERROR ((LM_ERROR,ACE_TEXT ("(%P|%t) %p\n"),	ACE_TEXT ("send")));
		else
			mb->rd_ptr(ACE_static_cast (size_t, send_cnt));
		if (mb->length () > 0)
		{
			ungetq(mb);
			break;
		}
		mb->release();
	}
	if (msg_queue()->is_empty ()) // we don't want to be woken up 
		reactor()->cancel_wakeup(this, ACE_Event_Handler::WRITE_MASK);
	else // unless there is something to send still
		reactor()->schedule_wakeup(this, ACE_Event_Handler::WRITE_MASK);
	return 0;
}
int AuthClientService::handle_close (ACE_HANDLE h, ACE_Reactor_Mask mask)
{
	client->Closed();
	if (mask == ACE_Event_Handler::WRITE_MASK)
		return 0;
	return super::handle_close (h, mask);
}
