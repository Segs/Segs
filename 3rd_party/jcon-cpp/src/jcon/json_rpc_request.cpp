#include "json_rpc_request.h"

namespace jcon {

JsonRpcRequest::JsonRpcRequest(QObject* parent,
                               QString id,
                               QDateTime timestamp)
    : QObject(parent)
    , m_id(id)
    , m_timestamp(timestamp)
{
}

JsonRpcRequest::~JsonRpcRequest()
{
}

QString JsonRpcRequest::id() const
{
    return m_id;
}

}
