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

//! @}
