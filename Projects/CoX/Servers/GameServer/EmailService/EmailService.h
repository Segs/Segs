/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#ifndef EMAILSERVICE_H
#define EMAILSERVICE_H

#include "EventProcessor.h"
#include "Common/Servers/HandlerLocator.h"
#include "Common/Servers/ClientManager.h"
#include "Servers/GameDatabase/GameDBSyncHandler.h"
#include "Servers/MapServer/MapClientSession.h"
#include <QVector>

namespace SEGSEvents
{
class EmailHeaderResponse;
class EmailReadResponse;
class EmailWasReadByRecipientMessage;
class EmailHeadersToClientMessage;
class EmailHeaderToClientMessage;
class EmailCreateStatusMessage;
}

class EmailService : public EventProcessor
{
private:
    using SessionStore = ClientSessionStore<MapClientSession>;
    SessionStore* m_session_store;

    void on_email_header_response(SEGSEvents::EmailHeaderResponse* ev);
    void on_email_headers_to_client(SEGSEvents::EmailHeadersToClientMessage *ev);
    void on_email_header_to_client(SEGSEvents::EmailHeaderToClientMessage *ev);
    void on_email_read_response(SEGSEvents::EmailReadResponse *ev);
    void on_email_read_by_recipient(SEGSEvents::EmailWasReadByRecipientMessage *ev);
    void on_email_create_status(SEGSEvents::EmailCreateStatusMessage *ev);

public:
    IMPL_ID(EmailService)
    EmailService(SessionStore& session_store) {m_session_store = &session_store;}
protected:
    // EventProcessor interface
    bool per_thread_startup() override;
    void dispatch(SEGSEvents::Event *ev) override;
    void serialize_from(std::istream &is) override;
    void serialize_to(std::ostream &is) override;

};

#endif // GAMEDBSYNCSERVICE_H
