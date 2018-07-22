/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#ifndef EMAILHANDLER_H
#define EMAILHANDLER_H

#include "EventProcessor.h"
#include "EmailEvents.h"
#include "Servers/MessageBusEndpoint.h"

struct ServerIds
{
    uint32_t m_server_id;
    uint32_t m_sub_server_id;
};

class EmailHandler : public EventProcessor
{
private:
    // EventProcessor interface
    void dispatch(SEGSEvent *ev) override;

    void on_email_header(EmailHeaderRequest* msg);
    void on_email_send(EmailSendMessage* msg);
    void on_email_read(EmailReadMessage* msg);
    void on_email_delete(EmailDeleteMessage* msg);
    void on_client_connected(ClientConnectedMessage* msg);
    void on_client_disconnected(ClientDisconnectedMessage *msg);
protected:
    MessageBusEndpoint m_message_bus_endpoint;
    std::map<uint64_t, ServerIds> m_stored_client_datas;
    // QVector<ClientConnectedData> m_stored_client_datas;
public:
    EmailHandler();
};

#endif // EMAILHANDLER_H
