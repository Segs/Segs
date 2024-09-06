#include "json_rpc_server.h"
#include "jcon_assert.h"
#include "json_rpc_endpoint.h"
#include "json_rpc_error.h"
#include "json_rpc_file_logger.h"
#include "string_util.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariant>
#include <QMetaMethod>
#include <QMetaType>

namespace {
    QString logInvoke(const QMetaMethod& meta_method,
                      const QVariantList& args,
                      const QVariant& return_value);
}

namespace jcon {

JsonRpcEndpoint* JsonRpcServer::sm_client_endpoint = nullptr;

JsonRpcServer::JsonRpcServer(QObject* parent,
                             std::shared_ptr<JsonRpcLogger> logger)
    : QObject(parent)
    , m_logger(logger)
    , m_allowNotification(false)
{
    if (!m_logger) {
        m_logger = std::make_shared<JsonRpcFileLogger>("json_server_log.txt");
    }
}

JsonRpcServer::~JsonRpcServer()
{
}

void JsonRpcServer::registerServices(const QObjectList& services)
{
    m_services.clear();

    // unsolicited notification signature
    QByteArray signature = QMetaObject::normalizedSignature(
        "sendUnsolicitedNotification(QString,QVariant)");

    for (auto s : services) {
        m_services[s] = "";
        /*
         * If the server allows sending unsolicited notifications,
         * and the service emits sendUnsolicitedNotification(QString,QVariant)
         *  .... add a queued connection
         */
        if (m_allowNotification) {
            int index = s->metaObject()->indexOfSignal(signature);
            if (index != -1)
                connect(s, SIGNAL(sendUnsolicitedNotification(QString,QVariant)),
                        this, SLOT(serviceNotificationReceived(QString,QVariant)),
                        Qt::QueuedConnection);
        }
    }

    m_ns_separator = "";
}

void JsonRpcServer::registerServices(const ServiceMap& services,
                                     const QString& ns_separator)
{
    m_services = services;
    m_ns_separator = ns_separator;

    // unsolicited notification signature
    QByteArray signature = QMetaObject::normalizedSignature(
        "sendUnsolicitedNotification(QString,QVariant)");

    /*
     * If the server allows sending unsolicited notifications,
     * and the service emits sendUnsolicitedNotification(QString,QVariant)
     *  .... add a queued connection
     */
    if (m_allowNotification) {
        for (auto it = m_services.begin(); it != m_services.end(); ++it) {
            QObject* s = it.key();
            int index = s->metaObject()->indexOfSignal(signature);

            if (index != -1)
                connect(s, SIGNAL(sendUnsolicitedNotification(QString,QVariant)),
                        this, SLOT(serviceNotificationReceived(QString,QVariant)),
                        Qt::QueuedConnection);
        }
    }
}

void JsonRpcServer::enableSendNotification(bool enabled)
{
    m_allowNotification = enabled;
}

JsonRpcEndpoint* JsonRpcServer::clientEndpoint()
{
    return sm_client_endpoint;
}

void JsonRpcServer::jsonRequestReceived(const QJsonObject& request,
                                        QObject* socket)
{
    JCON_ASSERT(request.value("jsonrpc").toString() == "2.0");

    if (request.value("jsonrpc").toString() != "2.0") {
        logError("invalid protocol tag");
        return;
    }

    sm_client_endpoint = findClient(socket);

    QString method_name = request.value("method").toString();
    if (method_name.isEmpty()) {
        logError("no method present in request");
    }

    QVariant params = request.value("params").toVariant();
    QString request_id = request.value("id").toVariant().toString();

    QVariant return_value;
    if (!dispatch(method_name, params, return_value)) {
        auto msg = QString("method '%1' not found, check name and "
                           "parameter types ").arg(method_name);
        logError(msg);

        // send error response if request had valid ID
        if (request_id != InvalidRequestId) {
            QJsonDocument error =
                createErrorResponse(request_id,
                                    JsonRpcError::EC_MethodNotFound,
                                    msg);

            if (!sm_client_endpoint) {
                logError("invalid client socket, cannot send response");
                return;
            }

            sm_client_endpoint->send(error);
            return;
        }
    }

    // send response if request had valid ID
    if (request_id != InvalidRequestId) {
        QJsonDocument response = createResponse(request_id,
                                                return_value,
                                                method_name);

        if (!sm_client_endpoint) {
            logError("invalid client socket, cannot send response");
            return;
        }

        sm_client_endpoint->send(response);
    }
}

bool JsonRpcServer::dispatch(const QString& method_name,
                             const QVariant& params,
                             QVariant& return_value)
{
    QString method_ns;
    QString method_name_without_ns;
    std::tie(method_ns, method_name_without_ns) =
        namespaceAndMethodName(method_name);

    QObjectList services;
    for (auto it = m_services.begin(); it != m_services.end(); ++it) {
        QObject* s = it.key();
        QString ns = it.value();
        if (ns.isEmpty() || ns == method_ns) {
            services.push_back(s);
        }
    }

    for (auto s : services) {
        const QMetaObject* meta_obj = s->metaObject();
        for (int i = meta_obj->methodOffset();
             i < meta_obj->methodCount();
             ++i)
        {
            auto meta_method = meta_obj->method(i);
            if (meta_method.name() == method_name_without_ns) {
                if (params.type() == QVariant::List ||
                    params.type() == QVariant::StringList)
                {
                    if (call(s, meta_method, params.toList(), return_value)) {
                        return true;
                    }
                } else if (params.type() == QVariant::Map) {
                    if (call(s, meta_method, params.toMap(), return_value)) {
                        return true;
                    }
                } else if (params.type() == QVariant::Invalid) {
                    if (call(s, meta_method, QVariantList(), return_value)) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

std::pair<QString, QString>
JsonRpcServer::namespaceAndMethodName(const QString& full_name)
{
    if (m_ns_separator.isEmpty()) {
        return {"", full_name};
    }
    QString ns;
    QString method_name;
    int li = full_name.lastIndexOf(m_ns_separator);
    if (li > 0) {
        ns = full_name.left(li);
        method_name = full_name.mid(li + m_ns_separator.length());
    } else {
        return {"", full_name};
    }
    return {ns, method_name};
}

bool JsonRpcServer::call(QObject* object,
                         const QMetaMethod& meta_method,
                         const QVariantList& args,
                         QVariant& return_value)
{
    return_value = QVariant();

    QVariantList converted_args;
    if (!convertArgs(meta_method, args, converted_args)) {
        return false;
    }

    return doCall(object, meta_method, converted_args, return_value);
}

bool JsonRpcServer::call(QObject* object,
                         const QMetaMethod& meta_method,
                         const QVariantMap& args,
                         QVariant& return_value)
{
    return_value = QVariant();

    QVariantList converted_args;
    if (!convertArgs(meta_method, args, converted_args)) {
        return false;
    }

    return doCall(object, meta_method, converted_args, return_value);
}

bool JsonRpcServer::convertArgs(const QMetaMethod& meta_method,
                                const QVariantList& args,
                                QVariantList& converted_args)
{
    QList<QByteArray> param_types = meta_method.parameterTypes();
    if (args.size() != param_types.size()) {
        logError(QString("wrong number of arguments to method %1 -- "
                         "expected %2 arguments, but got %3")
                 .arg(QString(meta_method.methodSignature()))
                 .arg(meta_method.parameterCount())
                 .arg(args.size()));
        return false;
    }

    for (int i = 0; i < param_types.size(); i++) {
        const QVariant& arg = args.at(i);
        if (!arg.isValid()) {
            logError(QString("argument %1 of %2 to method %3 is invalid")
                     .arg(i + 1)
                     .arg(param_types.size())
                     .arg(QString(meta_method.methodSignature())));
            return false;
        }

        QByteArray arg_type_name = arg.typeName();
        QByteArray param_type_name = param_types.at(i);

        QVariant::Type param_type = QVariant::nameToType(param_type_name);

        QVariant copy = QVariant(arg);

        if (copy.type() != param_type) {
            if (copy.canConvert(param_type)) {
                if (!copy.convert(param_type)) {
                    // qDebug() << "cannot convert" << arg_type_name
                    //          << "to" << param_type_name;
                    return false;
                }
            }
        }

        converted_args << copy;
    }
    return true;
}

bool JsonRpcServer::convertArgs(const QMetaMethod& meta_method,
                                const QVariantMap& args,
                                QVariantList& converted_args)
{
    QList<QByteArray> param_types = meta_method.parameterTypes();
    if (args.size() != param_types.size()) {
        logError(QString("wrong number of arguments to method %1 -- "
                         "expected %2 arguments, but got %3")
                 .arg(QString(meta_method.methodSignature()))
                 .arg(meta_method.parameterCount())
                 .arg(args.size()));
        return false;
    }

    for (int i = 0; i < param_types.size(); i++) {
        QByteArray param_name = meta_method.parameterNames().at(i);
        if (args.find(param_name) == args.end()) {
            // no arg with param name found
            return false;
        }
        const QVariant& arg = args.value(param_name);
        if (!arg.isValid()) {
            logError(QString("argument %1 of %2 to method %3 is invalid")
                     .arg(i + 1)
                     .arg(param_types.size())
                     .arg(QString(meta_method.methodSignature())));
            return false;
        }

        QByteArray arg_type_name = arg.typeName();
        QByteArray param_type_name = param_types.at(i);

        QVariant::Type param_type = QVariant::nameToType(param_type_name);

        QVariant copy = QVariant(arg);

        if (copy.type() != param_type) {
            if (copy.canConvert(param_type)) {
                if (!copy.convert(param_type)) {
                    // qDebug() << "cannot convert" << arg_type_name
                    //          << "to" << param_type_name;
                    return false;
                }
            }
        }

        converted_args << copy;
    }
    return true;
}

// based on https://gist.github.com/andref/2838534.
bool JsonRpcServer::doCall(QObject* object,
                           const QMetaMethod& meta_method,
                           QVariantList& converted_args,
                           QVariant& return_value)
{
    QList<QGenericArgument> arguments;

    for (int i = 0; i < converted_args.size(); i++) {

        // Notice that we have to take a reference to the argument, else we'd be
        // pointing to a copy that will be destroyed when this loop exits.
        QVariant& argument = converted_args[i];

        // A const_cast is needed because calling data() would detach the
        // QVariant.
        QGenericArgument generic_argument(
            QMetaType::typeName(argument.userType()),
            const_cast<void*>(argument.constData())
        );

        arguments << generic_argument;
    }

    const char* return_type_name = meta_method.typeName();
    QMetaType return_type = QMetaType::fromName(return_type_name);
    if (return_type.id() != QMetaType::Void) {
        return_value = QVariant(return_type, nullptr);
    }

    QGenericReturnArgument return_argument(
        return_type_name,
        const_cast<void*>(return_value.constData())
    );

    // perform the call
    bool ok = meta_method.invoke(
        object,
        Qt::DirectConnection,
        return_argument,
        arguments.value(0),
        arguments.value(1),
        arguments.value(2),
        arguments.value(3),
        arguments.value(4),
        arguments.value(5),
        arguments.value(6),
        arguments.value(7),
        arguments.value(8),
        arguments.value(9)
    );

    if (!ok) {
        // qDebug() << "calling" << meta_method.methodSignature() << "failed.";
        return false;
    }

    logInfo(logInvoke(meta_method, converted_args, return_value));

    return true;
}

QJsonDocument JsonRpcServer::createResponse(const QString& request_id,
                                            const QVariant& return_value,
                                            const QString& method_name)
{
    QJsonObject res_json_obj {
        { "jsonrpc", "2.0" },
        { "id", request_id }
    };

    if (return_value.type() == QVariant::Invalid) {
        res_json_obj["result"] = QJsonValue();
    } else if (return_value.type() == QVariant::List) {
        auto ret_doc = QJsonDocument::fromVariant(return_value);
        res_json_obj["result"] = ret_doc.array();
    } else if (return_value.type() == QVariant::Map) {
        auto ret_doc = QJsonDocument::fromVariant(return_value);
        res_json_obj["result"] = ret_doc.object();
    } else if (return_value.type() == QVariant::Int) {
        res_json_obj["result"] = return_value.toInt();
    } else if (return_value.type() == QVariant::LongLong) {
        res_json_obj["result"] = return_value.toLongLong();
    } else if (return_value.type() == QVariant::Double) {
        res_json_obj["result"] = return_value.toDouble();
    } else if (return_value.type() == QVariant::Bool) {
        res_json_obj["result"] = return_value.toBool();
    } else if (return_value.type() == QVariant::String) {
        res_json_obj["result"] = return_value.toString();
    } else {
        auto msg =
            QString("method '%1' has unknown return type: %2")
            .arg(method_name)
            .arg(return_value.type());
        logError(msg);
        return createErrorResponse(request_id,
                                   JsonRpcError::EC_InvalidRequest,
                                   msg);
    }

    return QJsonDocument(res_json_obj);
}

QJsonDocument JsonRpcServer::createErrorResponse(const QString& request_id,
                                                 int code,
                                                 const QString& message)
{
    QJsonObject error_object {
        { "code", code },
        { "message", message }
    };

    QJsonObject res_json_obj {
        { "jsonrpc", "2.0" },
        { "error", error_object },
        { "id", request_id }
    };
    return QJsonDocument(res_json_obj);
}

QJsonDocument JsonRpcServer::createNotification(const QString& key,
                                                const QVariant& value)
{
    QJsonObject noti_json_obj {
        { "jsonrpc", "2.0" },
        { "method", key }
    };

    if (value.type() == QVariant::Invalid) {
        noti_json_obj["params"] = QJsonValue();
    } else if (value.type() == QVariant::List) {
        auto ret_doc = QJsonDocument::fromVariant(value);
        noti_json_obj["params"] = ret_doc.array();
    } else if (value.type() == QVariant::Map) {
        auto ret_doc = QJsonDocument::fromVariant(value);
        noti_json_obj["params"] = ret_doc.object();
    } else if (value.type() == QVariant::Int) {
        noti_json_obj["params"] = value.toInt();
    } else if (value.type() == QVariant::LongLong) {
        noti_json_obj["params"] = value.toLongLong();
    } else if (value.type() == QVariant::Double) {
        noti_json_obj["params"] = value.toDouble();
    } else if (value.type() == QVariant::Bool) {
        noti_json_obj["params"] = value.toBool();
    } else if (value.type() == QVariant::String) {
        noti_json_obj["params"] = value.toString();
    } else {
        auto msg =
            QString("unknown return type: %1")
                .arg(value.type());
        logError(msg);
        return QJsonDocument();
    }

    return QJsonDocument(noti_json_obj);
}

void JsonRpcServer::logInfo(const QString& msg)
{
    m_logger->logInfo("JSON RPC server: " + msg);
}

void JsonRpcServer::logError(const QString& msg)
{
    m_logger->logError("JSON RPC server error: " + msg);
}

void JsonRpcServer::serviceNotificationReceived(const QString& key,
                                                const QVariant& value)
{
    if (key.isEmpty())
        return;

    QJsonDocument response = createNotification(key, value);
    if (response.isNull())
        return;

    for (JsonRpcEndpoint* endpoint : getAllClients())
        if (endpoint != nullptr)
            endpoint->send(response);
}

}

namespace {

QString logInvoke(const QMetaMethod& meta_method,
                  const QVariantList& args,
                  const QVariant& return_value)
{
    const auto ns = meta_method.parameterNames();
    auto ps = jcon::variantListToStringList(args);
    QStringList args_sl;
    std::transform(ns.begin(), ns.end(), ps.begin(),
                   std::back_inserter(args_sl),
                   [](auto x, auto y) -> QString {
                       return static_cast<QString>(x) + ": " + y;
                   }
        );

    auto msg = QString("%1 invoked ")
        .arg(static_cast<QString>(meta_method.name()));

    if (args_sl.empty()) {
        msg += "without arguments";
    } else {
        msg += QString("with argument%1: %2")
            .arg(args_sl.size() == 1 ? "" : "s")
            .arg(args_sl.join(", "));
    }

    if (return_value.isValid()) {
        msg += " -> returning: " + jcon::variantToString(return_value);
    }

    return msg;
}

}
