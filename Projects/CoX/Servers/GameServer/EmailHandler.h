/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#ifndef EMAILHANDLER_H
#define EMAILHANDLER_H

#include "EventProcessor.h"
#include "Servers/MessageBusEndpoint.h"
#include "Servers/InternalEvents.h"
#include "GameDatabase/GameDBSyncHandler.h"
#include <map>

namespace SEGSEvents
{
    struct EmailHeaderRequest;
    struct EmailSendMessage;
    struct EmailReadRequest;
    struct EmailDeleteMessage;
    struct ClientConnectedMessage;
    struct ClientDisconnectedMessage;
}

struct ClientSessionData
{
    uint64_t m_session_token;
    uint32_t m_server_id;
    uint32_t m_instance_id;
};

struct EmailData
{
    uint32_t m_email_id;
    uint32_t m_sender_id;
    uint32_t m_recipient_id;
    QString m_sender_name;          // take this from db during runtime based on sender_id?
    QString m_subject;
    QString m_message;
    uint32_t m_timestamp;
    bool m_is_read_by_recipient;
};

template<class Archive>
static void serialize(Archive & archive, EmailData & src)
{
    archive(cereal::make_nvp("SenderName",src.m_sender_name));
    archive(cereal::make_nvp("Subject",src.m_subject));
    archive(cereal::make_nvp("Message",src.m_message));
    archive(cereal::make_nvp("TimeStamp",src.m_timestamp));
    archive(cereal::make_nvp("IsRead",src.m_is_read_by_recipient));
}

struct EmailHandlerState
{
    std::map<uint64_t, ClientSessionData> m_stored_client_datas;
    int m_game_server_id;
};

struct PlayerEmailState
{
    std::vector<EmailData> m_received_emails;
    std::vector<EmailData> m_sent_emails;
    std::set<uint32_t> m_unread_email_ids;
};

class EmailHandler : public EventProcessor
{
private:
    // EventProcessor interface
    void dispatch(SEGSEvents::Event *ev) override;

    void on_email_header(SEGSEvents::EmailHeaderRequest* msg);
    void on_email_send(SEGSEvents::EmailSendMessage* msg);
    void on_email_read(SEGSEvents::EmailReadRequest* msg);
    void on_email_delete(SEGSEvents::EmailDeleteMessage* msg);
    void on_client_connected(SEGSEvents::ClientConnectedMessage* msg);
    void on_client_disconnected(SEGSEvents::ClientDisconnectedMessage *msg);
protected:
    // this is probably temporary until we have the DB!
    std::map<uint32_t, EmailData> m_emails;
    uint32_t m_stored_email_count;
    MessageBusEndpoint m_message_bus_endpoint;
    GameDBSyncHandler* m_db_handler;
    void serialize_from(std::istream &is) override;
    void serialize_to(std::ostream &is) override;
    EmailHandlerState m_state;
    // QVector<ClientConnectedData> m_stored_client_datas;
public:
    EmailHandler(int for_game_server_id);
    void set_db_handler(uint8_t id);
    IMPL_ID(EmailHandler)
};

#endif // EMAILHANDLER_H
