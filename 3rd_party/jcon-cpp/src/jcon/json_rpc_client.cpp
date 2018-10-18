#include "json_rpc_client.h"
#include "json_rpc_file_logger.h"
#include "json_rpc_success.h"
#include "jcon_assert.h"
#include "string_util.h"

#include <QSignalSpy>
#include <QUuid>

#include <memory>

namespace jcon {

const QString JsonRpcClient::InvalidRequestId = "";

JsonRpcClient::JsonRpcClient(std::shared_ptr<JsonRpcSocket> socket,
                             QObject* parent,
                             std::shared_ptr<JsonRpcLogger> logger,
                             int call_timeout_ms)
    : QObject(parent)
    , m_logger(logger)
    , m_call_timeout_ms(call_timeout_ms)
    , m_outstanding_request_count(0)
{
    if (!m_logger) {
        m_logger = std::make_shared<JsonRpcFileLogger>("json_client_log.txt");
    }

    m_endpoint = std::make_shared<JsonRpcEndpoint>(socket, m_logger, this);

    connect(m_endpoint.get(), &JsonRpcEndpoint::socketConnected,
            this, &JsonRpcClient::socketConnected);

    connect(m_endpoint.get(), &JsonRpcEndpoint::socketDisconnected,
            this, &JsonRpcClient::socketDisconnected);

    connect(m_endpoint.get(), &JsonRpcEndpoint::socketError,
            this, &JsonRpcClient::socketError);

    connect(m_endpoint.get(), &JsonRpcEndpoint::jsonObjectReceived,
            this, &JsonRpcClient::jsonResponseReceived);
}

JsonRpcClient::~JsonRpcClient()
{
    disconnectFromServer();
}

std::shared_ptr<JsonRpcResult>
JsonRpcClient::waitForSyncCallbacks(const JsonRpcRequest* request)
{
    connect(request, &JsonRpcRequest::result,
            [this, id = request->id()](const QVariant& result) {
                m_logger->logDebug(
                    QString("Received success response to synchronous "
                            "RPC call (ID: %1)").arg(qPrintable(id)));

                m_results[id] = std::make_shared<JsonRpcSuccess>(result);
            });

    connect(request, &JsonRpcRequest::error,
            [this, id = request->id()](int code,
                                       const QString& message,
                                       const QVariant& data)
            {
                m_logger->logError(
                    QString("Received error response to synchronous "
                            "RPC call (ID: %1)").arg(qPrintable(id)));

                m_results[id] =
                    std::make_shared<JsonRpcError>(code, message, data);
            });

    QTime timer;
    timer.start();
    while (m_outstanding_requests.contains(request->id()) &&
           timer.elapsed() < m_call_timeout_ms)
    {
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    }

    if (m_results.contains(request->id())) {
        auto res = m_results[request->id()];
        m_results.remove(request->id());
        return res;
    } else {
        return std::make_shared<JsonRpcError>(
            JsonRpcError::EC_InternalError,
            "RPC call timed out"
        );
    }
}

std::shared_ptr<JsonRpcResult>
JsonRpcClient::callExpandArgs(const QString& method, const QVariantList& args)
{
    auto req = doCallExpandArgs(method, false, args);
    return waitForSyncCallbacks(req.get());
}

std::shared_ptr<JsonRpcRequest>
JsonRpcClient::callAsyncExpandArgs(const QString& method,
                                   const QVariantList& args)
{
    return doCallExpandArgs(method, true, args);
}

std::shared_ptr<JsonRpcRequest>
JsonRpcClient::doCallExpandArgs(const QString& method,
                                bool async,
                                const QVariantList& args)
{
    std::shared_ptr<JsonRpcRequest> request;
    QJsonObject req_json_obj;
    std::tie(request, req_json_obj) = prepareCall(method);

    if (!args.empty()) {
        req_json_obj["params"] = QJsonArray::fromVariantList(args);
    }

    m_logger->logInfo(formatLogMessage(method, args, async, request->id()));
    m_endpoint->send(QJsonDocument(req_json_obj));

    return request;
}

int JsonRpcClient::outstandingRequestCount() const
{
    return m_outstanding_request_count;
}

void JsonRpcClient::verifyConnected(const QString& method)
{
    if (!isConnected()) {
        auto msg = QString("cannot call RPC method (%1) when not connected")
            .arg(method);
        m_logger->logError(msg);
        throw std::runtime_error(msg.toStdString());
    }
}

std::pair<std::shared_ptr<JsonRpcRequest>, QJsonObject>
JsonRpcClient::prepareCall(const QString& method)
{
    std::shared_ptr<JsonRpcRequest> request;
    RequestId id;
    std::tie(request, id) = createRequest();
    m_outstanding_requests[id] = request;
    ++m_outstanding_request_count;
    QJsonObject req_json_obj = createRequestJsonObject(method, id);
    return std::make_pair(request, req_json_obj);
}

std::pair<std::shared_ptr<JsonRpcRequest>, JsonRpcClient::RequestId>
JsonRpcClient::createRequest()
{
    auto id = createUuid();
    auto request = std::make_shared<JsonRpcRequest>(this, id);
    return std::make_pair(request, id);
}

JsonRpcClient::RequestId JsonRpcClient::createUuid()
{
    RequestId id = QUuid::createUuid().toString();
    int len = id.length();
    id = id.left(len - 1).right(len - 2);
    return id;
}

QJsonObject JsonRpcClient::createRequestJsonObject(const QString& method,
                                                   const QString& id)
{
    return QJsonObject {
        { "jsonrpc", "2.0" },
        { "method", method },
        { "id", id }
    };
}

QJsonObject JsonRpcClient::createNotificationJsonObject(const QString& method)
{
    return createRequestJsonObject(method, "null");
}

bool JsonRpcClient::connectToServer(const QString& host, int port)
{
    if (!m_endpoint->connectToHost(host, port)) {
        return false;
    }
    return true;
}

bool JsonRpcClient::connectToServer(const QUrl& url)
{
    if (!m_endpoint->connectToUrl(url)) {
        return false;
    }
    return true;
}

void JsonRpcClient::disconnectFromServer()
{
    m_endpoint->disconnectFromHost();
}

bool JsonRpcClient::isConnected() const
{
    return m_endpoint->isConnected();
}

QHostAddress JsonRpcClient::clientAddress() const
{
    return m_endpoint->localAddress();
}

int JsonRpcClient::clientPort() const
{
    return m_endpoint->localPort();
}

QHostAddress JsonRpcClient::serverAddress() const
{
    return m_endpoint->peerAddress();
}

int JsonRpcClient::serverPort() const
{
    return m_endpoint->peerPort();
}

void JsonRpcClient::jsonResponseReceived(const QJsonObject& response)
{
    JCON_ASSERT(response["jsonrpc"].toString() == "2.0");

    if (response.value("jsonrpc").toString() != "2.0") {
        logError("invalid protocol tag");
        return;
    }

    if (response.value("error").isObject()) {
        int code;
        QString msg;
        QVariant data;
        getJsonErrorInfo(response, code, msg, data);
        logError(QString("(%1) - %2").arg(code).arg(msg));

        RequestId id = response.value("id").toString(InvalidRequestId);
        if (id != InvalidRequestId) {
            auto it = m_outstanding_requests.find(id);
            if (it == m_outstanding_requests.end()) {
                logError(QString("got error response for non-existing "
                                 "request: %1").arg(id));
                return;
            }
            emit it.value()->error(code, msg, data);
            m_outstanding_requests.erase(it);
            --m_outstanding_request_count;
        }

        return;
    }

    if (response["result"].isUndefined()) {
        logError("result is undefined");
        return;
    }

    RequestId id = response.value("id").toString(InvalidRequestId);
    if (id == InvalidRequestId) {
        logError("response ID is undefined");
        return;
    }

    QVariant result = response.value("result").toVariant();

    auto it = m_outstanding_requests.find(id);
    if (it == m_outstanding_requests.end()) {
        logError(QString("got response to non-existing request: %1").arg(id));
        return;
    }

    emit it.value()->result(result);
    m_outstanding_requests.erase(it);
    --m_outstanding_request_count;
}

void JsonRpcClient::getJsonErrorInfo(const QJsonObject& response,
                                     int& code,
                                     QString& message,
                                     QVariant& data)
{
    QJsonObject error = response["error"].toObject();
    code = error["code"].toInt();
    message = error["message"].toString("unknown error");
    data = error.value("data").toVariant();
}

QString JsonRpcClient::formatLogMessage(const QString& method,
                                        const QVariantList& args,
                                        bool async,
                                        const QString& request_id)
{
    auto msg = QString("Calling (%1) RPC method: '%2' ")
        .arg(async ? "async" : "sync").arg(method);

    if (args.empty()) {
        msg += "without arguments";
    } else {
        msg += QString("with argument%1: %2")
            .arg(args.size() == 1 ? "" : "s")
            .arg(variantListToStringList(args).join(", "));
    }
    msg += QString(" (request ID: %1)").arg(request_id);
    return msg;
}

void JsonRpcClient::logError(const QString& msg)
{
    m_logger->logError("JSON RPC client error: " + msg);
}

}
