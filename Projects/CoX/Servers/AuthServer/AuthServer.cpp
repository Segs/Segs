/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *

 */

// segs includes
#include "AuthServer.h"

#include "ConfigExtension.h"
#include "AdminServer/AccountInfo.h"
#include "Common/Servers/AdminServerInterface.h"
#include "Common/Servers/ServerManager.h"
#include "AuthProtocol/AuthLink.h"
#include "AuthHandler.h"
#include "Settings.h"

#include <QtCore/QSettings>
#include <QtCore/QString>
#include <QtCore/QFile>
#include <QtCore/QDebug>

/*!
 * @class AuthServer
 * @brief main class of the authentication server.
 *
 * @note AuthServer should pull client data from database or from it's local,
 * in-memory cache currently there is no such thing, and 'client-cache' is just a hash-map
 */


AuthServer::AuthServer()
{
    m_acceptor = new ClientAcceptor;
    m_running=false;
}

AuthServer::~AuthServer()
{

    ShutDown();
    delete m_acceptor;
}
/*!
 * @brief Read server configuration
 * @param inipath is a path to our configuration file.
 * @return bool, if it's false, this function failed somehow.
 */
bool AuthServer::ReadConfig()
{
    if(m_running)
        ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("(%P|%t) AuthServer: Already initialized and running\n") ),false);

    qInfo() << "Loading AuthServer settings...";
    QSettings *config(Settings::getSettings());

    config->beginGroup(QStringLiteral("AuthServer"));
    if(!config->contains(QStringLiteral("location_addr")))
        qDebug() << "Config file is missing 'location_addr' entry in AuthServer group, will try to use default";

    QString location_addr = config->value(QStringLiteral("location_addr"),"127.0.0.1:2106").toString();

    if(!parseAddress(location_addr,m_location))
    {
        qCritical() << "Badly formed IP address" << location_addr;
        return false;
    }
    config->endGroup(); // AuthServer

    return true;
}
/*!
 * @brief Starts this server up, by opening the connection acceptor on given location.
 * @return bool, if it's false, we somehow failed to start. Error report sis logged by ACE_ERROR_RETURN
 */
bool AuthServer::Run()
{
    AuthLink::g_target = new AuthHandler(this);
    AuthLink::g_target->activate(THR_NEW_LWP|THR_JOINABLE|THR_INHERIT_SCHED,2);
    if (m_acceptor->open(m_location) == -1)
    {
        ACE_ERROR_RETURN ((LM_ERROR, ACE_TEXT ("%p\n"),ACE_TEXT ("Opening the Acceptor failed")), false);
    }
    m_running=true;
    return true;
}
/*!
 * @brief Shuts the server down
 * @param reason the value that should be stored the persistent log.
 * @return bool, if it's false, we failed to close down cleanly
 */
bool AuthServer::ShutDown(const QString &/* ="No particular reason" */)
{
    m_acceptor->close();
    // tell our handler to shut down too
    AuthLink::g_target->putq(new SEGSEvent(SEGS_EventTypes::evFinish,nullptr));
    m_running=false;
    return true;
}

/*!
 * @brief Returns AuthClient corresponding to given login.
 * @param login - client's login.
 * @return true if found
 * If the client with given login exist in clients hash map return it, otherwise create this object and fill it from db.
 */
bool AuthServer::GetClientByLogin(const char *login, AuthAccountData &toFill)
{
    //TODO: use Account cache, or push that functionality to db layer ?
    AdminServerInterface *adminserv;                            // this will be used in case when we don't have this client in the cache
    adminserv = ServerManager::instance()->GetAdminServer();
    assert(adminserv);
    toFill.m_login = login;
    if( !adminserv->FillClientInfo(toFill) )       // Can we fill the client account info from db ?
    {
        return false;
    }
    // check if object is filled in correctly, by validating it's db id.
    if(toFill.m_acc_server_acc_id==0)
    {
        return false;
    }
    return true;
}
