/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Common/Servers/HandlerLocator.h"
#include "Common/Servers/ClientManager.h"
#include "Common/Servers/InternalEvents.h"
#include "Servers/GameDatabase/GameDBSyncHandler.h"
#include "Servers/MapServer/MapClientSession.h"

#include <QVector>

namespace SEGSEvents
{
class EmailHeaderResponseData;
class EmailReadResponseData;
class EmailWasReadByRecipientData;
class EmailHeadersToClientData;
class EmailHeaderToClientData;
class EmailCreateStatusData;
class EmailHeaders;
class EmailMessageStatus;
}

class EmailService
{
private:
public:
    SEGSEvents::ServiceToClientData* on_email_header_response(SEGSEvents::Event* ev);
    SEGSEvents::ServiceToClientData* on_email_headers_to_client(SEGSEvents::Event* ev);
    SEGSEvents::ServiceToClientData* on_email_header_to_client(SEGSEvents::Event* ev);
    SEGSEvents::ServiceToClientData* on_email_read_response(SEGSEvents::Event* ev);
    SEGSEvents::ServiceToClientData* on_email_create_status(SEGSEvents::Event* ev);
    SEGSEvents::ServiceToClientData* on_email_read_by_recipient(SEGSEvents::Event* ev);

protected:
};


