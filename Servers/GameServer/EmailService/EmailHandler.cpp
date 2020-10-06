/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "EmailHandler.h"
#include "Common/Servers/HandlerLocator.h"
#include "Messages/GameDatabase/GameDBSyncEvents.h"
#include "Messages/EmailService/EmailEvents.h"
#include "Components/serialization_common.h"
#include "Components/serialization_types.h"

using namespace SEGSEvents;

void EmailHandler::dispatch(Event *ev)
{
    assert(ev);
    switch(ev->type())
    {
        // EmailEvents
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
        // GameDbEvents
        case GameDBEventTypes::evEmailCreateResponse:
            on_email_create_response(static_cast<EmailCreateResponse *>(ev));
            break;
        case GameDBEventTypes::evGetEmailsResponse:
            on_get_emails_response(static_cast<GetEmailsResponse *>(ev));
            break;
        case GameDBEventTypes::evFillEmailRecipientIdResponse:
            on_fill_email_recipient_id_response(static_cast<FillEmailRecipientIdResponse *>(ev));
            break;
        case GameDBEventTypes::evFillEmailRecipientIdErrorMessage:
            on_fill_email_recipient_id_error(static_cast<FillEmailRecipientIdErrorMessage *>(ev));
            break;
        // will be obtained from MessageBusEndpoint
        case Internal_EventTypes::evClientConnectedMessage:
            on_client_connected(static_cast<ClientConnectedMessage *>(ev));
            break;
        case Internal_EventTypes::evClientDisconnectedMessage:
            on_client_disconnected(static_cast<ClientDisconnectedMessage *>(ev));
            break;
        default:
            qCritical() << "EmailHandler dispatch hits default! Event info: " + QString(ev->info());
            break;
    }
}

void EmailHandler::set_db_handler(uint8_t id)
{
    m_db_handler = static_cast<GameDBSyncHandler*>(
                HandlerLocator::getGame_DB_Handler(id));
}

void EmailHandler::on_email_create_response(EmailCreateResponse* msg)
{
    EmailData email_data;
    serializeFromQString(email_data, msg->m_data.m_cerealized_email_data);

    m_state.m_stored_email_datas[msg->m_data.m_email_id] = email_data;
    m_state.m_stored_email_datas[msg->m_data.m_email_id].m_sender_id = msg->m_data.m_sender_id;
    m_state.m_stored_email_datas[msg->m_data.m_email_id].m_recipient_id = msg->m_data.m_recipient_id;

    m_state.m_stored_client_datas[msg->m_data.m_sender_id]
            .m_email_state.m_sent_email_ids.insert(msg->m_data.m_email_id);

    // send successful EmailCreateStatusMessage to sender
    // shouldn't need to check if sender is online... when the sender is sending the email in real-time
    const ClientSessionData &sender_data (m_state.m_stored_client_datas[msg->m_data.m_sender_id]);

    EventProcessor *sender_map_instance = HandlerLocator::getMapInstance_Handler(
                sender_data.m_server_id,
                sender_data.m_instance_id);

    sender_map_instance->putq(new EmailCreateStatusMessage(
        {true, msg->m_data.m_recipient_name}, sender_data.m_session_token));

    // if the recipient is not online during the time of send, end the function here
    if (m_state.m_stored_client_datas.count(msg->m_data.m_recipient_id) <= 0)
        return;

    const ClientSessionData &recipient_data (m_state.m_stored_client_datas[msg->m_data.m_recipient_id]);

    m_state.m_stored_client_datas[msg->m_data.m_recipient_id]
            .m_email_state.m_received_email_ids.insert(msg->m_data.m_email_id);
    m_state.m_stored_client_datas[msg->m_data.m_recipient_id]
            .m_email_state.m_unread_email_ids.insert(msg->m_data.m_email_id);

    EventProcessor *recipient_map_instance = HandlerLocator::getMapInstance_Handler(
                recipient_data.m_server_id,
                recipient_data.m_instance_id);

    recipient_map_instance->putq(new EmailHeaderToClientMessage({
                                          msg->m_data.m_email_id,
                                          email_data.m_sender_name,
                                          email_data.m_subject,
                                          email_data.m_timestamp
                                      }, recipient_data.m_session_token));
}

