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
    struct EmailReadMessage;
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

struct EmailHandlerState
{
    std::unordered_map<uint64_t, ClientSessionData> m_stored_client_datas;
    int m_game_server_id;
};

class EmailHandler : public EventProcessor
{
private:
    // EventProcessor interface
    void dispatch(SEGSEvents::Event *ev) override;

    void on_email_header(SEGSEvents::EmailHeaderRequest* msg);
    void on_email_send(SEGSEvents::EmailSendMessage* msg);
    void on_email_read(SEGSEvents::EmailReadMessage* msg);
    void on_email_delete(SEGSEvents::EmailDeleteMessage* msg);
    void on_client_connected(SEGSEvents::ClientConnectedMessage* msg);
    void on_client_disconnected(SEGSEvents::ClientDisconnectedMessage *msg);
protected:
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
