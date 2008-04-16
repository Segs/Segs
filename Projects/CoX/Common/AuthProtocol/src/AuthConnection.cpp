/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: ClientConnection.cpp 317 2007-01-25 15:44:29Z nemerle $
 */

#include "AuthConnection.h"
#include "AuthProtocol.h"
AuthConnection::AuthConnection(void) : m_received_bytes_storage(0x1000,0,40)
{
	m_queue  = NULL;
	m_current_proto = new AuthProtocol; //m_current_proto->setConnection(this); at connection establish point
}
AuthConnection::~AuthConnection(void)
{
	m_queue  = NULL;
	delete m_current_proto;
	m_current_proto = NULL;
}
void AuthConnection::sendBytes(GrowingBuffer &buffer)
{
	const size_t packet_size = buffer.GetReadableDataSize();
	ACE_ASSERT(packet_size<0x10000);
	ACE_ASSERT(m_queue);
	// let's create a message
	ACE_Message_Block *payload = new ACE_Message_Block(packet_size+2);
	ACE_ASSERT(payload);
	// putting the payload in
	buffer.uGetBytes(reinterpret_cast<u8 *>(payload->wr_ptr()),packet_size); //unchecked, because packet_size == ReadableSize
	payload->wr_ptr(packet_size);
	if(!m_queue)
	{
		ACE_ERROR ((LM_ERROR,ACE_TEXT ("(%P|%t) %p\n"),ACE_TEXT ("No target queue set for a connection")));
	}
	if(-1==m_queue->enqueue(payload))
		ACE_ERROR ((LM_ERROR,ACE_TEXT ("(%P|%t) %p\n"),ACE_TEXT ("SendPacket")));
}
void AuthConnection::ReceivedBytes(const u8 *buffer,size_t recv_cnt)
{
	m_received_bytes_storage.PutBytes(buffer,recv_cnt);
	ACE_ASSERT(m_received_bytes_storage.getLastError()==0);// all received data must fit into the buffer!
	m_received_bytes_storage.ResetReading(); //retrying

	if(m_received_bytes_storage.GetReadableDataSize()<2) // 
		return;
	m_current_proto->ReceivedBytes(m_received_bytes_storage); // protocol eats the bytes
	return; // not enough data left in buffer wait for more
}
void AuthConnection::Established(const ACE_INET_Addr &with_peer)
{
	//ServerManager::instance()->GetAdminServer()->Log("Established",with_peer)
	m_peer = with_peer;
	m_current_proto->setConnection(this); // not set in constructor
	m_current_proto->Established();
}
void AuthConnection::Closed()
{
	m_current_proto->Closed();
}
void AuthConnection::Choked()
{
	ACE_ASSERT(!"implemented yet");
}
void AuthConnection::DataSent()
{
	ACE_ASSERT(!"implemented yet");
}
