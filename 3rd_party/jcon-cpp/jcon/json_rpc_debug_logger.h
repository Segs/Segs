#pragma once

#include "jcon.h"
#include "json_rpc_logger.h"

namespace jcon {

class JCON_API JsonRpcDebugLogger : public JsonRpcLogger
{
public:
    void logDebug(const QString& message) override;
    void logInfo(const QString& message) override;
    void logWarning(const QString& message) override;
    void logError(const QString& message) override;
};

}
