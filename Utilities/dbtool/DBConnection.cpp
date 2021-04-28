/*
 * SEGS - Super Entity Game Server - dbtool
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "DBConnection.h"
#include "DatabaseConfig.h"
#include "Components/Logging.h"

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
    QSqlDatabase::removeDatabase(m_config.m_db_name);
}

void DBConnection::open()
{
    if(m_db != nullptr)
        close();

    qCDebug(logDB) << "Opening database connection at:" << m_config.m_db_name;

    m_db = std::make_unique<QSqlDatabase>(QSqlDatabase::addDatabase(m_config.m_driver, m_config.m_db_name));
    m_db->setDatabaseName(m_config.m_db_name); // must be path
    m_db->setHostName(m_config.m_host);
    m_db->setPort(m_config.m_port.toInt());
    m_db->setUserName(m_config.m_user);
    m_db->setPassword(m_config.m_pass);

    if(!m_db->open())
    {
        qWarning() << "Failed to open database" << m_config.m_db_name;
        return;
    }

    if(!m_db->driver()->hasFeature(QSqlDriver::Transactions))
    {
        qWarning() << m_db->driverName() << "does not support Transactions";
        return;
    }

    // Open a transaction here
    m_db->transaction();
    m_query = std::make_unique<QSqlQuery>(*m_db);
}

void DBConnection::close()
{
    if(m_db == nullptr)
        return;

    if(m_query != nullptr)
    {
        m_query->clear();
        m_query->finish();
        m_query.reset();
    }

    m_db->close();
    m_db->setConnectOptions();
    m_db.reset();
}
