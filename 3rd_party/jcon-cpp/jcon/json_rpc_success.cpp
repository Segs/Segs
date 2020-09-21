#include "json_rpc_success.h"

namespace jcon {

JsonRpcSuccess::JsonRpcSuccess(QVariant result) : m_result(result)
{
}

JsonRpcSuccess::~JsonRpcSuccess()
{
}

}
