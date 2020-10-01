/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

enum class ClientStates : uint8_t
{
    SIMPLE = 0,
    CREATE_TEAM_CONTAINER = 1,
    CREATE_TEAM_CONTAINER_WAIT_MAPSVRR_RESPONSE = 2,
    DEAD = 3,
    RESURRECT = 4,
    AWAITING_GURNEY_XFER = 5,
    MAX_CLIENT_STATE
};
