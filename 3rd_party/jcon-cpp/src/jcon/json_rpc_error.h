#pragma once

#include "jcon.h"
#include "json_rpc_result.h"

#include <QString>
#include <QVariant>

namespace jcon {

class JCON_API JsonRpcError : public JsonRpcResult
{
public:
    enum ErrorCodes {
        EC_ParseError     = -32700,
        EC_InvalidRequest = -32600,
        EC_MethodNotFound = -32601,
        EC_InvalidParams  = -32602,
        EC_InternalError  = -32603
    };

    JsonRpcError(int code = 0,
                 const QString& message = "",
                 const QVariant& data = QVariant());

    JsonRpcError(const JsonRpcError& other);

    virtual ~JsonRpcError();

    bool isSuccess() const override { return false; }
    QVariant result() const override { return QVariant(); }
    QString toString() const override;

private:
    int code() const { return m_code; }
    QString message() const { return m_message; }
    QVariant data() const { return m_data; }

    int m_code;
    QString m_message;
    QVariant m_data;
};

}
