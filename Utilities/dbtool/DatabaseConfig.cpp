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

#include "DatabaseConfig.h"
#include "Components/Logging.h"
#include "Components/Settings.h"


#include <QDir>
#include <QtSql/QtSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

/*!
 * @brief       initialize database from given settings file
 * @param[in]   Settings file name
 * @param[in]   Group name based upon database config
 * @returns     Returns a boolean, true if successful
 */
bool DatabaseConfig::initialize_from_settings(const QString &settings_file_name, const QString &group_name)
{
    // Set settings file path, since we're in a different directory
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
            m_filename = config.value(QStringLiteral("db_name"),"segs").toString();
            m_db_name = Settings::getSEGSDir() + QDir::separator() + m_filename; // don't add suffix here, so that it can be optional for users
            m_host = config.value(QStringLiteral("db_host"),"127.0.0.1").toString();
            m_port = config.value(QStringLiteral("db_port"),"5432").toString();
            m_user = config.value(QStringLiteral("db_user"),"segs").toString();
            m_pass = config.value(QStringLiteral("db_pass"),"segs123").toString();
            m_character_db = group_name.compare("AccountDatabase");
        config.endGroup(); // group_name
    config.endGroup(); // AdminServer

    // store database names so we can use them later in migrations
    m_short_name = QFileInfo(m_db_name).completeBaseName();

    if(!isSqlite())
        m_db_name = m_short_name; // for client-server db systems, use name instead of path

    qCDebug(logSettings) << m_db_name << "database settings loaded from" << settings_full_path;

    return putFilePath();
}

bool DatabaseConfig::putFilePath()
{
    // Find database templates directory
    QDir tpl_dir(Settings::getTemplateDirPath());
    if(!tpl_dir.exists())
    {
        qWarning() << "SEGS dbtool cannot find the SEGS root folder "
                   << "(where the default_setup directory resides)";
        return false;
    }

    qCDebug(logSettings) << "Templates Dir" << Settings::getTemplateDirPath();

    QString driver;
    if(isSqlite())
        driver = "sqlite";
    else if(isMysql())
        driver = "mysql";
    else if(isPostgresql())
        driver = "pgsql";
    else
    {
        qCritical("Unknown database driver.");
        return false;
    }

    QString base_dir(tpl_dir.absolutePath() + QDir::separator());
    m_template_path = QString("%1%2/segs%3_%2_create.sql").arg(base_dir).arg(driver).arg(m_character_db?"_game":"");

    qCDebug(logSettings) << "m_file_path" << m_template_path;
    return true;
}
