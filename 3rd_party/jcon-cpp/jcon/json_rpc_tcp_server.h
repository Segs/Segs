#pragma once

#include "jcon.h"
#include "json_rpc_server.h"
#include "json_rpc_endpoint.h"
#include "json_rpc_socket.h"

#include <QTcpServer>

#include <map>

namespace jcon {

class JCON_API JsonRpcTcpServer : public JsonRpcServer
{
    Q_OBJECT

public:
    JsonRpcTcpServer(QObject* parent = nullptr,
                     std::shared_ptr<JsonRpcLogger> logger = nullptr);
    virtual ~JsonRpcTcpServer();

    bool listen(int port) override;
    bool listen(const QHostAddress& addr, int port) override;
    bool isListening() const;
    void close() override;

protected:
    JsonRpcEndpoint* findClient(QObject* socket) override;
    QVector<JsonRpcEndpoint*> getAllClients() override;

private slots:
    /// Called when the underlying QTcpServer gets a new client connection.
    void newConnection() override;

    /// Called when the underlying QTcpServer loses a client connection.
    void disconnectClient(QObject* client_socket) override;

private:
    QTcpServer m_server;

    /// Clients are uniquely identified by their QTcpSocket*.
    std::map<QTcpSocket*, std::shared_ptr<JsonRpcEndpoint>> m_client_endpoints;
};

}
