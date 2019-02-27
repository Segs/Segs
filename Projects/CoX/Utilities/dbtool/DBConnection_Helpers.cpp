/*
 * SEGS - Super Entity Game Server - dbtool
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup dbtool Projects/CoX/Utilities/dbtool
 * @{
 */

#include "DBConnection.h"

#include "Logging.h"
#include "Settings.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtSql/QtSql>

/*!
 * @brief       Check if a database is connected
 * @returns     Returns a boolean, true if database exists
 */
bool DBConnection::isConnected()
{
    QString querytext;
    if(m_config.isSqlite())
    {
        if(!fileExists(m_config.m_db_path))
            return false;

        return true;
    }
    else if(m_config.isMysql())
    {
        querytext = "SHOW TABLES";
    }
    else if(m_config.isPostgresql())
    {
        querytext = "SELECT table_schema || '.' || table_name FROM";
        querytext.append("information_schema.tables WHERE table_type = 'BASE TABLE' AND table_schema ='");
        querytext.append(m_config.m_db_path);
        querytext.append("';");
    }
    else
    {
        qWarning() << "Database configuration is set to unknown DB Driver. Connection unknown!";
        return false; // can't tell if database is connected
    }

    if(!runQuery(querytext))
        return false;

    if(m_query->size() >= 1)
        return true;

    qWarning() << "Something went wrong! Database connection failed in isConnected()";
    return false;
}

/*!
 * @brief       Run a string as query
 * @param[in]   q
 * @returns     Returns a boolean, true if successful
 */
bool DBConnection::runQuery(QString &q)
{
    if(!m_query->prepare(q))
    {
        qCritical("One of the queries failed to prepare.\n Error detail: %s\n",
                  qPrintable(m_query->lastError().text()));
        qCritical() << m_query->lastQuery();
        return false;
    }

    if(!m_query->exec())
    {
        qCritical("One of the queries failed to execute.\n Error detail: %s\n",
                  qPrintable(m_query->lastError().text()));
        qCritical() << m_query->lastQuery();
        return false;
    }

    qCDebug(logDB) << "exec:" << m_query->lastQuery();
    return true;
}

/*!
 * @brief           Cereal adds a wrapper around the entire object
 *                  with a key of `value0`. We remove that here.
 * @param[in,out]   obj
 */
void DBConnection::loadBlob(QJsonObject &obj)
{
    obj = obj["value0"].toObject(); // strip outermost wrap
}

/*!
 * @brief           Cereal adds a wrapper around the entire object
 *                  with a key of `value0`. We add that here.
 * @param[in,out]   obj
 */
void DBConnection::saveBlob(QJsonObject &obj)
{
    QJsonObject wrapped_obj;
    wrapped_obj.insert("value0", obj);
    obj = wrapped_obj;
}

//! @}
