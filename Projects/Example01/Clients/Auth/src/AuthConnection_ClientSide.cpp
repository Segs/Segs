/*
* Super Entity Game Server Project
* http://segs.sf.net/
* Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
* This software is licensed! (See License.txt for details)
*
* $Id$
*/

#include <stdlib.h>
#include "Base.h"
#include "AuthFSM.h"
#include "AuthPacket.h"
#include "AuthProtocol.h"
#include "AuthConnection_ClientSide.h"

template class AuthProtocol< AuthFSM_Client >;

AuthConnection_ClientSide::AuthConnection_ClientSide(AuthObserver *ob) : AuthConnection()
{
	m_current_proto = new AuthProtocol< AuthFSM_Client >(true);
	static_cast<AuthProtocol< AuthFSM_Client > *>(m_current_proto)->set_observer(ob);
}
