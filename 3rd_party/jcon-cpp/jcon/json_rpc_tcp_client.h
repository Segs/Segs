#pragma once

#include "json_rpc_client.h"

namespace jcon {

class JCON_API JsonRpcTcpClient : public JsonRpcClient
{
    Q_OBJECT

public:
    JsonRpcTcpClient(QObject* parent = nullptr,
                     std::shared_ptr<JsonRpcLogger> logger = nullptr,
                     int call_timeout_ms = 60000);

    virtual ~JsonRpcTcpClient();
};

}
