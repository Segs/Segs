/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

// segs includes
#include "AdminServer.h"

#include "AuthDatabase/AuthDBSyncEvents.h"
#include "AdminLink.h"
#include "AccountInfo.h"
#include "ConfigExtension.h"
#include "CharacterDatabase.h"
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
_AdminServer::_AdminServer()
{
    m_char_db.reset(new CharacterDatabase);

}

// Defined destructor
_AdminServer::~_AdminServer()
{
}

/// later name will be used to read GameServer specific configuration
bool _AdminServer::ReadConfig()
{
    QStringList driver_list {"QSQLITE","QPSQL"};

    qInfo() << "Loading AdminServer settings...";
    QSettings config(Settings::getSettingsPath(),QSettings::IniFormat,nullptr);

    config.beginGroup("AdminServer");

    config.beginGroup("CharacterDatabase");
    QString dbdriver = config.value("db_driver","QSQLITE").toString();
    QString dbhost = config.value("db_host","127.0.0.1").toString();
    int dbport = config.value("db_port","5432").toInt();
    QString dbname = config.value("db_name","segs_game").toString();
    QString dbuser = config.value("db_user","segsadmin").toString();
    QString dbpass = config.value("db_pass","segs123").toString();
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

    config.endGroup(); // CharacterDatabase
    config.endGroup(); // AdminServer

    return true;
}
bool _AdminServer::Run()
{

    if(!m_char_db->getDb()->open())
        return false;
    m_char_db->getDb()->exec("PRAGMA foreign_keys = ON");
    if(m_char_db->getDb()->lastError().isValid()) {
        qWarning() << m_char_db->getDb()->lastError();
        return false;
    }
    m_char_db->on_connected(m_char_db->getDb());
    return true;
}
bool _AdminServer::ShutDown(const QString &reason/* ="No particular reason" */)
{
    bool res=true;
    qInfo() << "Shutting down AdminServer:"<<reason;

    m_char_db.reset();
    return res;
}

