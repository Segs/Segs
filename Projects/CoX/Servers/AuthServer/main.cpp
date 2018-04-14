/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *

 */
//#define ACE_NTRACE 0
#include "Servers/server_support.h"
#include "Servers/HandlerLocator.h"
#include "Servers/MessageBus.h"
#include "Servers/MessageBusEndpoint.h"
#include "Servers/InternalEvents.h"
#include "SEGSTimer.h"
#include "Settings.h"
#include "Logging.h"
#include "version.h"
//////////////////////////////////////////////////////////////////////////

#include "Servers/AdminServer/AdminServer.h"
#include "AuthServer.h"
#include "Servers/MapServer/MapServer.h"
#include "Servers/GameServer/GameServer.h"
#include "Servers/GameDatabase/GameDBSync.h"
#include "Servers/AuthDatabase/AuthDBSync.h"
//////////////////////////////////////////////////////////////////////////

#include <ace/ACE.h>
#include <ace/Singleton.h>
#include <ace/Null_Mutex.h>
#include <ace/Log_Record.h>
#include <ace/INET_Addr.h>
#include <ace/SOCK_Connector.h>
#include <ace/SOCK_Acceptor.h>

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
#include <QtCore/QElapsedTimer>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <memory>
#define TIMED_LOG(x,msg) {\
    QDebug log(qDebug());\
    log << msg << "..."; \
    QElapsedTimer timer;\
    timer.start();\
    x;\
    log << "done in"<<float(timer.elapsed())/1000.0f<<"s";\
}
struct MessageBusMonitor : private EventProcessor
{
    MessageBusEndpoint m_endpoint;
    MessageBusMonitor() : m_endpoint(*this)
    {
        m_endpoint.subscribe(MessageBus::ALL_EVENTS);
        activate();
    }

    // EventProcessor interface
public:
    void dispatch(SEGSEvent *ev)
    {
        switch(ev->type())
        {
        case Internal_EventTypes::evServiceStatus:
            on_service_status(static_cast<ServiceStatusMessage *>(ev));
            break;
        default:
            ;//qDebug() << "Unhandled message bus monitor command" <<ev->info();
        }
    }
private:
    void on_service_status(ServiceStatusMessage *msg)
    {
        if(msg->m_data.status_value!=0)
        {
            qCritical().noquote() << msg->m_data.status_message;
            exit(1);
        }
        else
            qInfo() << msg->m_data.status_message;
    }
};
namespace
{
static bool s_event_loop_is_done=false; //!< this is set to true when ace reactor is finished.
static std::unique_ptr<AuthServer> g_auth_server; // this is a global for now.
static std::unique_ptr<GameServer> g_game_server;
static std::unique_ptr<MapServer> g_map_server;
static MessageBus *g_message_bus=nullptr;
static MessageBusMonitor *s_bus_monitor;
static void destroyServers()
{

    g_auth_server->putq(new SEGSEvent(SEGS_EventTypes::evFinish));
}
ACE_THR_FUNC_RETURN event_loop (void *arg)
{
    ACE_Reactor *reactor = static_cast<ACE_Reactor *>(arg);
    reactor->owner (ACE_OS::thr_self ());
    reactor->run_reactor_event_loop ();
    shutDownAllActiveHandlers();
    GlobalTimerQueue::instance()->deactivate();
    s_event_loop_is_done = true;
    return (ACE_THR_FUNC_RETURN)nullptr;
}
bool CreateServers()
{
    static ReloadConfigMessage reload_config;
    GlobalTimerQueue::instance()->activate();

    TIMED_LOG(
                {
                    g_message_bus = new MessageBus;
                    HandlerLocator::setMessageBus(g_message_bus);
                    g_message_bus->ReadConfigAndRestart();
                }
                ,"Creating message bus");
    TIMED_LOG(s_bus_monitor = new MessageBusMonitor,"Starting message bus monitor");

    TIMED_LOG(startAuthDBSync(),"Starting auth db service");
    TIMED_LOG({
                  g_auth_server.reset(new AuthServer);
                  g_auth_server->activate();
              },"Starting auth service");
    AdminServer::instance()->ReadConfig();
    AdminServer::instance()->Run();
    TIMED_LOG(startGameDBSync(1),"Starting game(1) db service");
    TIMED_LOG({
                  g_game_server.reset(new GameServer(1));
                  g_game_server->activate();
              },"Starting game(1) server");
    TIMED_LOG({
                  g_map_server.reset(new MapServer());
                  g_map_server->sett_game_server_owner(1);
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

QFile segs_log_target;
void segsLogMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    static QTextStream stdOut(stdout);
    static QTextStream fileLog(&segs_log_target);
    QString message;
    switch (type)
    {
        case QtDebugMsg:
            message = "Debug   : ";
            break;
        case QtInfoMsg:
            message = "";  // no prefix for informational messages
            break;
        case QtWarningMsg:
            message = "Warning : ";
            break;
        case QtCriticalMsg:
            message = "Critical: ";
            break;
        case QtFatalMsg:
            stdOut << "Fatal error" << localMsg.constData();
            abort();
    }
    stdOut << message << localMsg.constData() << "\n";
    stdOut.flush();
    if(type!=QtInfoMsg)
    {
        fileLog << message << localMsg.constData() << "\n";
        fileLog.flush();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // End of anonymous namespace
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACE_INT32 ACE_TMAIN (int argc, ACE_TCHAR *argv[])
{
    segs_log_target.setFileName("output.log");
    if(!segs_log_target.open(QFile::WriteOnly|QFile::Append))
    {
        qCritical() << "Failed to open log file in write mode, will procede with console only logging";
    }
    setLoggingFilter(); // Set QT Logging filters
    qInstallMessageHandler(segsLogMessageOutput);
    QCoreApplication q_app(argc,argv);
    QCoreApplication::setOrganizationDomain("segs.nemerle.eu");
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

    const size_t N_THREADS = 1;
    ACE_TP_Reactor threaded_reactor;
    ACE_Reactor new_reactor(&threaded_reactor); //create concrete reactor
    std::unique_ptr<ACE_Reactor> old_instance(ACE_Reactor::instance(&new_reactor)); // this will delete old instance when app finishes

    ServerStopper st; // it'll register itself with current reactor, and shut it down on sigint
    new_reactor.register_handler(interesting_signals,&st);

    // Print out startup copyright messages

    qInfo().noquote() << VersionInfo::getCopyright();
    qInfo().noquote() << VersionInfo::getAuthVersion();

    qInfo().noquote() << "main";

    ACE_Thread_Manager::instance()->spawn_n(N_THREADS, event_loop, ACE_Reactor::instance());
    bool no_err = CreateServers();
    if(!no_err)
    {
        ACE_Reactor::instance()->end_event_loop();
        ACE_Thread_Manager::instance()->wait();
        ACE_Reactor::close_singleton();
        return -1;
    }
    // process all queued qt messages here.
    ACE_Time_Value event_processing_delay(0,1000*5);
    while( !s_event_loop_is_done )
    {
        QCoreApplication::processEvents();
        ACE_OS::sleep(event_processing_delay);
    }

    ACE_Thread_Manager::instance()->wait();
    ACE_Reactor::close_singleton();
    return 0;
}
