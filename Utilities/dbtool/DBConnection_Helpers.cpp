/*
 * SEGS - Super Entity Game Server - dbtool
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup dbtool Projects/CoX/Utilities/dbtool
 * @{
 */

#include "DBConnection.h"

#include "Components/Logging.h"
#include "Components/Settings.h"

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
        if(!fileExists(m_config.m_db_name))
            return false;

        return true;
    }
    else if(m_config.isMysql())
    {
        return m_db->isOpen();
    }
    else if(m_config.isPostgresql())
    {
        querytext = QString("SELECT table_schema || '.' || table_name FROM information_schema.tables WHERE table_type = 'BASE TABLE' AND table_schema ='%1';")
                .arg(m_config.m_db_name);
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
 * @param[in]   q a QString sql query
 * @returns     Returns a boolean, true if successful
 */
bool DBConnection::runQueries(const QStringList &qlist)
{
    for(const QString &q : qlist)
    {
        if(!runQuery(q))
            return false;
    }

    return true;
}

/*!
 * @brief       Run a string as query
 * @param[in]   q a QString sql query
 * @returns     Returns a boolean, true if successful
 */
bool DBConnection::runQuery(const QString &q)
{
    if(!m_query->prepare(q))
    {
        qCritical("One of the queries failed to prepare.\n Error detail: %s\n",
                  qPrintable(m_query->lastError().text()));
        qCritical().noquote() << m_query->lastQuery();
        return false;
    }

    if(!m_query->exec())
    {
        qCritical("One of the queries failed to execute.\n Error detail: %s\n",
                  qPrintable(m_query->lastError().text()));
        qCritical().noquote() << m_query->lastQuery();
        return false;
    }

    qCDebug(logDB).noquote() << "exec:" << m_query->lastQuery();
    return true;
}

/*!
 * @brief           Delete columns (col_to_remove) in tablename using
 *                  common sql methods
 * @param[in]       tablename as QString
 * @param[in]       col_to_remove a QStringList of column names
 */
bool DBConnection::deleteColumnsCommon(const QString &tablename, const QStringList &cols_to_remove)
{
    // iterate over all columns in tablename
    for(const QString &column : cols_to_remove)
    {
        QString tmp_query = QString("ALTER TABLE %1 DROP %2;").arg(tablename, column);
        if(!runQuery(tmp_query))
        {
            qWarning() << column << "does not exist in" << tablename;
            return false;
        }
    }

    return true;
}

/*!
 * @brief           Delete columns (col_to_remove) in tablename using
 *                  sqlite specific methods
 * @param[in]       tablename as QString
 * @param[in]       col_to_remove a QStringList of column names
 */
bool DBConnection::deleteColumnsSqlite(const QString &tablename, const QStringList &cols_to_remove)
{
    // SQLite has limited ALTER TABLE support and cannot drop columns.
    // the workaround is to duplicate the table without the "dropped" columns

    // Get Original Column List and fill it
    std::vector<ColumnSchema> old_cols_arr;
    if(!getColumnsFromTable(tablename, old_cols_arr))
        return false;

    // Take all old columns and add them to new vector
    // ignoring columns from list of cols to remove
    QStringList old_cols_namelist, new_cols_namelist;
    QStringList old_cols_as_sql, new_cols_as_sql;
    for(ColumnSchema &cs : old_cols_arr)
    {
        // Create sql strings here
        old_cols_namelist.push_back(cs.m_name);
        old_cols_as_sql.push_back(QString("'%1' %2").arg(cs.m_name).arg(cs.m_data_type));

        // if column should be removed, we're done here
        if(cols_to_remove.contains(cs.m_name, Qt::CaseInsensitive))
            continue;

        new_cols_namelist.push_back(cs.m_name);
        new_cols_as_sql.push_back(QString("'%1' %2").arg(cs.m_name).arg(cs.m_data_type));
    }

    // Replace current table with one that excludes our "deleted" columns
    QStringList queries;
    queries << QString("CREATE TABLE `%1_old` (%2);")
             .arg(tablename)
             .arg(old_cols_as_sql.join(","));
    queries << QString("INSERT INTO %1_old SELECT %2 FROM %1;")
             .arg(tablename)
             .arg(old_cols_namelist.join(","));
    queries << QString("DROP TABLE %1;")
             .arg(tablename);
    queries << QString("CREATE TABLE `%1` (%2);")
             .arg(tablename)
             .arg(new_cols_as_sql.join(","));
    queries << QString("INSERT INTO %1 SELECT %2 FROM %1_old;")
             .arg(tablename)
             .arg(new_cols_namelist.join(","));
    queries << QString("DROP TABLE %1_old;")
             .arg(tablename);

    return runQueries(queries);
}

/*!
 * @brief           Delete columns (cols_to_remove) in tablename
 * @param[in]       tablename as QString
 * @param[in]       cols_to_remove a QStringList of column names
 */
bool DBConnection::deleteColumns(const QString &tablename, const QStringList &cols_to_remove)
{
    if(!m_config.isSqlite())
        return deleteColumnsCommon(tablename, cols_to_remove);

    return deleteColumnsSqlite(tablename, cols_to_remove);
}

/*!
 * @brief           Delete single column (col_to_remove) in tablename
 * @param[in]       tablename as QString
 * @param[in]       col_to_remove a QStringList of column names
 */
bool DBConnection::deleteColumn(const QString &tablename, const QString &col_to_remove)
{
    QStringList cols;
    cols.push_back(col_to_remove);
    return deleteColumns(tablename, cols);
}

/*!
 * @brief           Get Columns from tablename
 * @param[in]       tablename as QString
 * @param[out]      std::vector<ColumnSchema> a vector of columns and their datatypes
 */
bool DBConnection::getColumnsFromTable(const QString &tablename, std::vector<ColumnSchema> &old_cols)
{
    // This is only ever called on SQLite tables
    // because other DB Drivers can delete columns
    if(m_db->driverName() != "QSQLITE")
        qWarning() << "Running getColumnsFromTable on non-SQLite DB. Why?";

    QString query = QString("PRAGMA table_info(%1)").arg(tablename);
    m_query->prepare(query);
    if(!m_query->exec())
        return false;

    while(m_query->next())
    {
        ColumnSchema c;
        c.m_name = m_query->value(1).toString();
        c.m_data_type = m_query->value(2).toString();
        old_cols.push_back(c);
        qCDebug(logDB) << "Reading column:" << c.m_name << c.m_data_type;

    }

    return true; // we're done here
}

/*!
 * @brief           Some objects have become arrays of objects
 *                  like costumes for example. Convert them here.
 * @param[in,out]   obj
 */
void DBConnection::prepareCerealArray(QJsonObject &obj)
{
    QJsonArray wrapped_arr;
    wrapped_arr.append(QJsonValue(obj));
    QJsonObject wrapped_obj;
    wrapped_obj.insert("value0", QJsonValue(wrapped_arr));
    obj = wrapped_obj;
}

/*!
 * @brief           Cereal adds a wrapper around the entire object
 *                  with a key of `value0`. We add that here.
 * @param[in,out]   obj
 */
void DBConnection::prepareCerealObject(QJsonObject &obj)
{
    QJsonObject wrapped_obj;
    wrapped_obj.insert("value0", obj);
    obj = wrapped_obj;
}

/*!
 * @brief           Return a QVariantMap that we can access easily
 * @param[in]       column_name of the column that contains the json
 * @param[out]      QVariantMap of json structure
 */
QJsonObject DBConnection::loadBlob(const QString &column_name)
{
    QJsonParseError err;
    QJsonDocument jdoc = QJsonDocument::fromJson(m_query->value(column_name).toByteArray(), &err);

    if(err.error != QJsonParseError::NoError)
        qWarning() << "Json Parser Error:" << qPrintable(err.errorString()) << err.offset;

    QJsonObject obj = jdoc.object();
    if(obj.size() < 2) // if single cereal object, remove wrapper
        obj = jdoc.object()["value0"].toObject();

    qCDebug(logMigration).noquote() << "Loading Blob:" << jdoc.toJson();
    return obj;
}

/*!
 * @brief           Save jsondocument to QString to save to database.
 * @param[in]       QVariantMap map
 * @param[out]      QString of json document
 */
QString DBConnection::saveBlob(QJsonObject &obj)
{
    // annoyingly cereal wraps every object with key "value0"
    // if there is no cereal wrapper, then add it here
    if(obj.size() > 1 && !obj.keys().contains("value0", Qt::CaseInsensitive))
        prepareCerealObject(obj);

    QJsonDocument json_doc(obj);
    QString sanitized_blob = json_doc.toJson();
    sanitized_blob.replace("'","''"); // sql uses ' as an escape character, escape the escape!
    return sanitized_blob;
}

//! @}
