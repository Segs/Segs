/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "EmailHandler.h"
#include "Common/Servers/HandlerLocator.h"
#include "GameEvents.h"
#include "EmailEvents.h"

using namespace SEGSEvents;

void EmailHandler::dispatch(Event *ev)
{
    assert(ev);
    switch(ev->type())
    {
        case EmailEventTypes::evEmailHeaderRequest:
        on_email_header(static_cast<EmailHeaderRequest *>(ev));
        break;
        case EmailEventTypes::evEmailReadRequest:
        on_email_read(static_cast<EmailReadRequest *>(ev));
        break;
        case EmailEventTypes::evEmailSendMessage:
        on_email_send(static_cast<EmailSendMessage *>(ev));
        break;
        case EmailEventTypes::evEmailDeleteMessage:
        on_email_delete(static_cast<EmailDeleteMessage *>(ev));
        break;

        // will be obtained from MessageBusEndpoint
        case Internal_EventTypes::evClientConnectedMessage:
        on_client_connected(static_cast<ClientConnectedMessage *>(ev));
        break;
        case Internal_EventTypes::evClientDisconnectedMessage:
        on_client_disconnected(static_cast<ClientDisconnectedMessage *>(ev));
        break;
        default: assert(false); break;
    }
}

void EmailHandler::set_db_handler(uint8_t id)
{
    m_db_handler = static_cast<GameDBSyncHandler*>(
                HandlerLocator::getGame_DB_Handler(id));
}

void EmailHandler::on_email_header(EmailHeaderRequest *msg)
{
    // this function is test to send email to self :)

    const ClientSessionData &recipient_data (m_state.m_stored_client_datas[msg->m_data.sender_id]);

    m_emails[m_stored_email_count] = EmailData{
                m_stored_email_count,
                msg->m_data.sender_id,
                msg->m_data.sender_id,
                QString("Test Sender"),
                msg->m_data.subject,
                QString("Placeholder Message \n Hi"),
                msg->m_data.timestamp,
                false};

    EventProcessor *tgt = HandlerLocator::getMapInstance_Handler(
                recipient_data.m_server_id,
                recipient_data.m_instance_id);

    tgt->putq(new EmailHeaderResponse({
                                          m_stored_email_count,
                                          msg->m_data.sender_name,
                                          msg->m_data.subject,
                                          msg->m_data.timestamp
                                      }, msg->session_token()));
    m_stored_email_count++;
}

void EmailHandler::on_email_read(EmailReadRequest *msg)
{
    EmailData& email_data (m_emails[msg->m_data.email_id]);
    email_data.m_is_read_by_recipient = true;

    const ClientSessionData &sender_data (m_state.m_stored_client_datas[email_data.m_sender_id]);
    const ClientSessionData &recipient_data (m_state.m_stored_client_datas[email_data.m_recipient_id]);

    EventProcessor *sender_map_instance = HandlerLocator::getMapInstance_Handler(
                sender_data.m_server_id,
                sender_data.m_instance_id);

    EventProcessor *recipient_map_instance = HandlerLocator::getMapInstance_Handler(
                recipient_data.m_server_id,
                recipient_data.m_instance_id);

    sender_map_instance->putq(new EmailWasReadByRecipientMessage(
        {msg->m_data.email_id}, sender_data.m_session_token));
    recipient_map_instance->putq(new EmailReadResponse(
        {msg->m_data.email_id, email_data.m_message, email_data.m_sender_name}, recipient_data.m_session_token));
}

void EmailHandler::on_email_send(EmailSendMessage *msg)
{
    /*EmailHeaders *header = new EmailHeaders(
                msg->m_data.id,
                msg->m_data.sender,
                msg->m_data.subject,
                msg->m_data.timestamp); */

    // saveEmailInDb()

    // msg->m_data.src->addCommandToSendNextUpdate(std::unique_ptr<EmailHeaders>(header));

    // TODO: Send EmailHeaderResponse to the relevant EventProcessor (in this case MapInstance)
    // based on the server_id and subserver_id retrieved based on session_id
    // then the MapInstance can addCommandToSendNextUpdate for the appropriate session/entity

    // addCommandToSendNextUpdate for recipient?
}

void EmailHandler::on_email_delete(EmailDeleteMessage *msg)
{
    // delete email in db and that's it, shouldn't be any need to toss responses and messages around
    m_emails.erase(msg->m_data.email_id);
}

void EmailHandler::on_client_connected(ClientConnectedMessage *msg)
{
    // m_session is the key, m_server_id and m_sub_server_id are the values
    m_state.m_stored_client_datas[msg->m_data.m_char_db_id] =
            ClientSessionData{msg->m_data.m_session, msg->m_data.m_server_id, msg->m_data.m_sub_server_id};

    // send all emails where this client is the recipient
}

void EmailHandler::on_client_disconnected(ClientDisconnectedMessage *msg)
{
    if (m_state.m_stored_client_datas.count(msg->m_data.m_char_db_id) > 0)
        m_state.m_stored_client_datas.erase(msg->m_data.m_char_db_id);
}

EmailHandler::EmailHandler(int for_game_server_id) : m_message_bus_endpoint(*this)
{
    m_state.m_game_server_id = for_game_server_id;

    assert(HandlerLocator::getEmail_Handler() == nullptr);
    HandlerLocator::setEmail_Handler(this);

    m_message_bus_endpoint.subscribe(evClientConnectedMessage);
    m_message_bus_endpoint.subscribe(evClientDisconnectedMessage);
}

void EmailHandler::serialize_from(std::istream &/*is*/)
{
    assert(false);
}

void EmailHandler::serialize_to(std::ostream &/*os*/)
{
    assert(false);
}
