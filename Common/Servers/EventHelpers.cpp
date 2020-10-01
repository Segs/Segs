/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "EventHelpers.h"
#include "Common/CRUDP_Protocol/ILink.h"

uint64_t get_session_token(Event* ev)
{
    return static_cast<LinkBase *>(ev->src())->session_token();
}

uint64_t get_session_token(InternalEvent* ev)
{
    return ev->session_token();
}