void EmailHandler::on_email_header(EmailHeaderRequest *msg)
{
    std::vector<EmailHeaderData> email_headers;
    int unread_emails_count = 0;
    fill_email_headers(email_headers, msg->m_data.m_user_id, unread_emails_count);

    const ClientSessionData &session (m_state.m_stored_client_datas[msg->m_data.m_user_id]);
    EventProcessor *session_map_instance = HandlerLocator::getMapInstance_Handler(
                session.m_server_id,
                session.m_instance_id);

    session_map_instance->putq(new EmailHeaderResponse({email_headers}, msg->session_token()));
}

void EmailHandler::on_get_emails_response(GetEmailsResponse *msg)
{
    for(const auto &data : msg->m_data.m_email_response_datas)
    {
        EmailData email_data;
        serializeFromQString(email_data, data.m_cerealized_email_data);

        email_data.m_sender_id = data.m_sender_id;
        email_data.m_recipient_id = data.m_recipient_id;

        m_state.m_stored_email_datas[data.m_email_id] = email_data;
    }
}

void EmailHandler::on_email_read(EmailReadRequest *msg)
{
    // on_email_read comes from recipient, so they are always online in this case
    EmailData& email_data (m_state.m_stored_email_datas[msg->m_data.m_email_id]);
    email_data.m_is_read_by_recipient = true;

    m_state.m_stored_client_datas[email_data.m_recipient_id].
            m_email_state.m_received_email_ids.erase(msg->m_data.m_email_id);

    if(email_data.m_sender_id == 0)
        email_data.m_sender_name = "DELETED CHARACTER";

    QString cerealizedEmailData;
    serializeToQString(email_data, cerealizedEmailData);

    m_db_handler->putq(new EmailMarkAsReadMessage({msg->m_data.m_email_id, cerealizedEmailData}, uint64_t(1)));

    // the sender is not always online in this case
    if(m_state.m_stored_client_datas.count(email_data.m_sender_id) > 0)
    {
        const ClientSessionData &sender_data (m_state.m_stored_client_datas[email_data.m_sender_id]);
        EventProcessor *sender_map_instance = HandlerLocator::getMapInstance_Handler(
                    sender_data.m_server_id,
                    sender_data.m_instance_id);

        QString msgToSender = QString("Your email with subject %1 has been read by its recipient!").arg(email_data.m_subject);

        sender_map_instance->putq(new EmailWasReadByRecipientMessage(
            {msgToSender}, sender_data.m_session_token));
    }

    const ClientSessionData &recipient_data (m_state.m_stored_client_datas[email_data.m_recipient_id]);

    EventProcessor *recipient_map_instance = HandlerLocator::getMapInstance_Handler(
                recipient_data.m_server_id,
                recipient_data.m_instance_id);

    recipient_map_instance->putq(new EmailReadResponse(
        {msg->m_data.m_email_id, email_data.m_message, email_data.m_sender_name}, recipient_data.m_session_token));
}

void EmailHandler::on_email_send(EmailSendMessage *msg)
{
    // from this, you still need:
    // 1. email_id, that comes from creating the email in db
    // 2. recipient_id, which will be searched based on recipient_name
    // so, the route is emailSend -> FillEmailRecipientId -> EmailCreate
    m_db_handler->putq(new FillEmailRecipientIdRequest({
                                                           msg->m_data.m_sender_id,
                                                           msg->m_data.m_sender_name,
                                                           msg->m_data.m_recipient_name,
                                                           msg->m_data.m_subject,
                                                           msg->m_data.m_message,
                                                           msg->m_data.m_timestamp
                                                       }, uint64_t(1)));
}

void EmailHandler::on_fill_email_recipient_id_response(FillEmailRecipientIdResponse *msg)
{
    EmailData email_data = EmailData{
                msg->m_data.m_sender_id,
                msg->m_data.m_recipient_id,
                msg->m_data.m_sender_name,
                msg->m_data.m_subject,
                msg->m_data.m_message,
                msg->m_data.m_timestamp,
                false};

    QString cerealizedEmailData;
    serializeToQString(email_data, cerealizedEmailData);

    m_db_handler->putq(new EmailCreateRequest({
                                                  msg->m_data.m_sender_id,
                                                  msg->m_data.m_recipient_id, //recipient id
                                                  msg->m_data.m_recipient_name,
                                                  cerealizedEmailData
                                              }, uint64_t(1)));
}

