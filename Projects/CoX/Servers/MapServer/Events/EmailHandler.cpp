/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "EmailHandler.h"
#include "Common/Servers/HandlerLocator.h"
#include "Servers/GameServer/GameEvents.h"
#include "MapEventTypes.h"
#include "EmailEvents.h"
#include "EmailHeaders.h"
#include "EmailRead.h"

void EmailHandler::dispatch(SEGSEvent *ev)
{
    assert(ev);
    switch(ev->type())
    {
        case EmailEventTypes::evEmailHeader:
        on_email_header(static_cast<EmailHeaderMessage *>(ev));
        break;
        case EmailEventTypes::evEmailRead:
        on_email_read(static_cast<EmailReadMessage *>(ev));
        break;
        case EmailEventTypes::evEmailSend:
        on_email_send(static_cast<EmailSendMessage *>(ev));
        break;
        case EmailEventTypes::evEmailDelete:
        on_email_delete(static_cast<EmailDeleteMessage *>(ev));
        break;

        // will be obtained from MessageBusEndpoint
        case Internal_EventTypes::evClientConnected:
        on_client_connected(static_cast<ClientConnectedMessage *>(ev));
        break;
        case Internal_EventTypes::evClientDisconnected:
        on_client_disconnected(static_cast<ClientDisconnectedMessage *>(ev));
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

void EmailHandler::on_email_read(EmailReadMessage *msg)
{
    // later on, find the message from DB
    QString message = "Email ID \n" + QString::number(msg->m_data.id);

    EmailRead *emailRead = new EmailRead(
                msg->m_data.id,
                message,
                msg->m_data.src->m_name);

    msg->m_data.src->addCommandToSendNextUpdate(std::unique_ptr<EmailRead>(emailRead));
}

void EmailHandler::on_email_send(EmailSendMessage *msg)
{
    EmailHeaders *header = new EmailHeaders(
                msg->m_data.id,
                msg->m_data.sender,
                msg->m_data.subject,
                msg->m_data.timestamp);

    // saveEmailInDb()

    msg->m_data.src->addCommandToSendNextUpdate(std::unique_ptr<EmailHeaders>(header));
    // addCommandToSendNextUpdate for recipient?
}

void EmailHandler::on_email_delete(EmailDeleteMessage *msg)
{
    // delete email in db
}

void EmailHandler::on_client_connected(ClientConnectedMessage *msg)
{
}

void EmailHandler::on_client_disconnected(ClientDisconnectedMessage *msg)
{}

EmailHandler::EmailHandler() : m_message_bus_endpoint(*this)
{
    assert(HandlerLocator::getEmail_Handler() == nullptr);
    HandlerLocator::setEmail_Handler(this);

    m_message_bus_endpoint.subscribe(Internal_EventTypes::evClientConnected);
    m_message_bus_endpoint.subscribe(Internal_EventTypes::evClientDisconnected);
}
