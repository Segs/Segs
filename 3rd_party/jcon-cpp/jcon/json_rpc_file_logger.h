#pragma once

#include "json_rpc_logger.h"
#include "jcon.h"

#include <QFile>
#include <QTextStream>

namespace jcon {

class JCON_API JsonRpcFileLogger : public JsonRpcLogger
{
public:
    JsonRpcFileLogger(const QString& filename);
    virtual ~JsonRpcFileLogger();

    void logDebug(const QString& message) override;
    void logInfo(const QString& message) override;
    void logWarning(const QString& message) override;
    void logError(const QString& message) override;

private:
    QFile m_file;
    QTextStream m_stream;
};

}
