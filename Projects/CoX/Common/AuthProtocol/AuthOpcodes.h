/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once

enum eAuthPacketType
{
    SMSG_AUTHVERSION = 0x00,
//    SMSG_AUTH_ALREADY_LOGGED_IN   = 0x01,
//    SMSG_AUTH_INVALID_PASSWORD    = 0x02,
    CMSG_AUTH_SELECT_DBSERVER = 0x02,
    PKT_AUTH_LOGIN_SUCCESS,
    CMSG_DB_CONN_FAILURE,
    CMSG_AUTH_LOGIN,
    CMSG_AUTH_REQUEST_SERVER_LIST,
    SMSG_AUTH_SERVER_LIST,
    PKT_SELECT_SERVER_RESPONSE,
    SMSG_AUTH_ERROR,
    MSG_AUTH_UNKNOWN //always last
};
