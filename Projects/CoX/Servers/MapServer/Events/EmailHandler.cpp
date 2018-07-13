/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "EmailHandler.h"
#include "Common/Servers/HandlerLocator.h"
#include "MapEventTypes.h"
#include "EmailEvents.h"
#include "EmailHeaders.h"

void EmailHandler::dispatch(SEGSEvent *ev)
{
    assert(ev);
    switch(ev->type())
    {
        case EmailEventTypes::evEmailHeader:
        on_email_header(static_cast<EmailHeaderMessage *>(ev));
        break;
        case EmailEventTypes::evEmailRead:
        on_email_read();
        break;
        case EmailEventTypes::evEmailSent:
        on_email_sent();
        break;
        case EmailEventTypes::evEmailDelete:
        on_email_delete();
        break;
        default: break;
    }
}

void EmailHandler::on_email_header(EmailHeaderMessage *msg)
{
    EmailHeaders *header = new EmailHeaders(
                msg->m_data.id,
                msg->m_data.sender,
                msg->m_data.subject,
                msg->m_data.timestamp);
    msg->m_data.src->addCommandToSendNextUpdate(std::unique_ptr<EmailHeaders>(header));
}

void EmailHandler::on_email_read()
{}

void EmailHandler::on_email_sent()
{}

void EmailHandler::on_email_delete()
{
}

EmailHandler::EmailHandler()
{
    HandlerLocator::setEmail_Handler(this);
}