void EmailHandler::on_fill_email_recipient_id_error(FillEmailRecipientIdErrorMessage *msg)
{
    const ClientSessionData &sender_data (m_state.m_stored_client_datas[msg->m_data.m_sender_id]);
    EventProcessor *sender_map_instance = HandlerLocator::getMapInstance_Handler(
                sender_data.m_server_id,
                sender_data.m_instance_id);

    // send error msg to sender map instance

    sender_map_instance->putq(new EmailCreateStatusMessage(
            {false, msg->m_data.m_recipient_name}, sender_data.m_session_token));

}

void EmailHandler::on_email_delete(EmailDeleteMessage *msg)
{
    m_db_handler->putq(new EmailRemoveMessage({msg->m_data.m_email_id}, uint64_t(1)));

    EmailData emailData = m_state.m_stored_email_datas[msg->m_data.m_email_id];
    m_state.m_stored_email_datas.erase(msg->m_data.m_email_id);

    // if the recipient requests an email delete, then surely he has read the email already.
    // so we don't need remove from m_unread_email_ids here
    m_state.m_stored_client_datas[emailData.m_sender_id].
            m_email_state.m_sent_email_ids.erase(msg->m_data.m_email_id);
    m_state.m_stored_client_datas[emailData.m_recipient_id].
            m_email_state.m_received_email_ids.erase(msg->m_data.m_email_id);
}

void EmailHandler::on_client_connected(ClientConnectedMessage *msg)
{
    PlayerEmailState email_state;
    fill_email_state(email_state, msg->m_data.m_char_db_id);

    // m_session is the key, m_server_id and m_sub_server_id are the values
    m_state.m_stored_client_datas[msg->m_data.m_char_db_id] =
            ClientSessionData{msg->m_data.m_session, msg->m_data.m_server_id, msg->m_data.m_sub_server_id, email_state};
    // send all emails where this client is the recipient

    std::vector<EmailHeaderData> email_headers;
    int unread_emails_count = 0;
    fill_email_headers(email_headers, msg->m_data.m_char_db_id, unread_emails_count);

    EventProcessor *tgt = HandlerLocator::getMapInstance_Handler(
                msg->m_data.m_server_id,
                msg->m_data.m_sub_server_id);

    tgt->putq(new EmailHeadersToClientMessage({email_headers, unread_emails_count}, msg->m_data.m_session));
}

void EmailHandler::on_client_disconnected(ClientDisconnectedMessage *msg)
{
    if(m_state.m_stored_client_datas.count(msg->m_data.m_char_db_id) > 0)
        m_state.m_stored_client_datas.erase(msg->m_data.m_char_db_id);
}

void EmailHandler::fill_email_state(PlayerEmailState& email_state, uint32_t char_id)
{
    for(const auto &data : m_state.m_stored_email_datas)
    {
        if(data.second.m_sender_id == char_id)
            email_state.m_sent_email_ids.insert(data.first);
        if(data.second.m_recipient_id == char_id)
            email_state.m_received_email_ids.insert(data.first);
        if(data.second.m_recipient_id == char_id && !data.second.m_is_read_by_recipient)
            email_state.m_unread_email_ids.insert(data.first);
    }
}

void EmailHandler::fill_email_headers(std::vector<EmailHeaderData>& email_headers, uint32_t char_id, int &unread_emails_count)
{
    for(const auto &email_id: m_state.m_stored_client_datas[char_id].m_email_state.m_received_email_ids)
    {
        EmailHeaderData email_header_data;
        email_header_data.m_email_id = email_id;
        email_header_data.m_sender_name = m_state.m_stored_email_datas[email_id].m_sender_name;
        email_header_data.m_subject = m_state.m_stored_email_datas[email_id].m_subject;
        email_header_data.m_timestamp = m_state.m_stored_email_datas[email_id].m_timestamp;

        email_headers.push_back(email_header_data);

        if(!m_state.m_stored_email_datas[email_id].m_is_read_by_recipient)
            unread_emails_count++;
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
