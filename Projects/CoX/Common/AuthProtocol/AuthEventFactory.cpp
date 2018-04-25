/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

/*!
 * @addtogroup AuthProtocol Projects/CoX/Common/AuthProtocol
 * @{
 */

#include "AuthEventFactory.h"
#include "AuthEvents.h"

AuthLinkEvent *AuthEventFactory::EventForType(eAuthPacketType type)
{
    switch(type)
    {
        case SMSG_AUTHVERSION:
            return new AuthorizationProtocolVersion();
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
