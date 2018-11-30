#include "AdminRPC.h"
#include "AuthHandler.h"
#include "version.h"
#include "ConfigExtension.h"
#include "Settings.h"

#include "jcon/json_rpc_server.h"
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
        qDebug() << "Config file is missing 'location_addr' entry in AdminRPC group, will try to use default";

    QString location_addr = config.value(QStringLiteral("location_addr"),"127.0.0.1:6001").toString();
    config.endGroup(); // AdminRPC

    if(!parseAddress(location_addr,m_location))
    {
        qCritical() << "Badly formed IP address: " << location_addr;
        return false;
    }

    return true;
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

QString AdminRPC::ping()
{
    QString response = "pong";
    return response;
}

QString AdminRPC::getStartTime()
{
    return m_start_time;
}

void startWebSocketServer()
{
    static jcon::JsonRpcWebSocketServer *m_server;
    if(!m_server)
    {
        m_server = new JsonRpcWebSocketServer();
    }
    AdminRPC* m_adminrpc = new AdminRPC();
    m_server->registerServices({ m_adminrpc });
    m_server->listen(QHostAddress(m_adminrpc->m_location.get_host_addr()),m_adminrpc->m_location.get_port_number());
}
