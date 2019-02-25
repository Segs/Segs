/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "EmailService.h"
#include "Common/Servers/HandlerLocator.h"
#include "Common/Servers/MessageBus.h"
#include "Common/Servers/InternalEvents.h"
#include "Messages/EmailService/EmailDefinitions.h"
#include "Messages/EmailService/EmailEvents.h"
#include "Messages/Map/EmailHeaders.h"
#include "Messages/Map/EmailMessageStatus.h"
#include "Messages/Map/EmailRead.h"
#include "Common/Servers/InternalEvents.h"
#include "Servers/MapServer/MapClientSession.h"
#include "Servers/MapServer/MessageHelpers.h"

using namespace SEGSEvents;

void EmailService::dispatch(Event *ev)
{
    // We are servicing a request from message queue, using dispatchSync as a common processing point.
    // nullptr result means that the given message is one-way
    switch (ev->type())
    {
    case evEmailHeaderResponse:
        on_email_header_response(static_cast<EmailHeaderResponse *>(ev)); break;
    case evEmailReadResponse:
        on_email_read_response(static_cast<EmailReadResponse *>(ev)); break;
    case evEmailWasReadByRecipientMessage:
        on_email_read_by_recipient(static_cast<EmailWasReadByRecipientMessage *>(ev)); break;
    case evEmailHeadersToClientMessage:
        on_email_headers_to_client(static_cast<EmailHeadersToClientMessage *>(ev)); break;
    case evEmailHeaderToClientMessage:
        on_email_header_to_client(static_cast<EmailHeaderToClientMessage *>(ev)); break;
    case evEmailCreateStatusMessage:
        on_email_create_status(static_cast<EmailCreateStatusMessage *>(ev)); break;
    default: assert(false); break;
    }
}

void EmailService::on_email_header_response(EmailHeaderResponse* ev)
{
    MapClientSession &map_session(m_session_store.session_from_token(ev->session_token()));

    std::vector<EmailHeaders::EmailHeader> email_headers;
    for (const auto &data : ev->m_data.m_email_headers)
    {
        email_headers.push_back(EmailHeaders::EmailHeader{data.m_email_id,
                                                          data.m_sender_name,
                                                          data.m_subject,
                                                          data.m_timestamp});
    }

    map_session.addCommandToSendNextUpdate(std::make_unique<EmailHeaders>(email_headers));
}

// EmailHandler will send this event here
void EmailService::on_email_header_to_client(EmailHeaderToClientMessage* ev)
{
    MapClientSession &map_session(m_session_store->session_from_token(ev->session_token()));
    map_session.addCommandToSendNextUpdate(std::make_unique<EmailHeaders>(ev->m_data.m_email_id,
                                                                          ev->m_data.m_sender_name,
                                                                          ev->m_data.m_subject,
                                                                          ev->m_data.m_timestamp));
}

void EmailService::on_email_headers_to_client(EmailHeadersToClientMessage *ev)
{
    MapClientSession &map_session(m_session_store->session_from_token(ev->session_token()));

    for (const auto &data : ev->m_data.m_email_headers)
    {
        map_session.addCommandToSendNextUpdate(std::make_unique<EmailHeaders>(data.m_email_id,
                                                                              data.m_sender_name,
                                                                              data.m_subject,
                                                                              data.m_timestamp));
    }

    QString message = QString("You have %1 unread emails.").arg(ev->m_data.m_unread_emails_count);
    sendInfoMessage(MessageChannel::DEBUG_INFO, message, map_session);
}

void EmailService::on_email_read_response(EmailReadResponse *ev)
{
    MapClientSession &map_session(m_session_store->session_from_token(ev->session_token()));
    map_session.addCommandToSendNextUpdate(std::make_unique<EmailRead>(ev->m_data.m_email_id,
                                                                       ev->m_data.m_message,
                                                                       ev->m_data.m_sender_name));
}

void EmailService::on_email_read_by_recipient(EmailWasReadByRecipientMessage *msg)
{
    MapClientSession &map_session(m_session_store->session_from_token(msg->session_token()));
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg->m_data.m_message, map_session);

    // this is sent from the reader back to the sender via EmailHandler
    // route is DataHelpers.onEmailRead() -> EmailHandler -> MapInstance
}

void EmailService::on_email_create_status(EmailCreateStatusMessage *msg)
{
    MapClientSession &map_session(m_session_store->session_from_token(msg->session_token()));
    map_session.addCommandToSendNextUpdate(std::unique_ptr<EmailMessageStatus>(
                new EmailMessageStatus(msg->m_data.m_status, msg->m_data.m_recipient_name)));
}
