/*
 * SEGS - Super Entity Game Server - dbtool
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "DBConnection.h"
#include "DatabaseConfig.h"
#include "Logging.h"

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
    delete m_db;
    // unload the database (this doesn't delete it)
    QSqlDatabase::removeDatabase(m_config.m_db_path);
}

void DBConnection::open()
{
    qCDebug(logDB) << "Opening database connection at:" << m_config.m_db_path;

    m_db = new QSqlDatabase(QSqlDatabase::addDatabase(m_config.m_driver, m_config.m_db_path));
    m_db->setDatabaseName(m_config.m_db_path); // must be path
    m_db->setHostName(m_config.m_host);
    m_db->setPort(m_config.m_port.toInt());
    m_db->setUserName(m_config.m_user);
    m_db->setPassword(m_config.m_pass);

    if(!m_db->open())
    {
        qWarning() << "Failed to open database" << m_config.m_db_path;
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
    m_query->clear();
    m_query->finish();
    m_db->close();
    m_db->setConnectOptions();
}