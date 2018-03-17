/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

// segs includes
#include "AdminServer.h"

#include "AdminDatabase.h"
#include "AdminLink.h"
#include "AccountInfo.h"
#include "Client.h"
#include "ConfigExtension.h"
#include "CharacterDatabase.h"
#include "ServerManager.h"
#include "Settings.h"

#include <QtCore/QSettings>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QFile>
#include <QtCore/QDebug>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

// AdminServer should pull client data from database or from it's local, in-memory cache
// currently there is no such thing, and 'client-cache' is just a hash-map

// Defined constructor
_AdminServer::_AdminServer() : m_running(false)
{
    m_db        = new AdminDatabase;
    m_char_db   = new CharacterDatabase;
    m_acceptor = new AdminClientAcceptor;

}

// Defined destructor
_AdminServer::~_AdminServer()
{
    (void)ShutDown();
}

/// later name will be used to read GameServer specific configuration
bool _AdminServer::ReadConfig()
{
    if(m_running)
        ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("(%P|%t) AdminServer: Already initialized and running\n") ),false);

    qDebug() << "Loading AdminServer settings...";
    QSettings *config(Settings::getSettings());

    config->beginGroup("AdminServer");

    config->beginGroup("AccountDatabase");
    QString dbdriver = config->value("db_driver","QSQLITE").toString();
    QString dbhost = config->value("db_host","127.0.0.1").toString();
    int dbport = config->value("db_port","5432").toInt();
    QString dbname = config->value("db_name","segs").toString();
    QString dbuser = config->value("db_user","segsadmin").toString();
    QString dbpass = config->value("db_pass","segs123").toString();
    config->endGroup(); // AccountDatabase
    QSqlDatabase *db1;
    QStringList driver_list {"QSQLITE","QPSQL"};
    if(!driver_list.contains(dbdriver.toUpper())) {
        qWarning() << "Database driver" << dbdriver << " not supported";
    }
    db1 = new QSqlDatabase(QSqlDatabase::addDatabase(dbdriver.toUpper(),"AccountDatabase"));
    db1->setHostName(dbhost);
    db1->setPort(dbport);
    db1->setDatabaseName(dbname);
    db1->setUserName(dbuser);
    db1->setPassword(dbpass);
    m_db->setDb(db1);

    config->beginGroup("CharacterDatabase");
    dbdriver = config->value("db_driver","QSQLITE").toString();
    dbhost = config->value("db_host","127.0.0.1").toString();
    dbport = config->value("db_port","5432").toInt();
    dbname = config->value("db_name","segs_game").toString();
    dbuser = config->value("db_user","segsadmin").toString();
    dbpass = config->value("db_pass","segs123").toString();
    QSqlDatabase *db2;
    if(!driver_list.contains(dbdriver.toUpper())) {
        qWarning() << "Database driver" << dbdriver << " not supported";
    }
    db2 = new QSqlDatabase(QSqlDatabase::addDatabase(dbdriver,"CharacterDatabase"));
    db2->setHostName(dbhost);
    db2->setPort(dbport);
    db2->setDatabaseName(dbname);
    db2->setUserName(dbuser);
    db2->setPassword(dbpass);
    m_char_db->setDb(db2);

    config->endGroup(); // CharacterDatabase
    config->endGroup(); // AdminServer

    return true;
}
bool _AdminServer::Run()
{
    if(!m_db->getDb()->open())
        return false;
    m_db->getDb()->exec("PRAGMA foreign_keys = ON");
    if(m_db->getDb()->lastError().isValid()) {
        qWarning() << m_db->getDb()->lastError();
        return false;
    }
    m_db->on_connected(m_db->getDb());

    if(!m_char_db->getDb()->open())
        return false;
    m_char_db->getDb()->exec("PRAGMA foreign_keys = ON");
    if(m_char_db->getDb()->lastError().isValid()) {
        qWarning() << m_char_db->getDb()->lastError();
        return false;
    }
    m_char_db->on_connected(m_char_db->getDb());
    m_running=true;
    return true;
}
bool _AdminServer::ShutDown(const QString &reason/* ="No particular reason" */)
{
    if(!m_running)
        return true;
    bool res=true;
    qInfo() << "Shutting down AdminServer:"<<reason;
    m_running=false;

    delete m_db;
    delete m_char_db;
    m_db      = nullptr;
    m_char_db = nullptr;
    return res;
}
/// Refresh client object from database
bool _AdminServer::fill_account_info( AccountInfo &client )
{
    if((client.login().size()>0) && (client.account_server_id()==0)) // existing client
    {
        return m_db->GetAccountByName(client,client.login());
    }
    if(client.account_server_id()) // existing client
    {
        return m_db->GetAccountById(client,client.account_server_id());
    }
    return false;

}
/// \note will record given client as logged in.
bool _AdminServer::Login(const AccountInfo &,const ACE_INET_Addr &)
{
    // Here we should log to the Db, a Login event for that client
    //client->setState(AuthClient::LOGGED_IN); modifying this should be done in AuthServer
    return true;
}

