/*
 * Super Entity Game Server
 * http://github.com/Segs
 * Copyright (c) 2006 - 2018 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */
 
#include "Settings.h"

bool fileExists(QString path) {
    QFileInfo check_file(path);
    // check if file exists and if yes: Is it really a file and not a directory?
    return check_file.exists() && check_file.isFile();
}

Settings::Settings()
{
}

QSettings *Settings::getSettings()
{
    Settings s;

    if(!fileExists(s.getSettingsPath()))
        s.createSettingsFile();

    static QSettings m_settings(s.getSettingsPath(),QSettings::IniFormat);
    //if(!s.m_settings.contains("AuthServer/location_addr"))
    //{
    //    s.m_settings.setPath(QSettings::IniFormat,QSettings::SystemScope,s.getSettingsPath());
    //}

    settingsDump(&m_settings); // Settings are correct
    return &m_settings;
}

void Settings::setSettingsPath(const QString path)
{
    if(path == NULL)
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
    if (!fileExists(Settings::getSettingsPath()))
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
                 << "\n#      AccountDatabase db_port:		5432"
                 << "\n#      CharacterDatabase db_port:	5432"
                 << "\n#      AuthServer listen_addr:		2106"
                 << "\n#      GameServer listen_addr:		7002"
                 << "\n#      GameServer location_addr:     7002"
                 << "\n#      MapServer listen_addr:		7003"
                 << "\n#      MapServer location_addr:		7003"
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
    QSettings s(getSettings());

    s.beginGroup("AdminServer");
        s.beginGroup("AccountDatabase");
            s.setValue("db_driver","QSQLITE");
            s.setValue("db_host","127.0.0.1");
            s.setValue("db_port","5432");
            s.setValue("db_name","segs");
            s.setValue("db_user","segsadmin");
            s.setValue("db_pass","segs123");
        s.endGroup();
        s.beginGroup("CharacterDatabase");
            s.setValue("db_driver","QSQLITE");
            s.setValue("db_host","127.0.0.1");
            s.setValue("db_port","5432");
            s.setValue("db_name","segs_game");
            s.setValue("db_user","segsadmin");
            s.setValue("db_pass","segs123");
        s.endGroup();
    s.endGroup();
    s.beginGroup("AuthServer");
        s.setValue("listen_addr","127.0.0.1:2106");
    s.endGroup();
    s.beginGroup("GameServer");
        s.setValue("server_name","SEGS Server");
        s.setValue("listen_addr","127.0.0.1:7002");
        s.setValue("location_addr","127.0.0.1:7002");
        s.setValue("max_players","200");
        s.setValue("max_account_slots","8");
    s.endGroup();
    s.beginGroup("MapServer");
        s.setValue("listen_addr","127.0.0.1:7003");
        s.setValue("location_addr","127.0.0.1:7003");
    s.endGroup();
    
    s.sync(); // sync changes or they wont be saved to file.
}

void settingsDump()
{
    QSettings *s(Settings::getSettings());
    settingsDump(s);
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
