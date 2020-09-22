/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Common/Servers/InternalEvents.h"

using namespace SEGSEvents;

// The EmailService is self-explanatory - it handles on email stuff
class EmailService
{
public:
    UPtrServiceToClientData on_email_header_response(Event* ev);
    UPtrServiceToClientData on_email_headers_to_client(Event* ev);
    UPtrServiceToClientData on_email_header_to_client(Event* ev);
    UPtrServiceToClientData on_email_read_response(Event* ev);
    UPtrServiceToClientData on_email_create_status(Event* ev);
    UPtrServiceToClientData on_email_read_by_recipient(Event* ev);

protected:
};
