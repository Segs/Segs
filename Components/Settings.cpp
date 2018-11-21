/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup Components
 * @{
 */

#include "Settings.h"
#include "Logging.h"

#include <QFileInfo>
#include <QDebug>
#include <QFile>
#include <QDir>

QString Settings::m_segs_dir;
QString Settings::m_settings_path = QStringLiteral("settings.cfg"); // default path 'settings.cfg' from args
QString Settings::m_default_tpl_dir = QStringLiteral("default_setup"); // default folder 'default_setup'
QString Settings::m_default_settings_path = QStringLiteral("settings_template.cfg"); // default template from folder 'default_setup'

bool fileExists(const QString &path)
{
    QFileInfo check_file(path);
    // check if file exists and if yes: Is it really a file and not a directory?
    return check_file.exists() && check_file.isFile();
}

Settings::Settings()
{
    if(!fileExists(getSettingsPath()))
        createSettingsFile();
}

void Settings::setSettingsPath(const QString path)
{
    if(path.isEmpty())
        qCritical() << "Settings path not defined? This is unpossible!";

    m_settings_path = getSEGSDir() + QDir::separator() + path;
    qCDebug(logSettings) << "Settings Path" << m_settings_path;
}

QString Settings::getSettingsPath()
{
    if(m_settings_path.isEmpty())
        setSettingsPath("settings.cfg"); // set default path to "settings.cfg"

    return m_settings_path;
}

void Settings::setSEGSDir()
{
    // Get the current SEGS directory. This library is shared
    // by dbtool and others, so make sure we're in the correct
    // working directory
    QDir curdir(QDir::current());
    qCDebug(logSettings) << "Settings Dir" << curdir.absolutePath();

    // if called from utilities, move up one directory
    if(curdir.absolutePath().endsWith("utilities", Qt::CaseInsensitive))
    {
        curdir.cdUp();
        qCDebug(logSettings) << "Modified Dir" << curdir.absolutePath();
    }

    if(!fileExists(curdir.absolutePath() + QDir::separator() + "segs_server"))
        qWarning() << "Cannot find SEGS Server!";

    m_segs_dir = curdir.absolutePath();
}

QString Settings::getSEGSDir()
{
    // if m_segs_dir is not empty, we've set it, return that instead
    if(m_segs_dir.isEmpty())
        setSEGSDir();

    return m_segs_dir;
}

QString Settings::getSettingsTplPath()
{
    QDir curdir(getSEGSDir()); // Get the SEGS working directory
    if(!fileExists(curdir.absolutePath() + QDir::separator() + m_default_settings_path))
        qWarning() << "Cannot find" << m_default_settings_path;

    return curdir.absolutePath() + QDir::separator() + m_default_settings_path;
}

QString Settings::getDefaultDirPath()
{
    QDir curdir(getSEGSDir()); // Get the SEGS working directory
    if(!QDir(curdir.absolutePath() + QDir::separator() + m_default_tpl_dir).exists())
        qWarning() << "Cannot find directory" << m_default_tpl_dir;

    return curdir.absolutePath() + QDir::separator() + m_default_tpl_dir;
}

void Settings::createSettingsFile()
{
    if(!fileExists(Settings::getSettingsPath()))
    {
        qCritical() << "Settings file" << Settings::getSettingsPath() <<"does not exist. Creating it now...";
        QFile sfile(Settings::getSettingsPath());
        if(!sfile.open(QIODevice::WriteOnly))
        {
            qDebug() << "Unable to create" << Settings::getSettingsPath() << "Check folder permissions.";
            return;
        }

        // QSettings setValue() methods delete all file comments, it's better to
        // simply copy the template over to our destination directory.
        QFile::copy(Settings::getSettingsTplPath(), Settings::getSettingsPath());

        return;
    }
    else
    {
        qDebug() << "Settings file already exists at" << Settings::getSettingsPath();
        return;
    }
}

// TODO: Any time you set settings values it deletes all file comments. There is no known workaround.
void Settings::setDefaultSettings()
{
    QSettings config(Settings::getSettingsPath(),QSettings::IniFormat,nullptr);

    config.beginGroup("AdminServer");
        config.beginGroup("AccountDatabase");
            config.setValue("db_driver","QSQLITE");
            config.setValue("db_host","127.0.0.1");
            config.setValue("db_port","5432");
            config.setValue("db_name","segs");
            config.setValue("db_user","segsadmin");
            config.setValue("db_pass","segs123");
        config.endGroup();
        config.beginGroup("CharacterDatabase");
            config.setValue("db_driver","QSQLITE");
            config.setValue("db_host","127.0.0.1");
            config.setValue("db_port","5432");
            config.setValue("db_name","segs_game");
            config.setValue("db_user","segsadmin");
            config.setValue("db_pass","segs123");
        config.endGroup();
    config.endGroup();
    config.beginGroup("AuthServer");
        config.setValue("location_addr","127.0.0.1:2106");
    config.endGroup();
    config.beginGroup("GameServer");
        config.setValue("listen_addr","127.0.0.1:7002");
        config.setValue("location_addr","127.0.0.1:7002");
        config.setValue("max_players","200");
        config.setValue("max_character_slots","8");
    config.endGroup();
    config.beginGroup("MapServer");
        config.setValue("listen_addr","127.0.0.1:7003");
        config.setValue("location_addr","127.0.0.1:7003");
        config.setValue("maps","DefaultMapInstances");
        config.setValue("player_fade_in", "380.0");
        config.setValue("motd_timer", "120.0");
        config.setValue("costume_slot_unlocks", "19,29,39,49");
    config.endGroup();
    config.beginGroup("AFK Settings");
        config.setValue("time_to_afk","300");
        config.setValue("time_to_logout_msg","1080");
        config.setValue("time_to_auto_logout","120");
        config.setValue("uses_auto_logout", "true");
    config.endGroup();
    config.beginGroup("StartingCharacter");
        config.setValue("inherent_powers", "Brawl");
        config.setValue("starting_temps", "EMP_Glove");
        config.setValue("starting_inspirations", "Resurgence");
        config.setValue("starting_level", 1);
        config.setValue("starting_inf", 0);
    config.endGroup();
    config.beginGroup("Logging");
        config.setValue("log_logging","false");
        config.setValue("log_keybinds","false");
        config.setValue("log_settings","false");
        config.setValue("log_gui","false");
        config.setValue("log_teams","false");
        config.setValue("log_db","false");
        config.setValue("log_input","false");
        config.setValue("log_position","false");
        config.setValue("log_orientation","false");
        config.setValue("log_movement","false");
        config.setValue("log_chat","false");
        config.setValue("log_infomsg","false");
        config.setValue("log_emotes","false");
        config.setValue("log_target","false");
        config.setValue("log_spawn","false");
        config.setValue("log_mapevents","false");
        config.setValue("log_mapxfers", "false");
        config.setValue("log_slashcommands","false");
        config.setValue("log_description","false");
        config.setValue("log_friends","false");
        config.setValue("log_minimap","false");
        config.setValue("log_lfg","false");
        config.setValue("log_npcs","false");
        config.setValue("log_animations","false");
        config.setValue("log_powers","false");
        config.setValue("log_trades","false");
        config.setValue("log_tailor","false");
        config.setValue("log_scripts","false");
        config.setValue("log_scenegraph","false");
        config.setValue("log_tasks","false");
    config.endGroup();

    config.sync(); // sync changes or they wont be saved to file.
}

void settingsDump()
{
    QSettings config(Settings::getSettingsPath(),QSettings::IniFormat,nullptr);
    settingsDump(&config);
}

void settingsDump(QSettings *s)
{
    QString output = "Settings File Dump\n";
    foreach (const QString &group, s->childGroups()) {
        QString groupString = QString("===== %1 =====\n").arg(group);
        s->beginGroup(group);

        foreach (const QString &key, s->allKeys()) {
            groupString.append(QString("  %1\t\t %2\n").arg(key, s->value(key).toString()));
        }

        s->endGroup();
        groupString.append("\n");

        output.append(groupString);
    }
    qDebug().noquote() << output;
}

//! @}
