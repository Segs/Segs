#pragma once

#include "jcon.h"
#include "json_rpc_endpoint.h"
#include "json_rpc_error.h"
#include "json_rpc_logger.h"
#include "json_rpc_request.h"
#include "json_rpc_result.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMap>

#include <memory>
#include <utility>

namespace jcon {

class JsonRpcSocket;

class JCON_API JsonRpcClient : public QObject
{
    Q_OBJECT

public:
    using RequestId = QString ;

    JsonRpcClient(std::shared_ptr<JsonRpcSocket> socket,
                  QObject* parent = nullptr,
                  std::shared_ptr<JsonRpcLogger> logger = nullptr,
                  int call_timeout_ms = 60000);

    virtual ~JsonRpcClient();

    /// @return true if connection was successful
    bool connectToServer(const QString& host, int port);
    bool connectToServer(const QUrl& url);
    void disconnectFromServer();

    bool isConnected() const;

    QHostAddress clientAddress() const;
    int clientPort() const;

    QHostAddress serverAddress() const;
    int serverPort() const;

    template<typename... T>
    std::shared_ptr<JsonRpcResult> call(const QString& method, T&&... args);

    template<typename... T>
    std::shared_ptr<JsonRpcRequest> callAsync(const QString& method,
                                              T&&... args);

    /// Expand arguments in list before making the RPC call
    std::shared_ptr<JsonRpcResult> callExpandArgs(const QString& method,
                                                  const QVariantList& args);

    /// Expand arguments in list before making the RPC call
    std::shared_ptr<JsonRpcRequest>
        callAsyncExpandArgs(const QString& method, const QVariantList& args);

    template<typename... T>
    void notification(const QString& method, T&&... args);

    int outstandingRequestCount() const;

signals:
    /// Emitted when a connection has been made to the server.
    void socketConnected(QObject* socket);

    /// Emitted when connection to server is lost.
    void socketDisconnected(QObject* socket);

    /// Emitted when the RPC socket has an error.
    void socketError(QObject* socket, QAbstractSocket::SocketError error);

protected:
    void logError(const QString& msg);

private slots:
    void jsonResponseReceived(const QJsonObject& obj);

private:
    static const QString InvalidRequestId;

    static QString formatLogMessage(const QString& method,
                                    const QVariantList& args,
                                    bool async,
                                    const QString& request_id);

    std::shared_ptr<JsonRpcResult>
        waitForSyncCallbacks(const JsonRpcRequest* request);

    template<typename... T>
    std::shared_ptr<JsonRpcRequest> doCall(const QString& method,
                                           bool async,
                                           T&&... args);

    std::shared_ptr<JsonRpcRequest>
        doCallExpandArgs(const QString& method,
                         bool async,
                         const QVariantList& args);

    template<typename... T>
    void doNotification(const QString& method, T&&... args);

    void verifyConnected(const QString& method);

    std::pair<std::shared_ptr<JsonRpcRequest>, QJsonObject>
        prepareCall(const QString& method);

    std::pair<std::shared_ptr<JsonRpcRequest>, RequestId> createRequest();
    static RequestId createUuid();
    QJsonObject createRequestJsonObject(const QString& method,
                                        const QString& id);

    QJsonObject createNotificationJsonObject(const QString& method);

    void convertToQVariantList(QVariantList& /*result*/) {}

    template<typename T>
    void convertToQVariantList(QVariantList& result, T&& x);

    template<typename T, typename... Ts>
    void convertToQVariantList(QVariantList& result, T&& head, Ts&&... tail);

    static void getJsonErrorInfo(const QJsonObject& response,
                                 int& code,
                                 QString& message,
                                 QVariant& data);

    std::shared_ptr<JsonRpcLogger> m_logger;
    std::shared_ptr<JsonRpcEndpoint> m_endpoint;

    int m_call_timeout_ms;

    using RequestMap = QMap<RequestId, std::shared_ptr<JsonRpcRequest>>;
    RequestMap m_outstanding_requests;
    int m_outstanding_request_count;

    using ResultMap = QMap<RequestId, std::shared_ptr<JsonRpcResult>>;
    ResultMap m_results;
};

template<typename... Ts>
std::shared_ptr<JsonRpcResult>
JsonRpcClient::call(const QString& method, Ts&&... args)
{
    auto req = doCall(method, false, std::forward<Ts>(args)...);
    return waitForSyncCallbacks(req.get());
}

template<typename... Ts>
std::shared_ptr<JsonRpcRequest>
JsonRpcClient::callAsync(const QString& method, Ts&&... args)
{
    return doCall(method, true, std::forward<Ts>(args)...);
}

template<typename... Ts>
void JsonRpcClient::notification(const QString& method, Ts&&... args)
{
    doNotification(method, std::forward<Ts>(args)...);
}

template<typename... Ts>
std::shared_ptr<JsonRpcRequest>
JsonRpcClient::doCall(const QString& method, bool async, Ts&&... args)
{
    verifyConnected(method);

    std::shared_ptr<JsonRpcRequest> request;
    QJsonObject req_json_obj;
    std::tie(request, req_json_obj) = prepareCall(method);

    QVariantList param_list;
    convertToQVariantList(param_list, std::forward<Ts>(args)...);
    req_json_obj["params"] = QJsonArray::fromVariantList(param_list);

    m_logger->logInfo(
        formatLogMessage(method, param_list, async, request->id()));

    m_endpoint->send(QJsonDocument(req_json_obj));

    return request;
}

template<typename... Ts>
void JsonRpcClient::doNotification(const QString& method, Ts&&... args)
{
    verifyConnected(method);

    QJsonObject req_json_obj;
    req_json_obj = createNotificationJsonObject(method);

    QVariantList param_list;
    convertToQVariantList(param_list, std::forward<Ts>(args)...);
    req_json_obj["params"] = QJsonArray::fromVariantList(param_list);

    m_endpoint->send(QJsonDocument(req_json_obj));
}

template<typename T>
void JsonRpcClient::convertToQVariantList(QVariantList& result, T&& x)
{
    result.push_front(x);
}

template<typename T, typename... Ts>
void JsonRpcClient::convertToQVariantList(QVariantList& result,
                                          T&& head, Ts&&... tail)
{
    convertToQVariantList(result, std::forward<Ts>(tail)...);
    result.push_front(head);
}

}
