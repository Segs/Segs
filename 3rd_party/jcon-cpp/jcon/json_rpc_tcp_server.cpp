#include "json_rpc_tcp_server.h"
#include "json_rpc_tcp_socket.h"
#include "jcon_assert.h"

namespace jcon {

JsonRpcTcpServer::JsonRpcTcpServer(QObject* parent,
                                   std::shared_ptr<JsonRpcLogger> logger)
    : JsonRpcServer(parent, logger)
    , m_server(this)
{
    m_server.connect(&m_server, &QTcpServer::newConnection,
                     this, &JsonRpcTcpServer::newConnection);
}

JsonRpcTcpServer::~JsonRpcTcpServer()
{
    m_server.disconnect(this);
    close();
}

bool JsonRpcTcpServer::listen(int port)
{
    logInfo(QString("listening on port %1").arg(port));
    if (!m_server.listen(QHostAddress::AnyIPv4, port)) {
        auto msg = QString("Error listening on port %1").arg(port);
        logError(qPrintable(msg));
        return false;
    }
    return true;
}

bool JsonRpcTcpServer::listen(const QHostAddress& addr, int port)
{
    logInfo(QString("listening on port %1").arg(port));
    if (!m_server.listen(addr, port)) {
        auto msg = QString("Error listening on %1:%2")
            .arg(addr.toString()).arg(port);
        logError(qPrintable(msg));
        return false;
    }
    return true;
}

void JsonRpcTcpServer::close()
{
    m_server.close();
}

JsonRpcEndpoint* JsonRpcTcpServer::findClient(QObject* socket)
{
    QTcpSocket* tcp_socket = qobject_cast<QTcpSocket*>(socket);
    auto it = m_client_endpoints.find(tcp_socket);
    return (it != m_client_endpoints.end()) ? it->second.get() : nullptr;
}

QVector<JsonRpcEndpoint*> JsonRpcTcpServer::getAllClients()
{
    int size = static_cast<int>(m_client_endpoints.size());
    QVector<JsonRpcEndpoint*> rpc_endpoints(size, nullptr);

    for (auto const& client_endpoint : m_client_endpoints)
        rpc_endpoints.append(client_endpoint.second.get());

    return rpc_endpoints;
}

void JsonRpcTcpServer::newConnection()
{
    JCON_ASSERT(m_server.hasPendingConnections());
    if (m_server.hasPendingConnections()) {
        QTcpSocket* tcp_socket = m_server.nextPendingConnection();
        JCON_ASSERT(m_server.nextPendingConnection() == nullptr);

        JCON_ASSERT(tcp_socket);
        if (!tcp_socket) {
            logError("pending socket was null");
            return;
        }

        logInfo("client connected: " + tcp_socket->peerAddress().toString());

        // TODO: maybe move this to base class?
        // {
        auto rpc_socket = std::make_shared<JsonRpcTcpSocket>(tcp_socket);

        auto endpoint =
            std::shared_ptr<JsonRpcEndpoint>(new JsonRpcEndpoint(rpc_socket, log(), this),
                [this](JsonRpcEndpoint* obj) {
                    if (this->m_server.isListening()) {
                        obj->deleteLater();
                    } else {
                        delete obj;
                    }
                });

        connect(endpoint.get(), &JsonRpcEndpoint::socketDisconnected,
                this, &JsonRpcTcpServer::disconnectClient);

        connect(endpoint.get(), &JsonRpcEndpoint::socketError,
                this, &JsonRpcServer::socketError);

        connect(endpoint.get(), &JsonRpcEndpoint::jsonObjectReceived,
                this, &JsonRpcServer::jsonRequestReceived);

        m_client_endpoints[tcp_socket] = endpoint;

        emit(clientConnected(tcp_socket));
        // }
    }
}

void JsonRpcTcpServer::disconnectClient(QObject* client_socket)
{
    QTcpSocket* tcp_socket = qobject_cast<QTcpSocket*>(client_socket);
    JCON_ASSERT(tcp_socket);
    if (!tcp_socket) {
        logError("client disconnected, but socket is null");
        return;
    }

    logInfo("client disconnected: " + tcp_socket->peerAddress().toString());
    auto it = m_client_endpoints.find(tcp_socket);
    JCON_ASSERT(it != m_client_endpoints.end());
    if (it == m_client_endpoints.end()) {
        logError("unknown client disconnected");
        return;
    }
    m_client_endpoints.erase(it);
    emit(clientDisconnected(client_socket));
}

bool jcon::JsonRpcTcpServer::isListening() const {
    return m_server.isListening();
}

}
