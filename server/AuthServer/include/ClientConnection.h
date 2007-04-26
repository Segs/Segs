/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: ClientConnection.h 316 2007-01-25 15:17:16Z nemerle $
 */

#pragma once
#include <ace/Message_Queue.h>
#include <ace/INET_Addr.h>
#include "types.h"
#include "Buffer.h"
//#include "Auth.h"
/******************************************************************************/
/* Servicer  --Bytes-->   ClientConnection  --Bytes-->  Protocol --Actions--> */
/* Servicer <--Messages-- ClientConnection <--Bytes--   Protocol <--Actions--    */
/******************************************************************************/
class IAuthProtocol;
class AuthClient;

class ClientConnection
{
	ClientConnection(const ClientConnection &); // no copy construction or assignment
	ClientConnection &operator=(const ClientConnection&);
public:

	ClientConnection(void);
	~ClientConnection(void);

	//////////////////////////////////////////////////////////////////////////
	// Client connection FSM support methods
	//////////////////////////////////////////////////////////////////////////
	
	int getClientState(void) const{return fsm_data;}; 
	void setClientState(int state) {fsm_data = state;}

	IAuthProtocol *	getProtocol();
	void			setProtocol(IAuthProtocol *,bool retain_old_settings); //retain_old_settings tells us to try and copy old settings from existing Protocol object

	void			setClient(AuthClient *cl){m_client=cl;}
	AuthClient *	getClient(){return m_client;}

	const ACE_INET_Addr &peer() const {return m_peer;}
	//////////////////////////////////////////////////////////////////////////
	// ClientServicer interfacing methods
	//////////////////////////////////////////////////////////////////////////
	
	void ReceivedBytes(const u8 *buffer,size_t recv_cnt);
	void sendBytes(GrowingBuffer &buffer);
	void setMessageQueue(ACE_Message_Queue_Base *q){m_queue=q;};

	void Established(const ACE_INET_Addr &peer);
	void Closed(); // called when client connection is closed
	void Choked(); // this is called when client sends waaay too much data, this situation should be logged, and after a while client IP added to blocked list
	void DataSent(); // this is pre-design interface for Async processing, it's called when last SendPacket finishes

protected:
	ACE_Message_Queue_Base *m_queue;
	ACE_INET_Addr m_peer;
	GrowingBuffer m_received_bytes_storage;
	IAuthProtocol *m_current_proto;
	AuthClient	*m_client;
	int fsm_data;
};
