/*
 * SEGS - Super Entity Game Server - dbtool
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include <QtCore/QString>
#include <QtCore/QMap>

class DatabaseConfig
{
private:
    bool putFilePath();
public:
    QString m_driver;
    QString m_short_name;
    QString m_filename;
    QString m_db_name;
    QString m_host;
    QString m_port;
    QString m_user;
    QString m_pass;
    QString m_template_path;
    bool m_character_db = false;

    DatabaseConfig() = default;

    bool initialize_from_settings(const QString &settings_file_name, const QString &group_name);

    bool isMysql() const { return m_driver.startsWith("QMYSQL"); }
    bool isPostgresql() const { return m_driver.startsWith("QPSQL"); }
    bool isSqlite() const { return m_driver.startsWith("QSQLITE"); }
};
