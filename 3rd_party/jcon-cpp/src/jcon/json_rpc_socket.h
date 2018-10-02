#pragma once

#include "jcon.h"

#include <QHostAddress>
#include <QObject>

namespace jcon {

class JCON_API JsonRpcSocket : public QObject
{
    Q_OBJECT

public:
    JsonRpcSocket() {}
    virtual ~JsonRpcSocket() {}

    virtual void connectToHost(const QString& host, int port) = 0;
    virtual void connectToUrl(const QUrl& url) = 0;
    virtual bool waitForConnected(int msecs = 30000) = 0;
    virtual void disconnectFromHost() = 0;
    virtual bool isConnected() const = 0;
    virtual size_t send(const QByteArray& data) = 0;
    virtual QString errorString() const = 0;
    virtual QHostAddress localAddress() const = 0;
    virtual int localPort() const = 0;
    virtual QHostAddress peerAddress() const = 0;
    virtual int peerPort() const = 0;

signals:
    void dataReceived(const QByteArray& bytes, QObject* socket);
    void socketConnected(QObject* socket);
    void socketDisconnected(QObject* socket);
    void socketError(QObject* socket, QAbstractSocket::SocketError error);
};

}
