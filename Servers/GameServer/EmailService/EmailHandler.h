/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Components/EventProcessor.h"
#include "Servers/MessageBusEndpoint.h"
#include "Servers/InternalEvents.h"
#include "GameDatabase/GameDBSyncHandler.h"
#include "Messages/EmailService/EmailDefinitions.h"
#include <map>

namespace SEGSEvents
{
    struct EmailHeaderRequest;
    struct EmailSendMessage;
    struct EmailReadRequest;
    struct EmailDeleteMessage;
    struct ClientConnectedMessage;
    struct ClientDisconnectedMessage;
    struct GetEmailsResponse;
    struct FillEmailRecipientIdResponse;
    struct FillEmailRecipientIdErrorMessage;
}

struct PlayerEmailState
{
    // this uint32_t that are keys for m_received_emails and m_sent_emails is email id
    std::set<uint32_t> m_sent_email_ids;
    std::set<uint32_t> m_received_email_ids;
    std::set<uint32_t> m_unread_email_ids;
};

struct ClientSessionData
{
    uint64_t m_session_token;
    uint32_t m_server_id;
    uint32_t m_instance_id;
    PlayerEmailState m_email_state;
};

struct EmailHandlerState
{
    // the uint32_t is character db id
    std::map<uint32_t, ClientSessionData> m_stored_client_datas;
    std::map<uint32_t, EmailData> m_stored_email_datas;
    int m_game_server_id;
};

class EmailHandler : public EventProcessor
{
private:
    GameDBSyncHandler* m_db_handler;

    // EventProcessor interface
    void dispatch(SEGSEvents::Event *ev) override;

    void on_email_header(SEGSEvents::EmailHeaderRequest* msg);
    void on_email_send(SEGSEvents::EmailSendMessage* msg);
    void on_email_read(SEGSEvents::EmailReadRequest* msg);
    void on_email_delete(SEGSEvents::EmailDeleteMessage* msg);
    void on_client_connected(SEGSEvents::ClientConnectedMessage* msg);
    void on_client_disconnected(SEGSEvents::ClientDisconnectedMessage *msg);
    void on_email_create_response(SEGSEvents::EmailCreateResponse* msg);
    void on_get_emails_response(SEGSEvents::GetEmailsResponse* msg);
    void on_fill_email_recipient_id_response(SEGSEvents::FillEmailRecipientIdResponse* msg);
    void on_fill_email_recipient_id_error(SEGSEvents::FillEmailRecipientIdErrorMessage* msg);

    void fill_email_state(PlayerEmailState& emailState, uint32_t char_id);
    void fill_email_headers(std::vector<EmailHeaderData>& emailHeaders, uint32_t char_id, int &unread_emails_count);
protected:
    MessageBusEndpoint m_message_bus_endpoint;
    void serialize_from(std::istream &is) override;
    void serialize_to(std::ostream &is) override;
    EmailHandlerState m_state;
public:
    EmailHandler(int for_game_server_id);
    void set_db_handler(uint8_t id);
    IMPL_ID(EmailHandler)
};
