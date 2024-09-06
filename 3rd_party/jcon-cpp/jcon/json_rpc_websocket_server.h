#pragma once

#include "jcon.h"
#include "json_rpc_server.h"
#include "json_rpc_endpoint.h"
#include "json_rpc_socket.h"

#include <map>

class QWebSocket;
class QWebSocketServer;

namespace jcon {

class JCON_API JsonRpcWebSocketServer : public JsonRpcServer
{
    Q_OBJECT

public:
    JsonRpcWebSocketServer(QObject* parent = nullptr,
                           std::shared_ptr<JsonRpcLogger> logger = nullptr);
    virtual ~JsonRpcWebSocketServer();

    bool listen(int port) override;
    bool listen(const QHostAddress& addr, int port) override;
    bool isListening() const;
    void close() override;

protected:
    JsonRpcEndpoint* findClient(QObject* socket) override;
    QVector<JsonRpcEndpoint*> getAllClients() override;

private slots:
    /// Called when the underlying QWebSocketServer gets a new client
    /// connection.
    void newConnection() override;

    /// Called when the underlying QWebSocketServer loses a client connection.
    void disconnectClient(QObject* client_socket) override;

private:
    QWebSocketServer* m_server;

    /// Clients are uniquely identified by their QWebSocket*.
    std::map<QWebSocket*, std::shared_ptr<JsonRpcEndpoint>> m_client_endpoints;
};

}