bool _AdminServer::Logout(const AccountInfo &) const
{
    // Here we should log to the Db, a Logout event for that client
    //if(client)
    //  client->setState(AuthClient::NOT_LOGGEDIN);
    return true;
}
/// Verifies entered password matches stored password
bool _AdminServer::ValidPassword( const AccountInfo &client, const char *password )
{
    return m_db->ValidPassword(qPrintable(client.login()),password);
}

/// Save user account credentials to storage
int _AdminServer::SaveAccount(const char *username, const char *password)
{
    int res=0;
    if(false==m_db->AddAccount(username, password))
        return 1;

    AccountInfo tmp;
    tmp.login(username); // Fix if username is preprocessed before db entry in AddAccount
    fill_account_info(tmp);
    // also register this account on all currently available gameservers
    // they really should check for sync with AdminDb on startup
    //for(size_t idx=0; idx<ServerManager::instance()->GameServerCount(); idx++)
    //{
    // TODO: support multiple game servers.
    GameServerInterface *igs = ServerManager::instance()->GetGameServer(0);
    if(!igs)
        return -1;
    if(!m_char_db->CreateLinkedAccount(tmp.account_server_id(),username,igs->getMaxCharacterSlots()))
        res=2;
    //}
    return res;   // Add the given account
}

int _AdminServer::RemoveAccount(AccountInfo &client)
{
    int res = m_db->RemoveAccountByID(client.account_server_id());
    if(0==res)
        res = m_char_db->remove_account(client.game_server_id());
    return res;
}

/*
*/

/*
bool AdminServer::AccountBlocked(const char *login) const
{
        IClient *cl = GetClientByLogin(login);
        if(cl)
        {
                if(cl->getState()!=IClient::ACCOUNT_BLOCKED)
                        return false; // Account is not blocked. Allow account to be used.
        }
        return true; // Account is blocked, or there is no such account. Deny them access.
}
*/
/// Add client's IP to the banlist.
int _AdminServer::AddIPBan(const ACE_INET_Addr &client_addr)
{
    m_ban_list.push_back(client_addr);
    return 0;
}

/**
 * This method will find the most occupied GameServer, and attach MapServer bundle
 * (map_h) to it.
 * @return ServerHandle
 * @param  map_h
 */
ServerHandle<IGameServer> _AdminServer::RegisterMapServer(const ServerHandle<IMapServer> &/*map_h*/)
{
    // For each server, find number of free Map handlers, and player occupancy ( servers with low number of Maps, and low occupancy should be prioritized)
    return ServerHandle<IGameServer>(nullptr);
}


/**
 * Idea: All GetAccessKeyForServer methods create a new 'allowed access' entry in their callers table.
 *       So each call using Handle, is marked/encoded with it, and can be verified as valid.
 * @return int
 * @param  map_h
 */
int _AdminServer::GetAccessKeyForServer(const ServerHandle<IMapServer> &/*map_h*/)
{
    return 0;
}

bool _AdminServer::Online( )
{
    return m_running;
}

void _AdminServer::InvalidGameServerConnection( const ACE_INET_Addr & )
{

}
/// Called from auth server during user authentication, might be useful for automatical firewall rules update
int _AdminServer::GetBlockedIpList( std::list<int> & ) /* Called from auth server during user authentication, might be useful for automatical firewall rules update */
{
    return 0;
}
