#include "json_rpc_tcp_client.h"
#include "json_rpc_tcp_socket.h"

#include <memory>

namespace jcon {

JsonRpcTcpClient::JsonRpcTcpClient(QObject* parent,
                                   std::shared_ptr<JsonRpcLogger> logger,
                                   int call_timeout_ms)
    : JsonRpcClient(std::make_shared<JsonRpcTcpSocket>(),
                    parent, logger, call_timeout_ms)
{
}

JsonRpcTcpClient::~JsonRpcTcpClient()
{
}

}
