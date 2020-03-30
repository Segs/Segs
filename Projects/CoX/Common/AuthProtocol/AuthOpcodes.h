/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include <stdint.h>

enum eAuthPacketType : uint8_t 
{
    SMSG_AUTHVERSION = 0x00,
    SMSG_AUTH_ALREADY_LOGGED_IN   = 0x01,
    SMSG_AUTH_INVALID_PASSWORD    = 0x02,
    CMSG_AUTH_SELECT_DBSERVER = 0x02,
    PKT_AUTH_LOGIN_SUCCESS,
    CMSG_DB_CONN_FAILURE,
    CMSG_AUTH_LOGIN,
    CMSG_AUTH_REQUEST_SERVER_LIST,
    SMSG_AUTH_SERVER_LIST,
    PKT_SELECT_SERVER_RESPONSE,
    SMSG_AUTH_ERROR,
    SMSG_AUTH_OK,
    SMSG_PLAY_OK,
    CMSG_RECONNECT_ATTEMPT, // packet 3
    MSG_AUTH_UNKNOWN //always last
};
