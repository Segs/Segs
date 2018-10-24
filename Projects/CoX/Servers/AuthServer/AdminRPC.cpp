#include "AdminRPC.h"
#include "jcon/json_rpc_server.h"
#include "jcon/json_rpc_websocket_server.h"

#include "AuthHandler.h"
#include "version.h"

#include <QVariant>
#include <QJsonDocument>

using namespace jcon;
AdminRPC::AdminRPC()
{
}

bool AdminRPC::heyServer()
{
    qDebug() << "Someone said hello !";
    return true;
}

QString AdminRPC::helloServer()
{
    QString response = "Hello Web Browser!";
    return response;
}

QString AdminRPC::getVersion()
{
    QString version = VersionInfo::getAuthVersionNumber() + QString(" ") + VersionInfo::getVersionName();
    return version;
}

void startWebSocketServer(const char *addr, int port)
{
    static jcon::JsonRpcWebSocketServer *m_server;
    if(!m_server)
    {
        m_server = new JsonRpcWebSocketServer();
    }
    m_server->registerServices({ new AdminRPC() });
    m_server->listen(QHostAddress(addr),port);
}
