/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup AuthProtocol Projects/CoX/Common/AuthProtocol
 * @{
 */

#include "AuthEventFactory.h"
#include "AuthOpcodes.h"
#include "Auth/AuthEvents.h"

using namespace SEGSEvents;
AuthLinkEvent *AuthEventFactory::EventForType(eAuthPacketType type)
{
    switch(type)
    {
        case SMSG_AUTHVERSION:
            return new AuthProtocolVersion();
        case SMSG_AUTH_ERROR:
            return new AuthorizationError();
        case CMSG_AUTH_SELECT_DBSERVER:
            return new ServerSelectRequest();
        case CMSG_DB_CONN_FAILURE:
            return new DbError();
        case PKT_AUTH_LOGIN_SUCCESS:
            return new LoginResponse();
        case CMSG_AUTH_LOGIN:
            return new LoginRequest();
        case SMSG_AUTH_SERVER_LIST:
            return new ServerListResponse();
        case CMSG_AUTH_REQUEST_SERVER_LIST:
            return new ServerListRequest();
        case PKT_SELECT_SERVER_RESPONSE:
            return new ServerSelectResponse();
        case CMSG_RECONNECT_ATTEMPT:
            return new ReconnectAttempt();
        case MSG_AUTH_UNKNOWN:
            return nullptr;
        default:
            assert(!"unimplemented auth packet type");
    }
    return nullptr;
}
void AuthEventFactory::Destroy(AuthLinkEvent *what)
{
    assert(what);
    what->release();
    //delete what;
}

//! @}
