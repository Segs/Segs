/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

/*!
 * @addtogroup ExampleAuthClient Projects/Example01/Clients/Auth
 * @{
 */

#include "Auth.h"
#include "AuthFSM.h"
#include "AuthPacket.h"

AuthPacket * AuthFSM_Client::ConnectionEstablished(AuthConnection *conn)
{
	m_observer->notify_connected();
	return 0;
}

void AuthFSM_Client::ConnectionClosed(AuthConnection *conn)
{
	m_observer->notify_disconnected();
}

AuthPacket *AuthFSM_Client::ReceivedPacket(AuthConnection *caller,AuthPacket *pkt)
{
	switch(pkt->GetPacketType())
	{
	case SMSG_AUTHVERSION:
		pktAuthVersion* vers_pkt = static_cast<pktAuthVersion*>(pkt);
		m_observer->notify_packet(vers_pkt);
		break;
	}
	return 0;
}

void AuthFSM_Client::set_observer( AuthObserver *ob )
{
	m_observer=ob;
}

AuthObserver * AuthFSM_Client::get_observer()
{
	return m_observer;
}

//! @}
