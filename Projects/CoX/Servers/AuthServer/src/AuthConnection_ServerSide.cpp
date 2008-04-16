/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: AuthFSM.cpp 271 2006-10-02 04:30:50Z nemerle $
 */

#include <stdlib.h>
#include "Base.h"
#include "AuthFSM.h"
#include "AuthPacket.h"
#include "AuthProtocol.h"
#include "AuthConnection_ServerSide.h"
#include "AuthClient.h"
#include "AdminServerInterface.h"
#include "ServerManager.h"
#include "AuthServer.h"
template class AuthProtocol< AuthFSM_Default >; // instantiation of Authprotocol working with AuthFSM_Default

AuthConnection_ServerSide::AuthConnection_ServerSide() : AuthConnection(),m_client(0),fsm_data(0)
{
	m_current_proto = new AuthProtocol< AuthFSM_Default >; //m_current_proto->setConnection(this); at connection establish point
}