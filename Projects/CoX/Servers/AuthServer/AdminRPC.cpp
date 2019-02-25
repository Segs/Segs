#include "AdminRPC.h"
#include "AuthHandler.h"
#include "Version.h"
#include "ConfigExtension.h"
#include "Settings.h"

#include "jcon/json_rpc_server.h"
#include "jcon/json_rpc_tcp_server.h"
#include "jcon/json_rpc_websocket_server.h"

#include <QVariant>
#include <QJsonDocument>
#include <QtCore/QSettings>
#include <QtCore/QString>

using namespace jcon;
AdminRPC::AdminRPC()
{
    SetStartTime();
    ReadConfig();
}
AdminRPC::~AdminRPC()
{
}

void AdminRPC::SetStartTime()
{
    m_start_time = QString::number(QDateTime::currentSecsSinceEpoch());
}

/*!
 * @brief Read server configuration
 * @note m_mutex is held locked during this function
 * @return bool, if it's false, this function failed somehow.
 */
bool AdminRPC::ReadConfig()
{
    ACE_Guard<ACE_Thread_Mutex> guard(m_mutex);

    qInfo() << "Loading AdminRPC settings...";
    QSettings config(Settings::getSettingsPath(),QSettings::IniFormat,nullptr);

    config.beginGroup(QStringLiteral("AdminRPC"));
    if(!config.contains(QStringLiteral("location_addr")))
        qCDebug(logRPC) << "Config file is missing 'location_addr' entry in AdminRPC group, will try to use default";

    QString location_addr = config.value(QStringLiteral("location_addr"),"127.0.0.1:6001").toString();

    if(!config.contains(QStringLiteral("server_type")))
        qCDebug(logRPC) << "Config file is missing 'server_type' entry in AdminRPC group, will try to use default";

    QString server_type = config.value(QStringLiteral("server_type"),"tcp").toString();

    if(server_type == "websocket")
        m_socket_type     = SocketType::websocket;
    else
        m_socket_type     = SocketType::tcp;

    config.endGroup(); // AdminRPC

    if(!parseAddress(location_addr,m_location))
    {
        qCritical() << "Badly formed IP address '" << location_addr << "' in AdminRPC.";
        return false;
    }
    qCInfo(logRPC) << "Loading AdminRPC settings complete...";
    return true;
}

bool AdminRPC::heyServer()
{
    qCDebug(logRPC) << "Someone said hey...";
    return true;
}

QString AdminRPC::helloServer()
{
    qCDebug(logRPC) << "Someone said hello...";
    QString response = "Hello Web Browser!";
    return response;
}

QString AdminRPC::getVersion()
{
    qCDebug(logRPC) << "Someone requested the version...";
    return VersionInfo::getAuthVersionNumber();
}

QString AdminRPC::getVersionName()
{
    qCDebug(logRPC) << "Someone requested the versions name...";
    return VersionInfo::getVersionName();
}

QString AdminRPC::ping()
{
    qCDebug(logRPC) << "Someone sent a ping...";
    QString response = "pong";
    return response;
}

QString AdminRPC::getStartTime()
{
    qCDebug(logRPC) << "Someone requested the server start time...";
    return m_start_time;
}

/*!
 * @brief Get a dictionary of information as used by the WebUI
 * @return Returns a QMap<QString,QVariant> with server information
 */

QVariantMap AdminRPC::getWebUIData(QString const& version)
{
    qCDebug(logRPC) << "Someone requested WebUI information...";
    QMap<QString,QVariant> ret;
    if (version.compare("0.6.1") == 0)
    {
      ret.insert("version", VersionInfo::getAuthVersionNumber());
      ret.insert("starttime", m_start_time);
    }
    else
    {
      // Default to v0.6.1 format
      ret.insert("version", VersionInfo::getAuthVersionNumber());
      ret.insert("starttime", m_start_time);
    }      
    return ret;
}

void startRPCServer()
{
    static jcon::JsonRpcServer *m_server;
    AdminRPC* m_adminrpc = new AdminRPC();
    if(!m_server)
    {
        if (m_adminrpc->m_socket_type == SocketType::tcp)
        {
            qCDebug(logRPC) << "Creating RPC server using TCP...";
            m_server = new jcon::JsonRpcTcpServer();
        }
        else
        {
            qCDebug(logRPC) << "Creating RPC server using WebSocket...";
            m_server = new jcon::JsonRpcWebSocketServer();
        }
    }
    m_server->registerServices({ m_adminrpc });
    m_server->listen(QHostAddress(m_adminrpc->m_location.get_host_addr()),m_adminrpc->m_location.get_port_number());
}
