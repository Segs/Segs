#include "json_rpc_websocket_client.h"
#include "json_rpc_websocket.h"

#include <memory>

namespace jcon {

JsonRpcWebSocketClient::JsonRpcWebSocketClient(
    QObject* parent,
    std::shared_ptr<JsonRpcLogger> logger,
    int call_timeout_ms)

    : JsonRpcClient(std::make_shared<JsonRpcWebSocket>(),
                    parent, logger, call_timeout_ms)
{
}

JsonRpcWebSocketClient::~JsonRpcWebSocketClient()
{
}

}
