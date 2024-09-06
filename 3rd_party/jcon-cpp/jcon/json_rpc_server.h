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
    using ServiceMap = QMap<QObject*, QString>;

    JsonRpcServer(QObject* parent = nullptr,
                  std::shared_ptr<JsonRpcLogger> logger = nullptr);
    virtual ~JsonRpcServer();

    /**
     * Register services containing RPC method invocation handlers.
     *
     * @param[in] services A list of services to register.
     */
    void registerServices(const QObjectList& services);

    /**
     * Register namespaced services containing RPC method invocation handlers.
     *
     * @param[in] services A map of (service, namespace) pairs to register.
     * @param[in] ns_separator String that is used to separate namespaces from
     *                         method name.
     */
    void registerServices(const ServiceMap& services,
                          const QString& ns_separator = "/");

    // Allow a server to send unsolicited notifications to client
    void enableSendNotification(bool enabled);

    virtual bool listen(int port) = 0;
    virtual bool listen(const QHostAddress& addr, int port) = 0;

    virtual void close() = 0;

    /**
     * Get the currently calling client endpoint.
     */
    static JsonRpcEndpoint* clientEndpoint();

protected:
    virtual JsonRpcEndpoint* findClient(QObject* socket) = 0;
    virtual QVector<JsonRpcEndpoint*> getAllClients() = 0;

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

private slots:
    void serviceNotificationReceived(const QString& key, const QVariant& value);

private:
    const QString InvalidRequestId = "";

    bool dispatch(const QString& method_name,
                  const QVariant& params,
                  QVariant& return_value);

    std::pair<QString, QString>
    namespaceAndMethodName(const QString& full_name);

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
    QJsonDocument createNotification(const QString& key,
                                     const QVariant& value);

    std::shared_ptr<JsonRpcLogger> m_logger;
    ServiceMap m_services;
    QString m_ns_separator;

    // allows unsolicited notifications (not part of JSON-RPC standard)
    bool m_allowNotification;

    static JsonRpcEndpoint* sm_client_endpoint;
};

}
