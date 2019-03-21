/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup Components
 * @{
 */

#include "Settings.h"

#include <QFileInfo>
#include <QDebug>
#include <QFile>

QString Settings::m_settings_path = QStringLiteral("settings.cfg"); // default path 'settings.cfg' from args

static bool fileExists(const QString &path) {
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
    if(path == nullptr)
        qCritical() << "Settings path not defined? This is unpossible!";

    m_settings_path = path;
}

QString Settings::getSettingsPath()
{
    if(m_settings_path.isEmpty())
        setSettingsPath("settings.cfg"); // set default path to "settings.cfg"

    return m_settings_path;
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

        QTextStream header(&sfile);
        header << "##############################################################"
                 << "\n#    SEGS configuration file."
                 << "\n#"
                 << "\n#    listen_addr values below should contain the IP the"
                 << "\n#      clients will connect to."
                 << "\n#"
                 << "\n#    location_addr values below should contain the IP the"
                 << "\n#      clients will receive data from."
                 << "\n#"
                 << "\n#    Both values are set to 127.0.0.1 by default but should"
                 << "\n#      be set to your local IP address on the network"
                 << "\n#      for example: 10.0.0.2"
                 << "\n#"
                 << "\n#    Default ports are listed below:"
                 << "\n#      AccountDatabase db_port:      5432"
                 << "\n#      CharacterDatabase db_port:    5432"
                 << "\n#      AuthServer location_addr:     2106"
                 << "\n#      GameServer listen_addr:       7002"
                 << "\n#      GameServer location_addr:     7002"
                 << "\n#      MapServer listen_addr:        7003"
                 << "\n#      MapServer location_addr:      7003"
                 << "\n#"
                 << "\n##############################################################";

        sfile.close();

        setDefaultSettings();

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
    config.beginGroup("Modifiers");
        config.setValue("uses_xp_mod", "false");
        config.setValue("xp_mod_multiplier", "2.00");
        config.setValue("xp_mod_startdate", "1/1/2000 12:00 AM");
        config.setValue("xp_mod_enddate", "1/1/2000 12:00 AM");
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
