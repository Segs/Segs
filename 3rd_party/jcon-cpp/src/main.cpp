#include "example_service.h"

#include <jcon/json_rpc_file_logger.h>
#include <jcon/json_rpc_tcp_client.h>
#include <jcon/json_rpc_tcp_server.h>
#include <jcon/json_rpc_websocket_client.h>
#include <jcon/json_rpc_websocket_server.h>

#include <QCoreApplication>
#include <QPushButton>
#include <QThread>
#include <QUrl>

#include <ctime>
#include <iostream>
#include <memory>

void startServer(QObject* parent, bool tcp = true)
{
    jcon::JsonRpcServer* rpc_server;
    if (tcp) {
        qDebug() << "Creating TCP server";
        rpc_server = new jcon::JsonRpcTcpServer(parent);
    } else {
        qDebug() << "Creating WebSocket server";
        rpc_server = new jcon::JsonRpcWebSocketServer(parent);
    }
    auto service = new ExampleService;
    rpc_server->registerServices({ service });
    rpc_server->listen(6002);
}

jcon::JsonRpcClient* startClient(QObject* parent, bool tcp = true)
{
    jcon::JsonRpcClient* rpc_client;
    if (tcp) {
        rpc_client = new jcon::JsonRpcTcpClient(parent);
        rpc_client->connectToServer("127.0.0.1", 6002);
    } else {
        rpc_client = new jcon::JsonRpcWebSocketClient(parent);
        // This is just to illustrate the fact that connectToServer also accepts
        // a QUrl argument.
        rpc_client->connectToServer(QUrl("ws://127.0.0.1:6002"));
    }
    return rpc_client;
}

void invokeMethodAsync(jcon::JsonRpcClient* rpc_client)
{
    qsrand(std::time(nullptr));

    auto req = rpc_client->callAsync("getRandomInt", 10);

    req->connect(req.get(), &jcon::JsonRpcRequest::result,
                 [](const QVariant& result) {
                     qDebug() << "result of asynchronous RPC call:" << result;
                 });

    req->connect(req.get(), &jcon::JsonRpcRequest::error,
                 [](int code, const QString& message, const QVariant& data) {
                     qDebug() << "RPC error:" << message
                              << " (" << code << ")";
                 });
}

void invokeMethodSync(jcon::JsonRpcClient* rpc_client)
{
    qsrand(std::time(nullptr));

    auto result = rpc_client->call("getRandomInt", 100);

    if (result->isSuccess()) {
        qDebug() << "result of synchronous RPC call:" << result->result();
    } else {
        qDebug() << "RPC error:" << result->toString();
    }
}

void invokeStringMethodAsync(jcon::JsonRpcClient* rpc_client)
{
    auto req = rpc_client->callAsync("printMessage", "hello, world");

    req->connect(req.get(), &jcon::JsonRpcRequest::result,
                 [](const QVariant& result) {
                     qDebug() << "result of asynchronous RPC call:" << result;
                 });

    req->connect(req.get(), &jcon::JsonRpcRequest::error,
                 [](int code, const QString& message, const QVariant& data) {
                     qDebug() << "RPC error:" << message
                              << " (" << code << ")";
                 });
}

void invokeStringMethodSync(jcon::JsonRpcClient* rpc_client)
{
    qsrand(std::time(nullptr));

    auto result = rpc_client->call("printMessage", "hello, world");

    if (result->isSuccess()) {
        qDebug() << "result of synchronous RPC call:" << result->result();
    } else {
        qDebug() << "RPC error:" << result->toString();
    }
}

void invokeNotification(jcon::JsonRpcClient *rpc_client) {
    rpc_client->notification("printNotification", "hello, world Notification");
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    startServer(&app, false);
    auto rpc_client = startClient(&app, false);

    invokeNotification(rpc_client);
    invokeMethodAsync(rpc_client);
    invokeMethodSync(rpc_client);
    invokeStringMethodSync(rpc_client);
    invokeStringMethodAsync(rpc_client);

    if (rpc_client->outstandingRequestCount() > 0) {
        qDebug().noquote() << QString("Waiting for %1 outstanding requests")
            .arg(rpc_client->outstandingRequestCount());

        while (rpc_client->outstandingRequestCount() > 0) {
            qDebug() << "Calling QCoreApplication::processEvents()";
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        }
    } else {
        qDebug() << "No outstanding requests, quitting";
    }
}
