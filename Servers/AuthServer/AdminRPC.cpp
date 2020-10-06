#include "AdminRPC.h"
#include "AuthHandler.h"
#include "Version.h"
#include "Components/ConfigExtension.h"
#include "Components/Settings.h"

#include "jcon/json_rpc_server.h"
#include "jcon/json_rpc_tcp_server.h"
#include "jcon/json_rpc_websocket_server.h"

#include <QEventLoop>
#include <QJsonDocument>
#include <QtCore/QSettings>
#include <QtCore/QString>
#include <QTimer>

using namespace jcon;
using namespace SEGSEvents;

uint64_t AdminRPC::s_last_token = 1;

AdminRPC::AdminRPC()
{
    SetStartTime();
    ReadConfig();
}
AdminRPC::~AdminRPC()
{

}

void AdminRPC::dispatch( Event *ev )
{
    assert(ev);
    switch(ev->type())
    {
    case evAuthDbStatusMessage:
        on_db_error(static_cast<AuthDbStatusMessage *>(ev)); break;
    }
}

void AdminRPC::serialize_from(std::istream &/*is*/)
{
    assert(false);
}

void AdminRPC::serialize_to(std::ostream &/*is*/)
{
    assert(false);
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

/*!
 * @brief Test method - heyServer
 * @return Returns a bool, always true
 */
bool AdminRPC::heyServer()
{
    qCDebug(logRPC) << "Someone said hey...";
    return true;
}

/*!
 * @brief Test method - helloServer
 * @return Returns a string "Hello Web Browser!"
 */
QString AdminRPC::helloServer()
{
    qCDebug(logRPC) << "Someone said hello...";
    QString response = "Hello Web Browser!";
    return response;
}

/*!
 * @brief Get the currently running server version number
 * @return Returns a string with the server version (e.g. "0.6.1")
 */
QString AdminRPC::getVersion()
{
    qCDebug(logRPC) << "Someone requested the version...";
    return VersionInfo::getAuthVersionNumber();
}

/*!
 * @brief Get the currently running server version name
 * @return Returns a string with the server version name (e.g. "Outbreak")
 */
QString AdminRPC::getVersionName()
{
    qCDebug(logRPC) << "Someone requested the versions name...";
    return VersionInfo::getVersionName();
}

/*!
 * @brief A simple ping/pong. Can be used to determine server online status
 * @return Returns a string "pong"
 */
QString AdminRPC::ping()
{
    qCDebug(logRPC) << "Someone sent a ping...";
    QString response = "pong";
    return response;
}

/*!
 * @brief Gets the start time of the currently running server
 * @return Returns a string with currentSecsSinceEpoch
 */
QString AdminRPC::getStartTime()
{
    qCDebug(logRPC) << "Someone requested the server start time...";
    return m_start_time;
}

/*!
 * @brief       Add a user
 * @param[in]   username
 * @param[in]   password
 * @param[in]   access_level
 * @return      Returns a string, if successful "OK" otherwise an error
 */
QString AdminRPC::addUser(const QString &username, const QString &password, int access_level)
{
    QString result;
    uint64_t fake_session_token = s_last_token++;
    int token = static_cast<int>(fake_session_token);

    // Create and insert a record of this request into m_completion_state
    m_completion_state.insert(token, "");

    qCDebug(logRPC) << "addUser call in progress";

    EventProcessor *tgt = HandlerLocator::getAuthDB_Handler();
    tgt->putq(new CreateAccountMessage({username, password, access_level}, fake_session_token, this));

    QTimer response_timer;
    QTimer timeout;
    timeout.setSingleShot(true);
    QEventLoop loop;
    loop.connect(&timeout, SIGNAL(timeout()), SLOT(quit()));
    loop.connect(this, SIGNAL(responseRecieved()), SLOT(quit()));
    loop.connect(&response_timer, &QTimer::timeout, [=] () {
        if (!m_completion_state[token].isEmpty())
        {
            emit responseRecieved(); // Response recieved we can break out of event loop
            return;
        }
        else
            return;
    });
    response_timer.start(500); // Checks completion status every 500ms
    timeout.start(5000); // Timeout of 5 seconds in case something goes wrong.
    loop.exec();

    if (!m_completion_state[token].isEmpty())
    {
        result = m_completion_state[token];
        response_timer.stop();
        m_completion_state.remove(token); // Tidy up
    }
    else
    {
        result = "Something went wrong";
        response_timer.stop();
        m_completion_state.remove(token);
    }

    return result;
}

// Internal methods

void AdminRPC::on_db_error(AuthDbStatusMessage *ev)
{
    m_completion_state[static_cast<int>(ev->session_token())] = ev->m_data.message;
}

/*
 * @brief Get a dictionary of information as used by the WebUI
 * @return Returns a QMap<QString,QVariant> with server information
 */
QVariantMap AdminRPC::getWebUIData(QString const& version)
{
    qCDebug(logRPC) << "Someone requested WebUI information...";
    QMap<QString,QVariant> ret;
    if (version.compare("0.7.0") == 0)
    {
      ret.insert("version", VersionInfo::getAuthVersionNumber());
      ret.insert("starttime", m_start_time);
    }
    else // TODO: what is this supposed to do?
    {
      // Default to v0.7.0 format
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
    m_adminrpc->activate();
    m_server->listen(QHostAddress(m_adminrpc->m_location.get_host_addr()),m_adminrpc->m_location.get_port_number());
}


