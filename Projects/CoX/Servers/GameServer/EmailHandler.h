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

struct ClientSessionData
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
    std::map<uint64_t, ClientSessionData> m_stored_client_datas;
    GameDBSyncHandler* m_db_handler;
    // QVector<ClientConnectedData> m_stored_client_datas;
public:
    EmailHandler();
    void set_db_handler(uint8_t id);
};

#endif // EMAILHANDLER_H
