#include "AdminRPC.h"
#include "AuthHandler.h"
#include "version.h"
#include "ConfigExtension.h"

#include "Settings.h"

#include "jcon/json_rpc_server.h"
#include "jcon/json_rpc_tcp_server.h"
#include "jcon/json_rpc_websocket_server.h"

#include <QVariant>
#include <QJsonDocument>
#include <QtCore/QSettings>
#include <QtCore/QString>
#include <QtCore/QDebug>

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

    qDebug() << "[ADMINRPC]: Start loading settings...";
    QSettings config(Settings::getSettingsPath(),QSettings::IniFormat,nullptr);

    config.beginGroup(QStringLiteral("AdminRPC"));
    if(!config.contains(QStringLiteral("location_addr")))
        qDebug() << "[ADMINRPC]: Config file is missing 'location_addr' entry in AdminRPC group, will try to use default";

    QString location_addr = config.value(QStringLiteral("location_addr"),"127.0.0.1:6001").toString();
    QString server_type = config.value(QStringLiteral("server_type"),"tcp").toString();
    if(server_type == "tcp")
    {
        m_socket_type     = SocketType::tcp;
    }
    else
    {
        m_socket_type     = SocketType::websocket;
    }
    config.endGroup(); // AdminRPC

    if(!parseAddress(location_addr,m_location))
    {
        qCritical() << "[ADMINRPC]: Badly formed IP address: " << location_addr;
        return false;
    }
    qDebug() << "[ADMINRPC]: Finished loading settings...";
    return true;
}

bool AdminRPC::heyServer()
{
    qDebug() << "[ADMINRPC::heyServer()]: Someone said hey!";
    return true;
}

QString AdminRPC::helloServer()
{
    qDebug() << "[ADMINRPC::helloServer()]: Someone said hello!";
    QString response = "Hello Web Browser!";
    return response;
}

QString AdminRPC::getVersion()
{
    qDebug() << "[ADMINRPC::getVersion()]: Someone got the version!";
    QString version = VersionInfo::getAuthVersionNumber() + QString(" ") + VersionInfo::getVersionName();
    return version;
}

QString AdminRPC::ping()
{
    qDebug() << "[ADMINRPC::ping()]: Someone sent a ping!";
    QString response = "pong";
    return response;
}

QString AdminRPC::getStartTime()
{
    qDebug() << "[ADMINRPC::getStartTime()]: Someone asked for the start time!";
    return m_start_time;
}

void startRPCServer()
{
    static jcon::JsonRpcServer *m_server;
    AdminRPC* m_adminrpc = new AdminRPC();
    if(!m_server)
    {
        if (m_adminrpc->m_socket_type == SocketType::tcp) {
            qDebug() << "[ADMINRPC]: Creating TCP server";
            m_server = new jcon::JsonRpcTcpServer();
        } else {
            qDebug() << "[ADMINRPC]: Creating WebSocket server";
            m_server = new jcon::JsonRpcWebSocketServer();
        }
    }
    m_server->registerServices({ m_adminrpc });
    m_server->listen(QHostAddress(m_adminrpc->m_location.get_host_addr()),m_adminrpc->m_location.get_port_number());
}
