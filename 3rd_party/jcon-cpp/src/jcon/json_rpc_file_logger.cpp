#include "json_rpc_file_logger.h"

namespace jcon {

JsonRpcFileLogger::JsonRpcFileLogger(const QString& filename)
    : m_file(filename)
{
    m_file.open(QIODevice::WriteOnly);
    m_stream.setDevice(&m_file);
}

JsonRpcFileLogger::~JsonRpcFileLogger()
{
    m_file.close();
}

void JsonRpcFileLogger::logDebug(const QString& message)
{
    m_stream << message << "\n";
}

void JsonRpcFileLogger::logInfo(const QString& message)
{
    m_stream << message << "\n";
}

void JsonRpcFileLogger::logWarning(const QString& message)
{
    m_stream << message << "\n";
}

void JsonRpcFileLogger::logError(const QString& message)
{
    m_stream << message << "\n";
}

}
