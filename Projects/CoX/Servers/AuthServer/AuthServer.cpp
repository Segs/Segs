/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup AuthServer Projects/CoX/Servers/AuthServer
 * @{
 */

// SEGS includes
#include "AuthServer.h"

#include "ConfigExtension.h"
#include "AuthProtocol/AuthLink.h"
#include "AuthHandler.h"
#include "Settings.h"
#include "SEGSEvent.h"
#include "Servers/InternalEvents.h"

#include <ace/Acceptor.h>
#include <ace/SOCK_Acceptor.h>

#include <QtCore/QSettings>
#include <QtCore/QString>
#include <QtCore/QFile>
#include <QtCore/QDebug>

struct ClientAcceptor : public ACE_Acceptor<AuthLink, ACE_SOCK_ACCEPTOR>
{
    AuthHandler *m_target;
    int make_svc_handler (AuthLink *&sh) override
    {
        if(sh)
            return 0;
        if(!m_target)
            return -1;
        sh = new AuthLink(m_target,AuthLinkType::Server);
        sh->reactor (this->reactor ());
        return 0;
    }
};

/*!
 * @class AuthServer
 * @brief main class of the authentication server, it controls the AuthHandler instances
 *
 */
AuthServer::AuthServer()
{
    m_acceptor = new ClientAcceptor;
    m_handler.reset(new AuthHandler(this));
    m_acceptor->m_target = m_handler.get();
    // Start two threads to handle Auth events.
    m_handler->activate(THR_NEW_LWP|THR_JOINABLE|THR_INHERIT_SCHED,2);
    m_running=false;
}

AuthServer::~AuthServer()
{
    delete m_acceptor;
}

void AuthServer::dispatch(SEGSEvent *ev)
{
    assert(ev);
    switch(ev->type())
    {
        case Internal_EventTypes::evReloadConfig:
            ReadConfigAndRestart();
            break;
        default:
            assert(!"Unknown event encountered in dispatch.");
    }
}

/*!
 * @brief Read server configuration
 * @note m_mutex is held locked during this function
 * @return bool, if it's false, this function failed somehow.
 */
bool AuthServer::ReadConfigAndRestart()
{
    ACE_Guard<ACE_Thread_Mutex> guard(m_mutex);

    qInfo() << "Loading AuthServer settings...";
    QSettings config(Settings::getSettingsPath(),QSettings::IniFormat,nullptr);

    config.beginGroup(QStringLiteral("AuthServer"));
    if(!config.contains(QStringLiteral("location_addr")))
        qDebug() << "Config file is missing 'location_addr' entry in AuthServer group, will try to use default";

    QString location_addr = config.value(QStringLiteral("location_addr"),"127.0.0.1:2106").toString();
    config.endGroup(); // AuthServer

    if(!parseAddress(location_addr,m_location))
    {
        qCritical() << "Badly formed IP address" << location_addr;
        return false;
    }

    return Run();
}

/*!
 * @brief Starts this server up, by opening the connection acceptor on given location.
 * This method can be called multiple times, to re-open the listening socket on different addresses.
 * @return bool, if it's false, we somehow failed to start. Error report is logged by qCritical
 *
 */
bool AuthServer::Run()
{
    if (m_running)
    {
        m_acceptor->close();
        m_running = false;
    }
    if (m_acceptor->open(m_location) == -1)
    {
        qCritical() << "Auth server failed to accept connections on:" << m_location.get_host_addr();
        return false;
    }
    m_running=true;
    return true;
}

/*!
 * @brief Shuts the server down
 * @return bool, if it's false, we failed to close down cleanly
 */
bool AuthServer::ShutDown()
{
    ACE_Guard<ACE_Thread_Mutex> guard(m_mutex);
    if (m_running)
    {
        qWarning() << "Auth server listener is closing down";
        m_acceptor->close();
    }
    // force our handler to finish
    m_handler->putq(SEGSEvent::s_ev_finish.shallow_copy());
    m_handler->wait();
    m_running = false;
    putq(SEGSEvent::s_ev_finish.shallow_copy());
    wait();
    return true;
}

//! @}
