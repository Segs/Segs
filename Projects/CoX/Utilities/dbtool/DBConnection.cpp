/*
 * SEGS - Super Entity Game Server - dbtool
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "DBConnection.h"
#include "DatabaseConfig.h"
#include "Logging.h"
#include "Settings.h"

#include <QtSql/QtSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>

DBConnection::DBConnection(const DatabaseConfig &cfg)
    : m_config(cfg)
{
}

DBConnection::~DBConnection()
{
    close();
    // unload the database (this doesn't delete it)
    m_db.removeDatabase(m_config.m_db_path);
    //QSqlDatabase::removeDatabase(m_config.m_db_path);
}

void DBConnection::open()
{
    m_db = QSqlDatabase::addDatabase(m_config.m_driver, m_config.m_db_path);
    m_db.setDatabaseName(m_config.m_db_path); // must be path
    m_db.setHostName(m_config.m_host);
    m_db.setPort(m_config.m_port.toInt());
    m_db.setUserName(m_config.m_user);
    m_db.setPassword(m_config.m_pass);

    if(!m_db.open())
    {
        qWarning() << "Failed to open database" << m_config.m_db_path;
        return;
    }

    if(!m_db.driver()->hasFeature(QSqlDriver::Transactions))
    {
        qWarning() << m_db.driverName() << "does not support Transactions";
        return;
    }

    // Cannot open transaction here because database creation scripts
    // have transactions built in. Seems better to leave the transactions
    // in the creation scripts and use this later when needed.
    m_db.transaction();
    m_query = std::make_unique<QSqlQuery>(m_db);
}

void DBConnection::close()
{
    m_query->clear();
    m_query->finish();
    m_db.close();
}

bool DBConnection::isConnected()
{
    QString querytext;
    if(m_config.isSqlite())
    {
        // For SQLite we can just check that the file exists
        return fileExists(m_config.m_db_path);
    }
    else if(m_config.isMysql() || m_config.isPostgresql())
    {
        if(m_config.isMysql())
            querytext = "show tables";
        else
        {
            querytext = "SELECT table_schema || '.' || table_name FROM";
            querytext.append("information_schema.tables WHERE table_type = 'BASE TABLE' AND table_schema ='");
            querytext.append(m_config.m_db_path);
            querytext.append("';");
        }

        m_query->exec(querytext);

        if(m_query->size() >= 1)
            return true;
    }
}
