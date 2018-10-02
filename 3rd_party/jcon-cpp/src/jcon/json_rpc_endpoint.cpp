#include "json_rpc_endpoint.h"
#include "json_rpc_socket.h"
#include "jcon_assert.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QTcpSocket>
#include <QUrl>

namespace jcon {

/// Remove \p n bytes from \p bytes.
static QByteArray chopLeft(const QByteArray& bytes, int n);

JsonRpcEndpoint::JsonRpcEndpoint(std::shared_ptr<JsonRpcSocket> socket,
                                 std::shared_ptr<JsonRpcLogger> logger,
                                 QObject* parent)
    : QObject(parent)
    , m_logger(logger)
    , m_socket(socket)
{
    connect(m_socket.get(), &JsonRpcSocket::socketConnected,
            this, &JsonRpcEndpoint::socketConnected);

    connect(m_socket.get(), &JsonRpcSocket::socketDisconnected,
            this, &JsonRpcEndpoint::socketDisconnected);

    connect(m_socket.get(), &JsonRpcSocket::dataReceived,
            this, &JsonRpcEndpoint::dataReady);

    connect(m_socket.get(), &JsonRpcSocket::socketError,
            this, &JsonRpcEndpoint::socketError);
}

JsonRpcEndpoint::~JsonRpcEndpoint()
{
    m_socket->disconnect(this);
}

bool JsonRpcEndpoint::connectToHost(const QString& host, int port, int msecs)
{
    m_logger->logInfo(QString("connecting to JSON RPC server at %1:%2")
                      .arg(host).arg(port));

    m_socket->connectToHost(host, port);

    if (!m_socket->waitForConnected(msecs)) {
        m_logger->logError("could not connect to JSON RPC server: " +
                           m_socket->errorString());
        return false;
    }

    m_logger->logInfo(QString("connected to JSON RPC server %1:%2 "
                              "(local port: %3)")
                      .arg(host).arg(port).arg(m_socket->localPort()));
    return true;
}

void JsonRpcEndpoint::connectToHostAsync(const QString& host, int port)
{
    m_socket->connectToHost(host, port);
}

bool JsonRpcEndpoint::connectToUrl(const QUrl& url, int msecs)
{
    m_logger->logInfo(QString("connecting to JSON RPC server at %1")
                      .arg(url.toString()));

    m_socket->connectToUrl(url);

    if (!m_socket->waitForConnected(msecs)) {
        m_logger->logError("could not connect to JSON RPC server: " +
                           m_socket->errorString());
        return false;
    }

    m_logger->logInfo(QString("connected to JSON RPC server %1 "
                              "(local port: %3)")
                      .arg(url.toString()).arg(m_socket->localPort()));
    return true;
}

void JsonRpcEndpoint::connectToUrlAsync(const QUrl& url)
{
    m_socket->connectToUrl(url);
}

void JsonRpcEndpoint::disconnectFromHost()
{
    m_socket->disconnectFromHost();
}

bool JsonRpcEndpoint::isConnected() const
{
    return m_socket->isConnected();
}

QHostAddress JsonRpcEndpoint::localAddress() const
{
    return m_socket->localAddress();
}

int JsonRpcEndpoint::localPort() const
{
    return m_socket->localPort();
}

QHostAddress JsonRpcEndpoint::peerAddress() const
{
    return m_socket->peerAddress();
}

int JsonRpcEndpoint::peerPort() const
{
    return m_socket->peerPort();
}

void JsonRpcEndpoint::send(const QJsonDocument& doc)
{
    QByteArray bytes = doc.toJson();
    m_socket->send(bytes);
}

void JsonRpcEndpoint::dataReady(const QByteArray& bytes, QObject* socket)
{
    JCON_ASSERT(bytes.length() > 0);
    // Copying data to new buffer, because the endpoint buffer may be
    // invalidated at any time by closing socket from outside which will cause
    // an exception.
    m_recv_buffer += QByteArray::fromRawData(bytes.data(), bytes.size());
    m_recv_buffer = processBuffer(m_recv_buffer.trimmed(), socket);
}

QByteArray JsonRpcEndpoint::processBuffer(const QByteArray& buffer,
                                          QObject* socket)
{
    QByteArray buf(buffer);

    JCON_ASSERT(buf[0] == '{');

    bool in_string = false;
    int brace_nesting_level = 0;
    QByteArray json_obj;

    int i = 0;
    while (i < buf.length() ) {
        const char curr_ch = buf[i++];

        if (curr_ch == '"')
            in_string = !in_string;

        if (!in_string) {
            if (curr_ch == '{')
                ++brace_nesting_level;

            if (curr_ch == '}') {
                --brace_nesting_level;
                JCON_ASSERT(brace_nesting_level >= 0);

                if (brace_nesting_level == 0) {
                    auto doc = QJsonDocument::fromJson(buf.left(i));
                    JCON_ASSERT(!doc.isNull());
                    JCON_ASSERT(doc.isObject());
                    if (doc.isObject())
                        emit jsonObjectReceived(doc.object(), socket);
                    buf = chopLeft(buf, i);
                    i = 0;
                    continue;
                }
            }
        }
    }
    return buf;
}

QByteArray chopLeft(const QByteArray& bytes, int n)
{
    return bytes.right(bytes.length() - n);
}

}
