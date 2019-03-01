/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "EmailService.h"
#include "Messages/EmailService/EmailDefinitions.h"
#include "Messages/EmailService/EmailEvents.h"
#include "Messages/Map/EmailHeaders.h"
#include "Messages/Map/EmailMessageStatus.h"
#include "Messages/Map/EmailRead.h"
#include "Common/Servers/InternalEvents.h"
#include "Servers/MapServer/MapClientSession.h"
#include "Servers/MapServer/MessageHelpers.h"

using namespace SEGSEvents;

ServiceToClientMessage* EmailService::on_email_header_response(Event* ev)
{
    EmailHeaderResponse* resp = static_cast<EmailHeaderResponse *>(ev);

    std::vector<EmailHeaders::EmailHeader> email_headers;
    for (const auto &email_header : resp->m_data.m_email_headers)
    {
        email_headers.push_back(EmailHeaders::EmailHeader{email_header.m_email_id,
                                                          email_header.m_sender_name,
                                                          email_header.m_subject,
                                                          email_header.m_timestamp});
    }

    return new ServiceToClientMessage({new EmailHeaders(email_headers), QString()}, resp->session_token());
}

// EmailHandler will send this event here
ServiceToClientMessage* EmailService::on_email_header_to_client(Event* ev)
{
    EmailHeaderToClientMessage* msg = static_cast<EmailHeaderToClientMessage *>(ev);
    EmailHeaders* email_header = new EmailHeaders(
                msg->m_data.m_email_id,
                msg->m_data.m_sender_name,
                msg->m_data.m_subject,
                msg->m_data.m_timestamp);
    return new ServiceToClientMessage({email_header, "message"}, msg->session_token());
}

ServiceToClientMessage* EmailService::on_email_headers_to_client(Event* ev)
{
    EmailHeadersToClientMessage* msg = static_cast<EmailHeadersToClientMessage *>(ev);

    std::vector<EmailHeaders::EmailHeader> email_headers;
    for (const auto &email_header : msg->m_data.m_email_headers)
    {
        email_headers.push_back(EmailHeaders::EmailHeader{email_header.m_email_id,
                                                          email_header.m_sender_name,
                                                          email_header.m_subject,
                                                          email_header.m_timestamp});
    }

    QString msgToClient = QString("You have %1 unread emails.").arg(msg->m_data.m_unread_emails_count);
    return new ServiceToClientMessage({new EmailHeaders(email_headers), msgToClient}, msg->session_token());
}

ServiceToClientMessage* EmailService::on_email_read_response(Event* ev)
{
    EmailReadResponse* resp = static_cast<EmailReadResponse *>(ev);
    EmailRead* email_read = new EmailRead(resp->m_data.m_email_id, resp->m_data.m_message, resp->m_data.m_sender_name);
    return new ServiceToClientMessage({email_read, QString()}, resp->session_token());
}

ServiceToClientMessage* EmailService::on_email_read_by_recipient(Event* ev)
{
    EmailWasReadByRecipientMessage* msg = static_cast<EmailWasReadByRecipientMessage *>(ev);
    return new ServiceToClientMessage({nullptr, msg->m_data.m_message}, msg->session_token());
}

ServiceToClientMessage* EmailService::on_email_create_status(Event* ev)
{
    EmailCreateStatusMessage* msg = static_cast<EmailCreateStatusMessage* >(ev);
    EmailMessageStatus* email_status = new EmailMessageStatus(msg->m_data.m_status, msg->m_data.m_recipient_name);
    return new ServiceToClientMessage({email_status, QString()}, msg->session_token());
}
