/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Common/Servers/InternalEvents.h"

// The EmailService is self-explanatory - it handles on email stuff
class EmailService
{
private:
    using GameCommandVector = std::vector<std::unique_ptr<SEGSEvents::GameCommandEvent>>;
public:
    std::unique_ptr<SEGSEvents::ServiceToClientData> on_email_header_response(SEGSEvents::Event* ev);
    std::unique_ptr<SEGSEvents::ServiceToClientData> on_email_headers_to_client(SEGSEvents::Event* ev);
    std::unique_ptr<SEGSEvents::ServiceToClientData> on_email_header_to_client(SEGSEvents::Event* ev);
    std::unique_ptr<SEGSEvents::ServiceToClientData> on_email_read_response(SEGSEvents::Event* ev);
    std::unique_ptr<SEGSEvents::ServiceToClientData> on_email_create_status(SEGSEvents::Event* ev);
    std::unique_ptr<SEGSEvents::ServiceToClientData> on_email_read_by_recipient(SEGSEvents::Event* ev);

protected:
};


