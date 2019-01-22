/*
 * SEGS - Super Entity Game Server - dbtool
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "DatabaseConfig.h"
#include "Settings.h"
#include "Logging.h"

#include <QDir>
#include <QtSql/QtSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

bool DatabaseConfig::initialize_from_settings(const QString &settings_file_name, const QString &group_name)
{
    // Set settings file path based upon possible cli switch defined
    Settings::setSettingsPath(settings_file_name);
    QString settings_full_path = Settings::getSettingsPath();

    if(!fileExists(settings_full_path))
    {
        qWarning() << "Cannot find settings template file" << settings_full_path;
        return false;
    }

    QSettings config(settings_full_path, QSettings::IniFormat, nullptr);

    config.beginGroup(QStringLiteral("AdminServer"));
        config.beginGroup(group_name);
            m_driver = config.value(QStringLiteral("db_driver"),"QSQLITE").toString();
            m_name = config.value(QStringLiteral("db_name"),"segs").toString();
            m_db_path = Settings::getSEGSDir() + QDir::separator() + m_name;
            m_host = config.value(QStringLiteral("db_host"),"127.0.0.1").toString();
            m_port = config.value(QStringLiteral("db_port"),"5432").toString();
            m_user = config.value(QStringLiteral("db_user"),"segs").toString();
            m_pass = config.value(QStringLiteral("db_pass"),"segs123").toString();
            m_character_db = group_name.compare("AccountDatabase");
        config.endGroup(); // group_name
    config.endGroup(); // AdminServer

    qCDebug(logSettings) << m_db_path << "database settings loaded from" << settings_full_path;

    return putFilePath();
}

bool DatabaseConfig::putFilePath()
{
    QDir db_dir(Settings::getDefaultDirPath());
    qCDebug(logSettings) << "Default Dir" << Settings::getDefaultDirPath();

    if(isSqlite())
    {
        if(m_character_db)
            m_template_path = db_dir.absolutePath() + QDir::separator() + "sqlite/segs_game_sqlite_create.sql";
        else
            m_template_path = db_dir.absolutePath() + QDir::separator() + "sqlite/segs_sqlite_create.sql";
    }
    else if(isMysql())
    {
        if(m_character_db)
            m_template_path = db_dir.absolutePath() + QDir::separator() + "mysql/segs_game_mysql_create.sql";
        else
            m_template_path = db_dir.absolutePath() + QDir::separator() + "mysql/segs_mysql_create.sql";
    }
    else if(isPostgresql())
    {
        if(m_character_db)
            m_template_path = db_dir.absolutePath() + QDir::separator() + "pgsql/segs_game_postgres_create.sql";
        else
            m_template_path = db_dir.absolutePath() + QDir::separator() + "pgsql/segs_postgres_create.sql";
    }
    else
    {
        qCritical("Unknown database driver.");
        return false;
    }

    qCDebug(logSettings) << "m_file_path" << m_template_path;
    return true;
}
