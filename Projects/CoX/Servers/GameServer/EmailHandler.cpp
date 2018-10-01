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
#include "serialization_common.h"
#include "serialization_types.h"

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
        case GameDBEventTypes::evEmailCreateResponse:
        on_email_create_response(static_cast<EmailCreateResponse *>(ev));
        case GameDBEventTypes::evGetEmailsResponse:
        on_get_emails_response(static_cast<GetEmailsResponse *>(ev));
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
    EmailData email_data = EmailData{
                msg->m_data.sender_id,
                msg->m_data.sender_id,
                QString("Test Sender"),
                msg->m_data.subject,
                QString("Placeholder Message \n Hi"),
                msg->m_data.timestamp,
                false};

    QString cerealizedEmailData;
    serializeToQString(email_data, cerealizedEmailData);

    m_db_handler->putq(new EmailCreateRequest({
                                                  msg->m_data.sender_id,
                                                  msg->m_data.sender_id, //recipient id
                                                  cerealizedEmailData
                                              }, uint64_t(1)));
}

void EmailHandler::on_email_create_response(EmailCreateResponse* msg)
{
    EmailData email_data;
    serializeFromQString(email_data, msg->m_data.m_email_data);

    m_state.m_stored_client_datas[msg->m_data.m_sender_id]
            .m_email_state.m_sent_email_ids.insert(msg->m_data.m_email_id);
    m_state.m_stored_client_datas[msg->m_data.m_recipient_id]
            .m_email_state.m_received_email_ids.insert(msg->m_data.m_email_id);
    m_state.m_stored_client_datas[msg->m_data.m_recipient_id]
            .m_email_state.m_unread_email_ids.insert(msg->m_data.m_email_id);

    const ClientSessionData &recipient_data (m_state.m_stored_client_datas[msg->m_data.m_sender_id]);

    EventProcessor *tgt = HandlerLocator::getMapInstance_Handler(
                recipient_data.m_server_id,
                recipient_data.m_instance_id);

    tgt->putq(new EmailHeaderResponse({
                                          msg->m_data.m_email_id,
                                          email_data.m_sender_name,
                                          email_data.m_subject,
                                          email_data.m_timestamp
                                      }, msg->session_token()));
}

void EmailHandler::on_get_emails_response(GetEmailsResponse *msg)
{
    EmailData email_data;
    m_state.m_stored_email_datas[0] = email_data;
}

void EmailHandler::on_email_read(EmailReadRequest *msg)
{
    EmailData& email_data (m_state.m_stored_email_datas[msg->m_data.email_id]);
    email_data.m_is_read_by_recipient = true;

    m_state.m_stored_client_datas[email_data.m_recipient_id].
            m_email_state.m_received_email_ids.erase(msg->m_data.email_id);

    QString cerealizedEmailData;
    serializeToQString(email_data, cerealizedEmailData);

    m_db_handler->putq(new EmailMarkAsReadMessage({msg->m_data.email_id, cerealizedEmailData}, uint64_t(1)));

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
    EmailData email_data = EmailData{
                msg->m_data.sender_id,
                msg->m_data.recipient_id,
                msg->m_data.sender_name,
                msg->m_data.subject,
                msg->m_data.message,
                msg->m_data.timestamp,
                false};

    QString cerealizedEmailData;
    serializeToQString(email_data, cerealizedEmailData);

    m_db_handler->putq(new EmailCreateRequest({
                                                  msg->m_data.sender_id,
                                                  msg->m_data.sender_id, //recipient id
                                                  cerealizedEmailData
                                              }, uint64_t(1)));
}

void EmailHandler::on_email_delete(EmailDeleteMessage *msg)
{
    m_db_handler->putq(new EmailRemoveMessage({msg->m_data.email_id}, uint64_t(1)));

    EmailData emailData = m_state.m_stored_email_datas[msg->m_data.email_id];
    m_state.m_stored_email_datas.erase(msg->m_data.email_id);

    // if the recipient requests an email delete, then surely he has read the email already.
    // so we don't need remove from m_unread_email_ids here
    m_state.m_stored_client_datas[emailData.m_sender_id].
            m_email_state.m_sent_email_ids.erase(msg->m_data.email_id);
    m_state.m_stored_client_datas[emailData.m_recipient_id].
            m_email_state.m_received_email_ids.erase(msg->m_data.email_id);
}

void EmailHandler::on_client_connected(ClientConnectedMessage *msg)
{
    PlayerEmailState emailState;
    fill_email_state(emailState, msg->m_data.m_char_db_id);

    // m_session is the key, m_server_id and m_sub_server_id are the values
    m_state.m_stored_client_datas[msg->m_data.m_char_db_id] =
            ClientSessionData{msg->m_data.m_session, msg->m_data.m_server_id, msg->m_data.m_sub_server_id, emailState};
    // send all emails where this client is the recipient
}

void EmailHandler::on_client_disconnected(ClientDisconnectedMessage *msg)
{
    if (m_state.m_stored_client_datas.count(msg->m_data.m_char_db_id) > 0)
        m_state.m_stored_client_datas.erase(msg->m_data.m_char_db_id);
}

void EmailHandler::fill_email_state(PlayerEmailState& emailState, uint32_t m_char_id)
{
    for(const auto &data : m_state.m_stored_email_datas)
    {
        if (data.second.m_sender_id == m_char_id)
            emailState.m_sent_email_ids.insert(data.first);
        if (data.second.m_recipient_id == m_char_id)
            emailState.m_received_email_ids.insert(data.first);
        if (data.second.m_recipient_id == m_char_id && !data.second.m_is_read_by_recipient)
            emailState.m_unread_email_ids.insert(data.first);
    }
}

EmailHandler::EmailHandler(int for_game_server_id) : m_message_bus_endpoint(*this)
{
    m_state.m_game_server_id = for_game_server_id;

    assert(HandlerLocator::getEmail_Handler() == nullptr);
    HandlerLocator::setEmail_Handler(this);

    m_message_bus_endpoint.subscribe(evClientConnectedMessage);
    m_message_bus_endpoint.subscribe(evClientDisconnectedMessage);

    set_db_handler(for_game_server_id);
    m_db_handler->putq(new GetEmailsRequest({}, uint64_t(1)));
}

void EmailHandler::serialize_from(std::istream &/*is*/)
{
    assert(false);
}

void EmailHandler::serialize_to(std::ostream &/*os*/)
{
    assert(false);
}
