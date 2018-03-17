#include "RoamingServer.h"
#include "ServerHandle.h"
#include "InterfaceManager.h"
#include "ConfigExtension.h"
#include "Settings.h"

#include <QtCore/QSettings>
#include <QtCore/QString>
#include <QtCore/QFile>
#include <QtCore/QDebug>
/**
 * @return int
 * @param  configpath This is a platform specific path to a config file containing
 * general RoamingServer vars.
 */
bool RoamingServer::ReadConfig()
{
    qDebug() << "Loading RoamingServer settings...";
    QSettings *config(Settings::getSettings());

    config->beginGroup("RoamingServer");
    if(!config->contains("location_addr"))
        qDebug() << "Config file is missing 'location_addr' entry, will try to use default";

    QString location_addr = config->value("location_addr","127.0.0.1:2106").toString();

    if(!parseAddress(location_addr,m_authaddr))
    {
        qCritical() << "Badly formed IP address" << location_addr;
        return false;
    }
    if(!config->contains("auth_pass")) {
        qDebug() << "Config file is missing 'auth_pass' entry, will try to use default";
    }
    m_passw = config->value("auth_pass","").toString();

    config->endGroup(); // RoamingServer

    return true;
}

/**
* This method will return a valid AuthServerInterface, or NULL if this server
* fails to connect to the AuthServer.
* It'll build an AuthServer handle, and pass it to InterfaceManager, to resolve it to concrete Interface.
*
* @return AuthServerInterface constructed during call
*/
AuthServerInterface *RoamingServer::getAuthServer ( )
{
    ServerHandle<IAuthServer> h_auth(m_authaddr,-1);
    return InterfaceManager::instance()->get(h_auth);

}
