#pragma once

#include "jcon.h"
#include "json_rpc_logger.h"

#include <QAbstractSocket>

#include <memory>

namespace jcon {

class JsonRpcEndpoint;
class JsonRpcSocket;

class JCON_API JsonRpcServer : public QObject
{
    Q_OBJECT

public:
    JsonRpcServer(QObject* parent = nullptr,
                  std::shared_ptr<JsonRpcLogger> logger = nullptr);
    virtual ~JsonRpcServer();

    void registerServices(const QObjectList& services);

    virtual void listen(int port) = 0;
    virtual void listen(const QHostAddress& addr, int port) = 0;

    virtual void close() = 0;

protected:
    virtual JsonRpcEndpoint* findClient(QObject* socket) = 0;

signals:
    void clientConnected(QObject* client_socket);
    void clientDisconnected(QObject* client_socket);

    void socketError(QObject* socket, QAbstractSocket::SocketError error);

public slots:
    void jsonRequestReceived(const QJsonObject& request, QObject* socket);

protected slots:
    virtual void newConnection() = 0;
    virtual void disconnectClient(QObject* client_socket) = 0;

protected:
    void logInfo(const QString& msg);
    void logError(const QString& msg);
    std::shared_ptr<JsonRpcLogger> log() { return m_logger; }

private:
    static const QString InvalidRequestId;

    bool dispatch(const QString& method_name,
                  const QVariant& params,
                  const QString& request_id,
                  QVariant& return_value);

    bool call(QObject* object,
              const QMetaMethod& meta_method,
              const QVariantList& args,
              QVariant& return_value);

    bool call(QObject* object,
              const QMetaMethod& meta_method,
              const QVariantMap& args,
              QVariant& return_value);


    bool convertArgs(const QMetaMethod& meta_method,
                     const QVariantList& args,
                     QVariantList& converted);

    bool convertArgs(const QMetaMethod& meta_method,
                     const QVariantMap& args,
                     QVariantList& converted);

    bool doCall(QObject* object,
                const QMetaMethod& meta_method,
                QVariantList& converted_args,
                QVariant& return_value);

    QJsonDocument createResponse(const QString& request_id,
                                 const QVariant& return_value,
                                 const QString& method_name);
    QJsonDocument createErrorResponse(const QString& request_id,
                                      int code,
                                      const QString& message);

    std::shared_ptr<JsonRpcLogger> m_logger;
    QObjectList m_services;
};

}
