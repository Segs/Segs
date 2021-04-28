/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup AuthServer Projects/CoX/Servers/AuthServer
 * @{
 */

//#define ACE_NTRACE 0
#include "Servers/HandlerLocator.h"
#include "Servers/MessageBus.h"
#include "Servers/MessageBusEndpoint.h"
#include "Servers/InternalEvents.h"
#include "Components/SEGSTimer.h"
#include "Components/Settings.h"
#include "Components/Logging.h"
#include "Version.h"
//////////////////////////////////////////////////////////////////////////

#include "AuthServer.h"
#include "Servers/MapServer/MapServer.h"
#include "Servers/GameServer/GameServer.h"
#include "Servers/GameDatabase/GameDBSync.h"
#include "Servers/AuthDatabase/AuthDBSync.h"
#include "AdminRPC.h"
//////////////////////////////////////////////////////////////////////////

#include <ace/ACE.h>
#include <ace/Singleton.h>
#include <ace/Log_Record.h>
#include <ace/INET_Addr.h>

#include <ace/Reactor.h>
#include <ace/TP_Reactor.h>
#include <ace/INET_Addr.h>
#include <ace/OS_main.h> //Included to enable file logging
#include <ace/streams.h> //Included to enable file logging

#include <QtCore/QCoreApplication>
#include <QtCore/QLoggingCategory>
#include <QtCore/QSettings>
#include <QtCore/QCommandLineParser>
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QElapsedTimer>
#include <thread>
#include <chrono>
#include <mutex>
#include <stdlib.h>
#include <memory>

using namespace SEGSEvents;

namespace
{
static bool s_event_loop_is_done=false; //!< this is set to true when ace reactor is finished.
static std::unique_ptr<AuthServer> g_auth_server; // this is a global for now.
static std::unique_ptr<GameServer> g_game_server;
static std::unique_ptr<MapServer> g_map_server;
static std::unique_ptr<MessageBus> g_message_bus;

struct MessageBusMonitor : public EventProcessor
{
    MessageBusEndpoint m_endpoint;
    MessageBusMonitor() : m_endpoint(*this)
    {
        m_endpoint.subscribe(MessageBus::ALL_EVENTS);
        activate();
    }
    IMPL_ID(MessageBusMonitor)

    // EventProcessor interface
public:
    void dispatch(Event *ev) override
    {
        switch(ev->type())
        {
        case evServiceStatusMessage:
            on_service_status(static_cast<ServiceStatusMessage *>(ev));
            break;
        default:
            ;//qDebug() << "Unhandled message bus monitor command" <<ev->info();
        }
    }
    // EventProcessor interface
protected:
    void serialize_from(std::istream &/*is*/) override
    {
        assert(false);
    }
    void serialize_to(std::ostream &/*os*/) override
    {
        assert(false);
    }
private:
    void on_service_status(ServiceStatusMessage *msg);
};

static std::unique_ptr<MessageBusMonitor> s_bus_monitor;

static void shutDownServers(const char *reason)
{
    qDebug() << "Reason for shutdown: " << reason;

    if(GlobalTimerQueue::instance()->thr_count())
    {
        GlobalTimerQueue::instance()->deactivate();
    }
    shutdown_event_processor_and_wait(g_game_server.get());
    shutdown_event_processor_and_wait(g_map_server.get());
    shutdown_event_processor_and_wait(g_auth_server.get());
    if(s_bus_monitor && s_bus_monitor->thr_count())
    {
        s_bus_monitor->putq(Finish::s_instance->shallow_copy());
    }
    if(g_message_bus && g_message_bus->thr_count())
    {
        g_message_bus->putq(Finish::s_instance->shallow_copy());
    }

    s_event_loop_is_done = true;
}

void MessageBusMonitor::on_service_status(ServiceStatusMessage *msg)
{
    if(msg->m_data.status_value != 0)
    {
        qCritical().noquote() << msg->m_data.status_message;
        shutDownServers("Configuration failure");
    }
    else
        qInfo().noquote() << msg->m_data.status_message;
}

// this event stops main processing loop of the whole server
class ServerStopper final : public ACE_Event_Handler
{
public:
    ServerStopper(int signum) // when instantiated adds itself to current reactor
    {
        ACE_Reactor::instance()->register_handler(signum, this);
}
    // Called when object is signaled by OS.
    int handle_signal(int, siginfo_t * /*s_i*/, ucontext_t * /*u_c*/) final
    {
        shutDownServers("Signal");
        return 0;
    }
};

bool CreateServers()
{
    static ReloadConfigMessage reload_config;
    GlobalTimerQueue::instance()->activate();

    TIMED_LOG(
                {
                    g_message_bus.reset(new MessageBus);
                    HandlerLocator::setMessageBus(g_message_bus.get());
                    g_message_bus->ReadConfigAndRestart();
                }
                ,"Creating message bus");
    TIMED_LOG(s_bus_monitor.reset(new MessageBusMonitor),"Starting message bus monitor");

    TIMED_LOG(startAuthDBSync(),"Starting auth db service");
    TIMED_LOG({
                  g_auth_server.reset(new AuthServer);
                  g_auth_server->activate();
              },"Starting auth service");
//    AdminServer::instance()->ReadConfig();
//    AdminServer::instance()->Run();
    TIMED_LOG(startGameDBSync(1),"Starting game(1) db service");
    TIMED_LOG({
                  g_game_server.reset(new GameServer(1));
                  g_game_server->activate();
              },"Starting game(1) server");
    TIMED_LOG({
                  g_map_server.reset(new MapServer(1));
                  g_map_server->set_game_server_owner(1);
                  g_map_server->activate();
              },"Starting map server");

    qDebug() << "Asking AuthServer to load configuration and begin listening for external connections.";
    g_auth_server->putq(reload_config.shallow_copy());
    qDebug() << "Asking GameServer(0) to load configuration on begin listening for external connections.";
    g_game_server->putq(reload_config.shallow_copy());
    qDebug() << "Asking MapServer to load configuration on begin listening for external connections.";
    g_map_server->putq(reload_config.shallow_copy());

//    server_manger->AddGameServer(game_instance);
    return true;
}
std::mutex log_mutex;

void segsLogMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    static char log_buffer[4096]={0};
    static char category_text[256];
    log_buffer[0] = 0;
    category_text[0] = 0;
    if(strcmp(context.category,"default")!=0)
        snprintf(category_text, sizeof(category_text), "[%s]", context.category);

    QFile segs_log_target;
    QDate todays_date(QDate::currentDate());
    QSettings settings(Settings::getSettingsPath(), QSettings::IniFormat);
    QString log_path = Settings::getSEGSDir() + QDir::separator() + QString("logs");
    settings.beginGroup("Logging");
    if (settings.value("combine_logs", "").toBool() == false) // If combine_logs is off will split logs by logging category.
    {
        // Format file name based on logging category. Splits into a file for each category.
        QString file_name = category_text;
        file_name.replace("[log.", "");
        file_name.replace("]", "");
        if (file_name.isEmpty())
            file_name = "generic";
        file_name = todays_date.toString("yyyy-MM-dd") + "_" + file_name;
        log_path += QDir::separator() +file_name + ".log";
        segs_log_target.setFileName(log_path);
    }
    else // If combine_logs is on will log all to a single file.
    {
        log_path += QDir::separator() +todays_date.toString("yyyy-MM-dd") + "_all.log";
        segs_log_target.setFileName(log_path);
    }
    settings.endGroup();

    if(!segs_log_target.open(QFile::WriteOnly | QFile::Append))
    {
        fprintf(stderr,"Failed to open log file in write mode, will procede with console only logging");
    }

    QByteArray localMsg = msg.toLocal8Bit();
    std::string timestamp  = QTime::currentTime().toString("hh:mm:ss").toStdString();

    switch (type)
    {
        case QtDebugMsg:
            snprintf(log_buffer, sizeof(log_buffer), "[%s] %sDebug   : %s\n",
                     timestamp.c_str(), category_text, localMsg.constData());
            break;
        case QtInfoMsg:
            // no prefix or category for informational messages, as these are end-user facing
            snprintf(log_buffer, sizeof(log_buffer), "[%s] %s\n",
                     timestamp.c_str(), localMsg.constData());
            break;
        case QtWarningMsg:
            snprintf(log_buffer, sizeof(log_buffer), "[%s] %sWarning : %s\n",
                     timestamp.c_str(), category_text, localMsg.constData());
            break;
        case QtCriticalMsg:
            snprintf(log_buffer, sizeof(log_buffer), "[%s] %sCritical: %s\n",
                     timestamp.c_str(), category_text, localMsg.constData());
            break;
        case QtFatalMsg:
            snprintf(log_buffer, sizeof(log_buffer), "[%s] %sFatal: %s\n",
                     timestamp.c_str(), category_text, localMsg.constData());
    }

    fprintf(stdout, "%s", log_buffer);
    fflush(stdout);

    if(segs_log_target.isOpen())
        segs_log_target.write(log_buffer);

    if(type == QtFatalMsg)
    {
        segs_log_target.close();
        abort();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // End of anonymous namespace
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACE_INT32 ACE_TMAIN (int argc, ACE_TCHAR *argv[])
{
    setLoggingFilter(); // Set QT Logging filters
    qInstallMessageHandler(segsLogMessageOutput);
    QCoreApplication q_app(argc,argv);
    QCoreApplication::setOrganizationDomain("segs.dev");
    QCoreApplication::setOrganizationName("SEGS Project");
    QCoreApplication::setApplicationName("segs_server");
    QCoreApplication::setApplicationVersion(VersionInfo::getAuthVersion());

    QCommandLineParser parser;
    parser.setApplicationDescription("SEGS - CoX server");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOptions({
                          {{"f","config"},
                           "Use the provided settings file, default value is <settings.cfg>",
                           "filename","settings.cfg"}
                      });
    parser.process(q_app);
    if(parser.isSet("help")||parser.isSet("version"))
        return 0;

    Settings::setSettingsPath(parser.value("config")); // set settings.cfg from args

    ACE_Sig_Set interesting_signals;
    interesting_signals.sig_add(SIGINT);
    interesting_signals.sig_add(SIGHUP);

    ServerStopper st(SIGINT); // it'll register itself with current reactor, and shut it down on sigint
    ACE_Reactor::instance()->register_handler(interesting_signals,&st);

    // Print out today's date and startup copyright messages
    qInfo().noquote() << QDateTime::currentDateTime().toString();
    qInfo().noquote() << VersionInfo::getCopyright();
    qInfo().noquote() << VersionInfo::getAuthVersion();

    // Create jsonrpc admin interface
    startRPCServer();

    bool no_err = CreateServers();
    if(!no_err)
    {
        ACE_Reactor::instance()->end_reactor_event_loop();
        return -1;
    }
    // process all queued qt messages here.
    while( !s_event_loop_is_done )
    {
        ACE_Time_Value event_processing_delay(0,1000*15);
        ACE_Reactor::instance()->handle_events(&event_processing_delay);
        QCoreApplication::processEvents();
    }
    GlobalTimerQueue::instance()->wait();
    g_game_server->wait();
    g_map_server->wait();
    g_auth_server->wait();
    s_bus_monitor->wait();
    g_message_bus->wait();
    g_game_server.reset();
    g_map_server.reset();
    g_auth_server.reset();
    s_bus_monitor.reset();
    g_message_bus.reset();
    ACE_Time_Value event_processing_delay(0,1000*15);
    ACE_Reactor::instance()->handle_events(&event_processing_delay);
    ACE_Reactor::instance()->remove_handler(interesting_signals);
    ACE_Reactor::end_event_loop();
    return 0;
}

//! @}
