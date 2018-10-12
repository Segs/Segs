#include "json_rpc_websocket_server.h"
#include "json_rpc_websocket.h"
#include "jcon_assert.h"

#include <QWebSocket>
#include <QWebSocketServer>

namespace jcon {

JsonRpcWebSocketServer::JsonRpcWebSocketServer(
    QObject* parent,
    std::shared_ptr<JsonRpcLogger> logger)
    : JsonRpcServer(parent, logger)
    , m_server(new QWebSocketServer("JSON RPC WebSocket server",
                                    QWebSocketServer::NonSecureMode,
                                    this))
{
    m_server->connect(m_server, &QWebSocketServer::newConnection,
                      this, &JsonRpcWebSocketServer::newConnection);
}

JsonRpcWebSocketServer::~JsonRpcWebSocketServer()
{
    m_server->disconnect(this);
    close();
    delete m_server;
    m_server = nullptr;
}

void JsonRpcWebSocketServer::listen(int port)
{
    logInfo(QString("listening on port %2").arg(port));
    m_server->listen(QHostAddress::AnyIPv4, port);
}


void JsonRpcWebSocketServer::listen(const QHostAddress& addr, int port)
{
    logInfo(QString("listening on port %2").arg(port));
    m_server->listen(addr, port);
}

void JsonRpcWebSocketServer::close()
{
    m_server->close();
}

JsonRpcEndpoint* JsonRpcWebSocketServer::findClient(QObject* socket)
{
    QWebSocket* web_socket = qobject_cast<QWebSocket*>(socket);
    auto it = m_client_endpoints.find(web_socket);
    return (it != m_client_endpoints.end()) ? it->second.get() : nullptr;
}

void JsonRpcWebSocketServer::newConnection()
{
    JCON_ASSERT(m_server->hasPendingConnections());
    if (m_server->hasPendingConnections()) {
        QWebSocket* web_socket = m_server->nextPendingConnection();

        JCON_ASSERT(web_socket);
        if (!web_socket) {
            logError("pending socket was null");
            return;
        }

        logInfo("client connected: " + web_socket->peerAddress().toString());

        // TODO: maybe move this to base class?
        // {
        auto rpc_socket = std::make_shared<JsonRpcWebSocket>(web_socket);

        auto endpoint =
            std::make_shared<JsonRpcEndpoint>(rpc_socket, log(), this);

        connect(endpoint.get(), &JsonRpcEndpoint::socketDisconnected,
                this, &JsonRpcWebSocketServer::disconnectClient);

        connect(endpoint.get(), &JsonRpcEndpoint::jsonObjectReceived,
                this, &JsonRpcServer::jsonRequestReceived);

        m_client_endpoints[web_socket] = endpoint;
        emit(clientConnected(web_socket));
        // }
    }
}

void JsonRpcWebSocketServer::disconnectClient(QObject* client_socket)
{
    QWebSocket* socket = qobject_cast<QWebSocket*>(client_socket);
    JCON_ASSERT(socket);
    if (!socket) {
        logError("client disconnected, but socket is null");
        return;
    }

    logInfo("client disconnected: " + socket->peerAddress().toString());
    auto it = m_client_endpoints.find(socket);
    JCON_ASSERT(it != m_client_endpoints.end());
    if (it == m_client_endpoints.end()) {
        logError("unknown client disconnected");
        return;
    }
    m_client_endpoints.erase(it);
    emit(clientDisconnected(client_socket));
}

}
