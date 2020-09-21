#include "json_rpc_error.h"

namespace jcon {

JsonRpcError::JsonRpcError(int code,
                           const QString& message,
                           const QVariant& data)
    : m_code(code)
    , m_message(message)
    , m_data(data)
{
}

JsonRpcError::JsonRpcError(const JsonRpcError& other)
    : m_code(other.m_code)
    , m_message(other.m_message)
    , m_data(other.m_data)
{
}

JsonRpcError::~JsonRpcError()
{
}

QString JsonRpcError::toString() const
{
    return QString("%1 (%2)").arg(message()).arg(code());
}

}
